// U11 parameter reach over the Performer and the controls every front end
// gives it (EngineParameters, the bridge hand on CC2, the bend range on RPN 0
// and the Sustain switch),
// through the shared portfolio harness (Tests/ParameterMatrix.h is a
// byte-identical copy of the master; change it in reason-rack-extensions).
// Aculele has no plug-in yet, so the device stores each control normalised,
// as the Rack adapter does.
//
//   AculeleParameterReachTests --reach [--jobs N] [--case PARAMETER]
//
// The reach phrase's chords start at G3, under a re-entrant ukulele's C4, so
// the base patch is the Low G Tenor preset, whose fourth string plays G3.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"
#include "ParameterMatrix.h"

#include <array>
#include <cmath>
#include <cstring>
#include <memory>

namespace {

using namespace aculele;
namespace matrix = protocodus::matrix;

enum Control {
    kShape, kWood, kStrings, kTuning, kPicking, kCapture, kStringAge, kPluckPosition, kTouch, kBodyAmount,
    kStereoWidth, kOutput, kPiezoMix, kPiezoPolarity, kReleaseNoise, kRoom, kDetune, kBridgeHand, kBendRange, kSustain,
    kControlCount
};

// Name and steps (0 continuous), in the Rack adapter's names. Capture lists
// the eight distinct pickups; Piezo is LoadedPiezo's alias.
const std::array<matrix::Parameter, kControlCount> kControls {{
    { "Body shape", 3, 0.0f }, { "Body wood", 4, 0.0f }, { "Strings", 2, 0.0f }, { "Tuning", 3, 0.0f },
    { "Picking", 4, 0.0f }, { "Capture", 8, 0.0f }, { "String age", 0, 0.0f }, { "Pluck position", 0, 0.0f },
    { "Touch", 0, 0.0f }, { "Body amount", 0, 0.0f }, { "Stereo width", 0, 0.0f }, { "Output", 0, 0.0f },
    { "Piezo mix", 0, 0.0f }, { "Piezo polarity", 2, 0.0f }, { "Release noise", 0, 0.0f }, { "Room", 0, 0.0f },
    { "Detune", 0, 0.0f }, { "Bridge hand", 0, 0.0f }, { "Bend range", 12, 0.0f }, { "Sustain", 2, 0.0f },
}};

constexpr float Step(int index, int steps) { return static_cast<float>(index) / static_cast<float>(steps - 1); }
int Index(float normalised, int steps) { return static_cast<int>(std::lround(normalised * static_cast<float>(steps - 1))); }
// The Output control, -24..+6 dB, as the Rack adapter stores it.
float OutputPosition(float decibels) { return (decibels + 24.0f) / 30.0f; }
constexpr int kBasePreset = 3; // Low G Tenor

class AculeleDevice final : public matrix::Device {
public:
    AculeleDevice() : performer(std::make_unique<Performer>()) { Load(kBasePreset); }

    std::vector<matrix::Parameter> Parameters() const override {
        std::vector<matrix::Parameter> parameters(kControls.begin(), kControls.end());
        const Values base = PresetValues(kBasePreset);
        for (std::size_t i = 0; i < parameters.size(); ++i) parameters[i].defaultValue = base[i];
        return parameters;
    }
    int PresetCount() const override { return static_cast<int>(factoryPresets.size()); }
    void Prepare(double rate, int block) override {
        performer->prepare(rate, block);
        Apply();
    }
    void LoadPreset(int index) override {
        Load(index);
        Apply();
    }
    float GetParameter(std::size_t index) const override { return values[index]; }
    void SetParameter(std::size_t index, float value) override {
        values[index] = value;
        if (index == kBridgeHand || index == kBendRange) controllersPending = true;
        Apply();
    }
    void Process(float* left, float* right, int frames, const matrix::Event* events, int count) override {
        performer->beginBlock(left, right, frames);
        if (controllersPending) {
            // The bend range, 1..12 semitones, as RPN 0 followed by the null RPN.
            const auto semitones = static_cast<std::uint8_t>(Index(values[kBendRange], 12) + 1);
            const std::uint8_t controllers[][3] {
                { 0xB0, 2, static_cast<std::uint8_t>(std::lround(values[kBridgeHand] * 127.0f)) },
                { 0xB0, 101, 0 }, { 0xB0, 100, 0 }, { 0xB0, 6, semitones }, { 0xB0, 38, 0 },
                { 0xB0, 101, 127 }, { 0xB0, 100, 127 },
            };
            for (const auto& cc : controllers) performer->handleMidi(0, cc, 3);
            controllersPending = false;
        }
        for (int e = 0; e < count; ++e) performer->handleMidi(events[e].offset, events[e].bytes, events[e].size);
        performer->endBlock();
    }
    std::vector<std::uint8_t> SaveState() override {
        std::vector<std::uint8_t> state(sizeof values);
        std::memcpy(state.data(), values.data(), sizeof values);
        return state;
    }
    void RestoreState(const std::vector<std::uint8_t>& state) override {
        if (state.size() != sizeof values) return;
        std::memcpy(values.data(), state.data(), sizeof values);
        controllersPending = true;
        Apply();
    }

