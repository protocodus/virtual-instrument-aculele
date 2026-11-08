// The radiation shelves must remain stable across the supported low-rate
// boundary. The original 5656.9 Hz shelf put poles outside the unit circle
// at 8 and 11.025 kHz. Its hidden microphone/room NaNs then muted even pure
// Piezo Main through 0 * NaN, while the separate pickup still sounded.
// Check actual configured coefficients against transfer-function invariants,
// then drive the complete Performer and inspect histories before sanitisation.
#include "DSP/AculeleEngine.h"
#include "DSP/AculelePerformer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace aculele
{
struct AculeleEngineTestAccess
{
    using CaptureFilter = AculeleEngine::BodyBank::CaptureFilter;

    static CaptureFilter capture(const AculeleEngine& engine)
    {
        return engine.bodyBank_.captureFilter;
    }

    static std::array<float, 2> render(CaptureFilter& filter, float left, float right)
    {
        AculeleEngine::BodyOutput output { left, right, 0.0f };
        filter.render(output);
        return { output.left, output.right };
    }

    template <typename T>
    static bool finite(T value) { return std::isfinite(value); }

    template <typename T, std::size_t N>
    static bool finite(const std::array<T, N>& values)
    {
        for (const auto& value : values)
            if (!finite(value))
                return false;
        return true;
    }

    static bool captureFinite(const CaptureFilter& filter)
    {
        return finite(filter.stateLeft) && finite(filter.stateRight);
    }

    static bool captureFinite(const AculeleEngine& engine)
    {
        return captureFinite(engine.bodyBank_.captureFilter)
            && captureFinite(engine.fadingBodyBank_.captureFilter)
            && finite(engine.bodyBank_.real) && finite(engine.bodyBank_.imaginary)
            && finite(engine.bodyBank_.momentReal) && finite(engine.bodyBank_.momentImaginary)
            && finite(engine.fadingBodyBank_.real) && finite(engine.fadingBodyBank_.imaginary)
            && finite(engine.fadingBodyBank_.momentReal)
            && finite(engine.fadingBodyBank_.momentImaginary)
            && finite(engine.micDelayLeft_) && finite(engine.micDelayRight_)
            && finite(engine.micDelayMono_);
    }

    static bool roomFinite(const AculeleEngine& engine)
    {
        const auto& room = engine.room_;
        return finite(room.lines) && finite(room.absorptionState)
            && finite(room.early) && finite(room.diffusers)
            && finite(room.converterInput) && finite(room.converterLeft)
            && finite(room.converterRight) && finite(room.sideHighpassState)
            && finite(room.inputState) && finite(room.earlyLeft) && finite(room.earlyRight)
            && finite(room.accumulator) && finite(room.previousLeft)
            && finite(room.previousRight) && finite(room.currentLeft)
            && finite(room.currentRight);
    }

    static bool roomExcited(const AculeleEngine& engine)
    {
        const auto& room = engine.room_;
        return room.active && (room.inputState != 0.0f || room.currentLeft != 0.0f
                               || room.currentRight != 0.0f);
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
using Coefficients = std::array<double, 5>;
constexpr double pi = 3.141592653589793238462643383279502884;
// Independently specified public voicing: all three shelves take the same
// Shape cut. Float values reflect the authored source data's precision.
constexpr std::array<float, 3> shelfHz { 1414.2f, 2828.4f, 5656.9f };
constexpr std::array<float, 3> shapeDb { -2.88f, -1.36f, -1.16f };

void expect(bool condition, const char* message, double rate, int shape,
            int section = -1, double value = 0.0)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAIL: %s (rate %.0f, shape %d, section/block %d): %.12g\n",
                     message, rate, shape, section, value);
        std::exit(1);
    }
}

std::complex<double> response(const Coefficients& c, double cyclesPerSample)
{
    const auto z = std::polar(1.0, -2.0 * pi * cyclesPerSample);
    return (c[0] + c[1] * z + c[2] * z * z) / (1.0 + c[3] * z + c[4] * z * z);
}

double poleRadius(const Coefficients& c)
{
    // Roots of z^2 + a1*z + a2, independent of the RBJ coefficient design.
    const auto discriminant = std::sqrt(std::complex<double>(c[3] * c[3] - 4.0 * c[4], 0.0));
    return std::max(std::abs((-c[3] + discriminant) / 2.0),
                    std::abs((-c[3] - discriminant) / 2.0));
}

