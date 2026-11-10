// A retained wave reads only the part of its lines it was given.
//
// When a string is plucked over a wave still ringing, the old wave goes on
// as a second branch of the string (captureTail). Its two loops used to take
// the voice's whole delay lines, 32 KB each at 48 kHz, where a retained
// wave, whose delay nothing sets again, reads no further back than its own
// round trip (StringLoop::captureFrom, which derives the reach). They now
// take that much.
//
// The check is a poison: with the engine's test switch on, captureFrom
// writes a value no string takes (StringLoop::unreachedLinePoison) over
// every sample of the retained line it did not copy. A wave that reads
// past what it was given reads it, and the output differs. Each phrase is
// played through the Performer twice at 48 kHz in 256-sample blocks, plain
// and poisoned, on every factory preset, and the two must be finite and
// equal bit for bit:
//
// - the preset's README demo;
// - eight notes under the pedal, each struck again, then eight more over
//   them (the heaviest block of Docs/benchmarks);
// - a chord, the body changing size under it, and chords after it every
//   0.2 s, by turns high and low on the neck;
// - chords struck again every 0.2 s while the pitch wheel, set to two
//   octaves, swings wide before and after each stroke, so that waves are
//   retained in the middle of a slide, and the bridge hand comes down,
//   lifts and sweeps;
// - chords bent far down, let go there, so that the damping hand lands on
//   a long string, and thrown far up 65 ms before the next chord;
// - String Age swept from new to old over 2 s under two chords high on the
//   neck struck by turns every 50 ms.
//
// The two terms of the reach these phrases need not meet, a wave retained
// with dispersion sections still to retire and one whose hand landed when
// its string was longer, are checked on the loop itself
// (Tests/DispersionStateTests.cpp).
//
// Each phrase must also retain waves whose lines hold the poison while they
// ring, or the comparison would prove nothing.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"
#include "PresetDemoRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static void poison(AculeleEngine& engine) { engine.poisonUnreachedLines_ = true; }
    // Retained waves ringing now whose lines hold the poison.
    static int poisonedTails(const AculeleEngine& engine)
    {
        int count = 0;
        for (const auto& voice : engine.voices_)
        {
            if (!voice.tailActive)
                continue;
            for (const auto* loop : { &voice.tailLoop, &voice.tailParallelLoop })
                for (std::size_t index = 0; index < loop->delay.size(); ++index)
                    if (loop->delay[index] == AculeleEngine::StringLoop::unreachedLinePoison)
                    {
                        ++count;
                        break;
                    }
        }
        return count;
    }
};
}

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 256;

// kind 0: MIDI bytes at its own sample; 1: the body changes size; 2: String
// Age takes value.
struct Event
{
    double seconds;
    int kind;
    std::array<std::uint8_t, 3> bytes;
    float value { 0.0f };
};

Event midi(double seconds, int status, int first, int second)
{
    return { seconds, 0, { static_cast<std::uint8_t>(status),
                           static_cast<std::uint8_t>(first),
                           static_cast<std::uint8_t>(second) }, 0.0f };
}

std::vector<Event> stressPhrase()
{
    std::vector<Event> events;
    events.push_back(midi(0.0, 0xB0, 64, 127));
    const int first[] { 60, 64, 67, 72, 76, 79, 84, 88 };
    const int second[] { 62, 65, 69, 74, 77, 81, 86, 89 };
    for (const int note : first)
        events.push_back(midi(0.05, 0x90, note, 127));
    for (int index = 0; index < 8; ++index)
    {
        events.push_back(midi(1.0 + 0.25 * index, 0x80, first[index], 0));
        events.push_back(midi(1.0 + 0.25 * index, 0x90, first[index], 127));
    }
    for (const int note : second)
        events.push_back(midi(3.5, 0x90, note, 127));
    for (const int note : first)
        events.push_back(midi(6.0, 0x80, note, 0));
    for (const int note : second)
        events.push_back(midi(6.0, 0x80, note, 0));
    events.push_back(midi(6.5, 0xB0, 64, 0));
    return events;
}

std::vector<Event> bodyChangePhrase()
{
    std::vector<Event> events;
    const int low[] { 60, 64, 67, 72 };
    const int high[] { 74, 79, 83, 86 };
    for (const int note : low)
        events.push_back(midi(0.05, 0x90, note, 110));
    events.push_back({ 1.0, 1, {}, 0.0f });
    for (int stroke = 0; stroke < 12; ++stroke)
    {
        const double seconds = 1.0 + block / rate + 0.2 * stroke;
        for (const int note : (stroke % 2 != 0 ? low : high))
        {
            events.push_back(midi(seconds, 0x80, note, 0));
            events.push_back(midi(seconds, 0x90, note, 110));
        }
    }
    return events;
}

