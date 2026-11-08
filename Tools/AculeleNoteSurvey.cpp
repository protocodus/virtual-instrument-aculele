// Every playable note of every factory preset, at two velocities, as a CSV for
// finding outliers in level, tone and resonance across the range:
//
//   AculeleNoteSurvey [--mode keys|strings] [--bank presets|constructions]
//                     [--velocities 60,110] [--jobs N] [--rate 48000]
//                     [--block 256] [--out note-survey.csv]
//
// keys (the default) plays every key from the tuning's lowest open string to
// its highest open string's 18th fret on channel 1, the ordinary one-channel
// mode, so the Performer chooses the string as a player's hand would; the
// string it took is read from AculeleEngine::heldString while the key is down.
// strings plays tab (CC126 = 4 on channel 1, a string per channel): every
// string from open to the 18th fret. Each note is played alone after a reset,
// held 1.6 s in a 2.5 s slot, through the preset exactly as shipped (room,
// release noise, capture and Output included). 48 kHz unless --rate says
// otherwise, 256-sample blocks unless --block says otherwise (Reason renders
// 64 at a time); the analysis windows are in seconds, so a survey at another
// rate measures the same thing.
// The constructions bank holds all Concert Strum controls fixed except Shape
// and Wood, so it covers the entire 3 x 4 matrix at the same tuning and stroke.
//
// Columns, per note:
//  - level_db: K-weighted (ITU-R BS.1770, the stages designed at the rate)
//    power of both channels over the first second, as Tools/AculeleEvenness.cpp; level_200ms_db over 0-200 ms
//    and level_1s_db over 900-1100 ms; decay_db_per_s is their difference
//    over 0.85 s, a rough decay rate of the held note;
//  - pre_release_db over 1.4-1.6 s; release_100ms_db and release_700ms_db
//    over 1.7-1.9 s and 2.3-2.5 s, respectively, on the same K-weighted scale;
//  - peak_dbfs: the largest |sample| of either channel;
//  - bright_db: the share of the channels' mean's power above 2.5 kHz over
//    the first 0.3 s (Hann), as AculeleEvenness; centroid_hz: the power
//    centroid of the same spectrum;
//  - f0_hz: the strongest peak within 1.5 semitones of the nominal pitch
//    (A4 = 440 Hz) in the 0.3-1.0 s mono segment (Hann, 65536-point FFT,
//    parabolic interpolation), and its error in cents; pitch_cents_error:
//    the pitch the partials carry together, the amplitude-weighted mean of
//    partials 1-5 each divided by its number (each the strongest peak within
//    2.5% of its place on f0), in cents from nominal. A fundamental drained
//    by a body resonance reads up to 10 cents off while the note's pitch,
//    carried by the partials above it, moves a third of that;
//  - spike_db: the strongest bin between 150 Hz and 6 kHz farther than 3%
//    from every harmonic of f0, against the strongest bin within 3% of one.
//    Harmonics up to 6 kHz count (at least 20), so a low note's 21st partial
//    is not taken for a resonance.
// stderr gets, per preset and velocity, the SD of level_db about a straight
// line in note number, the largest residual and its note, and the note with
// the largest spike_db and the note whose pitch_cents_error is largest.

#include "DSP/AculelePerformer.h"
#include "DSP/AculelePresets.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{
using namespace aculele;

double rate = 48000.0; // --rate
int block = 256; // --block
constexpr int frets = AculeleEngine::fretCount;
constexpr double holdSeconds = 1.6;
constexpr double pi = 3.14159265358979323846;

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

