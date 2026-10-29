// Every open string of every body, string set and tuning sounds its note.
//
// Each string is struck alone in tab input (CC126 = 4 on channel 1 makes
// channels 1-4 the G, C, E and A strings), at the default controls, 48 kHz
// and 256-sample blocks. Its fundamental is the peak of the Hann-windowed
// spectrum over 1-2 s, the pitch a tuner settles on, searched in 1-cent
// steps within 40 cents of the note and refined by a parabola through the
// log magnitudes. It must lie within 5 cents of equal temperament at
// A4 = 440 Hz, a tuner's tolerance; when the engine was forked (2026-10-27)
// every open string measured within 3 cents.
//
// The window began at 0.3 s, past the attack's tension glide, until 0.4.20.
// A string just under its body's T1 sounds two members: the polarisation
// normal to the top, pulled flat by the bridge and soon drained, and the
// lasting parallel one, which the delay tunes (AculeleEngine.cpp, the
// string gauges). The tenor's open F#4 in D tuning, 7% under T1, starts
// 7.6 cents flat over 0.1-0.5 s on fluorocarbon and settles at +0.8 cents
// by 1 s, and a 0.3-1.3 s window read whichever member outweighed the
// other in it: +0.09 cents at 0.4.19. The tenor's measured soundhole (0.4.20)
// left the flat member 7.2 cents under and 2 dB stronger, and the same
// window read it, -6.6 cents, though the note settles where it did. So the
// window now opens once the flat member has gone.

#include "DSP/AculelePerformer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 256;
constexpr double toleranceCents = 5.0;
constexpr double pi = 3.14159265358979323846;

int failures = 0;

void check(bool condition, const char* what, double value)
{
    if (! condition)
    {
        std::fprintf(stderr, "FAIL %s (%.2f)\n", what, value);
        ++failures;
    }
}

std::vector<float> renderOpenString(const aculele::Performer& prepared, int string, int note)
{
    auto performer = std::make_unique<aculele::Performer>(prepared);
    const auto total = static_cast<std::size_t>(2.1 * rate);
    std::vector<float> left(total), right(total);
    const std::uint8_t tab[3] { 0xB0, 126, 4 };
    const std::uint8_t on[3] { static_cast<std::uint8_t>(0x90 | string), static_cast<std::uint8_t>(note), 90 };
    for (std::size_t done = 0; done < total; done += block)
    {
        const auto frames = static_cast<int>(std::min<std::size_t>(block, total - done));
        performer->beginBlock(left.data() + done, right.data() + done, frames);
        if (done == 0)
        {
            performer->handleMidi(0, tab, 3);
            performer->handleMidi(0, on, 3);
        }
        performer->endBlock();
    }
    for (std::size_t i = 0; i < total; ++i)
        left[i] += right[i];
    return left;
}

// |X(f)| of the Hann-windowed window, by a rotating phasor.
double magnitude(const std::vector<float>& x, std::size_t start, std::size_t length, double frequency)
{
    const std::complex<double> step = std::polar(1.0, -2.0 * pi * frequency / rate);
    std::complex<double> phasor { 1.0, 0.0 }, sum { 0.0, 0.0 };
    for (std::size_t n = 0; n < length; ++n)
    {
        const double window = 0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(n) / static_cast<double>(length - 1));
        sum += window * static_cast<double>(x[start + n]) * phasor;
        phasor *= step;
    }
    return std::abs(sum);
}

double fundamentalCents(const std::vector<float>& x, double nominal)
{
    const auto start = static_cast<std::size_t>(1.0 * rate);
    const auto length = static_cast<std::size_t>(1.0 * rate);
    std::array<double, 81> levels {};
    std::size_t best = 0;
    for (std::size_t i = 0; i < levels.size(); ++i)
    {
        const double cents = static_cast<double>(i) - 40.0;
        levels[i] = std::log(magnitude(x, start, length, nominal * std::exp2(cents / 1200.0)) + 1e-30);
        if (levels[i] > levels[best])
            best = i;
    }
    if (best == 0 || best + 1 == levels.size())
        return static_cast<double>(best) - 40.0;
    const double a = levels[best - 1], b = levels[best], c = levels[best + 1];
    return static_cast<double>(best) - 40.0 + 0.5 * (a - c) / (a - 2.0 * b + c);
}
} // namespace

int main()
{
    struct TuningCase
    {
        aculele::Tuning tuning;
        const char* name;
        std::array<int, 4> notes; // G, C, E, A strings
    };
    constexpr std::array<TuningCase, 3> tunings {{
        { aculele::Tuning::Standard, "Standard", { 67, 60, 64, 69 } },
        { aculele::Tuning::LowG, "Low G", { 55, 60, 64, 69 } },
        { aculele::Tuning::DTuning, "D tuning", { 69, 62, 66, 71 } },
    }};
    constexpr std::array<const char*, 3> shapes { "soprano", "concert", "tenor" };
    constexpr std::array<const char*, 2> materials { "nylon", "fluorocarbon" };

    for (std::size_t shape = 0; shape < shapes.size(); ++shape)
        for (std::size_t material = 0; material < materials.size(); ++material)
            for (const auto& tuning : tunings)
            {
                aculele::EngineParameters parameters;
                parameters.shape = static_cast<aculele::BodyShape>(shape);
                parameters.strings = static_cast<aculele::StringMaterial>(material);
                parameters.tuning = tuning.tuning;
                auto prepared = std::make_unique<aculele::Performer>();
                prepared->setParameters(parameters);
                prepared->prepare(rate, block);
                prepared->setParameters(parameters);
                for (int string = 0; string < 4; ++string)
                {
                    const int note = tuning.notes[static_cast<std::size_t>(string)];
                    const double nominal = 440.0 * std::exp2((note - 69) / 12.0);
                    const double cents = fundamentalCents(renderOpenString(*prepared, string, note), nominal);
                    std::printf("%-8s %-13s %-9s string %d (MIDI %d): %+.2f cents\n", shapes[shape],
                                materials[material], tuning.name, string + 1, note, cents);
                    check(std::abs(cents) <= toleranceCents, "open string out of tune", cents);
                }
            }

    if (failures != 0)
    {
        std::fprintf(stderr, "%d open strings out of tune\n", failures);
        return EXIT_FAILURE;
    }
    std::puts("every open string within 5 cents");
    return EXIT_SUCCESS;
}
