// While F0 is held every swept stroke is a chuck (Performer::keyswitchChuck):
// each string the stroke crosses takes a dead stroke under the fretting hand
// and no key owns a string.
//
// A C chord of three keys on one channel (C4 E4 G4, which the stroke fills
// to 0003) is strummed at the default controls, 48 kHz and 256-sample
// blocks, and the strings' state and the output are read after it.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static bool muted(const AculeleEngine& engine, int string)
    {
        return engine.voices_[static_cast<std::size_t>(string)].mutedStroke;
    }
    static bool keyDown(const AculeleEngine& engine, int string)
    {
        return engine.voices_[static_cast<std::size_t>(string)].keyDown;
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
constexpr double rate = 48000.0;
constexpr int block = 256;
constexpr int chuckKey = aculele::Performer::keyswitchChuck;
// E0 is the Rack adapter's connected-gesture cue, so no keyswitch takes it.
static_assert(!aculele::Performer::isKeyswitch(28), "E0 stays free");
constexpr int chord[3] { 60, 64, 67 };

int failures = 0;

void check(bool ok, const char* what)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s\n", what);
        ++failures;
    }
}

struct Rig
{
    std::unique_ptr<aculele::Performer> performer = std::make_unique<aculele::Performer>();
    std::vector<float> left = std::vector<float>(block), right = std::vector<float>(block);
    std::vector<float> output;

    Rig()
    {
        auto parameters = aculele::engineParameters(aculele::factoryPresets[0]);
        parameters.picking = aculele::PickingTechnique::Strum;
        performer->setParameters(parameters);
        performer->prepare(rate, block);
        performer->setParameters(parameters);
    }

    // Key events at the start of one block, the keyswitch first.
    void keys(int keyswitch, bool keyswitchDown, bool chordUp, bool chordDown)
    {
        performer->beginBlock(left.data(), right.data(), block);
        if (keyswitch >= 0)
            keyswitchDown ? performer->noteOn(0, 1, keyswitch, 100)
                          : performer->noteOff(0, 1, keyswitch);
        for (const int note : chord)
        {
            if (chordUp)
                performer->noteOff(0, 1, note);
            if (chordDown)
                performer->noteOn(0, 1, note, 96);
        }
        performer->endBlock();
        output.insert(output.end(), left.begin(), left.end());
    }

    void render(int samples)
    {
        for (int done = 0; done < samples; done += block)
        {
            performer->process(left.data(), right.data(), block);
            output.insert(output.end(), left.begin(), left.end());
        }
    }

    [[nodiscard]] bool allMuted() const
    {
        for (int string = 0; string < aculele::AculeleEngine::stringCount; ++string)
            if (!Access::muted(performer->engine(), string)
                || Access::keyDown(performer->engine(), string))
                return false;
        return true;
    }

    // RMS in dB of the output between two times after a stroke at `from`.
    [[nodiscard]] double level(std::size_t from, double start, double end) const
    {
        const auto first = from + static_cast<std::size_t>(start * rate);
        const auto last = std::min(output.size(), from + static_cast<std::size_t>(end * rate));
        double sum = 0.0;
        for (auto index = first; index < last; ++index)
            sum += static_cast<double>(output[index]) * output[index];
        return 10.0 * std::log10(sum / static_cast<double>(last - first) + 1.0e-30);
    }
};
} // namespace

int main()
{
    const int stroke = static_cast<int>(0.05 * rate);
    {
        // The key alone sounds nothing; with it down, a chord is a chuck.
        Rig rig;
        rig.keys(chuckKey, true, false, false);
        rig.render(stroke);
        check(*std::max_element(rig.output.begin(), rig.output.end()) == 0.0f,
              "the chuck key sounds nothing");
        const auto chuckAt = rig.output.size();
        rig.keys(-1, false, false, true);
        rig.render(stroke);
        check(rig.allMuted(), "a chuck gives every string a dead stroke");
        rig.render(static_cast<int>(rate));
        rig.keys(-1, false, true, false);

        // Once the key is up the same chord sounds and rings on.
        rig.keys(chuckKey, false, false, false);
        const auto strumAt = rig.output.size();
        rig.keys(-1, false, false, true);
        rig.render(stroke);
        for (int string = 0; string < aculele::AculeleEngine::stringCount; ++string)
            check(!Access::muted(rig.performer->engine(), string),
                  "the chuck key's key-up ends its chucks");
        rig.render(static_cast<int>(rate));
        const double chuckAttack = rig.level(chuckAt, 0.0, 0.1);
        const double strumAttack = rig.level(strumAt, 0.0, 0.1);
        const double chuckTail = rig.level(chuckAt, 0.3, 0.5);
        const double strumTail = rig.level(strumAt, 0.3, 0.5);
        std::printf("0-0.1 s: chuck %.1f dB, strum %.1f dB; 0.3-0.5 s: chuck %.1f dB, strum %.1f dB\n",
                    chuckAttack, strumAttack, chuckTail, strumTail);
        // A dead stroke strikes at GuitarSet's 0.589 of a sounding one.
        check(chuckAttack > strumAttack - 10.0, "a chuck is heard");
        check(chuckTail < strumTail - 20.0, "a chuck dies away where a strum rings");
    }
    {
        // A chuck drawn straight after a held chord, its keys coming up on
        // the chuck's own sample, mutes the strings that chord holds.
        Rig rig;
        rig.keys(-1, false, false, true);
        rig.render(static_cast<int>(0.5 * rate));
        rig.keys(chuckKey, true, true, true);
        rig.render(stroke);
        check(rig.allMuted(), "a chuck straight after a chord mutes its strings");
    }

    if (failures == 0)
        std::printf("chucks pass\n");
    return failures == 0 ? 0 : 1;
}
