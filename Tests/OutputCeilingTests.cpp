// Output-only low-rate headroom. An ordinary independently prepared shared
// stage is the reference for quiet gain and the existing compressor transfer;
// loud inputs separately exercise the lowered effective limiter ceiling.
#include "DSP/AculeleEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace aculele {
struct AculeleEngineTestAccess {
    static void main(AculeleEngine& engine, std::array<float, 2>& frame) {
        engine.processOutputStage(engine.mainOutput_, frame);
    }
    static void piezo(AculeleEngine& engine, std::array<float, 1>& frame) {
        engine.processOutputStage(engine.piezoOutput_, frame);
    }
    static void omitThresholdShift(AculeleEngine& engine, double rate) {
        engine.mainOutput_.Prepare(rate, protocodus::kVuGuard);
    }
};
}
namespace {
using Access = aculele::AculeleEngineTestAccess;
constexpr double pi = 3.14159265358979323846;
void expect(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
std::unique_ptr<aculele::AculeleEngine> prepared(double rate) {
    auto result = std::make_unique<aculele::AculeleEngine>();
    result->prepare(rate, 64);
    return result;
}
void compressorTransfer() {
    double largestDifference = 0.0, smallestNegativeControl = 1.0;
    for (double rate : { 8000.0, 16000.0, 22050.0, 32000.0, 44100.0, 48000.0, 96000.0 }) {
        auto candidate = prepared(rate);
        auto wrong = prepared(rate);
        Access::omitThresholdShift(*wrong, rate);
        protocodus::OutputStage<2> reference;
        reference.Prepare(rate, protocodus::kVuGuard);
        double difference = 0.0, missingShiftDifference = 0.0;
        for (int i = 0; i < static_cast<int>(2.0 * rate); ++i) {
            const double time = static_cast<double>(i) / rate;
            // Active soft-knee compression, but neither effective limiter
            // reaches these low-crest signals. The amplitude also varies so
            // the equivalence includes the compressor's state trajectory.
            const double amplitude = .14 + .11 * (.5 + .5 * std::sin(2.0 * pi * 1.4 * time));
            const std::array<float, 2> input {
                static_cast<float>(amplitude * std::sin(2.0 * pi * 173.0 * time)),
                static_cast<float>(.8 * amplitude * std::cos(2.0 * pi * 231.0 * time)) };
            auto expected = input, actual = input, unshifted = input;
            reference.Process(expected);
            Access::main(*candidate, actual);
            Access::main(*wrong, unshifted);
            for (std::size_t channel = 0; channel < input.size(); ++channel) {
                difference = std::max(difference, std::abs(static_cast<double>(actual[channel]) - expected[channel]));
                missingShiftDifference = std::max(missingShiftDifference,
                    std::abs(static_cast<double>(unshifted[channel]) - expected[channel]));
                if (rate >= 44100.0)
                    expect(actual[channel] == expected[channel], "ordinary-rate path is not exact");
            }
        }
        expect(difference < 2.0e-5, "reserve changes existing compressor transfer or quiet gain");
        if (rate < 44100.0) {
            expect(missingShiftDifference > .001, "fixture would not catch an omitted threshold shift");
            smallestNegativeControl = std::min(smallestNegativeControl, missingShiftDifference);
        }
        largestDifference = std::max(largestDifference, difference);
    }
    std::printf("PASS compressor transfer: maximum absolute difference %.9g; missing-shift negative control >= %.9g\n",
        largestDifference, smallestNegativeControl);
}
void ceilingParityAndReset() {
    for (double rate : { 8000.0, 16000.0, 22050.0, 32000.0, 44100.0, 48000.0, 96000.0 }) {
        auto engine = prepared(rate);
        const double expectedCeiling = std::pow(10.0, (rate < 44100.0 ? -4.8 : -1.5) / 20.0);
        double peak = 0.0;
        for (int i = 0; i < static_cast<int>(rate); ++i) {
            const float x = static_cast<float>((i % 91 < 63 ? 3.0 : .04)
                * std::sin(2.0 * pi * .113 * i));
            std::array<float, 2> main { x, x };
            std::array<float, 1> piezo { x };
            Access::main(*engine, main); Access::piezo(*engine, piezo);
            expect(main[0] == main[1] && main[0] == piezo[0], "Main/Piezo output-stage parity changed");
            expect(std::isfinite(main[0]), "nonfinite guarded output");
            peak = std::max(peak, static_cast<double>(std::abs(main[0])));
        }
        expect(peak <= expectedCeiling + 2.0e-6, "effective sample ceiling exceeded");
        engine->reset();
        for (int i = 0; i < 2 * protocodus::OutputStage<2>::DelayFor(rate) + 128; ++i) {
            std::array<float, 2> main {}; std::array<float, 1> piezo {};
            Access::main(*engine, main); Access::piezo(*engine, piezo);
            expect(main[0] == 0.0f && main[1] == 0.0f && piezo[0] == 0.0f, "reset left output history");
        }
        engine->reset();
        const int delay = protocodus::OutputStage<2>::DelayFor(rate);
        for (int i = 0; i <= delay + 4; ++i) {
            std::array<float, 2> main { i == 0 ? .001f : 0.0f, 0.0f };
            Access::main(*engine, main);
            expect(i == delay ? std::abs(main[0] - .001f) < 2e-10f : main[0] == 0.0f,
                "reserve added latency or changed quiet impulse gain");
        }
    }
    std::puts("PASS effective ceilings, exact Main/Piezo parity, reset, and unchanged impulse latency");
}
}
int main() { compressorTransfer(); ceilingParityAndReset(); }
