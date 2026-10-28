// Prints how each Shape and Wood's radiation falls above 1 kHz, beside the
// one published ukulele measurement:
//
//   AculeleRadiation [--check]
//
// Labrana, Gajardo, Correa and Espinoza published the sound pressure per
// newton at the bridge of a soprano ukulele (its scan mesh's body is 162 by
// 235 mm) with an Engelmann spruce top on a printed PLA+ body: data for
// "Vibroacoustic reproducibility of 3D-printed PLA+ and wood soundboards for
// ukuleles", Zenodo, doi:10.5281/zenodo.22692775 (2026), CC BY 4.0,
// mean_rad_hybrid_XZ.txt and mean_rad_hybrid_YZ.txt. Each holds the
// magnitude in Pa/N at 1 Hz steps from 1 Hz to 20 kHz at nine angles, 0 to
// 180 degrees in 22.5 degree steps. Their power mean over all eighteen
// positions, averaged within each third octave from 1 to 6.3 kHz (the band
// at f_c from f_c 2^-1/6, inclusive, to f_c 2^1/6) and read in dB, falls
// 3.204 dB per octave: the least-squares slope against log2(f_c / 1 kHz).
// The three printed tops measured the same way fall 3.5-3.8. Above 8 kHz
// the measured levels rise again, so they are not used.
//
// The model's figure is taken the same way from the body bank's impulse
// response (bridge force to microphone pressure, the capture filter
// included, 16384 samples at 48 kHz), the power mean of its two channels
// averaged over the FFT bins in each band.
//
// --check is Aculele.Radiation: every spruce body must fall 3.20 dB per
// octave within 0.15, the measured slope radiationTiltShelfDb pins in
// Source/DSP/AculeleEngine.cpp.

#include "DSP/AculeleEngine.h"

#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static std::vector<std::array<float, 2>> impulseResponse(int shape, int wood, int length)
    {
        auto engine = std::make_unique<AculeleEngine>();
        engine->prepare(48000.0, 256);
        EngineParameters parameters;
        parameters.shape = static_cast<BodyShape>(shape);
        parameters.bodyMaterial = static_cast<BodyMaterial>(wood);
        engine->setParameters(parameters);
        engine->settleSuspendedParameters();
        engine->configureBody();
        engine->bodyBank_.reset();
        std::vector<std::array<float, 2>> response;
        for (int i = 0; i < length; ++i)
        {
            const auto out = engine->bodyBank_.render(i == 0 ? 1.0f : 0.0f, 0.0f);
            response.push_back({ out.left, out.right });
        }
        return response;
    }
};
} // namespace aculele

namespace
{
constexpr int bands = 9; // 1 to 6.3 kHz
constexpr double measuredSlope = -3.204;

double centre(int band)
{
    return 1000.0 * std::pow(2.0, band / 3.0);
}

// Each band's level in dB and the least-squares slope through them.
std::array<double, bands + 1> radiation(int shape, int wood)
{
    constexpr int length = 16384;
    constexpr double rate = 48000.0;
    constexpr double twoPi = 6.283185307179586;
    const auto response = aculele::AculeleEngineTestAccess::impulseResponse(shape, wood, length);
    std::array<double, bands + 1> result {};
    for (int band = 0; band < bands; ++band)
    {
        const double low = centre(band) * std::pow(2.0, -1.0 / 6.0);
        const double high = centre(band) * std::pow(2.0, 1.0 / 6.0);
        double power = 0.0;
        int count = 0;
        for (int bin = static_cast<int>(std::ceil(low * length / rate)); bin * rate / length < high; ++bin)
        {
            const std::complex<double> step = std::polar(1.0, -twoPi * bin / length);
            std::complex<double> phasor(1.0, 0.0), left, right;
            for (const auto& frame : response)
            {
                left += static_cast<double>(frame[0]) * phasor;
                right += static_cast<double>(frame[1]) * phasor;
                phasor *= step;
            }
            power += 0.5 * (std::norm(left) + std::norm(right));
            ++count;
        }
        result[static_cast<std::size_t>(band)] = 10.0 * std::log10(power / count);
    }
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    for (int band = 0; band < bands; ++band)
    {
        const double x = band / 3.0, y = result[static_cast<std::size_t>(band)];
        sx += x; sy += y; sxx += x * x; sxy += x * y;
    }
    result[bands] = (bands * sxy - sx * sy) / (bands * sxx - sx * sx);
    return result;
}
} // namespace

int main(int argc, char** argv)
{
    static const char* shapes[] { "Soprano", "Concert", "Tenor" };
    static const char* woods[] { "Spruce", "Mahogany", "Maple" };
    if (argc == 2 && std::strcmp(argv[1], "--check") == 0)
    {
        constexpr double tolerance = 0.15;
        int failures = 0;
        for (int shape = 0; shape < 3; ++shape)
        {
            const double slope = radiation(shape, 0)[bands];
            const bool pass = std::abs(slope - measuredSlope) <= tolerance;
            std::printf("%s %s spruce %+.3f dB/octave from 1 to 6.3 kHz (%+.3f +- %.2f)\n",
                        pass ? "ok  " : "FAIL", shapes[shape], slope, measuredSlope, tolerance);
            failures += pass ? 0 : 1;
        }
        return failures == 0 ? 0 : 1;
    }
    std::printf("Radiation per bridge force, third octaves in dB re the 1 kHz band, and the\n"
                "1-6.3 kHz slope (the measured soprano: %+.3f dB/octave)\n\n", measuredSlope);
    std::printf("%-8s %-9s", "Shape", "Wood");
    for (int band = 0; band < bands; ++band)
        std::printf(" %6.0f", centre(band));
    std::printf("  dB/oct\n");
    for (int shape = 0; shape < 3; ++shape)
        for (int wood = 0; wood < 3; ++wood)
        {
            const auto levels = radiation(shape, wood);
            std::printf("%-8s %-9s", shapes[shape], woods[wood]);
            for (int band = 0; band < bands; ++band)
                std::printf(" %+6.1f", levels[static_cast<std::size_t>(band)] - levels[0]);
            std::printf("  %+6.2f\n", levels[bands]);
        }
    return 0;
}