// BS.1770's two K-weighting stages, designed at the survey's rate from the
// analogue prototypes the standard's 48 kHz tables come from (a +4 dB shelf
// at 1682 Hz, Q 0.707, and a 38.1 Hz second-order high-pass, Q 0.5; the
// constants are the ones that reproduce the published 48 kHz coefficients to
// 1e-15). At 48 kHz the stages are the standard's tables exactly.
double kWeightedPower(const float* x, std::size_t begin, std::size_t end)
{
    double b1[3], a1[3], b2[3], a2[3];
    {
        const double gainDb = 3.999843853973347, q = 0.7071752369554196;
        const double k = std::tan(pi * 1681.974450955533 / rate);
        const double vh = std::pow(10.0, gainDb / 20.0), vb = std::pow(vh, 0.4996667741545416);
        const double a0 = 1.0 + k / q + k * k;
        b1[0] = (vh + vb * k / q + k * k) / a0;
        b1[1] = 2.0 * (k * k - vh) / a0;
        b1[2] = (vh - vb * k / q + k * k) / a0;
        a1[0] = 1.0;
        a1[1] = 2.0 * (k * k - 1.0) / a0;
        a1[2] = (1.0 - k / q + k * k) / a0;
    }
    {
        const double q = 0.5003270373238773;
        const double k = std::tan(pi * 38.13547087602444 / rate);
        const double a0 = 1.0 + k / q + k * k;
        b2[0] = 1.0; b2[1] = -2.0; b2[2] = 1.0;
        a2[0] = 1.0;
        a2[1] = 2.0 * (k * k - 1.0) / a0;
        a2[2] = (1.0 - k / q + k * k) / a0;
    }
    double s1[2] {}, s2[2] {}, sum = 0.0;
    // The filters run from the note's start so a later window is not a cold start.
    for (std::size_t i = 0; i < end; ++i)
    {
        const double in = x[i];
        const double y1 = b1[0] * in + s1[0];
        s1[0] = b1[1] * in - a1[1] * y1 + s1[1];
        s1[1] = b1[2] * in - a1[2] * y1;
        const double y2 = b2[0] * y1 + s2[0];
        s2[0] = b2[1] * y1 - a2[1] * y2 + s2[1];
        s2[1] = b2[2] * y1 - a2[2] * y2;
        if (i >= begin)
            sum += y2 * y2;
    }
    return sum / static_cast<double>(end - begin);
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

// The power spectrum of the channels' mean over [begin, end), Hann-windowed.
// size: the FFT's length at 48 kHz; at another rate it grows to the power of
// two that keeps the same window in seconds, so the resolution in Hz holds.
std::vector<double> spectrum(const float* left, const float* right, std::size_t begin, std::size_t end,
                             std::size_t size)
{
    const std::size_t length = end - begin;
    while (size < length)
        size *= 2;
    std::vector<std::complex<double>> x(size);
    for (std::size_t i = 0; i < length; ++i)
        x[i] = 0.5 * (static_cast<double>(left[begin + i]) + right[begin + i])
            * (0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(i) / static_cast<double>(length - 1)));
    fft(x);
    std::vector<double> power(size / 2 + 1);
    for (std::size_t k = 0; k < power.size(); ++k)
        power[k] = std::norm(x[k]);
    return power;
}

struct Note
{
    int velocity, string, fret, note;
    double level, peak, level200, level1s, decay, bright, centroid, f0, cents, pitch, spike;
    double preRelease, release100ms, release700ms;
};

