// The release point belongs to the physical string, not to the shorter
// delay-line frame. Locate the kink from the written waveform's curvature,
// with the optional plectrum velocity and slip removed, then convert its
// sample distance back through the complete round trip. A stiff upper fret
// must not clamp a neckward contact at the physical hand band's limit twice.
//
// Independently, a held point force F releases two waves at speed F/(2Z).
// Check the slip time against that force/impedance law under MPE tension
// bends; impedance and wave-speed ratios differ when linear density changes.
#include "DSP/AculeleEngine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <limits>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    struct Release
    {
        double physicalPoint {}, period {};
        std::array<std::vector<float>, 2> waves;
    };

    static Release release(const AculeleEngine& engine, int string)
    {
        const auto& voice = engine.voices_[static_cast<std::size_t>(string)];
        Release result;
        result.physicalPoint = voice.pluckPoint;
        result.period = voice.contactPeriodSamples;
        for (std::size_t plane = 0; plane < result.waves.size(); ++plane)
        {
            const auto& loop = voice.loops[plane];
            const int length = std::max(8, static_cast<int>(std::round(loop.currentDelay)));
            const auto mask = static_cast<unsigned>(loop.delay.size()) - 1u;
            for (int sample = 0; sample < length; ++sample)
                result.waves[plane].push_back(loop.delay[
                    static_cast<unsigned>(loop.writeIndex - sample - 1) & mask]);
        }
        return result;
    }

    static void centrePositionDraw(AculeleEngine& engine)
    {
        // A deterministic valid RNG state whose first position draw is
        // effectively zero. This reaches the published bridgeward edge
        // without changing either physical position bounds or wave math.
        for (auto& voice : engine.voices_)
            voice.randomState = 0xf2b58529u;
    }

    static std::array<double, 2> releaseSpeed(const AculeleEngine& engine, int string)
    {
        const auto& voice = engine.voices_[static_cast<std::size_t>(string)];
        constexpr float height = 0.03f;
        constexpr float edgeRadius = 0.0002f;
        constexpr float concertScale = 0.3759f;
        const float length = voice.speakingLengthMetres;
        const float distance = 0.2f * length;
        const double pole = engine.plectrumSlipPole(voice, height, distance,
            length, concertScale, edgeRadius);
        const double observed = -std::log(pole) * edgeRadius * engine.sampleRate_;
        const double displacement = height * engine.physicalCalibration_.steelDisplacementScaleMetres;
        const double force = voice.tensionNewtons * displacement
            * (1.0 / distance + 1.0 / (length - distance));
        const double impedance = voice.characteristicImpedance * voice.bendImpedanceScale;
        return { observed, force / (2.0 * impedance) };
    }

    static bool constantPotentialIsSilent()
    {
        // Physical displacement is the difference of the two folded waves;
        // their common offset is arbitrary. A bridge boundary may not turn
        // that gauge into a kink, even with the legacy crown flag enabled.
        auto engine = std::make_unique<AculeleEngine>();
        engine->prepare(48000.0, 64);
        engine->setStringPerChannelMode(true);
        for (float common : { -4.0f, -0.5f, -0.1f, 0.0f, 0.5f, 4.0f })
        {
            engine->reset();
            engine->noteOn(72, 0.8f, 4);
            auto& voice = engine->voices_[3];
            for (auto& loop : voice.loops)
            {
                loop.delay.fill(common);
                loop.gestureContact = {};
            }
            voice.contactTravelEnabled = true;
            voice.tailActive = false;
            engine->finishVoice(voice, 3, common, common, 0, 0, 0, 0, 0, 0);
            for (const auto& loop : voice.loops)
                for (float sample : loop.delay)
                    if (sample != common) return false;
        }
        return true;
    }

    static bool fractionalDelayPreservesPotential()
    {
        int changed = 0;
        double worstAc = 0.0;
        for (float common : { -4.0f, -0.3f, -0.1f, 0.0f, 0.1f, 0.3f, 4.0f })
        {
            auto loop = std::make_unique<AculeleEngine::StringLoop>();
            loop->delay.fill(common);
            loop->allpassY1 = loop->allpassY2 = common;
            // Cross every integer anchor in both directions. H(1)=1 must
            // remain exact as the coefficients and line taps move together.
            for (int n = 0; n < 8192; ++n)
            {
                const int step = n < 4096 ? n : 8191 - n;
                changed += loop->readDelay(3.1f + 0.024f * step) != common;
            }
        }
        for (float common : { -4.0f, 4.0f })
        {
            auto plain = std::make_unique<AculeleEngine::StringLoop>();
            auto shifted = std::make_unique<AculeleEngine::StringLoop>();
            for (std::size_t i = 0; i < plain->delay.size(); ++i)
            {
                const float signal = static_cast<float>(0.01 * std::sin(0.31 * i)
                    + 0.003 * std::cos(0.73 * i));
                plain->delay[i] = signal;
                shifted->delay[i] = common + signal;
            }
            shifted->allpassY1 = shifted->allpassY2 = common;
            for (int n = 0; n < 8192; ++n)
            {
                const float delay = 3.1f + 0.024f * n;
                const double difference = shifted->readDelay(delay) - common
                    - plain->readDelay(delay);
                worstAc = std::max(worstAc, std::abs(difference));
            }
        }
        // The shifted float inputs themselves quantize at the common
        // offset's ULP. Allow that representation noise, not arbitrary gain
        // or phase changes of the small AC signal.
        const double budget = 16.0 * std::numeric_limits<float>::epsilon() * 4.0;
        std::printf("fractional-delay gauge: %d constant changes, AC offset error %.9g\n",
                    changed, worstAc);
        return changed == 0 && worstAc < budget;
    }

    static bool dispersionPreservesPotential()
    {
        int changed = 0;
        for (float common : { -4.0f, -0.3f, -0.1f, 0.0f, 0.1f, 0.3f, 4.0f })
        {
            // The ordinary and optional dispersion stages share this
            // recurrence. Sweep stable coefficients through both signs;
            // each section's constant gain is one at every coefficient.
            std::array<AculeleEngine::SecondOrderAllpass, 3> sections;
            for (auto& section : sections)
                section.x1 = section.x2 = section.y1 = section.y2 = common;
            for (int n = 0; n < 8192; ++n)
            {
                const float a2 = 0.25f + 0.7f * (0.5f + 0.5f * std::sin(0.003f * n));
                const float a1 = -(1.0f + a2) * 0.9f * std::cos(0.0021f * n);
                float value = common;
                for (auto& section : sections)
                {
                    value = section.process(value, a1, a2);
                    changed += value != common;
                }
            }
        }
        std::printf("dispersion gauge: %d constant changes\n", changed);
        return changed == 0;
    }

    static bool connectedLoopPreservesPotential()
    {
        int changed = 0;
        for (double rate : { 44100.0, 48000.0, 96000.0 })
            for (int fret : { 0, 18 })
            {
                auto engine = std::make_unique<AculeleEngine>();
                engine->prepare(rate, 64);
                engine->setStringPerChannelMode(true);
                engine->noteOn(69 + fret, 0.8f, 4);
                const auto configured = engine->voices_[3].loops[0];
                for (float common : { -4.0f, -0.1f, 0.3f })
                {
                    auto loop = std::make_unique<AculeleEngine::StringLoop>(configured);
                    loop->delay.fill(common);
                    loop->allpassY1 = loop->allpassY2 = common;
                    for (auto* section : { &loop->dispersion, &loop->secondDispersion,
                                           &loop->thirdDispersion })
                        section->x1 = section->x2 = section->y1 = section->y2 = common;
                    // Close the actual delay and dispersion helpers around
                    // a rigid, lossless boundary. This isolates the updated
                    // recurrences from scalar decay and the bridge, so an
                    // arbitrary constant is an exact equilibrium.
                    for (int n = 0; n < 4096; ++n)
                    {
                        const float delay = std::max(3.1f, configured.currentDelay
                            + 8.0f * std::sin(0.003f * n));
                        loop->setCurrentDelay(delay);
                        if (n == 1022)
                        {
                            // Retirement hands a pure two-sample section
                            // back after its two history samples have run.
                            loop->secondDispersionA1 = loop->secondDispersionA2 = 0.0f;
                            loop->thirdDispersionA1 = loop->thirdDispersionA2 = 0.0f;
                        }
                        if (n == 1024)
                        {
                            loop->switchThirdDispersion(false);
                            loop->switchSecondDispersion(false);
                        }
                        if (n == 2048)
                        {
                            loop->switchSecondDispersion(true);
                            loop->switchThirdDispersion(true);
                        }
                        float output = loop->readDelay(loop->currentDelay);
                        if (loop->thirdDispersionActive)
                            output = loop->thirdDispersion.process(output,
                                loop->thirdDispersionA1, loop->thirdDispersionA2);
                        if (loop->secondDispersionActive)
                            output = loop->secondDispersion.process(output,
                                loop->secondDispersionA1, loop->secondDispersionA2);
                        output = loop->dispersion.process(output,
                            loop->dispersionA1, loop->dispersionA2);
                        changed += output != common;
                        loop->write(output);
                    }
                }
            }
        std::printf("connected string-loop gauge: %d constant changes\n", changed);
        return changed == 0;
    }
};
}

