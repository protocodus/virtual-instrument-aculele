// No note waits for the dispersion designs (AculeleEngine::ready). 0.4.58
// ignored note-ons until every design was made again after a construction
// change, 0.6-2.4 s, so that no note ran the search in its own block; this
// test then held that such a note-on changed no sample. From 0.4.59 the note
// sounds on an interim design, the search's first fit and one fallback
// start, and takes the finished design at the string's next fresh pluck, so
// the contract here is rewritten: nothing is lost and nothing is searched in
// full at the note. For every factory preset at 48 kHz:
//
// - a freshly prepared Performer is ready, and a note on its first sample
//   sounds;
// - after a Shape change it is not ready, and a note-on then sounds at once:
//   its samples differ from those of a twin that was sent none;
// - a note whose finished design the change left still to search takes an
//   interim design in its note-on block, not a search of its own, and keeps
//   it while it rings, ready or not; a string configured for a fresh pluck
//   then holds exactly the search's first two steps;
// - it is ready again within 10 s, every design then the one a freshly
//   prepared instrument of the new shape holds, bit for bit, and the
//   string's next fresh pluck takes a finished design: configured for one,
//   exactly its slot's;
// - a Wood change, which leaves every design as it was, is ready again
//   within eleven steps of the designs (2,816 samples): ten steps of
//   sixteen checks cover the 152 slots, and the first may come up to a
//   step late.
//
// 64-sample blocks, both main channels compared.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

namespace aculele
{
struct AculeleEngineTestAccess
{
    using Design = AculeleEngine::DispersionSolve;

    static bool arguments(const AculeleEngine& engine, int slot,
                          std::array<double, 13>& arguments)
    {
        return engine.preparedDispersionArguments(slot, arguments);
    }
    static const Design& prepared(const AculeleEngine& engine, int slot)
    {
        return engine.preparedDispersion_[static_cast<std::size_t>(slot)];
    }
    static const Design& sounding(const AculeleEngine& engine, int string)
    {
        return engine.voices_[static_cast<std::size_t>(string)].dispersionDesign;
    }
    // What a note asking for these arguments is given now.
    static Design asked(AculeleEngine& engine, const std::array<double, 13>& arguments)
    {
        return engine.solvedDispersion(arguments);
    }
    // A string configured as a fresh pluck's first configuration at a fret,
    // before the contact draws its own few cents (frettingPitchCents), which
    // continue the design from this one.
    static const Design& plucked(AculeleEngine& engine, int string, int fret)
    {
        auto& voice = engine.voices_[static_cast<std::size_t>(string)];
        engine.configureVoice(voice, string, voice.openMidi + fret, true, false);
        return voice.dispersionDesign;
    }
    static int openNote(const AculeleEngine& engine, int string)
    {
        return engine.voices_[static_cast<std::size_t>(string)].openMidi;
    }
    static bool same(const Design& a, const Design& b)
    {
        return a.valid == b.valid && a.complete == b.complete
            && a.arguments == b.arguments
            && std::memcmp(a.decayRatios.data(), b.decayRatios.data(), sizeof(a.decayRatios)) == 0
            && std::memcmp(a.poleRatios.data(), b.poleRatios.data(), sizeof(a.poleRatios)) == 0
            && std::memcmp(&a.worstCents, &b.worstCents, sizeof(float)) == 0;
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
constexpr double rate = 48000.0;
constexpr int block = 64;
constexpr int slotCount = aculele::AculeleEngine::stringCount
    * (aculele::AculeleEngine::fretCount + 1) * 2;
int failures = 0;

void check(bool ok, const char* what, const char* preset)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s: %s\n", preset, what);
        ++failures;
    }
}

struct Player
{
    std::unique_ptr<aculele::Performer> performer = std::make_unique<aculele::Performer>();
    std::array<float, block> left {}, right {};
    // Messages for the next block's first sample, which the Performer takes
    // between beginBlock and endBlock.
    std::array<std::array<std::uint8_t, 3>, 4> pending {};
    int pendingCount = 0;

