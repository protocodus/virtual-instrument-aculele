// How evenly Aculele plays across its strings, in every shape, wood and
// Picking:
//
//   AculeleEvenness [--strings N] [--jobs N]
//
// Each of the 48 cells, (shape * 4 + wood) * 4 + picking as in
// Tools/AculeleLevels.cpp, plays every string from open to the 12th fret, 52
// notes, each alone after a reset, at velocity 90, held 1.6 s, as tab on its
// own string (CC126 = 4). It plays at Concert Strum's other controls in
// Standard tuning on the stereo pair, with Strings nylon (0) unless --strings
// says otherwise, at -20 dB Output, below the safety limiter, and with no
// room and no release noise, so a note's level and brightness are its
// string's and the body's alone. 48 kHz, 256-sample blocks. For each note:
//  - level: the K-weighted (ITU-R BS.1770) power of both channels over its
//    first second, in dB;
//  - brightness: the share of the channels' mean's power above 2.5 kHz over
//    its first 0.3 s (Hann window), in dB, the measure of
//    Docs/validation/sample-pack-benchmark-2026-10-29.md.
// Each row prints, for the G, C, E and A strings, the mean level and the mean
// brightness of that string's 13 notes against the four strings' mean; the
// mean brightness of all 52, which compares cells; the SD of the notes' levels about a straight line in pitch; and how far apart
// one pitch plays on different strings, the loudest against the quietest,
// averaged over the pitches two or more strings reach. A Picking's mean over
// the 12 shapes and woods follows the cells.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace
{
using namespace aculele;

constexpr double rate = 48000.0;
constexpr int block = 256;
constexpr long slot = 96000, hold = 76800; // 2 s, 1.6 s
constexpr int frets = 12;
constexpr std::size_t cells = 3 * 4 * 4;
constexpr double pi = 3.14159265358979323846;
const char* const shapeNames[] { "soprano", "concert", "tenor" };
const char* const woodNames[] { "spruce", "mahogany", "koa", "cedar" };
const char* const pickingNames[] { "finger", "pick", "thumb", "strum" };

void parallel(std::size_t count, int jobs, const std::function<void(std::size_t)>& work)
{
    std::atomic<std::size_t> next { 0 };
    std::vector<std::thread> threads;
    for (int j = 0; j < jobs; ++j)
        threads.emplace_back([&] {
            for (std::size_t i = next++; i < count; i = next++)
                work(i);
        });
    for (auto& thread : threads)
        thread.join();
}

double decibels(double power) { return 10.0 * std::log10(std::max(power, 1.0e-30)); }

// BS.1770's two K-weighting stages at 48 kHz.
double kWeightedPower(const float* x, std::size_t n)
{
    constexpr double b1[3] { 1.53512485958697, -2.69169618940638, 1.19839281085285 };
    constexpr double a1[3] { 1.0, -1.69065929318241, 0.73248077421585 };
    constexpr double b2[3] { 1.0, -2.0, 1.0 };
    constexpr double a2[3] { 1.0, -1.99004745483398, 0.99007225036621 };
    double s1[2] {}, s2[2] {}, sum = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const double in = x[i];
        const double y1 = b1[0] * in + s1[0];
        s1[0] = b1[1] * in - a1[1] * y1 + s1[1];
        s1[1] = b1[2] * in - a1[2] * y1;
        const double y2 = b2[0] * y1 + s2[0];
        s2[0] = b2[1] * y1 - a2[1] * y2 + s2[1];
        s2[1] = b2[2] * y1 - a2[2] * y2;
        sum += y2 * y2;
    }
    return sum / static_cast<double>(n);
}

void fft(std::vector<std::complex<double>>& x)
{
    const std::size_t n = x.size();
    for (std::size_t i = 1, j = 0; i < n; ++i)
    {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(x[i], x[j]);
    }
    for (std::size_t length = 2; length <= n; length <<= 1)
    {
        const auto step = std::polar(1.0, -2.0 * pi / static_cast<double>(length));
        for (std::size_t start = 0; start < n; start += length)
        {
            std::complex<double> w { 1.0, 0.0 };
            for (std::size_t k = 0; k < length / 2; ++k, w *= step)
            {
                const auto odd = w * x[start + k + length / 2];
                x[start + k + length / 2] = x[start + k] - odd;
                x[start + k] += odd;
            }
        }
    }
}

double brightness(const float* left, const float* right)
{
    const auto length = static_cast<std::size_t>(0.3 * rate);
    std::vector<std::complex<double>> x(16384);
    for (std::size_t i = 0; i < length; ++i)
        x[i] = 0.5 * (static_cast<double>(left[i]) + right[i])
            * (0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(i) / static_cast<double>(length - 1)));
    fft(x);
    double above = 0.0, total = 0.0;
    for (std::size_t k = 0; k <= x.size() / 2; ++k)
    {
        const double power = std::norm(x[k]);
        total += power;
        if (static_cast<double>(k) * rate / static_cast<double>(x.size()) > 2500.0)
            above += power;
    }
    return decibels(above / total);
}

struct Note
{
    int string, note;
    double level, bright;
};