Note measure(const std::vector<float>& left, const std::vector<float>& right, int velocity, int string,
             int fret, int note)
{
    Note n { velocity, string, fret, note };
    const auto ms = [](double s) { return static_cast<std::size_t>(s * rate); };
    const auto power = [&](std::size_t b, std::size_t e) {
        return decibels(kWeightedPower(left.data(), b, e) + kWeightedPower(right.data(), b, e));
    };
    n.level = power(0, ms(1.0));
    n.level200 = power(0, ms(0.2));
    n.level1s = power(ms(0.9), ms(1.1));
    n.decay = (n.level200 - n.level1s) / 0.85;
    n.preRelease = power(ms(holdSeconds - 0.2), ms(holdSeconds));
    n.release100ms = power(ms(holdSeconds + 0.1), ms(holdSeconds + 0.3));
    n.release700ms = power(ms(holdSeconds + 0.7), left.size());
    double peak = 0.0;
    for (std::size_t i = 0; i < left.size(); ++i)
        peak = std::max({ peak, std::fabs(static_cast<double>(left[i])), std::fabs(static_cast<double>(right[i])) });
    n.peak = 20.0 * std::log10(std::max(peak, 1.0e-15));

    {
        const auto p = spectrum(left.data(), right.data(), 0, ms(0.3), 16384);
        double above = 0.0, total = 0.0, weighted = 0.0;
        for (std::size_t k = 0; k < p.size(); ++k)
        {
            const double f = static_cast<double>(k) * rate / static_cast<double>(2 * (p.size() - 1));
            total += p[k];
            weighted += p[k] * f;
            if (f > 2500.0)
                above += p[k];
        }
        n.bright = decibels(above / total);
        n.centroid = weighted / total;
    }

    const auto p = spectrum(left.data(), right.data(), ms(0.3), ms(1.0), 65536);
    const double binHz = rate / static_cast<double>(2 * (p.size() - 1));
    const double nominal = 440.0 * std::pow(2.0, (note - 69) / 12.0);
    const double ratio = std::pow(2.0, 1.5 / 12.0);
    auto lo = static_cast<std::size_t>(nominal / ratio / binHz), hi = static_cast<std::size_t>(nominal * ratio / binHz) + 1;
    std::size_t best = lo;
    for (std::size_t k = lo; k <= hi; ++k)
        if (p[k] > p[best])
            best = k;
    double offset = 0.0;
    if (best > 0 && best + 1 < p.size())
    {
        const double a = std::log(p[best - 1] + 1e-300), b = std::log(p[best] + 1e-300), c = std::log(p[best + 1] + 1e-300);
        const double d = a - 2.0 * b + c;
        if (d < 0.0)
            offset = 0.5 * (a - c) / d;
    }
    n.f0 = (static_cast<double>(best) + offset) * binHz;
    n.cents = 1200.0 * std::log2(n.f0 / nominal);
    {
        double weightedHz = 0.0, weights = 0.0;
        for (int partial = 1; partial <= 5; ++partial)
        {
            const double centre = partial * n.f0;
            const auto first = static_cast<std::size_t>(centre * 0.975 / binHz);
            const auto last = std::min(p.size() - 2, static_cast<std::size_t>(centre * 1.025 / binHz) + 1);
            if (first + 1 >= last)
                break;
            std::size_t peak = first;
            for (std::size_t k = first; k <= last; ++k)
                if (p[k] > p[peak])
                    peak = k;
            const double a = std::log(p[peak - 1] + 1e-300), b = std::log(p[peak] + 1e-300), c = std::log(p[peak + 1] + 1e-300);
            const double d = a - 2.0 * b + c;
            const double shift = d < 0.0 ? 0.5 * (a - c) / d : 0.0;
            const double amplitude = std::sqrt(p[peak]);
            weightedHz += amplitude * (static_cast<double>(peak) + shift) * binHz / partial;
            weights += amplitude;
        }
        n.pitch = weights > 0.0 ? 1200.0 * std::log2(weightedHz / weights / nominal) : n.cents;
    }

    const int harmonics = std::max(20, static_cast<int>(std::ceil(6000.0 / n.f0)));
    double harmonicPeak = 0.0, otherPeak = 0.0;
    for (auto k = static_cast<std::size_t>(150.0 / binHz); k <= static_cast<std::size_t>(6000.0 / binHz); ++k)
    {
        const double f = static_cast<double>(k) * binHz;
        const int h = std::max(1, static_cast<int>(std::lround(f / n.f0)));
        const bool near = h <= harmonics && std::fabs(f - h * n.f0) <= 0.03 * h * n.f0;
        (near ? harmonicPeak : otherPeak) = std::max(near ? harmonicPeak : otherPeak, p[k]);
    }
    n.spike = decibels(otherPeak) - decibels(harmonicPeak);
    return n;
}

