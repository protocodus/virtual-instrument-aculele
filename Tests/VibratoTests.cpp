// The modulation wheel's vibrato (AculeleEngine::vibratoSemitones).
//
// Every note plays at velocity 90 through the Performer, 48 kHz
// and 256-sample blocks, with the default controls and Finger: on channel 1,
// or as tab on a named string.
//
// - The wheel at zero sends the samples of a note with no wheel at all.
// - The full wheel swings a stopped note both ways about its own pitch, as
//   an axial vibrato does (Rodrigues 2014), 60 cents from top to bottom at
//   5.07 Hz from the 6th fret up and 16 cents at the 1st. C#5 on the G
//   string's 6th fret and F4 on the E string's 1st are tracked over
//   0.6-2.6 s with a 60 ms Hann window every 5 ms: each one's mean lies
//   within 3 cents of the same note's without the wheel (the one-sided swing
//   before 0.4.30 raised it 11 cents), C#5 swings 50-62 cents between its 2nd
//   and 98th percentiles (the window smooths the sine's 60 to about 56) and
//   F4 12-17, and each crosses its mean upward at 4.8-5.4 Hz.
// - With the wheel up, a hand stops what it can: C4 to A4 legato sounds no
//   open string but C4, which only the open C string reaches, and stays
//   within the first five frets.

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

std::unique_ptr<aculele::Performer> preparedPerformer()
{
    aculele::EngineParameters parameters;
    parameters.picking = aculele::PickingTechnique::Finger;
    auto performer = std::make_unique<aculele::Performer>();
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    return performer;
}

// One note held for the whole render; wheel < 0 sends no CC1 at all. A
// string >= 0 plays the note as tab on that string (CC126 = 4: channels 1-4
// are the G, C, E and A strings).
std::vector<float> renderNote(const aculele::Performer& prepared, int note, int wheel, double seconds,
                              int string = -1)
{
    auto performer = std::make_unique<aculele::Performer>(prepared);
    const auto total = static_cast<std::size_t>(seconds * rate);
    std::vector<float> left(total), right(total);
    for (std::size_t done = 0; done < total; done += block)
    {
        const auto frames = static_cast<int>(std::min<std::size_t>(block, total - done));
        performer->beginBlock(left.data() + done, right.data() + done, frames);
        if (done == 0)
        {
            const auto channel = static_cast<std::uint8_t>(string >= 0 ? string : 0);
            if (string >= 0)
            {
                const std::uint8_t tab[3] { 0xB0, 126, 4 };
                performer->handleMidi(0, tab, 3);
            }
            if (wheel >= 0)
            {
                const std::uint8_t modulation[3] { static_cast<std::uint8_t>(0xB0 | channel), 1,
                                                   static_cast<std::uint8_t>(wheel) };
                performer->handleMidi(0, modulation, 3);
            }
            const std::uint8_t on[3] { static_cast<std::uint8_t>(0x90 | channel),
                                       static_cast<std::uint8_t>(note), 90 };
            performer->handleMidi(0, on, 3);
        }
        performer->endBlock();
    }
    for (std::size_t i = 0; i < total; ++i)
        left[i] += right[i];
    return left;
}

// |X(f)| of the Hann-windowed segment, by a rotating phasor.
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

// The fundamental in cents from nominal: the peak within 60 cents in 1-cent
// steps, refined by a parabola through the log magnitudes.
double fundamentalCents(const std::vector<float>& x, std::size_t start, std::size_t length, double nominal)
{
    std::array<double, 121> levels {};
    std::size_t best = 0;
    for (std::size_t i = 0; i < levels.size(); ++i)
    {
        levels[i] = std::log(magnitude(x, start, length,
            nominal * std::exp2((static_cast<double>(i) - 60.0) / 1200.0)) + 1e-30);
        if (levels[i] > levels[best])
            best = i;
    }
    if (best == 0 || best + 1 == levels.size())
        return static_cast<double>(best) - 60.0;
    const double a = levels[best - 1], b = levels[best], c = levels[best + 1];
    return static_cast<double>(best) - 60.0 + 0.5 * (a - c) / (a - 2.0 * b + c);
}

std::vector<double> pitchTrack(const std::vector<float>& x, double nominal)
{
    const auto hop = static_cast<std::size_t>(0.005 * rate);
    const auto window = static_cast<std::size_t>(0.06 * rate);
    std::vector<double> track;
    for (auto start = static_cast<std::size_t>(0.6 * rate);
         start + window <= static_cast<std::size_t>(2.6 * rate); start += hop)
        track.push_back(fundamentalCents(x, start, window, nominal));
    return track;
}

double percentile(std::vector<double> values, double share)
{
    std::sort(values.begin(), values.end());
    return values[static_cast<std::size_t>(share * static_cast<double>(values.size() - 1))];
}