std::vector<Note> playCell(std::size_t cell, StringMaterial strings)
{
    auto parameters = engineParameters(factoryPresets[0]);
    parameters.shape = static_cast<BodyShape>(cell / 16);
    parameters.bodyMaterial = static_cast<BodyMaterial>(cell / 4 % 4);
    parameters.picking = static_cast<PickingTechnique>(cell % 4);
    parameters.strings = strings;
    parameters.tuning = Tuning::Standard;
    parameters.capture = CaptureType::StereoMic;
    parameters.piezoMix = 0.0f;
    parameters.outputGain = 0.1f;
    parameters.room = 0.0f;
    parameters.releaseNoise = 0.0f;
    auto performer = std::make_unique<Performer>();
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    performer->setTempoBpm(120.0);
    std::vector<float> left(slot), right(slot);
    performer->beginBlock(left.data(), right.data(), block);
    performer->controlChange(0, 1, 126, 4);
    performer->endBlock();

    const auto open = AculeleEngine::openNotes(Tuning::Standard);
    std::vector<Note> notes;
    for (int string = 0; string < 4; ++string)
        for (int fret = 0; fret <= frets; ++fret)
        {
            const int note = open[static_cast<std::size_t>(string)] + fret;
            performer->reset();
            for (long done = 0; done < slot; done += block)
            {
                performer->beginBlock(left.data() + done, right.data() + done, block);
                if (done == 0)
                    performer->noteOn(0, string + 1, note, 90);
                if (done <= hold && hold < done + block)
                    performer->noteOff(static_cast<int>(hold - done), string + 1, note);
                performer->endBlock();
            }
            const auto second = static_cast<std::size_t>(rate);
            notes.push_back({ string, note,
                              decibels(kWeightedPower(left.data(), second) + kWeightedPower(right.data(), second)),
                              brightness(left.data(), right.data()) });
        }
    return notes;
}

struct Row
{
    std::array<double, 4> level {}, bright {};
    double brightness = 0.0, spread = 0.0, samePitch = 0.0;
};

Row summarise(const std::vector<Note>& notes)
{
    Row row;
    for (const auto& n : notes)
    {
        row.level[static_cast<std::size_t>(n.string)] += n.level / (frets + 1);
        row.bright[static_cast<std::size_t>(n.string)] += n.bright / (frets + 1);
        row.brightness += n.bright / static_cast<double>(notes.size());
    }
    for (auto* values : { &row.level, &row.bright })
    {
        double mean = 0.0;
        for (const double v : *values)
            mean += v / 4.0;
        for (double& v : *values)
            v -= mean;
    }
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    const auto count = static_cast<double>(notes.size());
    for (const auto& n : notes)
    {
        sx += n.note; sy += n.level; sxx += static_cast<double>(n.note) * n.note; sxy += n.note * n.level;
    }
    const double slope = (count * sxy - sx * sy) / (count * sxx - sx * sx), intercept = (sy - slope * sx) / count;
    for (const auto& n : notes)
        row.spread += std::pow(n.level - (intercept + slope * n.note), 2.0) / count;
    row.spread = std::sqrt(row.spread);
    std::map<int, std::vector<double>> byPitch;
    for (const auto& n : notes)
        byPitch[n.note].push_back(n.level);
    int shared = 0;
    for (const auto& [pitch, levels] : byPitch)
        if (levels.size() >= 2)
        {
            row.samePitch += *std::max_element(levels.begin(), levels.end())
                - *std::min_element(levels.begin(), levels.end());
            ++shared;
        }
    row.samePitch /= shared;
    return row;
}

void print(const char* name, const Row& r)
{
    std::printf("%-24s %+5.1f %+5.1f %+5.1f %+5.1f | %+6.1f %+6.1f %+6.1f %+6.1f | %6.1f | %5.2f %5.2f\n", name,
                r.level[0], r.level[1], r.level[2], r.level[3], r.bright[0], r.bright[1], r.bright[2],
                r.bright[3], r.brightness, r.spread, r.samePitch);
}
} // namespace

int main(int argc, char** argv)
{
    int jobs = 4, strings = 0;
    for (int i = 1; i + 1 < argc; ++i)
    {
        if (!std::strcmp(argv[i], "--jobs"))
            jobs = std::max(1, std::atoi(argv[i + 1]));
        else if (!std::strcmp(argv[i], "--strings"))
            strings = std::atoi(argv[i + 1]);
    }
    std::vector<Row> rows(cells);
    parallel(cells, jobs, [&](std::size_t cell) {
        rows[cell] = summarise(playCell(cell, static_cast<StringMaterial>(strings)));
    });
    std::printf("%-24s %5s %5s %5s %5s | %6s %6s %6s %6s | %6s | %5s %5s\n", "cell (level, brightness)", "G",
                "C", "E", "A", "G", "C", "E", "A", "mean", "SD", "pitch");
    for (std::size_t cell = 0; cell < cells; ++cell)
    {
        char name[64];
        std::snprintf(name, sizeof name, "%s %s %s", shapeNames[cell / 16], woodNames[cell / 4 % 4],
                      pickingNames[cell % 4]);
        print(name, rows[cell]);
    }
    for (std::size_t picking = 0; picking < 4; ++picking)
    {
        Row mean;
        for (std::size_t cell = picking; cell < cells; cell += 4)
            for (std::size_t s = 0; s < 4; ++s)
            {
                mean.level[s] += rows[cell].level[s] / 12.0;
                mean.bright[s] += rows[cell].bright[s] / 12.0;
                if (s == 0)
                {
                    mean.brightness += rows[cell].brightness / 12.0;
                    mean.spread += rows[cell].spread / 12.0;
                    mean.samePitch += rows[cell].samePitch / 12.0;
                }
            }
        char name[64];
        std::snprintf(name, sizeof name, "mean %s", pickingNames[picking]);
        print(name, mean);
    }
    return 0;
}