std::vector<Note> play(const FactoryPreset& preset, int velocity, bool tab)
{
    const auto parameters = engineParameters(preset);
    auto performer = std::make_unique<Performer>();
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    performer->setTempoBpm(120.0);
    const auto slot = static_cast<long>(2.5 * rate), hold = static_cast<long>(holdSeconds * rate);
    const auto settledOnset = static_cast<long>(0.1 * rate);
    std::vector<float> left(static_cast<std::size_t>(slot)), right(static_cast<std::size_t>(slot));
    if (tab)
    {
        performer->beginBlock(left.data(), right.data(), block);
        performer->controlChange(0, 1, 126, 4);
        performer->endBlock();
    }

    const auto open = AculeleEngine::openNotes(preset.tuning);
    const int lowest = *std::min_element(open.begin(), open.end());
    const int highest = *std::max_element(open.begin(), open.end()) + frets;
    struct Key { int channel, note, string, fret; };
    std::vector<Key> keys;
    if (tab)
        for (int string = 0; string < 4; ++string)
            for (int fret = 0; fret <= frets; ++fret)
                keys.push_back({ string + 1, open[static_cast<std::size_t>(string)] + fret, string, fret });
    else
        for (int note = lowest; note <= highest; ++note)
            keys.push_back({ 1, note, -1, -1 });

    std::vector<Note> notes;
    for (auto key : keys)
    {
        performer->reset();
        for (long done = 0; done < slot; done += block)
        {
            const int length = static_cast<int>(std::min<long>(block, slot - done));
            performer->beginBlock(left.data() + done, right.data() + done, length);
            if (done == 0)
                performer->noteOn(0, key.channel, key.note, velocity);
            if (done <= hold && hold < done + length)
                performer->noteOff(static_cast<int>(hold - done), key.channel, key.note);
            performer->endBlock();
            if (done <= settledOnset && settledOnset < done + length)
            {
                const int playedString = performer->engine().heldString(key.note, key.channel);
                if (playedString < 0 || (tab && playedString != key.string))
                {
                    std::fprintf(stderr, "%s: note %d on channel %d took string %d (expected %d)\n",
                                 preset.slug, key.note, key.channel, playedString, key.string);
                    std::abort();
                }
                key.string = playedString;
                key.fret = key.note - open[static_cast<std::size_t>(key.string)];
            }
        }
        notes.push_back(measure(left, right, velocity, key.string, key.fret, key.note));
    }
    return notes;
}

void summarise(const char* preset, int velocity, const std::vector<Note>& notes)
{
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    const auto count = static_cast<double>(notes.size());
    for (const auto& n : notes)
    {
        sx += n.note; sy += n.level; sxx += static_cast<double>(n.note) * n.note; sxy += n.note * n.level;
    }
    const double slope = (count * sxy - sx * sy) / (count * sxx - sx * sx), intercept = (sy - slope * sx) / count;
    double spread = 0.0, worst = 0.0;
    const Note* worstNote = &notes.front();
    const Note* spiky = &notes.front();
    const Note* detuned = &notes.front();
    for (const auto& n : notes)
    {
        if (std::fabs(n.pitch) > std::fabs(detuned->pitch))
            detuned = &n;
        const double residual = n.level - (intercept + slope * n.note);
        spread += residual * residual / count;
        if (std::fabs(residual) > std::fabs(worst))
        {
            worst = residual;
            worstNote = &n;
        }
        if (n.spike > spiky->spike)
            spiky = &n;
    }
    std::fprintf(stderr,
                 "%-32s v%-3d SD %4.2f dB (slope %+5.2f dB/st) | worst %+5.1f dB at note %d (s%d f%d) | "
                 "spike %+5.1f dB at note %d (s%d f%d) | pitch %+5.1f c at note %d (s%d f%d)\n",
                 preset, velocity, std::sqrt(spread), slope, worst, worstNote->note, worstNote->string,
                 worstNote->fret, spiky->spike, spiky->note, spiky->string, spiky->fret,
                 detuned->pitch, detuned->note, detuned->string, detuned->fret);
}
} // namespace

