// A one-channel strum of three notes sounds all four strings, the free one
// fretted with a chord tone (AculeleEngine::chordFillNote, fillStroke), and
// that string is held and let go with the chord's keys, never by a key of
// its own pitch.
//
// Each case plays the chord on channel 1 at velocity 90 through the
// Performer, 48 kHz and 256-sample blocks, with the default controls and the
// named technique, and reads the strings 0.2 s later, after the stroke.

#include "DSP/AculelePerformer.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <vector>

namespace
{
constexpr double rate = 48000.0;
constexpr int block = 256;

int failures = 0;

void check(bool condition, const char* what)
{
    if (! condition)
    {
        std::fprintf(stderr, "FAIL %s\n", what);
        ++failures;
    }
}

struct Player
{
    std::unique_ptr<aculele::Performer> performer = std::make_unique<aculele::Performer>();
    std::vector<float> left = std::vector<float>(block), right = std::vector<float>(block);

    Player(aculele::PickingTechnique picking, bool chordFill = true)
    {
        aculele::EngineParameters parameters;
        parameters.picking = picking;
        aculele::PerformanceRealism realism;
        realism.chordFill = chordFill;
        performer->engine().setPerformanceRealism(realism);
        performer->setParameters(parameters);
        performer->prepare(rate, block);
        performer->setParameters(parameters);
    }

    void send(std::initializer_list<std::array<std::uint8_t, 3>> messages)
    {
        performer->beginBlock(left.data(), right.data(), block);
        for (const auto& message : messages)
            performer->handleMidi(0, message.data(), 3);
        performer->endBlock();
    }

    void play(double seconds)
    {
        for (int done = 0; done < static_cast<int>(seconds * rate); done += block)
            send({});
    }

    void chord(std::initializer_list<int> notes)
    {
        performer->beginBlock(left.data(), right.data(), block);
        for (const int note : notes)
        {
            const std::uint8_t on[3] { 0x90, static_cast<std::uint8_t>(note), 90 };
            performer->handleMidi(0, on, 3);
        }
        performer->endBlock();
        play(0.2);
    }

    void release(int note) { send({ { 0x80, static_cast<std::uint8_t>(note), 64 } }); }

    [[nodiscard]] aculele::AculeleEngine::StringActivity string(int index) const
    {
        return performer->engine().getStringActivity()[static_cast<std::size_t>(index)];
    }

    // The frets of the strings that sound, G C E A, as tab writes them: -1
    // for a string that is not played.
    [[nodiscard]] std::array<int, 4> frets() const
    {
        std::array<int, 4> result {};
        for (int index = 0; index < 4; ++index)
            result[static_cast<std::size_t>(index)] = string(index).played ? string(index).fret : -1;
        return result;
    }
};

void expectShape(aculele::PickingTechnique picking, std::initializer_list<int> notes,
                 std::array<int, 4> expected, const char* name)
{
    Player player(picking);
    player.chord(notes);
    const auto frets = player.frets();
    std::printf("%-26s %d %d %d %d\n", name, frets[0], frets[1], frets[2], frets[3]);
    check(frets == expected, name);
    for (int index = 0; index < 4; ++index)
        check(player.string(index).keyDown && player.string(index).level > 0.0f, name);
}
} // namespace

int main()
{
    using Technique = aculele::PickingTechnique;
    expectShape(Technique::Strum, { 60, 64, 67 }, { 0, 0, 0, 3 }, "C4 E4 G4 strum");
    expectShape(Technique::Strum, { 60, 64, 69 }, { 2, 0, 0, 0 }, "C4 E4 A4 strum");
    expectShape(Technique::Strum, { 62, 66, 69 }, { 2, 2, 2, 0 }, "D4 F#4 A4 strum");
    expectShape(Technique::Strum, { 62, 67, 71 }, { 0, 2, 3, 2 }, "D4 G4 B4 strum");
    expectShape(Technique::Pick, { 60, 64, 67 }, { 0, 0, 0, 3 }, "C4 E4 G4 pick sweep");

    {
        // Fingers pluck the three strings they are given; nothing sweeps the
        // fourth. Without the fill a strum passes the free string by, as
        // Acustra's does a string outside its span.
        Player finger(Technique::Finger);
        finger.chord({ 60, 64, 67 });
        check(!finger.string(3).played, "finger chord left its free string alone");
        Player off(Technique::Strum, false);
        off.chord({ 60, 64, 67 });
        check(!off.string(3).played, "chordFill off sounded the free string");
    }

    {
        // D4 G4 B4: the fill doubles G4 on the E string in unison with the
        // open G. G4's Note Off lets go of the open G it owns, not the fill;
        // the fill is let go with the last of the chord's keys.
        Player player(Technique::Strum);
        player.chord({ 62, 67, 71 });
        player.release(67);
        check(!player.string(0).keyDown, "G4's key-up left its own string held");
        check(player.string(2).keyDown, "G4's key-up let go of the fill");
        player.release(62);
        check(player.string(2).keyDown, "the fill went before the last key");
        player.release(71);
        check(!player.string(2).keyDown && !player.string(2).pedalHeld,
              "the fill outlived the chord's keys");
    }

    {
        // Under the pedal the fill is held on as the keys are, until pedal-up.
        Player player(Technique::Strum);
        player.send({ { 0xB0, 64, 127 } });
        player.chord({ 60, 64, 67 });
        player.release(60);
        player.release(64);
        player.release(67);
        check(!player.string(3).keyDown && player.string(3).pedalHeld,
              "the pedal did not hold the fill");
        player.send({ { 0xB0, 64, 0 } });
        check(!player.string(3).pedalHeld, "pedal-up kept the fill");
    }

    {
        // C's keys up, then F4 A4 C5: the new shape's fill is an F chord
        // tone too.
        Player player(Technique::Strum);
        player.chord({ 60, 64, 67 });
        player.release(60);
        player.release(64);
        player.release(67);
        player.chord({ 65, 69, 72 });
        const auto frets = player.frets();
        std::printf("%-26s %d %d %d %d\n", "C, then F4 A4 C5 strum", frets[0], frets[1], frets[2], frets[3]);
        for (int index = 0; index < 4; ++index)
        {
            const int pitchClass = player.string(index).midiNote % 12;
            check(player.string(index).keyDown
                      && (pitchClass == 5 || pitchClass == 9 || pitchClass == 0),
                  "F4 A4 C5 sounded a string outside the chord");
        }
    }

    {
        // F4 A4 C5 while C's keys are still down: the new shape replaces the
        // hand, C's fill among the strings it takes, and every string is let
        // go once both chords' keys are up.
        Player player(Technique::Strum);
        player.chord({ 60, 64, 67 });
        player.chord({ 65, 69, 72 });
        const auto frets = player.frets();
        std::printf("%-26s %d %d %d %d\n", "C held, F4 A4 C5 strum", frets[0], frets[1], frets[2], frets[3]);
        int held = 0;
        for (int index = 0; index < 4; ++index)
            held += player.string(index).keyDown ? 1 : 0;
        check(held == 4, "a chord change left more or fewer than four strings held");
        player.release(65);
        player.release(69);
        player.release(72);
        player.release(60);
        player.release(64);
        player.release(67);
        for (int index = 0; index < 4; ++index)
            check(!player.string(index).keyDown, "a string stayed held after every key-up");
    }

    if (failures != 0)
    {
        std::fprintf(stderr, "%d chord fill checks failed\n", failures);
        return EXIT_FAILURE;
    }
    std::puts("every one-channel strum of three sounds four strings");
    return EXIT_SUCCESS;
}