    void prepare(const aculele::EngineParameters& parameters)
    {
        performer->setParameters(parameters);
        performer->prepare(rate, block);
        performer->setParameters(parameters);
    }
    void message(std::uint8_t status, std::uint8_t data1, std::uint8_t data2)
    {
        pending[static_cast<std::size_t>(pendingCount++)] = { status, data1, data2 };
    }
    void render()
    {
        performer->beginBlock(left.data(), right.data(), block);
        for (int index = 0; index < pendingCount; ++index)
            performer->handleMidi(0, pending[static_cast<std::size_t>(index)].data(), 3);
        pendingCount = 0;
        performer->endBlock();
    }
    // Blocks until the engine is ready, at most `limit`; the samples taken.
    int renderUntilReady(int limit)
    {
        int samples = 0;
        while (!performer->engine().ready() && samples < limit)
        {
            render();
            samples += block;
        }
        return samples;
    }
};

double blockEnergy(const Player& player)
{
    double sum = 0.0;
    for (int i = 0; i < block; ++i)
        sum += static_cast<double>(player.left[static_cast<std::size_t>(i)])
                * player.left[static_cast<std::size_t>(i)]
            + static_cast<double>(player.right[static_cast<std::size_t>(i)])
                * player.right[static_cast<std::size_t>(i)];
    return sum;
}

bool sameBlock(const Player& a, const Player& b)
{
    return a.left == b.left && a.right == b.right;
}
} // namespace

