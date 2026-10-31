// An audio reset (Performer::reset, which the Rack adapter calls on the
// host's reset request) leaves nothing of what was played: after a phrase
// that strums, picks, bends, shakes the vibrato and leaves notes ringing, a
// reset plays the next phrase sample for sample as a freshly prepared
// Performer does, for every factory preset. Before 0.4.37 a reset kept the
// strings' and strums' random draws going, so the same phrase after it
// differed from a fresh instance's by more than the phrase itself.
//
// 48 kHz, 64-sample blocks, both main channels compared.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <array>
#include <cstdio>
#include <memory>
#include <vector>

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 64;
int failures = 0;

void check(bool ok, const char* what, const char* preset)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s: %s\n", preset, what);
        ++failures;
    }
}

std::vector<float> phrase(aculele::Performer& performer, bool bend)
{
    std::vector<float> out;
    std::array<float, block> left {}, right {};
    for (int index = 0; index < 400; ++index)
    {
        performer.beginBlock(left.data(), right.data(), block);
        if (index == 0)
            for (int note : { 62, 65, 71 })
                performer.noteOn(0, 1, note, 100);
        if (index == 120)
        {
            performer.noteOff(0, 1, 65);
            performer.noteOff(0, 1, 71);
            performer.noteOn(17, 1, 74, 90);
            if (bend)
            {
                performer.pitchWheel(5, 1, 0.7f);
                performer.controlChange(9, 1, 1, 100);
            }
        }
        performer.endBlock();
        out.insert(out.end(), left.begin(), left.end());
        out.insert(out.end(), right.begin(), right.end());
    }
    return out;
}

std::unique_ptr<aculele::Performer> prepared(const aculele::FactoryPreset& preset)
{
    auto performer = std::make_unique<aculele::Performer>();
    const auto parameters = aculele::engineParameters(preset);
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    return performer;
}
} // namespace

int main()
{
    for (const auto& preset : aculele::factoryPresets)
    {
        auto fresh = prepared(preset);
        auto played = prepared(preset);
        phrase(*played, true);
        played->reset();
        const auto expected = phrase(*fresh, false);
        check(phrase(*played, false) == expected, "a reset plays as a fresh instance, sample for sample", preset.name);
        // And again: a second reset is no different from the first.
        played->reset();
        check(phrase(*played, false) == expected, "a second reset plays as a fresh instance too", preset.name);
    }
    std::printf(failures == 0 ? "Aculele.Reset: all passed\n" : "Aculele.Reset: %d failed\n", failures);
    return failures == 0 ? 0 : 1;
}
