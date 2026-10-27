#pragma once

// Reason's channel meter, the scale Aculele's levels are calibrated on.
//
// The Main Mixer's channel meter is a VU meter: RMS with 300 ms ballistics,
// read against the Big Meter's VU Offset (Reason 13 manual, "The Main Mixer"
// and "Hardware Interface"). The manual gives no default offset; Reason users
// report 12 dB, so 0 VU here is -12 dBFS RMS. A reading is the RMS over a
// sliding 300 ms window, of the louder channel, plus 12 dB.
//
// Playing is measured on one MIDI channel, so the Performer chooses the
// strings, at 48 kHz in 256-sample blocks and 120 bpm, each event alone in a
// 1.5 s slot and held for 0.8 s:
//  - typical playing: single notes every fourth semitone from the tuning's
//    lowest open string to an octave above its highest, and eight chords
//    (C, F, G and Am in first position; four four-note chords up the neck),
//    at velocities 70, 90 and 110. Its reading is the median over events of
//    each event's highest reading.
//  - the loudest chords: the same eight chords at velocity 127. Its reading
//    is the highest of them, and its sample peak is the output's.

#include "DSP/AculelePerformer.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace aculele::meter
{
inline constexpr double rate = 48000.0;
inline constexpr int block = 256;
inline constexpr double slotSeconds = 1.5;
inline constexpr double holdSeconds = 0.8;
inline constexpr double vuOffsetDb = 12.0;

struct Event
{
    std::vector<int> notes;
    int velocity;
};

inline const std::vector<std::vector<int>>& chords()
{
    static const std::vector<std::vector<int>> list {
        { 60, 64, 67 }, { 60, 65, 69 }, { 62, 67, 71 }, { 60, 64, 69 },
        { 64, 67, 72, 76 }, { 65, 69, 72, 77 }, { 67, 71, 74, 79 }, { 69, 72, 76, 81 } };
    return list;
}

inline std::vector<Event> typicalPlaying(Tuning tuning)
{
    const int lowest = tuning == Tuning::LowG ? 55 : tuning == Tuning::DTuning ? 62 : 60;
    const int highest = (tuning == Tuning::DTuning ? 71 : 69) + 12;
    std::vector<Event> events;
    for (const int velocity : { 70, 90, 110 })
    {
        for (int note = lowest; note <= highest; note += 4)
            events.push_back({ { note }, velocity });
        for (const auto& chord : chords())
            events.push_back({ chord, velocity });
    }
    return events;
}

inline std::vector<Event> loudestChords()
{
    std::vector<Event> events;
    for (const auto& chord : chords())
        events.push_back({ chord, 127 });
    return events;
}

struct Render
{
    std::vector<double> eventReadings; // each event's highest reading, VU
    double peakDbfs = -300.0;
};

// Plays the events through a fresh Performer at `parameters` and reads them.
inline Render render(const EngineParameters& parameters, const std::vector<Event>& events)
{
    const auto slot = static_cast<long long>(slotSeconds * rate);
    const auto hold = static_cast<long long>(holdSeconds * rate);
    const auto total = slot * static_cast<long long>(events.size());
    auto performer = std::make_unique<Performer>();
    performer->setParameters(parameters);
    performer->prepare(rate, block);
    performer->setParameters(parameters);
    performer->setTempoBpm(120.0);

    struct Key
    {
        long long at;
        bool on;
        int note, velocity;
    };
    std::vector<Key> keys;
    for (std::size_t index = 0; index < events.size(); ++index)
        for (const int note : events[index].notes)
        {
            const auto on = slot * static_cast<long long>(index);
            keys.push_back({ on, true, note, events[index].velocity });
            keys.push_back({ on + hold, false, note, 0 });
        }
    std::stable_sort(keys.begin(), keys.end(), [](const Key& a, const Key& b) { return a.at < b.at; });

    std::vector<float> left(static_cast<std::size_t>(total + block)), right(left.size());
    std::size_t next = 0;
    for (long long done = 0; done < total; done += block)
    {
        performer->beginBlock(left.data() + done, right.data() + done, block);
        for (; next < keys.size() && keys[next].at < done + block; ++next)
        {
            const auto offset = static_cast<int>(keys[next].at - done);
            if (keys[next].on)
                performer->noteOn(offset, 1, keys[next].note, keys[next].velocity);
            else
                performer->noteOff(offset, 1, keys[next].note);
        }
        performer->endBlock();
    }

    Render result;
    const auto window = static_cast<std::size_t>(0.3 * rate);
    double sumLeft = 0.0, sumRight = 0.0, peak = 0.0, highest = -300.0;
    for (std::size_t i = 0; i < static_cast<std::size_t>(total); ++i)
    {
        sumLeft += static_cast<double>(left[i]) * left[i];
        sumRight += static_cast<double>(right[i]) * right[i];
        if (i >= window)
        {
            sumLeft -= static_cast<double>(left[i - window]) * left[i - window];
            sumRight -= static_cast<double>(right[i - window]) * right[i - window];
        }
        const double meanSquare = std::max(std::max(sumLeft, sumRight) / static_cast<double>(window), 1.0e-30);
        highest = std::max(highest, 10.0 * std::log10(meanSquare) + vuOffsetDb);
        peak = std::max({ peak, static_cast<double>(std::abs(left[i])), static_cast<double>(std::abs(right[i])) });
        if ((i + 1) % static_cast<std::size_t>(slot) == 0)
        {
            result.eventReadings.push_back(highest);
            highest = -300.0;
        }
    }
    result.peakDbfs = 20.0 * std::log10(std::max(peak, 1.0e-15));
    return result;
}

inline double median(std::vector<double> values)
{
    std::sort(values.begin(), values.end());
    const auto middle = values.size() / 2;
    return values.size() % 2 ? values[middle] : 0.5 * (values[middle - 1] + values[middle]);
}

inline double typicalReading(const EngineParameters& parameters)
{
    return median(render(parameters, typicalPlaying(parameters.tuning)).eventReadings);
}
} // namespace aculele::meter