int main()
{
    int interimNotes = 0;
    for (const auto& preset : aculele::factoryPresets)
    {
        const auto parameters = aculele::engineParameters(preset);
        Player played;
        played.prepare(parameters);
        check(played.performer->engine().ready(), "ready once prepared", preset.name);

        // A note on the first sample sounds.
        played.message(0x90, 64, 100);
        double energy = 0.0;
        for (int index = 0; index < 20; ++index)
        {
            played.render();
            energy += blockEnergy(played);
        }
        check(energy > 0.0, "a note on the first sample sounds", preset.name);

        // a plays through a Shape change, b is its silent twin, c has its
        // strings configured directly, and fresh is an instrument prepared
        // in the new shape.
        Player a, b, c, fresh;
        a.prepare(parameters);
        b.prepare(parameters);
        c.prepare(parameters);
        auto reshaped = parameters;
        reshaped.shape = parameters.shape == aculele::BodyShape::Tenor
            ? aculele::BodyShape::Soprano : aculele::BodyShape::Tenor;
        fresh.prepare(reshaped);
        a.performer->setParameters(reshaped);
        b.performer->setParameters(reshaped);
        c.performer->setParameters(reshaped);
        auto& engine = a.performer->engine();
        check(!engine.ready(), "unready after a Shape change", preset.name);

        // The stiffest note the change left without a finished design: the
        // last slot whose design, asked for now, comes back interim.
        int stiffSlot = -1;
        aculele::AculeleEngineTestAccess::Design interim;
        for (int slot = 0; slot < slotCount; ++slot)
        {
            std::array<double, 13> arguments {};
            if (!Access::arguments(c.performer->engine(), slot, arguments))
                continue;
            const auto design = Access::asked(c.performer->engine(), arguments);
            if (design.valid && !design.complete)
            {
                stiffSlot = slot;
                interim = design;
            }
        }
        // Without one the preset tests nothing of the interim designs; every
        // factory preset has 5-17 at 48 kHz.
        check(stiffSlot >= 0, "the Shape change leaves a design to search", preset.name);
        if (stiffSlot < 0)
            continue;
        const int string = stiffSlot / (2 * (aculele::AculeleEngine::fretCount + 1));
        const int fret = (stiffSlot / 2) % (aculele::AculeleEngine::fretCount + 1);
        const auto status = static_cast<std::uint8_t>(0x90 + string);
        const auto note = static_cast<std::uint8_t>(Access::openNote(engine, string) + fret);
        check(Access::same(Access::plucked(c.performer->engine(), string, fret), interim),
              "a fresh pluck while unready holds the search's first two steps", preset.name);

        // Each string on its own channel (CC126 = 4 on channel 1), so the
        // note is that string at that fret. Both twins take the mode.
        a.message(0xB0, 126, 4);
        b.message(0xB0, 126, 4);
        a.message(status, note, 100);
        bool differs = false;
        energy = 0.0;
        for (int index = 0; index < 8; ++index)
        {
            a.render();
            b.render();
            differs = differs || !sameBlock(a, b);
            energy += blockEnergy(a);
            if (index == 0)
            {
                // A search of its own would have left a complete design.
                const auto& design = Access::sounding(engine, string);
                check(design.valid && !design.complete,
                      "the note takes an interim design in its note-on block", preset.name);
                interimNotes += design.valid && !design.complete ? 1 : 0;
            }
        }
        check(differs && energy > 0.0, "a note-on while unready sounds at once", preset.name);

        const int waited = a.renderUntilReady(static_cast<int>(10.0 * rate));
        check(engine.ready(), "ready within 10 s of a Shape change", preset.name);
        b.renderUntilReady(static_cast<int>(10.0 * rate));
        check(!Access::sounding(engine, string).complete,
              "a ringing note keeps its interim design", preset.name);

        // Every design made while playing is the one prepare() makes.
        int differing = 0;
        for (int slot = 0; slot < slotCount; ++slot)
        {
            std::array<double, 13> arguments {};
            if (Access::arguments(engine, slot, arguments)
                && !(Access::prepared(engine, slot).complete
                     && Access::same(Access::prepared(engine, slot),
                                     Access::prepared(fresh.performer->engine(), slot))))
                ++differing;
        }
        check(differing == 0, "the designs made while playing are prepare()'s", preset.name);

        // The fret below, then the stiff note again: a fresh pluck each.
        a.message(status, static_cast<std::uint8_t>(note - 1), 100);
        for (int index = 0; index < 8; ++index)
            a.render();
        a.message(static_cast<std::uint8_t>(0x80 + string), static_cast<std::uint8_t>(note - 1), 0);
        a.message(status, note, 100);
        a.render();
        check(Access::sounding(engine, string).complete,
              "the next fresh pluck takes a finished design", preset.name);
        a.message(static_cast<std::uint8_t>(0x80 + string), note, 0);
        c.renderUntilReady(static_cast<int>(10.0 * rate));
        check(Access::same(Access::plucked(c.performer->engine(), string, fret),
                           Access::prepared(c.performer->engine(), stiffSlot)),
              "a fresh pluck once ready holds its slot's finished design", preset.name);

        // A Wood change leaves every design as it was.
        auto rewooded = reshaped;
        rewooded.bodyMaterial = reshaped.bodyMaterial == aculele::BodyMaterial::Spruce
            ? aculele::BodyMaterial::Mahogany : aculele::BodyMaterial::Spruce;
        a.performer->setParameters(rewooded);
        const int rewoodWait = a.renderUntilReady(static_cast<int>(10.0 * rate));
        check(engine.ready() && rewoodWait <= 2816,
              "ready within 2,816 samples of a Wood change", preset.name);
        std::printf("%s: string %d fret %d interim %.1f cents, finished %.1f; ready %.2f s after a Shape change, %d samples after a Wood change\n",
                    preset.name, string, fret, static_cast<double>(interim.worstCents),
                    static_cast<double>(Access::prepared(engine, stiffSlot).worstCents),
                    waited / rate, rewoodWait);
    }
    check(interimNotes == static_cast<int>(aculele::factoryPresets.size()),
          "every preset played a note on an interim design", "all presets");
    if (failures == 0)
        std::printf("PASS ready\n");
    return failures == 0 ? 0 : 1;
}
