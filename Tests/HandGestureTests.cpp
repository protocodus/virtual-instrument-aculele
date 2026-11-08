#include "DSP/AculelePerformer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

namespace
{
using aculele::Performer;
using aculele::AculeleEngine;
constexpr int rate = 48000;
constexpr std::array<int, 4> gestures { Performer::keyswitchStringBrush,
    Performer::keyswitchFretShift, Performer::keyswitchBodyTap, Performer::keyswitchMuteStrings };

int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition)
    {
        ++failures;
        std::fprintf(stderr, "FAIL: %s\n", message);
    }
}

struct Event
{
    int sample, status, note, velocity;
};

struct Audio
{
    std::vector<float> left, right, piezo;
};

std::unique_ptr<Performer> prepare(aculele::PickingTechnique picking = aculele::PickingTechnique::Finger)
{
    auto player = std::make_unique<Performer>();
    aculele::EngineParameters parameters;
    parameters.picking = picking;
    parameters.room = 0.0f;
    parameters.releaseNoise = 0.0f;
    player->setParameters(parameters);
    player->prepare(rate, 256);
    return player;
}

Audio render(Performer& player, const std::vector<Event>& events, int samples,
             int block = 64)
{
    Audio audio { std::vector<float>(samples), std::vector<float>(samples),
                  std::vector<float>(samples) };
    std::size_t next = 0;
    for (int start = 0; start < samples; start += block)
    {
        const int frames = std::min(block, samples - start);
        player.beginBlock(audio.left.data() + start, audio.right.data() + start,
                          { audio.piezo.data() + start }, frames);
        for (; next < events.size() && events[next].sample < start + frames; ++next)
        {
            const auto& event = events[next];
            const std::uint8_t bytes[] { static_cast<std::uint8_t>(event.status),
                static_cast<std::uint8_t>(event.note), static_cast<std::uint8_t>(event.velocity) };
            player.handleMidi(event.sample - start, bytes, 3);
        }
        player.endBlock();
    }
    return audio;
}

double rms(const std::vector<float>& samples, int first, int last)
{
    double energy = 0.0;
    for (int i = first; i < last; ++i)
        energy += static_cast<double>(samples[i]) * samples[i];
    return std::sqrt(energy / static_cast<double>(last - first));
}

bool finite(const std::vector<float>& samples)
{
    return std::all_of(samples.begin(), samples.end(),
        [](float sample) { return std::isfinite(sample); });
}

void oneShots()
{
    auto player = prepare();
    for (const int key : gestures)
    {
        const std::vector<Event> events { { 37, 0x93, key, 110 } };
        player->reset();
        check(player->engine().suspend(), "the idle engine can sleep before a gesture");
        const auto loud = render(*player, events, 12000);
        check(player->engine().getActiveVoiceCount() == 0, "a gesture takes no string voice");
        check(finite(loud.left) && finite(loud.right) && finite(loud.piezo), "gestures are finite");
        const int onset = events.front().sample + AculeleEngine::outputLatency;
        check(rms(loud.left, 0, onset) == 0.0 && rms(loud.piezo, 0, onset) == 0.0,
              "an idle gesture starts at its event plus the output pipeline");
        const auto mainRms = rms(loud.left, onset, 4800);
        const auto piezoRms = rms(loud.piezo, onset, 4800);
        std::printf("MIDI %d: Main %.1f dBFS, Piezo %.1f dBFS over 100 ms\n", key,
                    20.0 * std::log10(mainRms), 20.0 * std::log10(piezoRms));
        check(mainRms > 0.0005 && piezoRms > 0.0005,
              "every gesture wakes and sounds on both outputs with Release Noise at zero");
        check(rms(loud.left, 9600, 12000) < mainRms * 0.05,
              "the gesture is transient");

        player->reset();
        const auto withOff = render(*player,
            { events[0], { 83, 0x83, key, 100 }, { 101, 0x93, key, 0 } }, 12000, 113);
        check(withOff.left == loud.left && withOff.piezo == loud.piezo,
              "key-up leaves the one-shot intact and reset reproduces it across block sizes");
        player->reset();
        const auto soft = render(*player, { { 37, 0x93, key, 45 } }, 4800);
        check(rms(soft.left, onset, 4800) < mainRms * 0.55,
              "velocity changes the force of a hand contact");
    }
    player->reset();
    const auto stopped = render(*player, { { 0, 0x93, Performer::keyswitchFretShift, 100 },
                                           { 1000, 0xb3, 120, 0 } }, 4800);
    check(rms(stopped.left, 0, 1000) > 0.0 && rms(stopped.left, 1000, 4800) == 0.0,
          "All Sound Off stops a noise on its own channel");
}

