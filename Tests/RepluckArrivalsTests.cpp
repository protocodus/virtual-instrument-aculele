// The re-pluck arrival rings clear and copy only their pending slots
// (AculeleEngine::RepluckArrivals). That is the whole ring's clear and copy
// only while every slot outside the pending ones is zero, which this holds:
//
// - on a ring of its own, through 20,000 seeded adds, reads, clears and
//   copies: after each, no slot outside the pending ones is set, the ring
//   equals a plain model that clears and copies whole planes, and a copy
//   onto a ring holding other arrivals equals its source slot for slot;
// - in the engine, for every factory preset: a note struck again every
//   35 ms on two strings, then moved a fret, leaves no slot set outside the
//   pending ones of any string's ring or its tail's, after every block.
//
// 48 kHz, 256-sample blocks.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    using Arrivals = AculeleEngine::RepluckArrivals;
    static constexpr int capacity = Arrivals::capacity;

    static const Arrivals& arrivals(const AculeleEngine& engine, int string, bool tail)
    {
        const auto& voice = engine.voices_[static_cast<std::size_t>(string)];
        return tail ? voice.tailRepluckArrivals : voice.repluckArrivals;
    }
    // Slots set outside the pending ones.
    static int strays(const Arrivals& ring)
    {
        int count = 0;
        for (int slot = 0; slot < capacity; ++slot)
        {
            const int ahead = (slot - ring.readIndex + capacity) % capacity;
            const auto index = static_cast<std::size_t>(slot);
            if (ahead >= ring.remaining
                && (ring.wave[0][index] != 0.0f || ring.wave[1][index] != 0.0f))
                ++count;
        }
        return count;
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
constexpr double rate = 48000.0;
constexpr int block = 256;
int failures = 0;

void check(bool ok, const char* what, const char* where)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s: %s\n", where, what);
        ++failures;
    }
}

// The ring as it was before 0.4.60: whole planes cleared and copied.
struct Model
{
    std::vector<float> normal = std::vector<float>(Access::capacity);
    std::vector<float> parallel = std::vector<float>(Access::capacity);
    int readIndex = 0, remaining = 0;

    void clear()
    {
        normal.assign(normal.size(), 0.0f);
        parallel.assign(parallel.size(), 0.0f);
        readIndex = remaining = 0;
    }
    void add(int offset, float a, float b)
    {
        const auto slot = static_cast<std::size_t>((readIndex + offset) % Access::capacity);
        normal[slot] += a;
        parallel[slot] += b;
        remaining = remaining > offset + 1 ? remaining : offset + 1;
    }
    std::array<float, 2> process()
    {
        if (remaining <= 0) return {};
        const auto slot = static_cast<std::size_t>(readIndex);
        const std::array<float, 2> result { normal[slot], parallel[slot] };
        normal[slot] = parallel[slot] = 0.0f;
        readIndex = (readIndex + 1) % Access::capacity;
        --remaining;
        return result;
    }
    bool equals(const Access::Arrivals& ring) const
    {
        if (ring.readIndex != readIndex || ring.remaining != remaining)
            return false;
        for (std::size_t slot = 0; slot < normal.size(); ++slot)
            if (ring.wave[0][slot] != normal[slot] || ring.wave[1][slot] != parallel[slot])
                return false;
        return true;
    }
};

void ringOfItsOwn()
{
    auto ring = std::make_unique<Access::Arrivals>();
    auto other = std::make_unique<Access::Arrivals>();
    Model model, otherModel;
    std::uint32_t seed = 0x2545f491u;
    const auto next = [&seed] { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
    bool same = true, stray = false, readsSame = true;
    for (int step = 0; step < 20000; ++step)
    {
        const auto choice = next() % 100;
        if (choice < 40)
        {
            // Mostly near arrivals, now and then one far round the ring.
            const int offset = static_cast<int>(next() % (choice < 4 ? Access::capacity : 600));
            const float a = static_cast<float>(next() % 2001) / 1000.0f - 1.0f;
            const float b = static_cast<float>(next() % 2001) / 1000.0f - 1.0f;
            ring->add(offset, a, b);
            if (a != 0.0f || b != 0.0f) model.add(offset, a, b);
        }
        else if (choice < 90)
        {
            for (int count = static_cast<int>(next() % 400); count > 0; --count)
                readsSame = readsSame && ring->process() == model.process();
        }
        else if (choice < 93)
        {
            ring->clear();
            model.clear();
        }
        else if (choice < 97)
        {
            // The tail takes the string's ring, which starts again
            // (captureTail).
            *other = *ring;
            otherModel = model;
            ring->clear();
            model.clear();
        }
        else
        {
            *ring = *other;
            model = otherModel;
        }
        same = same && model.equals(*ring) && otherModel.equals(*other);
        stray = stray || Access::strays(*ring) != 0 || Access::strays(*other) != 0;
    }
    check(readsSame, "every arrival read is the whole-plane ring's", "a ring of its own");
    check(same, "the ring equals one cleared and copied whole", "a ring of its own");
    check(!stray, "no slot is set outside the pending ones", "a ring of its own");
    const Access::Arrivals copied(*ring);
    check(model.equals(copied), "a copy made equals its source", "a ring of its own");
}

void inTheEngine()
{
    for (const auto& preset : aculele::factoryPresets)
    {
        auto performer = std::make_unique<aculele::Performer>();
        const auto parameters = aculele::engineParameters(preset);
        performer->setParameters(parameters);
        performer->prepare(rate, block);
        performer->setParameters(parameters);
        std::array<float, block> left {}, right {};
        const auto& engine = performer->engine();
        int strays = 0, mostPending = 0, mostTailPending = 0;
        const int blocks = static_cast<int>(3.0 * rate) / block;
        const int every = static_cast<int>(0.035 * rate);
        int due = 0, strokes = 0;
        for (int index = 0; index < blocks; ++index)
        {
            performer->beginBlock(left.data(), right.data(), block);
            for (; due < (index + 1) * block; due += every, ++strokes)
            {
                // Two notes a fourth apart, a fret higher every eighth
                // stroke, soft and hard by turns.
                const auto note = static_cast<std::uint8_t>(62 + (strokes / 8) % 5);
                const auto velocity = static_cast<std::uint8_t>(strokes % 2 == 0 ? 120 : 50);
                const std::uint8_t on[2][3] = { { 0x90, note, velocity },
                    { 0x90, static_cast<std::uint8_t>(note + 5), velocity } };
                for (const auto& message : on)
                    performer->handleMidi(due - index * block, message, 3);
            }
            performer->endBlock();
            for (int string = 0; string < aculele::AculeleEngine::stringCount; ++string)
                for (const bool tail : { false, true })
                {
                    const auto& ring = Access::arrivals(engine, string, tail);
                    strays += Access::strays(ring);
                    auto& most = tail ? mostTailPending : mostPending;
                    most = most > ring.remaining ? most : ring.remaining;
                }
        }
        check(strays == 0, "no slot is set outside the pending ones", preset.name);
        // Without an arrival pending at a block's end the phrase tests
        // nothing of the engine's rings.
        check(mostPending > 0, "the phrase leaves arrivals pending", preset.name);
        std::printf("%s: at most %d arrivals pending at a block's end, %d in a tail\n",
                    preset.name, mostPending, mostTailPending);
    }
}
} // namespace

int main()
{
    ringOfItsOwn();
    inTheEngine();
    if (failures == 0)
        std::printf("PASS repluck arrivals\n");
    return failures == 0 ? 0 : 1;
}