namespace
{
using Engine = aculele::AculeleEngine;
using Access = aculele::AculeleEngineTestAccess;
int failures = 0;

void check(bool pass, const char* message, double observed, double expected)
{
    if (!pass)
    {
        ++failures;
        std::fprintf(stderr, "FAIL %s: %.9g versus %.9g\n", message, observed, expected);
    }
}

double kinkSample(const std::vector<float>& wave)
{
    const auto peak = std::max_element(wave.begin(), wave.end()) - wave.begin();
    double mass = 0.0, moment = 0.0;
    // The contact's Gaussian width is small here. Stay clear of the
    // endpoint's opposite curvature; the centroid of a symmetric smoothing
    // kernel preserves the original triangle's kink location.
    const int radius = std::max(3, static_cast<int>(wave.size() / 8));
    for (int i = std::max(1, static_cast<int>(peak) - radius);
         i <= std::min(static_cast<int>(wave.size()) - 2, static_cast<int>(peak) + radius); ++i)
    {
        const double curvature = std::max(0.0,
            2.0 * wave[static_cast<std::size_t>(i)]
                - wave[static_cast<std::size_t>(i - 1)]
                - wave[static_cast<std::size_t>(i + 1)]);
        mass += curvature;
        moment += i * curvature;
    }
    return mass > 0.0 ? moment / mass : -1.0;
}

void releasePointStaysOnString()
{
    int transformedPastHandBand = 0;
    int transformedBelowHandBand = 0;
    double worst = 0.0;
    for (double rate : { 44100.0, 48000.0, 96000.0 })
        for (int shape = 0; shape < 3; ++shape)
            for (float position : { 0.0f, 0.35f, 1.0f })
            {
                auto engine = std::make_unique<Engine>();
                auto calibration = aculele::fittedPhysicalCalibration;
                calibration.pickReleaseVelocityShare = 0.0f;
                calibration.pickEdgeRadiusMetres = 0.0f;
                calibration.steel.apertureScale = 0.35f;
                calibration.steel.pluckDistanceScale = position == 0.0f ? 0.7f : 3.0f;
                // No tension transient after writing, so the diagnostic
                // reads precisely the period used for this initial shape.
                calibration.steelDisplacementScaleMetres = 0.0f;
                engine->setPhysicalCalibration(calibration);
                auto realism = engine->performanceRealism();
                realism.frettingPitch = false;
                engine->setPerformanceRealism(realism);
                aculele::EngineParameters parameters;
                parameters.shape = static_cast<aculele::BodyShape>(shape);
                parameters.picking = aculele::PickingTechnique::Pick;
                parameters.pluckPosition = position;
                engine->setParameters(parameters);
                engine->prepare(rate, 64);
                engine->setStringPerChannelMode(true);
                constexpr int open[] { 67, 60, 64, 69 };
                for (int string = 0; string < 4; ++string)
                    for (int fret : { 0, 9, 18 })
                    {
                        // Near the bridge use an open string: its endpoint
                        // and kink are separated by several kernel widths,
                        // so their opposite curvatures can be observed alone.
                        if (position == 0.0f && fret != 0) continue;
                        engine->reset();
                        if (position == 0.0f) Access::centrePositionDraw(*engine);
                        engine->noteOn(open[string] + fret, 0.85f, string + 1);
                        const auto release = Access::release(*engine, string);
                        for (std::size_t plane = 0; plane < release.waves.size(); ++plane)
                        {
                            const double expected = release.physicalPoint
                                + (plane == 0 ? -0.006 : 0.009);
                            const double observed = kinkSample(release.waves[plane]) / release.period;
                            const double mapped = expected * release.period / release.waves[plane].size();
                            transformedPastHandBand += mapped > 0.46 ? 1 : 0;
                            transformedBelowHandBand += mapped < 0.05 ? 1 : 0;
                            worst = std::max(worst, std::abs(observed - expected));
                            check(std::abs(observed - expected) < 0.0005,
                                  "released kink stays at its physical contact", observed, expected);
                        }
                    }
            }
    check(transformedPastHandBand > 30, "geometry cases exercise the old clamp",
          transformedPastHandBand, 30);
    check(transformedBelowHandBand > 5, "geometry cases exercise the old lower clamp",
          transformedBelowHandBand, 5);
    std::printf("release location: worst %.6g of string, %d above and %d below old frame clamps\n",
                worst, transformedPastHandBand, transformedBelowHandBand);
}

void releaseSpeedIsForceOverTwoImpedances()
{
    double worst = 0.0;
    for (double rate : { 44100.0, 48000.0, 96000.0 })
        for (int material = 0; material < 2; ++material)
        {
            auto engine = std::make_unique<Engine>();
            aculele::EngineParameters parameters;
            parameters.strings = static_cast<aculele::StringMaterial>(material);
            engine->setParameters(parameters);
            engine->prepare(rate, 64);
            engine->setLowerZoneMemberCount(15);
            for (float bend : { -2.0f, 0.0f, 2.0f, 6.0f, 12.0f })
            {
                engine->reset();
                engine->setPitchBend(bend, 2);
                engine->noteOn(74, 0.8f, 2);
                const int string = engine->heldString(74, 2);
                check(string >= 0, "MPE note has a physical string", string, 0);
                if (string < 0) continue;
                const auto speeds = Access::releaseSpeed(*engine, string);
                const double relative = std::abs(speeds[0] / speeds[1] - 1.0);
                worst = std::max(worst, relative);
                check(std::isfinite(relative) && relative < 0.000005,
                      "contact speed is F/(2Z) under a tension bend", speeds[0], speeds[1]);
            }
        }
    std::printf("contact-speed force/impedance relative error: %.9g\n", worst);
}
}

int main()
{
    releasePointStaysOnString();
    releaseSpeedIsForceOverTwoImpedances();
    check(Access::constantPotentialIsSilent(),
          "a common travelling-wave potential cannot create string motion", 0, 0);
    check(Access::fractionalDelayPreservesPotential(),
          "fractional retuning preserves the potential reference", 0, 0);
    check(Access::dispersionPreservesPotential(),
          "dispersion retuning preserves the potential reference", 0, 0);
    check(Access::connectedLoopPreservesPotential(),
          "a connected string loop preserves its constant reference", 0, 0);
    return failures == 0 ? 0 : 1;
}