void stopsAllStrings()
{
    auto player = prepare(aculele::PickingTechnique::Strum);
    player->setSustainHold(true);
    const std::vector<Event> chord { { 0, 0x90, 60, 100 }, { 0, 0x90, 64, 100 },
        { 0, 0x90, 67, 100 }, { 6000, 0x80, 60, 64 }, { 6000, 0x80, 64, 64 } };
    const auto tailWithoutMute = [&] {
        render(*player, chord, 12000);
        return render(*player, {}, 18000);
    }();
    player->reset();
    render(*player, chord, 12000);
    const auto before = player->engine().getStringActivity();
    check(std::any_of(before.begin(), before.end(), [](const auto& string) { return string.keyDown; })
        && std::any_of(before.begin(), before.end(), [](const auto& string) { return string.pedalHeld; }),
        "the mute test starts with held and pedal-latched strings");

    const auto attack = render(*player, { { 17, 0x95, Performer::keyswitchMuteStrings, 104 } }, 256);
    const auto after = player->engine().getStringActivity();
    check(std::none_of(after.begin(), after.end(),
                       [](const auto& string) { return string.keyDown || string.pedalHeld; }),
          "mute clears owners across MIDI channels");
    check(player->isSustainActive(), "mute leaves the Sustain control engaged");
    check(std::equal(attack.left.begin(), attack.left.begin() + 17, tailWithoutMute.left.begin()),
          "the string damping starts no earlier than the keyswitch event");
    const auto tail = render(*player, {}, 18000);
    const double normal = rms(tailWithoutMute.left, 9600, 14400);
    const double muted = rms(tail.left, 9600, 14400);
    std::printf("Held/sustained chord tail at 200–300 ms: %.1f dBFS open, %.1f dBFS muted\n",
                20.0 * std::log10(normal), 20.0 * std::log10(muted));
    check(normal > 0.001 && muted < normal * 0.056, "mute damps a ringing chord by at least 25 dB");

    render(*player, { { 0, 0x90, 64, 100 }, { 32, 0x80, 64, 64 } }, 4800);
    const auto replayed = player->engine().getStringActivity();
    check(std::any_of(replayed.begin(), replayed.end(),
                       [](const auto& string) { return string.midiNote == 64 && string.pedalHeld; }),
          "a new note can be played and sustain after the mute");
}

void scheduledNotes()
{
    auto player = prepare(aculele::PickingTechnique::Strum);
    player->setSustainHold(true);
    auto& engine = player->engine();
    // Cancel a delayed string strike even if a pedal caught the key-up.
    engine.noteOn(64, 0.8f, 1, 2000);
    engine.noteOff(64, 1);
    const auto cancelled = render(*player, { { 0, 0x90, Performer::keyswitchMuteStrings, 100 } }, 8000);
    player->reset();
    const auto alone = render(*player, { { 0, 0x90, Performer::keyswitchMuteStrings, 100 } }, 8000);
    check(rms(cancelled.left, 2400, 4800) < rms(alone.left, 0, 2400) * 0.1,
          "a strum member muted before its pluck stays silent");

    player->reset();
    player->setGatherChords(true);
    player->setFingerLookahead(true);
    render(*player, { { 0, 0x90, 60, 100 }, { 48, 0x92, Performer::keyswitchMuteStrings, 100 },
                      { 96, 0x90, 64, 100 } }, 6000, 113);
    check(engine.heldString(60, 1) < 0 && engine.heldString(64, 1) >= 0,
          "gathering and lookahead preserve a note after an intervening mute on another channel");

    player->reset();
    render(*player, { { 0, 0x90, 64, 100 }, { 0, 0x90, Performer::keyswitchMuteStrings, 100 } }, 6000);
    check(engine.heldString(64, 1) >= 0, "same-sample notes start after mute regardless of event order");
}
} // namespace

int main()
{
    oneShots();
    stopsAllStrings();
    scheduledNotes();
    return failures == 0 ? 0 : 1;
}