std::vector<Event> bendAndHandPhrase()
{
    std::vector<Event> events;
    // RPN 0: a pitch wheel of two octaves each way.
    events.push_back(midi(0.0, 0xB0, 101, 0));
    events.push_back(midi(0.0, 0xB0, 100, 0));
    events.push_back(midi(0.0, 0xB0, 6, 24));
    events.push_back(midi(0.0, 0xB0, 64, 127));
    const int low[] { 60, 64, 67, 72 };
    const int high[] { 74, 79, 83, 86 };
    // The wheel's 14-bit positions by turns: far down, centre, far up, half
    // down. It moves 2 ms before each stroke and again 3 ms after it.
    const int wheel[] { 0, 8192, 16383, 4096 };
    for (int stroke = 0; stroke < 20; ++stroke)
    {
        const double seconds = 0.05 + 0.2 * stroke;
        const int before = wheel[stroke % 4];
        const int after = wheel[(stroke + 1) % 4];
        events.push_back(midi(seconds - 0.002, 0xE0, before & 127, before >> 7));
        for (const int note : (stroke % 2 != 0 ? low : high))
        {
            events.push_back(midi(seconds, 0x80, note, 0));
            events.push_back(midi(seconds, 0x90, note, 110));
        }
        events.push_back(midi(seconds + 0.003, 0xE0, after & 127, after >> 7));
    }
    // The bridge hand: down at 0.5 s, off at 1.5 s, then a sweep.
    events.push_back(midi(0.5, 0xB0, 2, 100));
    events.push_back(midi(1.5, 0xB0, 2, 0));
    for (int step = 0; step < 254; ++step)
        events.push_back(midi(2.5 + step * block / rate, 0xB0, 2,
                              step < 127 ? step + 1 : 253 - step));
    std::stable_sort(events.begin(), events.end(),
        [] (const Event& a, const Event& b) { return a.seconds < b.seconds; });
    return events;
}

std::vector<Event> handThenBendPhrase()
{
    std::vector<Event> events;
    events.push_back(midi(0.0, 0xB0, 101, 0));
    events.push_back(midi(0.0, 0xB0, 100, 0));
    events.push_back(midi(0.0, 0xB0, 6, 24));
    const int low[] { 60, 64, 67, 72 };
    const int high[] { 74, 79, 83, 86 };
    for (int stroke = 0; stroke < 16; ++stroke)
    {
        const double seconds = 0.05 + 0.25 * stroke;
        for (const int note : (stroke % 2 != 0 ? low : high))
        {
            events.push_back(midi(seconds, 0x90, note, 110));
            events.push_back(midi(seconds + 0.18, 0x80, note, 64));
        }
        events.push_back(midi(seconds + 0.01, 0xE0, 0, 0));
        events.push_back(midi(seconds + 0.185, 0xE0, 127, 127));
    }
    std::stable_sort(events.begin(), events.end(),
        [] (const Event& a, const Event& b) { return a.seconds < b.seconds; });
    return events;
}

std::vector<Event> ageSweepPhrase()
{
    std::vector<Event> events;
    const int high[] { 74, 79, 83, 86 };
    const int higher[] { 76, 81, 84, 88 };
    for (int stroke = 0; stroke < 64; ++stroke)
    {
        const double seconds = 0.05 + 0.05 * stroke;
        for (const int note : (stroke % 2 != 0 ? higher : high))
            events.push_back(midi(seconds, 0x90, note, 110));
        for (const int note : (stroke % 2 != 0 ? high : higher))
            events.push_back(midi(seconds, 0x80, note, 0));
    }
    for (int step = 0; step < 375; ++step)
        events.push_back({ 0.5 + step * block / rate, 2, {},
                           static_cast<float>(step) / 374.0f });
    std::stable_sort(events.begin(), events.end(),
        [] (const Event& a, const Event& b) { return a.seconds < b.seconds; });
    return events;
}

std::vector<Event> demoPhrase(const std::filesystem::path& file)
{
    std::vector<Event> events;
    for (const auto& event : protocodus::demos::ReadMidi(file))
        if (event.size == 3)
            events.push_back(midi(event.seconds, event.bytes[0], event.bytes[1],
                                  event.bytes[2]));
    return events;
}

struct Render
{
    std::vector<float> audio;
    int poisonedTailBlocks { 0 };
};