double mean(const std::vector<double>& values)
{
    double sum = 0.0;
    for (const double value : values)
        sum += value;
    return sum / static_cast<double>(values.size());
}
} // namespace

int main()
{
    const auto prepared = preparedPerformer();

    {
        const auto none = renderNote(*prepared, 65, -1, 1.0);
        const auto zero = renderNote(*prepared, 65, 0, 1.0);
        check(none == zero, "the wheel at zero changed the note", 0.0);
    }

    // F4 on the E string's 1st fret, where the swing is 16 cents, and C#5 on
    // the G string's 6th, where it is all 60 (vibratoSemitones). The window
    // smooths both by about 7%. Notes beside a body mode swing wider or
    // narrower, as the body pulls their normal plane (README, Known gaps), so
    // neither is near one. F#4 on the C string was the 6th-fret note until
    // 0.4.36: graduated tops moved Concert Mahogany's modes 3% down
    // (T1 466 -> 453 Hz) and its swing read 74 cents at 6.4 Hz; C#5, at
    // 554 Hz between T1 and the 684 Hz mode, swings 57.6 cents at 5.07 Hz.
    struct Swing { int note; int string; double low; double high; const char* what; };
    for (const Swing& test : { Swing { 65, 2, 12.0, 17.0, "the full wheel's swing at the 1st fret is not 16 cents" },
                               Swing { 73, 0, 50.0, 62.0, "the full wheel's swing at the 6th fret is not 60 cents" } })
    {
        const int note = test.note;
        const double nominal = 440.0 * std::exp2((note - 69) / 12.0);
        const auto still = pitchTrack(renderNote(*prepared, note, -1, 2.7, test.string), nominal);
        const auto swung = pitchTrack(renderNote(*prepared, note, 127, 2.7, test.string), nominal);
        const double offset = mean(swung) - mean(still);
        const double swing = percentile(swung, 0.98) - percentile(swung, 0.02);
        // Cycles between the first and the last upward crossing of the mean.
        const double centre = mean(swung);
        std::vector<double> crossings;
        for (std::size_t i = 1; i < swung.size(); ++i)
            if (swung[i - 1] < centre && swung[i] >= centre)
                crossings.push_back(0.005 * (static_cast<double>(i - 1)
                    + (centre - swung[i - 1]) / (swung[i] - swung[i - 1])));
        const double perSecond = crossings.size() < 2 ? 0.0
            : static_cast<double>(crossings.size() - 1) / (crossings.back() - crossings.front());
        std::printf("full wheel, note %d: mean %+.2f cents from the still note, swing %.1f cents, %.2f Hz\n",
                    note, offset, swing, perSecond);
        check(std::abs(offset) <= 3.0, "the full wheel moved the note's mean pitch", offset);
        check(swing >= test.low && swing <= test.high, test.what, swing);
        check(perSecond >= 4.8 && perSecond <= 5.4, "the full wheel's rate is not 5 Hz", perSecond);
    }

    {
        auto performer = std::make_unique<aculele::Performer>(*prepared);
        std::vector<float> left(block), right(block);
        const auto send = [&] (std::vector<std::array<std::uint8_t, 3>> messages, double seconds)
        {
            for (int done = 0; done < static_cast<int>(seconds * rate); done += block)
            {
                performer->beginBlock(left.data(), right.data(), block);
                if (done == 0)
                    for (const auto& message : messages)
                        performer->handleMidi(0, message.data(), 3);
                performer->endBlock();
            }
        };
        send({ { 0xB0, 1, 127 } }, 0.1);
        const std::array<int, 6> scale { 60, 62, 64, 65, 67, 69 };
        int previous = -1;
        for (const int note : scale)
        {
            std::vector<std::array<std::uint8_t, 3>> messages;
            if (previous >= 0)
                messages.push_back({ 0x80, static_cast<std::uint8_t>(previous), 64 });
            messages.push_back({ 0x90, static_cast<std::uint8_t>(note), 90 });
            send(messages, 0.2);
            const auto strings = performer->engine().getStringActivity();
            int fret = -1, string = -1;
            for (int index = 0; index < 4; ++index)
                if (strings[static_cast<std::size_t>(index)].keyDown
                    && strings[static_cast<std::size_t>(index)].midiNote == note)
                {
                    string = index;
                    fret = strings[static_cast<std::size_t>(index)].fret;
                }
            std::printf("wheel up, note %d: string %d fret %d\n", note, string, fret);
            check(note == 60 ? fret == 0 : fret > 0, "a note the hand could stop rang open under the wheel", note);
            check(fret <= 5, "a scale note left first position under the wheel", note);
            previous = note;
        }
    }

    if (failures == 0)
        std::printf("Vibrato: all checks passed\n");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
