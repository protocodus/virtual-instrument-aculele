// A construction change under a ringing chord adds no tick.
//
// Changing Wood swaps the body's radiation and the bridge's modes. Both
// fade, so nothing steps where the strings meet the bridge. The sound above
// 5 kHz in the 20 ms after the change must stay within 6 dB of its peak over
// the 40 ms that follow, or 60 dB under the chord's peak just before it:
// changing back, the chord's own sound above 5 kHz has died away, and what
// is left of the change reads 67-71 dB under the chord. When the bridge's
// motion was carried across a step that never happened
// (AculeleEngine::applyDiscreteParameters), that first 20 ms rose 31-46 dB
// over the rest and stood 39-53 dB under the chord.
//
// Strings and Shape restring the ringing chord, which brightens it for a
// moment however smoothly it is done, so they are held to 45 and 40 dB under
// the chord. Nylon to Fluorocarbon drops the second dispersion section; cut
// out at full strength it stood 27 dB under, and retired at zero strength
// 49-53 dB (AculeleEngine::configureVoice). Concert to Tenor stood 27 dB
// under while its sections ramped over one round trip, and 47 dB under over
// 30 ms.
//
// A C chord is strummed on channel 1 at velocity 90 through the Performer at
// 48 kHz in 256-sample blocks, with the default controls and Strum. The
// control changes 1 s later and changes back 1 s after that, on a block
// boundary, as a host sends it.
//
// Strings and Shape also restring the strings that are not played: one block
// after the change, a held G4 and the three idle strings ringing with it each
// carry the tension an instrument built that way from the start gives them.
// Each string's pitch geometry was cached without the construction, so an
// idle string kept the old tension, and the held note took the new one only
// at its next attack-glide step, all at once (AculeleEngine::PitchGeometryKey).

#include "DSP/AculelePerformer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static float tension(const AculeleEngine& engine, int string)
    {
        return engine.voices_[static_cast<std::size_t>(string)].pitchGeometry.value.tension;
    }
};
} // namespace aculele

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 256;

int failures = 0;

void check(bool condition, const char* what)
{
    if (! condition)
    {
        std::fprintf(stderr, "FAIL %s\n", what);
        ++failures;
    }
}

// RBJ high-pass biquad, run twice for a 4th-order 5 kHz high-pass.
struct HighPass
{
    double b0, b1, b2, a1, a2;
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    explicit HighPass(double frequency)
    {
        const double w = 2.0 * 3.14159265358979323846 * frequency / rate;
        const double alpha = std::sin(w) / (2.0 * std::sqrt(0.5));
        const double a0 = 1.0 + alpha;
        b0 = (1.0 + std::cos(w)) / 2.0 / a0;
        b1 = -(1.0 + std::cos(w)) / a0;
        b2 = b0;
        a1 = -2.0 * std::cos(w) / a0;
        a2 = (1.0 - alpha) / a0;
    }

    double process(double x)
    {
        const double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        return y;
    }
};

