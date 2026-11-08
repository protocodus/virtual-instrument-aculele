// Mechanical port checks, not an acoustic-energy or complete string-storage
// ledger. The waveguide uses displacement divided by its calibrated metre
// scale d; differencing over 1/48000 s makes these power readings physical
// watts divided by (48000*d)^2. Both heave and normalized rocking contribute.
// A fixed, zero-state passive body must not return more cumulative work than
// entered it. Acoustic output gain, room, microphone EQ and peak limits are
// deliberately outside this check. Parameter-change work is not assumed zero.
#include "DSP/AculeleEngine.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static std::array<float, 6> portWaves(const AculeleEngine& engine)
    {
        const auto& port = engine.bridgeLoad_;
        return { port.displacement, port.rotation, port.mainIntegratedForce,
                 port.mainIntegratedMoment, port.bodyIntegratedForce,
                 port.bodyIntegratedMoment };
    }
};
}

namespace
{
using Engine = aculele::AculeleEngine;
using Access = aculele::AculeleEngineTestAccess;
int failures = 0;

void expect(bool pass, const char* message, double rate, int shape = -1)
{
    if (!pass)
    {
        ++failures;
        std::fprintf(stderr, "FAIL %s at %.0f Hz shape %d\n", message, rate, shape);
    }
}

void passiveBody(double rate, int shape)
{
    auto engine = std::make_unique<Engine>();
    aculele::EngineParameters parameters;
    parameters.shape = static_cast<aculele::BodyShape>(shape);
    parameters.touch = 0.72f;
    engine->setParameters(parameters);
    engine->prepare(rate, 1);
    engine->setStringPerChannelMode(true);
    const int notes[] { 67, 60, 64, 69 };
    for (int string = 0; string < 4; ++string)
        engine->noteOn(notes[string], 0.82f, string + 1);
    double total = 0.0, body = 0.0, minimum = 0.0, magnitude = 0.0;
    double forceError = 0.0, forcePeak = 0.0;
    for (int sample = 0; sample < static_cast<int>(0.5 * rate); ++sample)
    {
        float left {}, right {};
        engine->process(&left, &right, 1);
        const double totalPower = engine->getLastBridgePower();
        const double bodyPower = engine->getLastBridgeBodyPower();
        total += totalPower / rate;
        body += bodyPower / rate;
        magnitude += (std::abs(totalPower) + std::abs(bodyPower)) / rate;
        minimum = std::min({ minimum, total, body });
        const double reaction = engine->getLastBridgeReactionForce();
        forcePeak = std::max(forcePeak, std::abs(reaction));
        forceError = std::max(forceError,
            std::abs(reaction - engine->getLastBridgeBodyForce()));
    }
    // Absolute rounding allowance is far below the measured input work;
    // require nonzero work so an accidentally disconnected observer fails.
    expect(magnitude > 1.0e-14, "nonzero work reaches the bridge", rate, shape);
    expect(minimum >= -1.0e-14, "body does not generate cumulative work", rate, shape);
    expect(forceError <= forcePeak * 1.0e-4 + 1.0e-10,
           "all transverse saddle force reaches the body", rate, shape);
    std::printf("bridge %.0f Hz shape %d work %.9g minimum %.9g\n",
                rate, shape, body, minimum);
}

void independentWorkObserver(double rate)
{
    auto engine = std::make_unique<Engine>();
    engine->prepare(rate, 1);
    std::vector<std::array<float, 6>> history;
    history.reserve(static_cast<std::size_t>(rate * 0.06));
    bool agrees = true;
    for (int sample = 0; sample < static_cast<int>(rate * 0.06); ++sample)
    {
        if (sample == 0) engine->noteOn(60, 0.72f);
        if (sample == static_cast<int>(0.020 * rate)) engine->noteOn(67, 1.0f);
        if (sample == static_cast<int>(0.030 * rate)) engine->noteOff(60);
        if (sample == static_cast<int>(0.035 * rate)) engine->noteOn(67, 0.95f);
        float left {}, right {};
        engine->process(&left, &right, 1);
        history.push_back(Access::portWaves(*engine));
        const double past = sample - rate / 48000.0;
        const int before = static_cast<int>(std::floor(past));
        const double fraction = past - before;
        std::array<double, 6> change {}, errorBound {};
        for (std::size_t coordinate = 0; coordinate < change.size(); ++coordinate)
        {
            const auto at = [&] (int index)
            { return index < 0 ? 0.0 : static_cast<double>(history[static_cast<std::size_t>(index)][coordinate]); };
            change[coordinate] = history.back()[coordinate]
                - ((1.0 - fraction) * at(before) + fraction * at(before + 1));
            errorBound[coordinate] = 8.0 * std::numeric_limits<float>::epsilon()
                * (std::abs(history.back()[coordinate]) + std::abs(at(before)) + std::abs(at(before + 1)));
        }
        const double observed[] { engine->getLastBridgePower(), engine->getLastBridgeBodyPower() };
        for (std::size_t branch = 0; branch < 2; ++branch)
        {
            const std::size_t normal = 2 + 2 * branch, moment = normal + 1;
            const double heave = change[0] * change[normal];
            const double rock = change[1] * change[moment];
            const auto productBound = [&] (std::size_t a, std::size_t b)
            { return errorBound[a] * std::abs(change[b]) + errorBound[b] * std::abs(change[a])
                     + errorBound[a] * errorBound[b]; };
            const double bound = productBound(0, normal) + productBound(1, moment)
                + 4.0 * std::numeric_limits<float>::epsilon() * (std::abs(heave) + std::abs(rock)) + 1.0e-22;
            agrees = agrees && std::abs(observed[branch] - heave - rock) <= bound;
        }
    }
    expect(agrees, "independent work includes initial and repeated contacts", rate);
}
}

int main()
{
    for (const double rate : { 44100.0, 48000.0, 96000.0 })
    {
        for (int shape = 0; shape < 3; ++shape)
            passiveBody(rate, shape);
        independentWorkObserver(rate);
    }
    if (failures == 0) std::puts("PASS mechanical bridge work and force routing");
    return failures == 0 ? 0 : 1;
}
