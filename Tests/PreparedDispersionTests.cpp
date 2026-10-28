// The dispersion designs made ahead of their notes (AculeleEngine::
// prepareDispersion) are the ones a fresh pluck asks for, and the same
// designs a whole search at the pluck would make.
//
// For every body, string set and tuning at 48 kHz, and two at 44.1 kHz with
// the Pick (which adds the 48 kHz pick reference), the engine runs silent
// until every slot is ready. Then each string is configured as a fresh pluck
// at every fret: the arguments it asks for must be a prepared slot's. For
// one configuration at each rate, every prepared design must equal, bit for
// bit, the design a search from nothing makes for its arguments.

#include "DSP/AculelePerformer.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    // Whether every slot the configuration uses holds its current design.
    static bool ready(const AculeleEngine& engine)
    {
        for (int slot = 0; slot < AculeleEngine::preparedDispersionCount; ++slot)
        {
            std::array<double, 13> arguments {};
            const auto& prepared = engine.preparedDispersion_[static_cast<std::size_t>(slot)];
            if (engine.preparedDispersionArguments(slot, arguments)
                && !(prepared.valid && prepared.arguments == arguments))
                return false;
        }
        return true;
    }

    static bool isPrepared(const AculeleEngine& engine,
                           const std::array<double, 13>& arguments)
    {
        for (const auto& prepared : engine.preparedDispersion_)
            if (prepared.valid && prepared.arguments == arguments)
                return true;
        return false;
    }

    // A fresh pluck's first configuration of a string at a fret, then the
    // string back at rest. Returns the arguments of its design and, when
    // the picking needs one, of its 48 kHz reference.
    static int freshArguments(AculeleEngine& engine, int string, int fret,
                              std::array<double, 13> (&arguments)[2])
    {
        auto& voice = engine.voices_[static_cast<std::size_t>(string)];
        voice.referencePickDispersion = {};
        engine.configureVoice(voice, string, voice.openMidi + fret, true, false);
        arguments[0] = voice.dispersionDesign.arguments;
        arguments[1] = voice.referencePickDispersion.arguments;
        const int count = voice.referencePickDispersion.valid ? 2 : 1;
        engine.returnToOpenString(voice, string, true);
        return count;
    }

    // Every prepared design against a search from nothing for its arguments.
    static int differingFromSearch(AculeleEngine& engine)
    {
        const auto prepared = engine.preparedDispersion_;
        engine.preparedDispersion_ = {};
        engine.dispersionSolves_ = {};
        int differing = 0;
        for (const auto& design : prepared)
        {
            if (!design.valid)
                continue;
            const auto searched = engine.solvedDispersion(design.arguments);
            if (std::memcmp(searched.decayRatios.data(), design.decayRatios.data(), sizeof(design.decayRatios)) != 0
                || std::memcmp(searched.poleRatios.data(), design.poleRatios.data(), sizeof(design.poleRatios)) != 0
                || std::memcmp(&searched.worstCents, &design.worstCents, sizeof(float)) != 0)
                ++differing;
        }
        engine.preparedDispersion_ = prepared;
        return differing;
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
constexpr int block = 256;

int failures = 0;

void check(bool ok, const char* what, int shape, int strings, int tuning, double rate)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s (shape %d, strings %d, tuning %d, %.0f Hz)\n",
                     what, shape, strings, tuning, rate);
        ++failures;
    }
}

void run(int shape, int strings, int tuning, double rate,
         aculele::PickingTechnique picking, bool compareSearch)
{
    auto performer = std::make_unique<aculele::Performer>();
    aculele::EngineParameters parameters;
    parameters.shape = static_cast<aculele::BodyShape>(shape);
    parameters.strings = static_cast<aculele::StringMaterial>(strings);
    parameters.tuning = static_cast<aculele::Tuning>(tuning);
    parameters.picking = picking;
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    std::vector<float> left(block), right(block);
    auto& engine = performer->engine();
    // About 2 s is typical; 20 s bounds a slot that never gets there.
    int samples = 0;
    while (!Access::ready(engine) && samples < static_cast<int>(20.0 * rate))
    {
        performer->process(left.data(), right.data(), block);
        samples += block;
    }
    check(Access::ready(engine), "slots ready within 20 s", shape, strings, tuning, rate);
    for (int string = 0; string < aculele::AculeleEngine::stringCount; ++string)
        for (int fret = 0; fret <= aculele::AculeleEngine::fretCount; ++fret)
        {
            std::array<double, 13> arguments[2] {};
            const int count = Access::freshArguments(engine, string, fret, arguments);
            for (int design = 0; design < count; ++design)
                if (!Access::isPrepared(engine, arguments[design]))
                {
                    std::fprintf(stderr, "  string %d fret %d %s\n", string, fret,
                                 design == 0 ? "design" : "48 kHz reference");
                    check(false, "a fresh pluck's design was prepared", shape, strings, tuning, rate);
                }
        }
    if (compareSearch)
        check(Access::differingFromSearch(engine) == 0,
              "prepared designs equal a search from nothing", shape, strings, tuning, rate);
}
} // namespace

int main()
{
    for (int shape = 0; shape < 3; ++shape)
        for (int strings = 0; strings < 2; ++strings)
            for (int tuning = 0; tuning < 3; ++tuning)
                run(shape, strings, tuning, 48000.0, aculele::PickingTechnique::Finger,
                    shape == 1 && strings == 0 && tuning == 0);
    run(1, 0, 0, 44100.0, aculele::PickingTechnique::Pick, true);
    run(2, 1, 1, 44100.0, aculele::PickingTechnique::Pick, false);
    if (failures == 0)
        std::printf("PASS prepared dispersion designs\n");
    return failures == 0 ? 0 : 1;
}