    matrix::ReachRule ReachFor(std::size_t index, float) const override {
        using matrix::Reach;
        switch (static_cast<int>(index)) {
        // AculelePerformer: a sounding string keeps the pluck it was given.
        case kPicking: case kPluckPosition: case kTouch:
            return { Reach::NewNotes, "the finger, place and contact of each new pluck" };
        // Each held contact keeps its draw (EngineParameters::detune).
        case kDetune: return { Reach::NewNotes, "each new contact's tuning draw" };
        // AculeleEngine::startReleaseNoise reads it at each key-up, which it
        // sounds 24-45 dB under the note's attack and in proportion to what
        // is left of the note: a ukulele's chord, more than Acustra's, has
        // decayed by the probe's key-up, so the touch stays below what the
        // probe window can hear even under Pick.
        case kReleaseNoise:
            return { Reach::Exempt, "a key-up touch 24-45 dB under the note's attack, read at each key-up" };
        // The held chord is not bent.
        case kBendRange: return { Reach::NewNotes, "the range of the next bend; the held chord is not bent" };
        // Only beneath a microphone Capture, and only with a piezo blended.
        case kPiezoPolarity: return { Reach::Held, "", { { kPiezoMix, 1.0f } } };
        default: return {};
        }
    }

private:
    using Values = std::array<float, kControlCount>;

    // A preset as the Rack adapter stores it; the controls a preset does not
    // set start where a fresh instance has them.
    static Values PresetValues(int index) {
        const FactoryPreset& p = factoryPresets[static_cast<std::size_t>(index)];
        Values values {};
        values[kShape] = Step(static_cast<int>(p.shape), 3);
        values[kWood] = Step(static_cast<int>(p.bodyMaterial), 4);
        values[kStrings] = Step(static_cast<int>(p.strings), 2);
        values[kTuning] = Step(static_cast<int>(p.tuning), 3);
        values[kPicking] = Step(static_cast<int>(p.picking), 4);
        values[kCapture] = Step(static_cast<int>(p.capture), 8);
        values[kStringAge] = p.stringAge;
        values[kPluckPosition] = p.pluckPosition;
        values[kTouch] = p.touch;
        values[kBodyAmount] = p.bodyAmount;
        values[kStereoWidth] = p.stereoWidth;
        values[kOutput] = OutputPosition(p.outputDecibels);
        values[kPiezoMix] = p.piezoMix;
        values[kReleaseNoise] = p.releaseNoise;
        values[kRoom] = p.room;
        values[kBendRange] = Step(1, 12); // 2 semitones
        return values;
    }
    void Load(int index) {
        values = PresetValues(index);
        controllersPending = true;
    }
    void Apply() {
        EngineParameters e;
        e.shape = static_cast<BodyShape>(Index(values[kShape], 3));
        e.bodyMaterial = static_cast<BodyMaterial>(Index(values[kWood], 4));
        e.strings = static_cast<StringMaterial>(Index(values[kStrings], 2));
        e.tuning = static_cast<Tuning>(Index(values[kTuning], 3));
        e.picking = static_cast<PickingTechnique>(Index(values[kPicking], 4));
        e.capture = static_cast<CaptureType>(Index(values[kCapture], 8));
        e.stringAge = values[kStringAge];
        e.pluckPosition = values[kPluckPosition];
        e.touch = values[kTouch];
        e.bodyAmount = values[kBodyAmount];
        e.stereoWidth = values[kStereoWidth];
        e.outputGain = std::pow(10.0f, (-24.0f + 30.0f * values[kOutput]) / 20.0f);
        e.piezoMix = values[kPiezoMix];
        e.piezoPolarityInverted = values[kPiezoPolarity] >= 0.5f;
        e.releaseNoise = values[kReleaseNoise];
        e.room = values[kRoom];
        e.detune = values[kDetune];
        performer->setParameters(e);
        performer->setSustainHold(values[kSustain] >= 0.5f);
    }

    std::unique_ptr<Performer> performer;
    Values values {};
    bool controllersPending = false;
};

} // namespace

int main(int argc, char** argv) {
    return protocodus::matrix::Main(argc, argv, [] { return std::make_unique<AculeleDevice>(); });
}