Render play(std::size_t preset, const std::vector<Event>& events,
            double seconds, bool poisoned)
{
    auto performer = std::make_unique<aculele::Performer>();
    auto parameters = aculele::engineParameters(aculele::factoryPresets[preset]);
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    performer->setTempoBpm(120.0);
    if (poisoned)
        aculele::AculeleEngineTestAccess::poison(performer->engine());
    const int blocks = static_cast<int>(seconds * rate / block);
    Render render;
    render.audio.resize(static_cast<std::size_t>(blocks) * block * 2);
    std::array<float, block> left {}, right {};
    std::size_t next = 0;
    for (int index = 0; index < blocks; ++index)
    {
        const double start = static_cast<double>(index) * block / rate;
        const double end = static_cast<double>(index + 1) * block / rate;
        for (std::size_t scan = next; scan < events.size() && events[scan].seconds < end; ++scan)
        {
            if (events[scan].kind == 1)
                parameters.shape = parameters.shape == aculele::BodyShape::Tenor
                    ? aculele::BodyShape::Soprano : aculele::BodyShape::Tenor;
            else if (events[scan].kind == 2)
                parameters.stringAge = events[scan].value;
            if (events[scan].kind != 0)
                performer->setParameters(parameters);
        }
        performer->beginBlock(left.data(), right.data(), block);
        for (; next < events.size() && events[next].seconds < end; ++next)
        {
            if (events[next].kind != 0)
                continue;
            const int offset = std::clamp(
                static_cast<int>((events[next].seconds - start) * rate), 0, block - 1);
            performer->handleMidi(offset, events[next].bytes.data(), 3);
        }
        performer->endBlock();
        if (aculele::AculeleEngineTestAccess::poisonedTails(performer->engine()) > 0)
            ++render.poisonedTailBlocks;
        const auto at = static_cast<std::size_t>(index) * block * 2;
        std::copy(left.begin(), left.end(), render.audio.begin() + static_cast<std::ptrdiff_t>(at));
        std::copy(right.begin(), right.end(),
                  render.audio.begin() + static_cast<std::ptrdiff_t>(at + block));
    }
    return render;
}

bool check(const char* phrase, std::size_t preset, const std::vector<Event>& events,
           double seconds)
{
    const auto plain = play(preset, events, seconds, false);
    const auto poisoned = play(preset, events, seconds, true);
    const char* name = aculele::factoryPresets[preset].name;
    bool passed = true;
    for (const float sample : poisoned.audio)
        if (!std::isfinite(sample))
        {
            std::fprintf(stderr, "FAIL %s, %s: the poisoned render is not finite\n",
                         phrase, name);
            passed = false;
            break;
        }
    if (std::memcmp(plain.audio.data(), poisoned.audio.data(),
                    plain.audio.size() * sizeof(float)) != 0)
    {
        std::size_t first = 0;
        while (std::memcmp(&plain.audio[first], &poisoned.audio[first], sizeof(float)) == 0)
            ++first;
        std::fprintf(stderr, "FAIL %s, %s: a retained wave read past its reach "
                     "(the renders part in block %zu)\n",
                     phrase, name, first / (2 * static_cast<std::size_t>(block)));
        passed = false;
    }
    if (plain.poisonedTailBlocks != 0)
    {
        std::fprintf(stderr, "FAIL %s, %s: the plain render holds the poison\n",
                     phrase, name);
        passed = false;
    }
    if (poisoned.poisonedTailBlocks == 0)
    {
        std::fprintf(stderr, "FAIL %s, %s: no retained wave held the poison, "
                     "so nothing was tested\n", phrase, name);
        passed = false;
    }
    std::printf("%-12s %-32s %5.1f s, poison beside a ringing retained wave in %d blocks: %s\n",
                phrase, name, seconds, poisoned.poisonedTailBlocks,
                passed ? "equal" : "FAIL");
    return passed;
}
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: AculeleTailReachTests <Docs/audio/midi>\n");
        return 2;
    }
    std::vector<std::filesystem::path> demos;
    for (const auto& entry : std::filesystem::directory_iterator(argv[1]))
        if (entry.path().extension() == ".mid")
            demos.push_back(entry.path());
    std::sort(demos.begin(), demos.end());
    bool passed = true;
    if (demos.size() != aculele::factoryPresets.size())
    {
        std::fprintf(stderr, "FAIL %zu demo phrases for %zu presets\n",
                     demos.size(), aculele::factoryPresets.size());
        passed = false;
    }
    const auto stress = stressPhrase();
    const auto bodyChange = bodyChangePhrase();
    const auto bendAndHand = bendAndHandPhrase();
    const auto handThenBend = handThenBendPhrase();
    const auto ageSweep = ageSweepPhrase();
    for (std::size_t preset = 0; preset < aculele::factoryPresets.size(); ++preset)
    {
        if (preset < demos.size())
        {
            const auto demo = demoPhrase(demos[preset]);
            passed = check("demo", preset, demo,
                std::min(protocodus::demos::kMaxSeconds,
                         demo.back().seconds + protocodus::demos::kTailSeconds))
                && passed;
        }
        passed = check("stress", preset, stress, 9.5) && passed;
        passed = check("body change", preset, bodyChange, 4.5) && passed;
        passed = check("bend, hand", preset, bendAndHand, 5.5) && passed;
        passed = check("hand, bend", preset, handThenBend, 4.5) && passed;
        passed = check("age sweep", preset, ageSweep, 4.0) && passed;
    }
    std::printf(passed ? "PASS\n" : "FAIL\n");
    return passed ? 0 : 1;
}