void checkImpulse(Access::CaptureFilter filter, double rate, int shape)
{
    filter.reset();
    double energy = 0.0;
    std::array<float, 2> last {};
    // At the highest tested rate the slowest shelf pole is below 0.95;
    // 2048 samples exceed 100 decay time constants, without a long render.
    for (int sample = 0; sample < 2048; ++sample)
    {
        last = Access::render(filter, sample == 0 ? 1.0f : 0.0f,
                             sample == 0 ? -0.25f : 0.0f);
        expect(Access::finite(last) && Access::captureFinite(filter),
               "capture impulse produced nonfinite output/history", rate, shape, sample);
        // The state floor independently clears each channel near 1e-30.
        expect(std::abs(last[1] + 0.25f * last[0]) < 1.0e-25f,
               "capture channels do not share the same response",
               rate, shape, sample);
        const double normalized = last[0] / static_cast<double>(filter.gain);
        energy += normalized * normalized;
    }
    // Parseval: a cascade with |H| <= 1 cannot give a unit impulse more
    // than unit energy. Allow only float output rounding, not a fitted limit.
    expect(energy > 0.01 && energy <= 1.0 + 2.0e-6,
           "capture impulse is silent or adds energy", rate, shape, -1, energy);
    expect(std::abs(last[0]) < 1.0e-20f && std::abs(last[1]) < 1.0e-20f,
           "capture impulse did not decay", rate, shape, -1, last[0]);
    filter.reset();
    expect(Access::render(filter, 0.0f, 0.0f) == std::array<float, 2> {},
           "capture reset retained impulse output", rate, shape);
}

void checkCoefficients()
{
    double largestRadius = 0.0;
    for (double rate : { 8000.0, 11025.0, 12000.0, 16000.0, 44100.0, 48000.0, 96000.0 })
    {
        auto engine = std::make_unique<aculele::AculeleEngine>();
        engine->prepare(rate, 64);
        for (int shape = 0; shape < 3; ++shape)
        {
            aculele::EngineParameters parameters;
            parameters.shape = static_cast<aculele::BodyShape>(shape);
            engine->setParameters(parameters);
            engine->settleSuspendedParameters();
            const auto filter = Access::capture(*engine);
            expect(filter.enabled && filter.activeCount == 3,
                   "actual body does not run the three radiation shelves", rate, shape);
            expect(std::isfinite(filter.gain) && filter.gain > 0.0f,
                   "invalid capture gain", rate, shape);
            for (std::size_t section = 0; section < filter.coefficients.size(); ++section)
            {
                const auto& c = filter.coefficients[section];
                const auto index = static_cast<int>(section);
                expect(Access::finite(c), "nonfinite capture coefficients", rate, shape, index);
                // All three strict second-order Jury conditions are needed;
                // finite frequency response alone cannot detect an unstable shelf.
                expect(1.0 + c[3] + c[4] > 0.0 && 1.0 - c[3] + c[4] > 0.0
                           && 1.0 - c[4] > 0.0,
                       "capture section fails Jury stability", rate, shape, index);
                const double radius = poleRadius(c);
                expect(radius < 1.0, "capture pole is outside the unit circle",
                       rate, shape, index, radius);
                largestRadius = std::max(largestRadius, radius);
                if (section >= shelfHz.size())
                {
                    expect(c == Coefficients { 1.0, 0.0, 0.0, 0.0, 0.0 },
                           "inactive capture section is not identity", rate, shape, index);
                    continue;
                }
                expect(filter.active[section] == section,
                       "radiation shelf is omitted/reordered", rate, shape, index);
                const double gain = std::pow(10.0, static_cast<double>(shapeDb[shape]) / 20.0);
                const double dc = std::abs(response(c, 0.0));
                const double nyquist = std::abs(response(c, 0.5));
                expect(std::abs(dc - 1.0) < 1.0e-10, "shelf changed DC gain", rate, shape, index, dc);
                expect(std::abs(nyquist - gain) < 1.0e-10,
                       "shelf changed Nyquist gain", rate, shape, index, nyquist);
                // A shelf's half-dB point remains at its authored frequency
                // above the low-rate boundary. Below it, the accepted bounded
                // approximation puts that point at 90% of Nyquist.
                const double cornerHz = std::min(static_cast<double>(shelfHz[section]), 0.45 * rate);
                const double cornerDb = 20.0 * std::log10(std::abs(response(c, cornerHz / rate)));
                expect(std::abs(cornerDb - 0.5 * shapeDb[shape]) < 1.0e-8,
                       "shelf half-dB corner moved", rate, shape, index, cornerDb);
                double previous = dc;
                for (int point = 1; point <= 128; ++point)
                {
                    const double magnitude = std::abs(response(c, 0.5 * point / 128.0));
                    expect(magnitude >= gain - 1.0e-10 && magnitude <= 1.0 + 1.0e-10
                               && magnitude <= previous + 1.0e-10,
                           "cut shelf boosts or ceases to be monotonic", rate, shape, index, magnitude);
                    previous = magnitude;
                }
            }
            checkImpulse(filter, rate, shape);
        }
    }
    std::printf("PASS capture coefficients/impulses: 3 shapes, 7 rates; maximum pole radius %.9f\n",
                largestRadius);
}

