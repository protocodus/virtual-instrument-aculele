// Each factory preset's Output level (AculelePresets.h) leaves headroom for
// the hardest playing a keyboard gives it, without leaving the preset quiet.
//
// Two phrases are struck in eighths at 120 bpm, velocity 127, on one MIDI
// channel, so the Performer chooses the strings and fills the triads to four:
// the open-position triads C, Am, F and G, and four-note chords up the neck.
// The louder of the two peaks at -3 dBFS, within half a decibel: 2 dB under
// the -1 dBFS knee of the engine's safety limiter (safetyLimit), which is
// linear below it. The chords up the neck struck in sixteenths, the hardest
// tremolo, stay under the knee. When an engine change moves these peaks,
// re-trim the presets to the eighths' -3 dBFS rather than move the bounds.
//
// 48 kHz, 256-sample blocks, each preset's own controls.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 256;
constexpr double eighth = 0.25;  // seconds at 120 bpm
constexpr double limiterKneeDbfs = -1.0;
constexpr double targetDbfs = -3.0;
constexpr double tolerance = 0.5;

using Chord = std::vector<int>;
const std::vector<Chord> openTriads { { 60, 64, 67 }, { 60, 64, 69 }, { 60, 65, 69 }, { 62, 67, 71 } };
const std::vector<Chord> chordsUpTheNeck {
    { 64, 67, 72, 76 }, { 65, 69, 72, 77 }, { 67, 71, 74, 79 }, { 69, 72, 76, 81 } };

int failures = 0;

void check(bool ok, const char* what, const char* preset)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s: %s\n", preset, what);
        ++failures;
    }
}

// The output's peak in dBFS with each chord struck `times` times, `step`
// seconds apart and held for 0.9 of a step, then 2.5 s of ringing.
double peakDbfs(const aculele::FactoryPreset& preset, const std::vector<Chord>& chords,
                int times, double step)
{
    struct Event
    {
        long long at;
        bool on;
        int note;
    };
    std::vector<Event> events;
    double start = 0.0;
    for (const auto& chord : chords)
        for (int repeat = 0; repeat < times; ++repeat, start += step)
            for (const int note : chord)
            {
                events.push_back({ std::llround(start * rate), true, note });
                events.push_back({ std::llround((start + 0.9 * step) * rate), false, note });
            }
    // A key-up and the next key-down share a sample: the key-up goes first.
    std::stable_sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.at != b.at ? a.at < b.at : !a.on && b.on;
    });

    auto parameters = aculele::engineParameters(preset);
    auto performer = std::make_unique<aculele::Performer>();
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    performer->setTempoBpm(120.0);

    std::vector<float> left(block), right(block);
    const long long total = events.back().at + std::llround(2.5 * rate);
    float peak = 0.0f;
    std::size_t next = 0;
    for (long long done = 0; done < total; done += block)
    {
        performer->beginBlock(left.data(), right.data(), block);
        for (; next < events.size() && events[next].at < done + block; ++next)
        {
            const auto offset = static_cast<int>(events[next].at - done);
            if (events[next].on)
                performer->noteOn(offset, 1, events[next].note, 127);
            else
                performer->noteOff(offset, 1, events[next].note);
        }
        performer->endBlock();
        for (int i = 0; i < block; ++i)
            peak = std::max({ peak, std::abs(left[static_cast<std::size_t>(i)]),
                              std::abs(right[static_cast<std::size_t>(i)]) });
    }
    return 20.0 * std::log10(static_cast<double>(peak) + 1.0e-30);
}
} // namespace

int main()
{
    for (const auto& preset : aculele::factoryPresets)
    {
        const double triads = peakDbfs(preset, openTriads, 8, eighth);
        const double upTheNeck = peakDbfs(preset, chordsUpTheNeck, 8, eighth);
        const double tremolo = peakDbfs(preset, chordsUpTheNeck, 16, eighth / 2.0);
        std::printf("%-32s Output %+5.1f dB  eighths: triads %6.2f, up the neck %6.2f;"
                    "  sixteenths %6.2f dBFS\n",
                    preset.name, static_cast<double>(preset.outputDecibels), triads, upTheNeck, tremolo);
        const double loudest = std::max(triads, upTheNeck);
        check(loudest <= targetDbfs + tolerance, "the eighths leave 2 dB under the limiter", preset.name);
        check(loudest >= targetDbfs - tolerance, "the eighths are not trimmed quieter than that", preset.name);
        check(tremolo < limiterKneeDbfs, "the sixteenths stay under the limiter", preset.name);
    }
    if (failures == 0)
        std::printf("headroom passes\n");
    return failures == 0 ? 0 : 1;
}
