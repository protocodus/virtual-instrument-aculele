// Prints the bridge's driving-point mobility each Shape and Wood configures,
// as the strings see it, beside the one published ukulele measurement:
//
//   AculeleBridgeMobility [--curve shape wood string | --check]
//
// Elie, Gautier, David and Curtit, "Analysis of bridge admittance of plucked
// string instruments in the high frequency range", Forum Acusticum 2011
// (https://members.loria.fr/BElie/papers/Elie_et_al_FA_2011.pdf), measured
// one ukulele at the bridge by impact hammer and accelerometer, strings
// damped (their Figs. 3 and 5): its characteristic mobility G_C, Skudrzyk's
// mean value, is about 0.012 s/kg with a mahogany bridge and 0.009 s/kg
// with a rosewood one, flat from about 1 to 6 kHz, and 0.034 s/kg with no
// bridge. Their Fig. 5 gives the values as plotted points; they are read off
// it, not tabulated in the paper.
//
// The mean value is taken here as Elie et al. take theirs over a measured
// curve: the band's mean conductance, Re(Y), which in a band of many
// overlapping modes is the characteristic conductance G_C. It is printed
// for 1-2, 2-4 and 4-6 kHz and over 1-6 kHz, on a log-spaced grid of 2400
// points, at the normal port of each string.
//
// --check is Aculele.BridgeMobility: Concert Mahogany's G string, at the
// bass-side edge where the paper's accelerometer sat on its guitars, must
// read 0.009 s/kg over 1-6 kHz within 3%, the level ukuleleTopMobilityRatio
// pins, and every body's within 10%, the equal-thickness plate law's claim.

#include "DSP/AculeleEngine.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace aculele
{
struct AculeleEngineTestAccess
{
    static std::complex<float> mobility(const AculeleEngine& engine, float hz, int string)
    {
        return engine.bridgePortMobility(hz, string).normal;
    }
};
} // namespace aculele

namespace
{
std::unique_ptr<aculele::AculeleEngine> configured(int shape, int wood)
{
    auto engine = std::make_unique<aculele::AculeleEngine>();
    engine->prepare(48000.0, 256);
    aculele::EngineParameters parameters;
    parameters.shape = static_cast<aculele::BodyShape>(shape);
    parameters.bodyMaterial = static_cast<aculele::BodyMaterial>(wood);
    engine->setParameters(parameters);
    engine->settleSuspendedParameters();
    return engine;
}

double meanConductance(const aculele::AculeleEngine& engine, int string, double low, double high)
{
    constexpr int points = 2400;
    double sum = 0.0;
    for (int i = 0; i < points; ++i)
    {
        const double hz = low * std::pow(high / low, (i + 0.5) / points);
        sum += aculele::AculeleEngineTestAccess::mobility(engine, static_cast<float>(hz), string).real();
    }
    return sum / points;
}
} // namespace

int main(int argc, char** argv)
{
    static const char* shapes[] { "Soprano", "Concert", "Tenor" };
    if (argc == 2 && std::strcmp(argv[1], "--check") == 0)
    {
        constexpr double measured = 0.009;
        constexpr int concert = 1, mahogany = 1;
        int failures = 0;
        for (int shape = 0; shape < 3; ++shape)
        {
            const auto engine = configured(shape, mahogany);
            const double g = meanConductance(*engine, 0, 1000.0, 6000.0);
            const double tolerance = shape == concert ? 0.03 : 0.10;
            const bool pass = std::abs(g / measured - 1.0) <= tolerance;
            std::printf("%s %s mahogany G string %.5f s/kg (%.5f +- %.0f%%)\n",
                        pass ? "ok  " : "FAIL", shapes[shape], g, measured, 100.0 * tolerance);
            failures += pass ? 0 : 1;
        }
        return failures == 0 ? 0 : 1;
    }
    if (argc == 5 && std::strcmp(argv[1], "--curve") == 0)
    {
        const auto engine = configured(std::atoi(argv[2]), std::atoi(argv[3]));
        const int string = std::atoi(argv[4]);
        for (int i = 0; i <= 600; ++i)
        {
            const double hz = 50.0 * std::pow(8000.0 / 50.0, i / 600.0);
            const auto y = aculele::AculeleEngineTestAccess::mobility(*engine, static_cast<float>(hz), string);
            std::printf("%.2f %.6e %.6e\n", hz, static_cast<double>(y.real()), static_cast<double>(y.imag()));
        }
        return 0;
    }
    static const char* woods[] { "Spruce", "Mahogany", "Maple" };
    std::printf("Mean conductance Re(Y) at the normal port, s/kg (Elie et al. 2011, one ukulele:\n"
                "0.012 mahogany bridge, 0.009 rosewood bridge, 1-6 kHz)\n\n");
    std::printf("%-8s %-9s %-6s %9s %9s %9s %9s\n", "Shape", "Wood", "String", "1-2 kHz", "2-4 kHz", "4-6 kHz", "1-6 kHz");
    for (int shape = 0; shape < 3; ++shape)
        for (int wood = 0; wood < 3; ++wood)
        {
            const auto engine = configured(shape, wood);
            for (int string = 0; string < 4; ++string)
            {
                static const char* names[] { "G", "C", "E", "A" };
                std::printf("%-8s %-9s %-6s %9.5f %9.5f %9.5f %9.5f\n", shapes[shape], woods[wood], names[string],
                            meanConductance(*engine, string, 1000.0, 2000.0),
                            meanConductance(*engine, string, 2000.0, 4000.0),
                            meanConductance(*engine, string, 4000.0, 6000.0),
                            meanConductance(*engine, string, 1000.0, 6000.0));
            }
        }
    return 0;
}
