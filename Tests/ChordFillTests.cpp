// A one-channel strum of three notes sounds all four strings, the free one
// fretted with a chord tone (AculeleEngine::chordFillNote, fillStroke) and
// the shape chosen for those four strings (AculeleEngine::planChord), and
// that string is held and let go with the chord's keys, never by a key of
// its own pitch. A chord's notes below the range are moved up into it
// (Performer::foldChord), a chord no shape holds is lifted an octave note by
// note until one does (Performer::liftChord), and the moved keys let go of
// the strings they sound.
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
    // A shape is chosen with its fill, by the four strings the stroke
    // sounds: F4 A4 C5 is 2013 with the open C, as C4 F4 A4 C5 is, not 5550
    // with the open A doubled.
    expectShape(Technique::Strum, { 65, 69, 72 }, { 2, 0, 1, 3 }, "F4 A4 C5 strum");
    expectShape(Technique::Strum, { 60, 65, 69, 72 }, { 2, 0, 1, 3 }, "C4 F4 A4 C5 strum");
    // Open strings lead the shape only in first position (the owner's rule,
    // AculeleEngine::firstPositionFret): E4 G4 B4 keeps its open G and E,
    // but G and D7 take the hand's lowest shape, as the chord books fret
    // them, and above first position the hand frets the free string too.
    expectShape(Technique::Strum, { 64, 67, 71 }, { 0, 4, 0, 2 }, "E4 G4 B4 strum");
    expectShape(Technique::Strum, { 71, 74, 79 }, { 7, 7, 7, 10 }, "B4 D5 G5 strum");
    expectShape(Technique::Strum, { 62, 66, 69, 72 }, { 2, 2, 2, 3 }, "D4 F#4 A4 C5 strum");
    expectShape(Technique::Strum, { 67, 72, 76 }, { 5, 7, 8, 7 }, "G4 C5 E5 strum");
    // The hand frets the fill too, so its stretch counts: Fdim is 4-5-4-2,
    // not 1-5-1-2, which spans four frets.
    expectShape(Technique::Strum, { 65, 68, 71 }, { 4, 5, 4, 2 }, "F4 G#4 B4 strum");
    // Two notes below E4 need the one C string: the lowest moves up an
    // octave (Performer::liftChord), as the chord books fret Cm, F7 and Ab7.
    expectShape(Technique::Strum, { 60, 63, 67 }, { 0, 3, 3, 3 }, "C4 Eb4 G4 strum");
    expectShape(Technique::Strum, { 60, 63, 65, 69 }, { 2, 3, 1, 3 }, "C4 Eb4 F4 A4 strum");
    expectShape(Technique::Strum, { 60, 63, 66, 68 }, { 1, 3, 2, 3 }, "C4 Eb4 F#4 G#4 strum");
    // Notes below the C string move up by octaves into reach
    // (Performer::foldChord): A3 C4 E4 is Am, C3 E3 G3 and C2 G3 E4 are C.
    expectShape(Technique::Strum, { 57, 60, 64 }, { 2, 0, 0, 0 }, "A3 C4 E4 strum");
    expectShape(Technique::Strum, { 48, 52, 55 }, { 0, 0, 0, 3 }, "C3 E3 G3 strum");
    expectShape(Technique::Strum, { 36, 55, 64 }, { 0, 0, 0, 3 }, "C2 G3 E4 strum");

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

    {
        // Cm lifted to Eb4 G4 C5: C4's key-ups let go of the C5 it sounds,
        // a second strike of the held key included, and nothing stays held.
        Player player(Technique::Strum);
        player.chord({ 60, 63, 67 });
        check(player.string(3).midiNote == 72, "C4 was not lifted to the A string's C5");
        player.send({ { 0x90, 60, 90 } });
        player.play(0.1);
        player.release(60);
        check(player.string(3).keyDown, "one of two C4 key-ups let go of C5");
        player.release(60);
        check(!player.string(3).keyDown, "C4's key-ups left its C5 held");
        player.release(63);
        player.release(67);
        for (int index = 0; index < 4; ++index)
            check(!player.string(index).keyDown, "a lifted chord left a string held");
    }

    {
        // A3 C4 E4 sounds A4 for A3 on the open A string, the fill A4 on
        // the G: A3's key-up lets go of the open A. A low note alone is
        // outside the range and sounds nothing.
        Player player(Technique::Strum);
        player.chord({ 57, 60, 64 });
        check(player.string(3).midiNote == 69 && player.string(3).keyDown,
              "A3 was not folded to the open A string");
        player.release(57);
        check(!player.string(3).keyDown, "A3's key-up left its A4 held");
        check(player.string(0).keyDown, "A3's key-up let go of the fill");
        player.release(60);
        player.release(64);
        for (int index = 0; index < 4; ++index)
            check(!player.string(index).keyDown, "a folded chord left a string held");
        Player alone(Technique::Strum);
        alone.chord({ 57 });
        for (int index = 0; index < 4; ++index)
            check(!alone.string(index).played, "A3 alone sounded");
    }

    {
        // All Notes Off forgets the lift: C4 alone afterwards is C4.
        Player player(Technique::Strum);
        player.chord({ 60, 63, 67 });
        player.send({ { 0xB0, 123, 0 } });
        player.chord({ 60 });
        check(player.string(1).keyDown && player.string(1).midiNote == 60,
              "C4 after All Notes Off did not sound C4");
        check(!player.string(3).keyDown, "C4 after All Notes Off sounded C5");
    }

    if (failures != 0)
    {
        std::fprintf(stderr, "%d chord fill checks failed\n", failures);
        return EXIT_FAILURE;
    }
    std::puts("every one-channel strum of three sounds four strings");
    return EXIT_SUCCESS;
}
