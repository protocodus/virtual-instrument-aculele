// The body radiates the same at every sample rate: each Shape and Wood's
// response from the bridge to the two microphones at 44.1, 88.2, 96 and
// 192 kHz, against its 48 kHz response, where the bank was fitted and the
// instrument voiced.
//
// Protocol: the body bank's impulse response (BodyBank::render, the capture
// filter included), 0.4 s, for the bridge force and for the bridge moment;
// its transform at twelve points an octave from 100 Hz; the power of five
// adjacent points (a third of an octave) at each rate against the same five
// at 48 kHz, per microphone, for every centre up to 10 kHz.
//
// Until 0.4.64 the bank sounded every measured mode below 0.46 fs. The
// ukulele's scale carries modes to about 30 kHz, so 88.2 kHz and above
// sounded some twenty that 48 kHz never plays, 44.1 kHz dropped those
// between 20.3 and 22.1 kHz, and every rate kept each mode's level at its
// resonance at the cost of its tail's. The high modes' tails nearly cancel
// under 1 kHz, so a third octave stood up to 18.6 dB from the 48 kHz
// response below 5 kHz and 4.4 dB from there up on the force input (27.6
// and 3.7 dB on the moment). With the same modes at every rate and the
// static term (AculeleEngine::buildBodyConfiguration) the worst are 0.46
// and 0.89 dB on the force and 1.21 and 2.09 dB on the moment.

#include "DSP/AculeleEngine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <memory>
#include <vector>

namespace aculele
{
struct AculeleEngineTestAccess
{
    // One body's impulse response on an engine prepared at its rate.
    static std::vector<std::array<float, 2>> impulseResponse(AculeleEngine& engine, double rate,
                                                             int shape, int wood, bool moment)
    {
        EngineParameters parameters;
        parameters.shape = static_cast<BodyShape>(shape);
        parameters.bodyMaterial = static_cast<BodyMaterial>(wood);
        engine.setParameters(parameters);
        engine.settleSuspendedParameters();
        engine.configureBody();
        engine.bodyBank_.reset();
        std::vector<std::array<float, 2>> response(static_cast<std::size_t>(0.4 * rate));
        for (std::size_t i = 0; i < response.size(); ++i)
        {
            const float input = i == 0 ? 1.0f : 0.0f;
            const auto out = moment ? engine.bodyBank_.render(0.0f, input)
                                    : engine.bodyBank_.render(input, 0.0f);
            response[i] = { out.left, out.right };
        }
        return response;
    }
};
} // namespace aculele

namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr int pointsPerOctave = 12;
// 100 Hz to 10 kHz and the two points past each end a third octave takes.
constexpr int pointCount = 80 + 4;
constexpr int bodies = 3 * 4;

double hzOf(int point)
{
    return 100.0 * std::exp2(static_cast<double>(point - 2) / pointsPerOctave);
}

// |H|^2 of both microphones at each point: [body][input][point][microphone].
using Powers = std::vector<std::array<std::array<double, 2>, pointCount>>;

Powers powers(double rate)
{
    auto engine = std::make_unique<aculele::AculeleEngine>();
    engine->prepare(rate, 256);
    Powers result;
    for (int body = 0; body < bodies; ++body)
        for (int input = 0; input < 2; ++input)
        {
            const auto response = aculele::AculeleEngineTestAccess::impulseResponse(
                *engine, rate, body / 4, body % 4, input == 1);
            result.emplace_back();
            for (int point = 0; point < pointCount; ++point)
            {
                const std::complex<double> step = std::polar(1.0, -2.0 * pi * hzOf(point) / rate);
                std::complex<double> phasor { 1.0, 0.0 }, left, right;
                for (const auto& sample : response)
                {
                    left += static_cast<double>(sample[0]) * phasor;
                    right += static_cast<double>(sample[1]) * phasor;
                    phasor *= step;
                }
                result.back()[static_cast<std::size_t>(point)] = { std::norm(left), std::norm(right) };
            }
        }
    return result;
}
} // namespace

int main()
{
    static const char* const inputs[] { "force", "moment" };
    // [input][below or from 5 kHz]. The tails of the modes above 10 kHz are
    // right at 0 Hz and at their resonances and approximate between, which
    // shows from 5 kHz up; the moment's response is the weaker and the more
    // deeply notched of the two, so the same difference reads larger on it.
    constexpr double limitDb[2][2] { { 0.6, 1.2 }, { 1.5, 2.5 } };
    double worst[2][2] {};
    int failures = 0;
    const auto reference = powers(48000.0);
    for (const double rate : { 44100.0, 88200.0, 96000.0, 192000.0 })
    {
        const auto played = powers(rate);
        for (std::size_t set = 0; set < played.size(); ++set)
            for (int centre = 2; centre + 2 < pointCount; ++centre)
                for (std::size_t microphone = 0; microphone < 2; ++microphone)
                {
                    double here = 0.0, there = 0.0;
                    for (int point = centre - 2; point <= centre + 2; ++point)
                    {
                        here += played[set][static_cast<std::size_t>(point)][microphone];
                        there += reference[set][static_cast<std::size_t>(point)][microphone];
                    }
                    const double difference = std::abs(10.0 * std::log10(here / there));
                    const std::size_t input = set % 2, band = hzOf(centre) < 5000.0 ? 0 : 1;
                    worst[input][band] = std::max(worst[input][band], difference);
                    if (!(difference <= limitDb[input][band]))
                    {
                        std::fprintf(stderr,
                            "FAIL shape %d, wood %d, %s, %s microphone, %.0f Hz: "
                            "the third octave at %.0f Hz is %.2f dB from 48 kHz's\n",
                            static_cast<int>(set / 2) / 4, static_cast<int>(set / 2) % 4,
                            inputs[input], microphone == 0 ? "left" : "right", rate,
                            hzOf(centre), difference);
                        ++failures;
                    }
                }
    }
    std::printf("radiation against 48 kHz, worst third octave below and from 5 kHz: "
                "force %.2f and %.2f dB, moment %.2f and %.2f dB\n",
                worst[0][0], worst[0][1], worst[1][0], worst[1][1]);
    if (failures == 0)
        std::printf("radiation rate checks passed\n");
    return failures == 0 ? 0 : 1;
}
