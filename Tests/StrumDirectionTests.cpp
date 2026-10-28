// Automatic strokes follow the strumming hand's down-up swing
// (Performer::pendulumUpstroke), and each direction meets the strings with
// its technique's nail or pad (AculeleEngine::nailStroke). Fingerpicking,
// the thumb and fingers take their nails.
//
// Each rhythm is played as four-string tab strokes (CC126 = 4 on channel 1)
// under Strum, or the technique named, at the default controls, 48 kHz and
// 256-sample blocks, with no direction keyswitch, and each stroke's
// direction is read from the strings it sounds. A rhythm is a list of onsets
// in beats; the hand swings in eighths unless a gap of an odd number of
// sixteenths puts it in sixteenths.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static float strokeSign(const AculeleEngine& engine, int string)
    {
        return engine.voices_[static_cast<std::size_t>(string)].pluckParallelSign;
    }
    static bool nail(const AculeleEngine& engine, int string)
    {
        return engine.nailStroke(engine.voices_[static_cast<std::size_t>(string)]);
    }
};
} // namespace aculele

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 256;
constexpr double bpm = 120.0;

int failures = 0;

// "D" or "U" per stroke, read from the G string's direction just after it;
// contacts gets "N" for a nail's stroke and "P" for a pad's.
std::string directions(const std::vector<double>& beats, bool pendulum,
                       aculele::PickingTechnique picking = aculele::PickingTechnique::Strum,
                       std::string* contacts = nullptr)
{
    auto performer = std::make_unique<aculele::Performer>();
    auto realism = performer->engine().performanceRealism();
    realism.strumPendulum = pendulum;
    performer->engine().setPerformanceRealism(realism);
    auto parameters = aculele::engineParameters(aculele::factoryPresets[0]);
    parameters.picking = picking;
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    performer->setTempoBpm(bpm);
    std::vector<float> left(block), right(block);
    std::string result;
    std::int64_t clock = 0;
    const std::uint8_t tab[3] { 0xB0, 126, 4 };
    performer->beginBlock(left.data(), right.data(), 1);
    performer->handleMidi(0, tab, 3);
    performer->endBlock();
    ++clock;
    constexpr std::uint8_t notes[4] { 67, 60, 64, 72 }; // C: 0003
    for (const double beat : beats)
    {
        const auto at = static_cast<std::int64_t>(std::llround(beat * 60.0 / bpm * rate)) + 1;
        while (clock < at)
        {
            const auto frames = static_cast<int>(std::min<std::int64_t>(block, at - clock));
            performer->beginBlock(left.data(), right.data(), frames);
            performer->endBlock();
            clock += frames;
        }
        performer->beginBlock(left.data(), right.data(), 1);
        for (int string = 0; string < 4; ++string)
        {
            const std::uint8_t off[3] { static_cast<std::uint8_t>(0x80 | string), notes[string], 0 };
            performer->handleMidi(0, off, 3);
        }
        for (int string = 0; string < 4; ++string)
        {
            const std::uint8_t on[3] { static_cast<std::uint8_t>(0x90 | string), notes[string], 96 };
            performer->handleMidi(0, on, 3);
        }
        performer->endBlock();
        ++clock;
        result += aculele::AculeleEngineTestAccess::strokeSign(performer->engine(), 0) < 0.0f
            ? 'U' : 'D';
        if (contacts != nullptr)
            *contacts += aculele::AculeleEngineTestAccess::nail(performer->engine(), 0) ? 'N' : 'P';
    }
    return result;
}

void expect(const char* name, const std::vector<double>& beats, const std::string& wanted,
            bool pendulum = true)
{
    const auto got = directions(beats, pendulum);
    if (got != wanted)
    {
        std::fprintf(stderr, "FAIL %s: %s, expected %s\n", name, got.c_str(), wanted.c_str());
        ++failures;
    }
}
} // namespace

int main()
{
    // D, D-U, U-D-U on the eighths of two bars, then the next downbeat.
    const std::vector<double> island { 0, 1, 1.5, 2.5, 3, 3.5, 4, 5, 5.5, 6.5, 7, 7.5, 8 };
    expect("island strum", island, "DDUUDUDDUUDUD");
    expect("quarter notes", { 0, 1, 2, 3, 4 }, "DDDDD");
    expect("eighth notes", { 0, 0.5, 1, 1.5, 2 }, "DUDUD");
    // D..U D..U: a dotted eighth and a sixteenth, swung in sixteenths.
    expect("dotted eighths", { 0, 0.75, 1, 1.75, 2 }, "DUDUD");
    // Sixteenths, then an eighth-note pattern within the bar: the hand is
    // still in sixteenths, so two sixteenths apart is the same direction.
    expect("sixteenths", { 0, 0.25, 0.5, 0.75, 1, 1.5 }, "DUDUDD");
    // A bar without an odd-sixteenth gap returns the hand to eighths.
    expect("back to eighths", { 0, 0.25, 0.5, 1.5, 2.5, 3.5, 4.5, 5.0, 5.5 }, "DUDDDDDUD");
    expect("swung eighths", { 0, 2.0 / 3.0, 1, 5.0 / 3.0, 2 }, "DUDUD");
    // An off-beat stroke after more than a bar's rest starts again downward.
    expect("after a rest", { 0, 0.5, 5.5 }, "DUD");
    // The former rule: a dotted eighth's gap alternates, a longer one restarts.
    expect("dotted-eighth rule", island, "DDUDUDUDUDUDU", false);

    // Under Pick a quarter-note line is all downstrokes, an eighth-note one
    // alternates, by the same swing.
    const auto picked = directions({ 0, 1, 2, 2.5, 3 }, true, aculele::PickingTechnique::Pick);
    if (picked != "DDDUD")
    {
        std::fprintf(stderr, "FAIL picked strums: %s\n", picked.c_str());
        ++failures;
    }

    // The nail meets the strings on a Strum's downstrokes and on a Thumb's
    // upstrokes, the pad on the others (AculeleEngine::nailStroke).
    for (const auto picking : { aculele::PickingTechnique::Strum, aculele::PickingTechnique::Thumb })
    {
        std::string contacts;
        const auto strokes = directions({ 0, 0.5, 1, 1.5 }, true, picking, &contacts);
        const bool thumb = picking == aculele::PickingTechnique::Thumb;
        if (strokes != "DUDU" || contacts != (thumb ? "PNPN" : "NPNP"))
        {
            std::fprintf(stderr, "FAIL %s contacts: %s %s\n", thumb ? "thumb" : "strum",
                         strokes.c_str(), contacts.c_str());
            ++failures;
        }
    }

    // Under Finger every string is plucked with a nail.
    {
        auto performer = std::make_unique<aculele::Performer>();
        auto parameters = aculele::engineParameters(aculele::factoryPresets[2]);
        performer->setParameters(parameters);
        performer->prepare(rate, block);
        performer->setParameters(parameters);
        std::string contacts;
        for (int string = 0; string < 4; ++string)
            contacts += aculele::AculeleEngineTestAccess::nail(performer->engine(), string) ? 'N' : 'P';
        if (contacts != "NNNN")
        {
            std::fprintf(stderr, "FAIL finger contacts: %s\n", contacts.c_str());
            ++failures;
        }
    }

    if (failures == 0)
        std::printf("strum directions pass\n");
    return failures == 0 ? 0 : 1;
}
