// The 1-4.4 kHz plate band (AculeleEngine::buildBodyConfiguration) has the
// measured ukulele's density and no lone peak, on every Shape and Wood at
// 44.1, 48 and 96 kHz.
//
// Density: the band's sounding modes. Elie et al. 2011 counted 57 there; the
// clusters stop at a sixth of an octave either side of a bank mode, so where
// the bank's modes lie further apart the band keeps a gap, and the nine
// bodies have 37-47 (the bank alone placed 18-25). At least 35 must sound.
//
// Peaks: the band's own modes' power summed over the four microphone and
// input parts on a 5 Hz grid, averaged over a sixth of an octave either side
// of each point (a cluster's widest span), against its mean over the band.
// The bank alone rose 4.1-5.2 dB over it, the clusters 1.0-2.0 dB; no point
// may rise 3 dB, half the mean's power again, over it.

#include "DSP/AculeleEngine.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static const AculeleEngine::BodyConfiguration& body(const AculeleEngine& engine, int index)
    {
        return engine.bodyConfigurations_[static_cast<std::size_t>(index)];
    }
};
} // namespace aculele

namespace
{
using Access = aculele::AculeleEngineTestAccess;
constexpr double pi = 3.14159265358979323846;
constexpr double bandLowHz = 1000.0;
constexpr double bandHighHz = 4400.0;

int failures = 0;

void check(bool ok, const char* what, int shape, int wood, double rate, double value)
{
    if (!ok)
    {
        std::fprintf(stderr, "FAIL %s (shape %d, wood %d, %.0f Hz): %.2f\n",
                     what, shape, wood, rate, value);
        ++failures;
    }
}

template <typename Mode>
double frequencyOf(const Mode& mode, double rate)
{
    return std::atan2(mode.poleImaginary, mode.poleReal) * rate / (2.0 * pi);
}

template <typename Configuration>
double bandPower(const Configuration& configuration, double hz, double rate)
{
    const std::complex<double> inverseZ = std::polar(1.0, -2.0 * pi * hz / rate);
    std::complex<double> response[4] {};
    for (int index = 0; index < configuration.count; ++index)
    {
        const auto& mode = configuration.modes[static_cast<std::size_t>(index)];
        const double modeHz = frequencyOf(mode, rate);
        if (mode.poleImaginary == 0.0f || modeHz < bandLowHz || modeHz >= bandHighHz)
            continue;
        const std::complex<double> pole(mode.poleReal, mode.poleImaginary);
        const std::complex<double> direct = 1.0 / (1.0 - pole * inverseZ);
        const std::complex<double> mirror = 1.0 / (1.0 - std::conj(pole) * inverseZ);
        const float parts[8] { mode.leftReal, mode.leftImaginary, mode.rightReal,
            mode.rightImaginary, mode.leftMomentReal, mode.leftMomentImaginary,
            mode.rightMomentReal, mode.rightMomentImaginary };
        for (int part = 0; part < 4; ++part)
        {
            const std::complex<double> residue(parts[2 * part], parts[2 * part + 1]);
            response[part] += residue * direct + std::conj(residue) * mirror;
        }
    }
    double power = 0.0;
    for (const auto& part : response)
        power += std::norm(part);
    return power;
}

void run(double rate)
{
    auto engine = std::make_unique<aculele::AculeleEngine>();
    engine->prepare(rate, 256);
    const double sixthOctave = std::exp2(1.0 / 6.0);
    for (int shape = 0; shape < 3; ++shape)
        for (int wood = 0; wood < 3; ++wood)
        {
            const auto& configuration = Access::body(*engine, 3 * shape + wood);
            int sounding = 0;
            for (int index = 0; index < configuration.count; ++index)
            {
                const auto& mode = configuration.modes[static_cast<std::size_t>(index)];
                const double hz = frequencyOf(mode, rate);
                if (mode.poleImaginary != 0.0f && hz >= bandLowHz && hz < bandHighHz
                    && (mode.leftReal != 0.0f || mode.leftImaginary != 0.0f
                        || mode.rightReal != 0.0f || mode.rightImaginary != 0.0f))
                    ++sounding;
            }
            check(sounding >= 35, "at least 35 modes sound in the band", shape, wood, rate, sounding);

            std::vector<double> grid, power;
            for (double hz = bandLowHz / sixthOctave; hz < bandHighHz * sixthOctave; hz += 5.0)
            {
                grid.push_back(hz);
                power.push_back(bandPower(configuration, hz, rate));
            }
            double mean = 0.0;
            int inBand = 0;
            for (std::size_t point = 0; point < grid.size(); ++point)
                if (grid[point] >= bandLowHz && grid[point] < bandHighHz)
                {
                    mean += power[point];
                    ++inBand;
                }
            mean /= inBand;
            double highest = -1.0e9;
            for (std::size_t point = 0; point < grid.size(); ++point)
            {
                if (grid[point] < bandLowHz * sixthOctave || grid[point] >= bandHighHz / sixthOctave)
                    continue;
                double local = 0.0;
                int count = 0;
                for (std::size_t other = 0; other < grid.size(); ++other)
                    if (grid[other] >= grid[point] / sixthOctave && grid[other] < grid[point] * sixthOctave)
                    {
                        local += power[other];
                        ++count;
                    }
                highest = std::max(highest, 10.0 * std::log10(local / count / mean));
            }
            check(highest < 3.0, "no sixth-octave peak 3 dB over the band's mean",
                  shape, wood, rate, highest);
        }
}
} // namespace

int main()
{
    for (const double rate : { 44100.0, 48000.0, 96000.0 })
        run(rate);
    if (failures == 0)
        std::printf("PASS plate band density and peaks\n");
    return failures == 0 ? 0 : 1;
}