template <typename Change>
void checkChange(Change change, double underChord, const char* what)
{
    aculele::EngineParameters parameters;
    parameters.picking = aculele::PickingTechnique::Strum;
    auto changed = parameters;
    change(changed);

    auto performer = std::make_unique<aculele::Performer>();
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);

    const int changeBlock = static_cast<int>(std::ceil(1.0 * rate / block));
    const int backBlock = static_cast<int>(std::ceil(2.0 * rate / block));
    const int blocks = backBlock + static_cast<int>(0.1 * rate / block);
    std::vector<float> left(block), right(block), output;
    for (int index = 0; index < blocks; ++index)
    {
        performer->setParameters(index >= changeBlock && index < backBlock ? changed : parameters);
        performer->beginBlock(left.data(), right.data(), block);
        if (index == 10)
            for (const int note : { 67, 60, 64, 72 })
            {
                const std::uint8_t on[3] { 0x90, static_cast<std::uint8_t>(note), 90 };
                performer->handleMidi(0, on, 3);
            }
        performer->endBlock();
        output.insert(output.end(), left.begin(), left.end());
    }

    HighPass first(5000.0), second(5000.0);
    std::vector<double> high(output.size());
    for (std::size_t sample = 0; sample < output.size(); ++sample)
        high[sample] = second.process(first.process(output[sample]));
    const auto peak = [&] (int from, int to)
    {
        double value = 0.0;
        for (int sample = from; sample < to; ++sample)
            value = std::max(value, std::abs(high[static_cast<std::size_t>(sample)]));
        return value;
    };
    const int window = static_cast<int>(0.02 * rate);
    for (const int at : { changeBlock * block, backBlock * block })
    {
        const double tick = peak(at, at + window);
        const double after = peak(at + window, at + 3 * window);
        double chord = 0.0;
        for (int sample = at - window; sample < at; ++sample)
            chord = std::max(chord, static_cast<double>(std::abs(output[static_cast<std::size_t>(sample)])));
        const double rise = 20.0 * std::log10((tick + 1e-12) / (after + 1e-12));
        const double depth = 20.0 * std::log10((chord + 1e-12) / (tick + 1e-12));
        std::printf("%-28s at %.3f s: first 20 ms %+.1f dB over the next 40 ms, "
                    "%.1f dB under the chord\n", what, at / rate, rise, depth);
        check(rise < 6.0 || depth > underChord, what);
    }
}
// Holds G4 on the G string, changes the control 1 s later, and returns each
// string's tension one block after that; with atOneSecond false the control is
// changed from the start.
template <typename Change>
std::vector<float> tensions(Change change, bool atOneSecond)
{
    aculele::EngineParameters parameters;
    parameters.picking = aculele::PickingTechnique::Strum;
    auto changed = parameters;
    change(changed);
    const auto& first = atOneSecond ? parameters : changed;

    auto performer = std::make_unique<aculele::Performer>();
    performer->setParameters(first);
    performer->prepare(rate, block);
    performer->setParameters(first);
    const int changeBlock = static_cast<int>(std::ceil(1.0 * rate / block));
    std::vector<float> left(block), right(block);
    for (int index = 0; index <= changeBlock; ++index)
    {
        performer->setParameters(index >= changeBlock ? changed : first);
        performer->beginBlock(left.data(), right.data(), block);
        if (index == 10)
        {
            const std::uint8_t on[3] { 0x90, 67, 90 };
            performer->handleMidi(0, on, 3);
        }
        performer->endBlock();
    }
    std::vector<float> result;
    for (int string = 0; string < aculele::AculeleEngine::stringCount; ++string)
        result.push_back(aculele::AculeleEngineTestAccess::tension(performer->engine(), string));
    return result;
}

template <typename Change>
void checkTension(Change change, const char* what)
{
    const auto after = tensions(change, true);
    const auto built = tensions(change, false);
    for (std::size_t string = 0; string < after.size(); ++string)
    {
        std::printf("%-28s string %zu: %.3f N, built so %.3f N\n", what, string,
                    static_cast<double>(after[string]), static_cast<double>(built[string]));
        check(after[string] == built[string], what);
    }
}
} // namespace

int main()
{
    using P = aculele::EngineParameters;
    checkChange([] (P& p) { p.bodyMaterial = aculele::BodyMaterial::Spruce; }, 60.0,
                "Mahogany to Spruce and back");
    checkChange([] (P& p) { p.bodyMaterial = aculele::BodyMaterial::Maple; }, 60.0,
                "Mahogany to Maple and back");
    checkChange([] (P& p) { p.strings = aculele::StringMaterial::Fluorocarbon; }, 45.0,
                "Nylon to Fluorocarbon and back");
    checkChange([] (P& p) { p.shape = aculele::BodyShape::Tenor; }, 40.0,
                "Concert to Tenor and back");
    checkTension([] (P& p) { p.strings = aculele::StringMaterial::Fluorocarbon; },
                 "Nylon to Fluorocarbon");
    checkTension([] (P& p) { p.shape = aculele::BodyShape::Tenor; }, "Concert to Tenor");
    if (failures > 0)
    {
        std::fprintf(stderr, "%d construction change checks failed\n", failures);
        return EXIT_FAILURE;
    }
    std::puts("a Wood, Strings or Shape change under a ringing chord adds no tick, "
              "and Strings and Shape restring every string");
    return EXIT_SUCCESS;
}
