// A sequencer ends a note on the sample the next one starts. The Performer
// lets the held keys of that sample come up before its Note Ons
// (Performer::flushNoteGroup), so the new notes find the strings a player's
// lifted fingers leave free: chords back to back keep all four strings, and
// a line stays in position instead of hopping to strings still held. A
// zero-length note, a key struck again and a connected gesture's source
// keep their order.
//
// Everything plays on channel 1 at velocity 90 through the Performer, 48 kHz
// and 256-sample blocks, with the default controls and the named technique.
// The strings are read 0.2 s after each stroke.

#include "DSP/AculelePerformer.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

    explicit Player(aculele::PickingTechnique picking)
    {
        aculele::EngineParameters parameters;
        parameters.picking = picking;
        performer->setParameters(parameters);
        performer->prepare(rate, block);
        performer->setParameters(parameters);
    }

    // Key-ups, controllers and Note Ons on one sample, in the order given.
    void send(const std::vector<std::array<std::uint8_t, 3>>& messages)
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

    // The previous notes' key-ups and these notes' Note Ons on one sample,
    // or the key-ups a block earlier.
    void change(const std::vector<int>& from, const std::vector<int>& to, bool gap)
    {
        std::vector<std::array<std::uint8_t, 3>> messages;
        for (const int note : from)
            messages.push_back({ 0x80, static_cast<std::uint8_t>(note), 64 });
        if (gap)
        {
            send(messages);
            messages.clear();
        }
        for (const int note : to)
            messages.push_back({ 0x90, static_cast<std::uint8_t>(note), 90 });
        send(messages);
        play(0.2);
    }

    [[nodiscard]] aculele::AculeleEngine::StringActivity string(int index) const
    {
        return performer->engine().getStringActivity()[static_cast<std::size_t>(index)];
    }

    // The held strings' frets, G C E A, as tab writes them: -1 for a string
    // no key or chord holds.
    [[nodiscard]] std::array<int, 4> frets() const
    {
        std::array<int, 4> result {};
        for (int index = 0; index < 4; ++index)
            result[static_cast<std::size_t>(index)]
                = string(index).keyDown ? string(index).fret : -1;
        return result;
    }

    // The string a lone held note sounds on, as string * 100 + fret.
    [[nodiscard]] int heldPlace(int note) const
    {
        for (int index = 0; index < 4; ++index)
            if (string(index).keyDown && string(index).midiNote == note)
                return index * 100 + string(index).fret;
        return -1;
    }
};
} // namespace

int main()
{
    using Technique = aculele::PickingTechnique;
    {
        // C, Am, F, G, C and Am again, each chord's keys up on the next one's
        // sample: the same full shapes as with the key-ups a block before.
        const std::vector<std::vector<int>> chords {
            { 60, 64, 67 }, { 60, 64, 69 }, { 60, 65, 69 },
            { 62, 67, 71 }, { 60, 64, 67 }, { 64, 69, 72 } };
        const std::array<std::array<int, 4>, 6> shapes {
            { { 0, 0, 0, 3 }, { 2, 0, 0, 0 }, { 2, 0, 1, 0 },
              { 0, 2, 3, 2 }, { 0, 0, 0, 3 }, { 2, 0, 0, 3 } } };
        for (const bool gap : { false, true })
        {
            Player player(Technique::Strum);
            std::vector<int> previous;
            for (std::size_t index = 0; index < chords.size(); ++index)
            {
                player.change(previous, chords[index], gap);
                const auto frets = player.frets();
                std::printf("%s chord %zu: %d %d %d %d\n", gap ? "gap " : "same",
                            index + 1, frets[0], frets[1], frets[2], frets[3]);
                check(frets == shapes[index], gap ? "a chord after a gap lost its shape"
                                                  : "a chord on its predecessor's key-ups lost its shape");
                previous = chords[index];
            }
        }
    }

    {
        // C4 to D5 legato, each key up as the next goes down: first
        // position, then up the A string, as a player fingers the scale.
        const std::array<int, 9> scale { 60, 62, 64, 65, 67, 69, 71, 72, 74 };
        const std::array<int, 9> places { 100, 102, 200, 201, 0, 300, 302, 303, 305 };
        Player player(Technique::Finger);
        std::vector<int> previous;
        for (std::size_t index = 0; index < scale.size(); ++index)
        {
            player.change(previous, { scale[index] }, false);
            std::printf("scale note %d: string %d fret %d\n", scale[index],
                        player.heldPlace(scale[index]) / 100, player.heldPlace(scale[index]) % 100);
            check(player.heldPlace(scale[index]) == places[index], "a scale note left first position");
            previous = { scale[index] };
        }
    }

    {
        // A zero-length note still sounds and is let go; a key that ends
        // where it starts again stays held, and its last key-up lets it go.
        Player player(Technique::Finger);
        player.send({ { 0x80, 60, 64 }, { 0x90, 60, 90 } });
        player.play(0.2);
        check(player.string(1).played && !player.string(1).keyDown,
              "a zero-length note stayed held or never sounded");
        player.change({}, { 64 }, false);
        player.change({ 64 }, { 64 }, false);
        player.change({ 64 }, { 64 }, false);
        check(player.heldPlace(64) == 200, "a repeated key left its string");
        player.send({ { 0x80, 64, 64 } });
        for (int index = 0; index < 4; ++index)
            check(!player.string(index).keyDown, "a repeated key stayed held");
    }

    {
        // A connected gesture (CC65 on, CC84 naming the source) whose
        // source's key comes up on the target's sample still refrets the
        // source's string: G4 to A4 hammers onto the G string's second fret,
        // where a new note would take the open A.
        Player player(Technique::Finger);
        player.send({ { 0xB0, 65, 127 }, { 0x90, 67, 90 } });
        player.play(0.2);
        player.send({ { 0xB0, 84, 67 }, { 0x80, 67, 64 }, { 0x90, 69, 90 } });
        player.play(0.2);
        check(player.heldPlace(69) == 2, "a hammer-on on its source's key-up was replucked");
        player.send({ { 0x80, 69, 64 } });
        for (int index = 0; index < 4; ++index)
            check(!player.string(index).keyDown, "a hammer-on left a string held");
    }

    if (failures != 0)
    {
        std::fprintf(stderr, "%d note change checks failed\n", failures);
        return EXIT_FAILURE;
    }
    std::puts("notes that end where the next begins play as a player's hand plays them");
    return EXIT_SUCCESS;
}