int main(int argc, char** argv)
{
    int jobs = 4;
    bool tab = false;
    bool constructions = false;
    std::vector<int> velocities { 60, 110 };
    std::string out = "note-survey.csv";
    for (int i = 1; i + 1 < argc; ++i)
    {
        if (!std::strcmp(argv[i], "--jobs"))
            jobs = std::max(1, std::atoi(argv[i + 1]));
        else if (!std::strcmp(argv[i], "--mode"))
            tab = !std::strcmp(argv[i + 1], "strings");
        else if (!std::strcmp(argv[i], "--bank"))
            constructions = !std::strcmp(argv[i + 1], "constructions");
        else if (!std::strcmp(argv[i], "--out"))
            out = argv[i + 1];
        else if (!std::strcmp(argv[i], "--rate"))
            rate = std::max(8000.0, std::atof(argv[i + 1]));
        else if (!std::strcmp(argv[i], "--block"))
            block = std::max(1, std::atoi(argv[i + 1]));
        else if (!std::strcmp(argv[i], "--velocities"))
        {
            velocities.clear();
            for (char* s = argv[i + 1]; *s;)
            {
                char* end = s;
                const long v = std::strtol(s, &end, 10);
                if (end == s)
                    break;
                velocities.push_back(static_cast<int>(v));
                s = *end == ',' ? end + 1 : end;
            }
        }
    }
    std::vector<FactoryPreset> presets(factoryPresets.begin(), factoryPresets.end());
    if (constructions)
    {
        presets.clear();
        constexpr BodyShape shapes[] { BodyShape::Soprano, BodyShape::Concert, BodyShape::Tenor };
        constexpr BodyMaterial woods[] { BodyMaterial::Spruce, BodyMaterial::Mahogany,
                                         BodyMaterial::Koa, BodyMaterial::Cedar };
        constexpr const char* slugs[][4] {
            { "soprano-spruce", "soprano-mahogany", "soprano-koa", "soprano-cedar" },
            { "concert-spruce", "concert-mahogany", "concert-koa", "concert-cedar" },
            { "tenor-spruce", "tenor-mahogany", "tenor-koa", "tenor-cedar" }
        };
        for (std::size_t shape = 0; shape < std::size(shapes); ++shape)
            for (std::size_t wood = 0; wood < std::size(woods); ++wood)
            {
                auto preset = factoryPresets.front();
                preset.slug = slugs[shape][wood];
                preset.shape = shapes[shape];
                preset.bodyMaterial = woods[wood];
                presets.push_back(preset);
            }
    }
    const std::size_t pairs = presets.size() * velocities.size();
    std::vector<std::vector<Note>> results(pairs);
    parallel(pairs, jobs, [&](std::size_t job) {
        results[job] = play(presets[job / velocities.size()], velocities[job % velocities.size()], tab);
    });

    FILE* file = std::fopen(out.c_str(), "w");
    if (!file)
    {
        std::fprintf(stderr, "cannot write %s\n", out.c_str());
        return 1;
    }
    std::fprintf(file, "mode,preset,velocity,string,fret,note,level_db,peak_dbfs,level_200ms_db,level_1s_db,"
                       "decay_db_per_s,bright_db,centroid_hz,f0_hz,f0_cents_error,pitch_cents_error,spike_db,"
                       "pre_release_db,release_100ms_db,release_700ms_db\n");
    for (std::size_t job = 0; job < pairs; ++job)
    {
        const char* preset = presets[job / velocities.size()].slug;
        for (const auto& n : results[job])
            std::fprintf(file, "%s,%s,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%.3f,%.2f,%.2f,%.2f,"
                               "%.2f,%.2f,%.2f\n",
                         tab ? "strings" : "keys", preset, n.velocity, n.string, n.fret, n.note, n.level, n.peak,
                         n.level200, n.level1s, n.decay, n.bright, n.centroid, n.f0, n.cents, n.pitch, n.spike,
                         n.preRelease, n.release100ms, n.release700ms);
        summarise(preset, velocities[job % velocities.size()], results[job]);
    }
    std::fclose(file);
    return 0;
}