void checkPerformer()
{
    constexpr double rate = 8000.0;
    constexpr int blockSize = 64;
    constexpr int blocksPerStrike = 16;
    constexpr int strikes = 16;
    // Standard re-entrant ukulele tuning, explicitly assigned to its four
    // physical strings by the performer's normal CC126 channel mode.
    constexpr std::array<int, 4> notes { 67, 60, 64, 69 };
    double quietestWindow = 1.0;
    for (int shape = 0; shape < 3; ++shape)
    {
        auto performer = std::make_unique<aculele::Performer>();
        aculele::EngineParameters parameters;
        parameters.shape = static_cast<aculele::BodyShape>(shape);
        parameters.capture = aculele::CaptureType::Piezo;
        parameters.picking = aculele::PickingTechnique::Pick;
        parameters.touch = 1.0f;
        parameters.bodyAmount = 1.0f;
        parameters.outputGain = 1.0f;
        parameters.room = parameters.piezoMix = parameters.stereoWidth = 1.0f;
        performer->setParameters(parameters);
        performer->prepare(rate, blockSize);
        performer->setSustainHold(true);
        std::array<float, blockSize> left {}, right {}, direct {};
        double windowEnergy = 0.0;
        int windowNonzero = 0;
        bool heardRoom = false;
        for (int block = 0; block < blocksPerStrike * strikes; ++block)
        {
            performer->beginBlock(left.data(), right.data(), { direct.data() }, blockSize);
            if (block == 0)
                performer->controlChange(0, 1, 126, 4);
            if (block % blocksPerStrike == 0)
                for (std::size_t string = 0; string < notes.size(); ++string)
                    performer->noteOn(0, static_cast<int>(string) + 1, notes[string], 127);
            if (block % blocksPerStrike == blocksPerStrike - 1)
                for (std::size_t string = 0; string < notes.size(); ++string)
                    performer->noteOff(48, static_cast<int>(string) + 1, notes[string], 127);
            performer->endBlock();
            // Do not trust final output finiteness: its sanitizer can replace
            // an internal NaN with zero, precisely how this bug hid itself.
            expect(Access::captureFinite(performer->engine()),
                   "nonfinite body/capture/microphone history", rate, shape, block);
            expect(Access::roomFinite(performer->engine()),
                   "nonfinite internal room history", rate, shape, block);
            heardRoom = heardRoom || Access::roomExcited(performer->engine());
            for (std::size_t sample = 0; sample < left.size(); ++sample)
            {
                expect(std::isfinite(left[sample]) && std::isfinite(right[sample])
                           && std::isfinite(direct[sample]),
                       "nonfinite rendered output", rate, shape, block);
                expect(left[sample] == direct[sample] && right[sample] == direct[sample],
                       "pure Piezo Main differs from the direct bus", rate, shape, block);
                expect(std::abs(direct[sample]) <= 1.0f,
                       "rendered output exceeded full scale", rate, shape, block, direct[sample]);
                windowEnergy += static_cast<double>(direct[sample]) * direct[sample];
                windowNonzero += direct[sample] != 0.0f ? 1 : 0;
            }
            if (block % blocksPerStrike == blocksPerStrike - 1)
            {
                const double rms = std::sqrt(windowEnergy / (blockSize * blocksPerStrike));
                expect(rms > 1.0e-6 && windowNonzero > blockSize * blocksPerStrike / 2,
                       "a retrigger window lost sustained pickup/Main audio", rate, shape, block, rms);
                quietestWindow = std::min(quietestWindow, rms);
                windowEnergy = 0.0;
                windowNonzero = 0;
            }
        }
        expect(heardRoom, "room was never driven by the fixture", rate, shape);
        expect(performer->droppedEventCount() == 0,
               "hard-retrigger fixture dropped MIDI events", rate, shape);
    }
    std::printf("PASS 8 kHz hard retriggers: finite internal capture/room, exact sustained Main/Piezo parity; minimum window RMS %.9g\n",
                quietestWindow);
}
} // namespace

int main(int argc, char** argv)
{
    // Lets an old-code mutation demonstrate the audible path independently
    // after the coefficient test has already rejected its unstable poles.
    if (argc == 2 && std::strcmp(argv[1], "--render-only") == 0)
        checkPerformer();
    else if (argc == 2 && std::strcmp(argv[1], "--filter-only") == 0)
        checkCoefficients();
    else
    {
        checkCoefficients();
        checkPerformer();
    }
    return 0;
}
