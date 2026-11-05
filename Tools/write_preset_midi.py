#!/usr/bin/env python3
"""Write the preset demo phrases to Docs/audio/midi/<NN>-<slug>.mid.

One phrase per preset in Source/DSP/AculelePresets.h, written for what that
preset is for. Each phrase's last event falls 5.5-9.5 s in, so with the demo
renderer's 2.5 s tail every demo is 8-12 s long.

Most phrases are written as ukulele tab: Mono Mode On (CC126 = 4) on
channel 1 makes channels 1-4 the G, C, E and A strings, so a chord names the
string of every note and can double a pitch, as G (0232) and Am (2000) do.
A group of notes on the string channels at one moment is one stroke, timed
across the strings it names. Strum directions are latched with the
keyswitches C0 (down) and C#0 (up) before each stroke, as a player's hand
keeps its down-up motion through the skipped strokes. The picked melody
plays on one channel and lets the fretting hand choose its strings.

Docs/audio/midi/songs/ holds longer public-domain pieces played with a
person's timing (Performance, below), one file per part. They are not
preset demos: Tools/render_songs.py plays each part on the ukulele SONGS
gives it and mixes them.

    python3 Tools/write_preset_midi.py           # write the sources, remove stale ones
    python3 Tools/write_preset_midi.py --check   # fail unless the committed files match
"""

import argparse
import math
import pathlib
import random
import re
import struct
import sys

PPQ = 480
ROOT = pathlib.Path(__file__).resolve().parent.parent
MIDI_DIR = ROOT / "Docs/audio/midi"
SONG_DIR = MIDI_DIR / "songs"

DOWN, UP = 24, 25  # Performer::keyswitchDownstroke, keyswitchUpstroke
CHUCK = 29  # Performer::keyswitchChuck: held down, a swept stroke's strings take dead strokes
# What a phrase may play: the stroke keyswitches C0-D0, and the strings from
# the Low G string's G3 to the A string's 18th fret.
KEYSWITCHES = range(24, 27)
STRING_NOTES = range(55, 69 + 18 + 1)

# Open strings, G C E A as channels 0-3, and chord shapes as frets.
STANDARD = (67, 60, 64, 69)
LOW_G = (55, 60, 64, 69)
D_TUNING = (69, 62, 66, 71)
SHAPES = {
    "C": (0, 0, 0, 3), "Am": (2, 0, 0, 0), "F": (2, 0, 1, 0), "G": (0, 2, 3, 2),
    "G7": (0, 2, 1, 2), "Em": (0, 4, 3, 2), "Dm": (2, 2, 1, 0), "D7": (2, 2, 2, 3),
    "C7": (0, 0, 0, 1), "E7": (1, 2, 0, 2), "B7": (2, 3, 2, 2), "B": (4, 3, 2, 2),
}


class Phrase:
    def __init__(self, bpm, tab=True):
        self.bpm = bpm
        self.events = []  # (tick, order, bytes); note-offs sort first
        if tab:
            self.events.append((0, 0, bytes([0xB0, 126, 4])))

    def ticks(self, beats):
        return round(beats * PPQ)

    def note(self, start, length, note, velocity, channel=0):
        self.events.append((self.ticks(start), 1, bytes([0x90 | channel, note, velocity])))
        self.events.append((self.ticks(start + length), 0, bytes([0x80 | channel, note, 0])))

    def keyswitch(self, at, key):
        self.events.append((self.ticks(at), 1, bytes([0x90, key, 100])))
        self.events.append((self.ticks(at) + 1, 0, bytes([0x80, key, 0])))

    def cc(self, at, number, value):
        self.events.append((self.ticks(at), 0, bytes([0xB0, number, value])))

    def stroke(self, start, length, tuning, shape, velocity, direction=None, strings=range(4)):
        """One stroke of a chord shape across the named strings."""
        if direction is not None:
            self.keyswitch(start, direction)
        for string in strings:
            self.note(start, length, tuning[string] + SHAPES[shape][string], velocity, string)

    def pluck(self, start, length, tuning, string, fret, velocity):
        self.note(start, length, tuning[string] + fret, velocity, string)

    def smf(self):
        tempo = round(60_000_000 / self.bpm)
        track = bytearray()
        track += b"\x00\xff\x51\x03" + tempo.to_bytes(3, "big")
        track += b"\x00\xff\x58\x04\x04\x02\x18\x08"
        last = 0
        for tick, _, data in sorted(self.events, key=lambda e: (e[0], e[1])):
            track += vlq(tick - last) + data
            last = tick
        track += b"\x00\xff\x2f\x00"
        return (b"MThd" + struct.pack(">IHHH", 6, 0, 1, PPQ)
                + b"MTrk" + struct.pack(">I", len(track)) + bytes(track))


def vlq(value):
    out = [value & 0x7F]
    value >>= 7
    while value:
        out.append(0x80 | (value & 0x7F))
        value >>= 7
    return bytes(reversed(out))


def strum_bars(p, tuning, chords, pattern, start=0.0):
    """pattern: (eighth, direction, velocity) per stroke; each stroke rings
    until just before the next one, as a strummed chord does."""
    strokes = [(start + 4 * bar + 0.5 * eighth, name, direction, velocity)
               for bar, name in enumerate(chords) for eighth, direction, velocity in pattern]
    for index, (at, name, direction, velocity) in enumerate(strokes):
        following = strokes[index + 1][0] if index + 1 < len(strokes) else start + 4 * len(chords)
        p.stroke(at, following - at - 0.05, tuning, name, velocity, direction)
    return start + 4 * len(chords)


# The island strum: D, D-U, U-D-U, the hand moving down-up on every eighth.
ISLAND = [(0, DOWN, 100), (2, DOWN, 92), (3, UP, 70), (5, UP, 74), (6, DOWN, 90), (7, UP, 70)]


def concert_strum():
    # The default sound: the island strum on the four chords every player
    # learns first, with the hand's own down-up timing.
    p = Phrase(120)
    end = strum_bars(p, STANDARD, ["C", "Am", "F", "G7"], ISLAND)
    p.stroke(end, 2.0, STANDARD, "C", 104, DOWN)
    return p


def soprano_d_tuning():
    # The soprano's old "my dog has fleas" tuning a tone up, brighter and
    # tighter: a quick calypso strum, D-D-U-U-D-U, in D.
    p = Phrase(124)
    calypso = [(0, DOWN, 104), (2, DOWN, 96), (3, UP, 76), (5, UP, 80), (6, DOWN, 94), (7, UP, 74)]
    # In D tuning the C shape is D, F is G, G7 is A7 and Am is Bm.
    end = strum_bars(p, D_TUNING, ["C", "F", "G7", "C"], calypso)
    p.stroke(end, 3.0, D_TUNING, "C", 108, DOWN)
    return p


def ring(p, tuning, events, longest):
    """Plucks (beat, string, fret, velocity); each note rings until its own
    string is played again, or for at most longest beats."""
    for index, (beat, string, fret, velocity) in enumerate(events):
        later = [b for b, s, _, _ in events[index + 1:] if s == string and b > beat]
        length = min(later[0] - beat - 0.05, longest) if later else longest
        p.pluck(beat, length, tuning, string, fret, velocity)


def tenor_fluorocarbon_fingerstyle():
    # Campanella: a melody spread across the re-entrant strings so each note
    # rings into the next, the ukulele's harp-like fingerstyle.
    p = Phrase(92)
    # (beat, string, fret) per eighth; strings G C E A are 0-3.
    line = [
        (0.0, 1, 0), (0.5, 0, 0), (1.0, 2, 0), (1.5, 3, 3), (2.0, 0, 0), (2.5, 3, 0), (3.0, 2, 0), (3.5, 0, 0),
        (4.0, 1, 0), (4.5, 0, 2), (5.0, 2, 0), (5.5, 3, 0), (6.0, 0, 2), (6.5, 3, 2), (7.0, 2, 3), (7.5, 0, 2),
        (8.0, 1, 2), (8.5, 0, 0), (9.0, 2, 1), (9.5, 3, 2), (10.0, 0, 0), (10.5, 3, 0), (11.0, 2, 3),
    ]
    ring(p, STANDARD, [(beat, string, fret, 92 if beat % 2 == 0 else 78)
                       for beat, string, fret in line], 2.5)
    p.stroke(12, 2.4, STANDARD, "C", 88)
    return p


def low_g_tenor():
    # Chord melody over the low G string's bass: the thumb and fingers pinch
    # the chord, then the melody moves on the top strings above it.
    p = Phrase(96)
    events = [
        # G: G3 D4 G4 B4, melody D5 C5 B4 A4
        (0, 0, 0, 96), (0, 1, 2, 80), (0, 2, 3, 82), (0, 3, 2, 92),
        (1, 3, 5, 90), (2, 0, 0, 86), (2, 3, 3, 88), (3, 3, 2, 86), (3.5, 3, 0, 80),
        # Em: G3 E4 G4 B4, melody C5 B4 G4 F#4
        (4, 0, 0, 94), (4, 1, 4, 80), (4, 2, 3, 82), (4, 3, 2, 90),
        (5, 3, 3, 88), (6, 0, 0, 84), (6, 3, 2, 86), (7, 2, 3, 84), (7.5, 2, 2, 78),
        # C over its root on the low G string, then D7 over A3
        (8, 0, 5, 96), (8, 1, 0, 80), (8, 2, 0, 82), (8, 3, 3, 92), (9, 2, 3, 84),
        (10, 0, 2, 92), (10, 1, 2, 80), (10, 2, 2, 82), (10, 3, 3, 90),
        (11, 3, 2, 86), (11.5, 3, 0, 80),
        # G
        (12, 0, 0, 98), (12, 1, 2, 82), (12, 2, 3, 84), (12, 3, 2, 92),
    ]
    ring(p, LOW_G, events, 2.5)
    return p


def soprano_thumb():
    # A slow ballad brushed with the thumb, every stroke a soft downstroke:
    # an accented stroke and a lighter one on each chord.
    p = Phrase(84)
    for index, name in enumerate(["C", "Em", "Am", "F"]):
        p.stroke(2 * index, 0.95, STANDARD, name, 92)
        p.stroke(2 * index + 1, 0.95, STANDARD, name, 72)
    p.stroke(8, 4.0, STANDARD, "C", 96)
    return p


def tenor_picked_melody():
    # A single-note line with a felt pick on low-G tenor: the fretting hand
    # chooses the strings, as on a guitar.
    p = Phrase(108, tab=False)
    line = [(0, 55), (0.5, 59), (1, 62), (1.5, 64), (2, 67), (3, 66), (3.5, 64), (4, 62),
            (5, 59), (5.5, 62), (6, 64), (6.5, 67), (7, 69), (7.5, 71), (8, 74), (9, 71),
            (9.5, 69), (10, 67)]
    for index, (beat, note) in enumerate(line):
        following = line[index + 1][0] if index + 1 < len(line) else beat + 3
        p.note(beat, following - beat - 0.08, note, 100 if beat % 1 == 0 else 84)
    p.note(12, 3.0, 55, 96)
    p.note(12, 3.0, 62, 90)
    p.note(12, 3.0, 67, 90)
    p.note(12, 3.0, 71, 90)
    return p


def concert_piezo_stage():
    # A syncopated pop strum through the saddle pickup, the off-beats choked
    # by the strumming hand's palm (CC2, the bridge hand) as a "chuck".
    p = Phrase(112)
    t = 0.0
    for name in ["Am", "F", "G"]:
        for eighth, direction, velocity, chuck in [(0, DOWN, 104, False), (2, DOWN, 96, True),
                                                   (3, UP, 74, False), (5, UP, 78, False),
                                                   (6, DOWN, 96, True), (7, UP, 74, False)]:
            at = t + 0.5 * eighth
            p.cc(at, 2, 110 if chuck else 0)
            p.stroke(at, 0.2 if chuck else 0.45, STANDARD, name, velocity, direction)
        t += 4
    p.cc(t, 2, 0)
    p.stroke(t, 2.5, STANDARD, "Am", 104, DOWN)
    return p


# ---- Songs ---------------------------------------------------------------
#
# Longer public-domain pieces, played as a person plays them rather than as
# a sequencer does. The score is written in beats; Performance plays it:
# the tempo breathes along a curve of (beat, bpm) points, every onset lands
# a few milliseconds off the grid, a chord's notes are struck as one gesture
# and velocities wander a little around the line the score shapes. A fixed
# seed makes every run write the same bytes.


LEAD_IN = 0.3  # seconds


class Performance:
    def __init__(self, tempo, seed, timing=0.007, dynamics=4.0, feel=None):
        self.tempo = sorted(tempo)  # [(beat, bpm)], linear between, held past the ends
        self.random = random.Random(seed)
        self.seed = seed
        self.timing = timing        # seconds, the onset scatter's standard deviation
        self.dynamics = dynamics    # velocity steps, likewise
        self.events = []            # (seconds, order, bytes, voice); note-offs sort first
        self.voice = None           # the part being written; each is its own file
        # Each player's own clock, where a song gives one: {voice: (lean,
        # wander)} in seconds. A player sits a fixed lean behind the beat
        # (ahead when negative) and wanders around it slowly, so two parts
        # pull apart and come back together as players in one room do.
        self.feel = feel or {}
        self.walks = {}

    def drift(self, beat):
        """The current voice's lean plus its wander at `beat`: a random walk
        sampled once a beat, pulled back towards zero, whose spread settles
        at `wander`. Each voice draws from its own seeded stream, so adding
        or reordering a part leaves the others' timing alone."""
        lean, wander = self.feel.get(self.voice, (0.0, 0.0))
        if not wander:
            return lean
        stream, walk = self.walks.setdefault(self.voice, (random.Random(f"{self.seed}:{self.voice}"), [0.0]))
        beat = max(0.0, beat)
        while len(walk) < beat + 2:
            walk.append(0.8 * walk[-1] + stream.gauss(0.0, 0.6 * wander))
        index = int(beat)
        return lean + walk[index] + (walk[index + 1] - walk[index]) * (beat - index)

    def bpm(self, beat):
        return interpolate(self.tempo, beat)

    def seconds(self, beat):
        """Clock time of a written beat: the integral of 60 / bpm, after a
        moment's silence before the first note."""
        total, previous = LEAD_IN, 0.0
        for b, _ in self.tempo[1:] + [(math.inf, 0)]:
            end = min(beat, b)
            if end > previous:
                t0, t1 = self.bpm(previous), self.bpm(end)
                slope = (t1 - t0) / (end - previous)
                total += (60.0 * (end - previous) / t0 if abs(slope) < 1e-9
                          else 60.0 / slope * math.log(t1 / t0))
                previous = end
            if beat <= b:
                break
        return total

    def scatter(self, scale=1.0):
        deviation = self.timing * scale
        return max(-2.5 * deviation, min(2.5 * deviation, self.random.gauss(0.0, deviation)))

    def velocity(self, value):
        return max(1, min(127, round(value + self.random.gauss(0.0, self.dynamics))))

    def at(self, beat, lean=0.0, scale=1.0):
        """A played onset: the written beat on the breathing clock, plus a
        deliberate lean in seconds and the hand's scatter."""
        return max(0.0, self.seconds(beat) + lean + self.drift(beat) + self.scatter(scale))

    def message(self, seconds, data, order=1):
        self.events.append((seconds, order, bytes(data), self.voice))

    def play(self, onset, beats_held, beat, note, velocity, channel=0):
        """One note struck at `onset` seconds and let go `beats_held` written
        beats after `beat`, with the release scattered too."""
        release = max(onset + 0.04, self.seconds(beat + beats_held) + self.drift(beat + beats_held)
                      + self.scatter(0.6))
        self.message(onset, (0x90 | channel, note, self.velocity(velocity)))
        self.message(release, (0x80 | channel, note, 0), 0)

    def smf(self, voice=None):
        """One part as a format 0 file at the piece's opening tempo, so it
        sits roughly on a DAW's bars; the performance's own time is in the
        ticks."""
        bpm = self.tempo[0][1]
        events = sorted((e[:3] for e in self.events if e[3] == voice), key=lambda e: (e[0], e[1]))
        # A key struck again before its last release lets go just before.
        struck = {}
        trimmed = []
        for index in range(len(events) - 1, -1, -1):
            seconds, order, data = events[index]
            key = (data[0] & 0x0F, data[1])
            if data[0] & 0xF0 == 0x90:
                struck[key] = seconds
            elif data[0] & 0xF0 == 0x80 and key in struck and seconds >= struck[key]:
                seconds = struck[key] - 0.004
            trimmed.append((seconds, order, data))
        events = sorted(trimmed, key=lambda e: (e[0], e[1]))
        ticks_per_second = PPQ * bpm / 60.0
        tempo = round(60_000_000 / bpm)
        track = bytearray(b"\x00\xff\x51\x03" + tempo.to_bytes(3, "big"))
        last = 0
        for seconds, _, data in events:
            tick = max(last, round(seconds * ticks_per_second))
            track += vlq(tick - last) + data
            last = tick
        track += b"\x00\xff\x2f\x00"
        return (b"MThd" + struct.pack(">IHHH", 6, 0, 1, PPQ)
                + b"MTrk" + struct.pack(">I", len(track)) + bytes(track))


def interpolate(points, x):
    """Linear between sorted (x, y) points, held past the ends."""
    if x <= points[0][0]:
        return points[0][1]
    for (x0, y0), (x1, y1) in zip(points, points[1:]):
        if x <= x1:
            return y0 + (y1 - y0) * (x - x0) / (x1 - x0)
    return points[-1][1]


def breathe(bpm, phrases, swell=0.03, settle=0.07, last=0.22):
    """A tempo curve over phrases of (first beat, length in beats): each
    pushes on a little towards its middle and settles at its end; the last
    one broadens into the close."""
    points = []
    for index, (start, length) in enumerate(phrases):
        final = index == len(phrases) - 1
        # After a breath the hand takes the tempo up again within a beat.
        points += [(start if index == 0 else start + 0.75, bpm), (start + 0.55 * length, bpm * (1 + swell)),
                   (start + 0.85 * length, bpm), (start + length, bpm * (1 - (last if final else settle)))]
    return points


def ode_to_joy():
    """Beethoven's Ode to Joy (Symphony No. 9, 1824) for three ukuleles. A
    spruce concert ukulele picks the tune; a mahogany tenor strums the
    chords, a soft downstroke on each half bar the first time and the island
    strum the second; and the second time a cedar soprano joins, thumbing a
    third below the tune. The second time is a little faster and louder,
    and its last phrase broadens into a slow C."""
    bars = [
        [(0, 76, 1), (1, 76, 1), (2, 77, 1), (3, 79, 1)], [(0, 79, 1), (1, 77, 1), (2, 76, 1), (3, 74, 1)],
        [(0, 72, 1), (1, 72, 1), (2, 74, 1), (3, 76, 1)], [(0, 76, 1.5), (1.5, 74, 0.5), (2, 74, 2)],
        [(0, 76, 1), (1, 76, 1), (2, 77, 1), (3, 79, 1)], [(0, 79, 1), (1, 77, 1), (2, 76, 1), (3, 74, 1)],
        [(0, 72, 1), (1, 72, 1), (2, 74, 1), (3, 76, 1)], [(0, 74, 1.5), (1.5, 72, 0.5), (2, 72, 2)],
        [(0, 74, 1), (1, 74, 1), (2, 76, 1), (3, 72, 1)], [(0, 74, 1), (1, 76, 0.5), (1.5, 77, 0.5), (2, 76, 1), (3, 72, 1)],
        [(0, 74, 1), (1, 76, 0.5), (1.5, 77, 0.5), (2, 76, 1), (3, 74, 1)], [(0, 72, 1), (1, 74, 1), (2, 67, 2)],
        [(0, 76, 1), (1, 76, 1), (2, 77, 1), (3, 79, 1)], [(0, 79, 1), (1, 77, 1), (2, 76, 1), (3, 74, 1)],
        [(0, 72, 1), (1, 72, 1), (2, 74, 1), (3, 76, 1)], [(0, 74, 1.5), (1.5, 72, 0.5), (2, 72, 2)],
    ]
    # The chord under each half bar.
    harmony = [("C", "C"), ("G", "G"), ("C", "C"), ("G", "G"), ("C", "C"), ("G", "G"), ("C", "C"), ("G", "C"),
               ("G", "C"), ("G", "C"), ("G", "Am"), ("D7", "G"), ("C", "C"), ("G", "G"), ("C", "C"), ("G", "C")]
    c_major = [n for n in range(55, 90) if n % 12 in (0, 2, 4, 5, 7, 9, 11)]

    tempo = breathe(96, [(0, 16), (16, 16), (32, 16), (48, 16)]) + breathe(
        108, [(64, 16), (80, 16), (96, 16), (112, 16)], last=0.35)
    p = Performance(tempo, seed=1824, timing=0.008)
    p.voice = "strum"
    p.message(0.0, (0xB0, 126, 4), 0)

    def stroke(at, chord, direction, velocity, held):
        onset = p.at(at, scale=0.8)
        key = DOWN if direction == "D" else UP
        p.message(onset - 0.002, (0x90, key, 100))
        p.message(onset + 0.01, (0x80, key, 0), 0)
        for string in range(4):
            p.play(onset, held, at, STANDARD[string] + SHAPES[chord][string], velocity, string)

    island = [(0, "D", 0), (2, "D", -8), (3, "U", -28), (5, "U", -24), (6, "D", -10), (7, "U", -28)]
    for start, loud, strummed in ((0, 70, False), (64, 86, True)):
        for bar in range(16):
            at = start + 4 * bar
            rise = 5 * math.sin(math.pi * ((bar % 4) + 0.5) / 4)
            last = strummed and bar == 15
            pattern = [(0, "D", 0), (4, "D", -6)] if not strummed or last else island
            for index, (eighth, direction, accent) in enumerate(pattern):
                following = pattern[index + 1][0] if index + 1 < len(pattern) else 8
                held = 4.5 if last and eighth == 4 else 0.5 * (following - eighth) - 0.05
                stroke(at + 0.5 * eighth, harmony[bar][int(eighth >= 4)], direction, loud + accent + rise, held)

    for voice, start, loud in (("tune", 0, 80), ("tune", 64, 92), ("harmony", 64, 74)):
        p.voice = voice
        for bar, notes in enumerate(bars):
            rise = 6 * math.sin(math.pi * ((bar % 4) + 0.5) / 4)
            for index, (beat, melody, length) in enumerate(notes):
                at = start + 4 * bar + beat
                last = start == 64 and bar == 15 and index == len(notes) - 1
                note = melody if voice == "tune" else c_major[c_major.index(melody) - 2]
                p.play(p.at(at, lean=0.006), 4.5 if last else length * 0.95, at, note,
                       loud + rise + (8 if beat in (0, 2) else -4 if beat % 1 else 0))
    return p


def minuet_in_g():
    """The Minuet in G from the 1725 notebook for Anna Magdalena Bach, now
    credited to Christian Petzold, as a duet: a koa soprano picks the tune
    and a mahogany low-G tenor the bass, each fingerpicked with the hand
    choosing the strings (one channel). Played twice, the repeat softer, as
    an echo, with the bass a touch ahead of the tune."""
    first = [
        [(0, 74, 1), (1, 67, 0.5), (1.5, 69, 0.5), (2, 71, 0.5), (2.5, 72, 0.5)],
        [(0, 74, 1), (1, 67, 1), (2, 67, 1)],
        [(0, 76, 1), (1, 72, 0.5), (1.5, 74, 0.5), (2, 76, 0.5), (2.5, 78, 0.5)],
        [(0, 79, 1), (1, 67, 1), (2, 67, 1)],
        [(0, 72, 1), (1, 74, 0.5), (1.5, 72, 0.5), (2, 71, 0.5), (2.5, 69, 0.5)],
        [(0, 71, 1), (1, 72, 0.5), (1.5, 71, 0.5), (2, 69, 0.5), (2.5, 67, 0.5)],
    ]
    tune = first + [
        [(0, 66, 1), (1, 67, 0.5), (1.5, 69, 0.5), (2, 71, 0.5), (2.5, 67, 0.5)],
        [(0, 69, 3)],
    ] + first + [
        [(0, 69, 1), (1, 71, 0.5), (1.5, 69, 0.5), (2, 67, 0.5), (2.5, 66, 0.5)],
        [(0, 67, 3)],
    ]
    bass = [
        [(0, 55, 2), (2, 57, 1)], [(0, 59, 3)], [(0, 60, 3)], [(0, 59, 3)], [(0, 57, 3)], [(0, 55, 3)],
        [(0, 62, 1), (1, 59, 1), (2, 55, 1)], [(0, 62, 3)],
        [(0, 55, 2), (2, 57, 1)], [(0, 59, 3)], [(0, 60, 3)], [(0, 59, 3)], [(0, 57, 3)], [(0, 55, 3)],
        [(0, 60, 1), (1, 62, 2)], [(0, 55, 3)],
    ]
    phrases = [(48 * time + 12 * n, 12) for time in range(2) for n in range(4)]
    p = Performance(breathe(112, phrases, swell=0.025, settle=0.06, last=0.3), seed=1725, timing=0.006)
    for time, loud in enumerate((84, 70)):
        for bar in range(16):
            start = 48 * time + 3 * bar
            rise = 7 * math.sin(math.pi * ((bar % 4) + 0.5) / 4)
            final = time == 1 and bar == 15
            p.voice = "tune"
            for beat, note, length in tune[bar]:
                metric = 8 if beat == 0 else -6 if beat % 1 else 0
                p.play(p.at(start + beat), 4.5 if final else length * 0.95, start + beat, note,
                       loud + metric + rise)
            p.voice = "bass"
            for beat, note, length in bass[bar]:
                p.play(p.at(start + beat, lean=-0.012), 4.5 if final else length * 0.9, start + beat, note,
                       loud - 8 + (4 if beat == 0 else 0) + rise / 2)
            if final:
                # The last G, filled out under the tune: D4 and B4 with it.
                for offset, note in enumerate((62, 71)):
                    p.play(p.at(start, lean=0.012 * (offset + 1)), 4.5, start, note, loud - 12)
    return p


def aloha_oe():
    """Aloha ʻOe, Queen Liliʻuokalani's song (Maunawili, 1878), in C, a third
    up from the A-flat of the 1913 Eclipse Publishing edition (Levy Sheet
    Music Collection, box 151 item 019), whose vocal line gives the notes. A
    low-G tenor plays it alone, fingerpicked as tab: the tune on the A and E
    strings, the bass on the C and G strings, a chord tone on the E string
    between. It opens with the refrain's last two bars, plays the verse,
    then the refrain twice; a concert ukulele strums under the refrain,
    softly the first time and with the island strum the second, and both
    close on a slow C. It moves at about 100 beats a minute, a walking
    ballad rather than a hymn; each player keeps a clock of their own
    (Performance.feel), and an up-stroke rarely reaches the G string."""
    # The tune as the edition writes it, a third higher: (beat, note, beats).
    verse = [
        [(0, 76, 1.5), (1.5, 74, 0.5), (2, 72, 0.75), (2.75, 71, 0.25), (3, 72, 0.5), (3.5, 69, 0.5)],
        [(0, 67, 3), (3, 76, 1)],
        [(0, 74, 1.5), (1.5, 73, 0.5), (2, 74, 0.5), (2.5, 74, 0.5), (3, 77, 0.5), (3.5, 76, 0.5)],
        [(0, 74, 3), (3, 67, 0.5), (3.5, 72, 0.5)],
        [(0, 76, 1.5), (1.5, 74, 0.5), (2, 72, 0.75), (2.75, 71, 0.25), (3, 72, 0.5), (3.5, 69, 0.5)],
        [(0, 67, 2), (2, 67, 1), (3, 72, 0.5), (3.5, 71, 0.5)],
        [(0, 69, 1), (1, 74, 0.5), (1.5, 72, 0.5), (2, 71, 0.5), (2.5, 71, 0.5), (3, 76, 0.5), (3.5, 74, 0.5)],
        [(0, 72, 3), (3.5, 67, 0.5)],
    ]
    refrain = [
        [(0, 69, 1), (1, 72, 1), (2, 77, 1.5), (3.5, 69, 0.5)],
        [(0, 67, 1), (1, 72, 1), (2, 76, 1.5), (3.5, 72, 0.5)],
        [(0, 71, 0.75), (0.75, 69, 0.25), (1, 71, 0.5), (1.5, 72, 0.5), (2, 74, 0.5), (2.5, 74, 0.5),
         (3, 77, 0.5), (3.5, 77, 0.5)],
        [(0, 76, 2), (2, 72, 1.5), (3.5, 67, 0.5)],
        [(0, 69, 1), (1, 72, 1), (2, 77, 1.5), (3.5, 69, 0.5)],
        [(0, 67, 0.5), (0.5, 67, 0.5), (1, 72, 0.5), (1.5, 72, 0.5), (2, 76, 1.5), (3.5, 72, 0.5)],
        [(0, 71, 1.5), (1.5, 72, 0.5), (2, 76, 0.5), (2.5, 74, 1), (3.5, 71, 0.5)],
        [(0, 72, 3.5), (3.5, 67, 0.5)],
    ]
    # The chord on each beat, from the edition's piano part.
    verse_harmony = ["C" * 4, "C" * 4, "G" * 4, "G" * 4, "C" * 4, "CCCE", "FFGG", "C" * 4]
    refrain_harmony = ["F" * 4, "C" * 4, "G" * 4, "C" * 4, "F" * 4, "C" * 4, "G" * 4, "C" * 4]
    names = {"C": "C", "F": "F", "G": "G7", "E": "E7"}
    # The intro is the refrain's last two bars, its C cut to three beats for
    # the verse's two pick-up notes.
    intro = refrain[6:7] + [[(0, 72, 3), (3, 67, 0.5), (3.5, 72, 0.5)]]
    sections = [  # (tune, harmony, first beat, loudness, strum)
        (intro, refrain_harmony[6:], 0, 70, None),
        (verse, verse_harmony, 8, 72, None),
        (refrain, refrain_harmony, 40, 82, "soft"),
        (refrain[:-1] + [[(0, 72, 6)]], refrain_harmony, 72, 90, "island"),
    ]
    # Where the bass and the chord tone between sit on a low-G tenor (G3 C4
    # E4 A4): the thumb's two strings and the E string's note per chord.
    low = {"C": (60, 55), "F": (57, 60), "G7": (55, 62), "E7": (56, 56)}  # beats 1 and 3
    inner = {"C": 64, "F": 65, "G7": 65, "E7": 64}

    tempo = (breathe(94, [(0, 8)], settle=0.12) + breathe(100, [(8, 16), (24, 16)], settle=0.08)
             + breathe(102, [(40, 16), (56, 16)]) + breathe(104, [(72, 16), (88, 16)], last=0.4))
    p = Performance(tempo, seed=1878, timing=0.008, dynamics=5.0,
                    feel={"tune": (0.0, 0.008), "strum": (0.008, 0.009)})
    plucks = []  # (beat, string, note, velocity, beats held, lean)

    def string_of(note):
        # The tune keeps to the A string, the G and G-sharp below it to the E string.
        return 3 if note >= 69 else 2

    for tune, harmony, start, loud, _ in sections:
        for bar, notes in enumerate(tune):
            rise = 5 * math.sin(math.pi * ((bar % 4) + 0.5) / 4)
            at = start + 4 * bar
            tune_on_e = [(at + b, at + b + length) for b, note, length in notes if string_of(note) == 2]
            for beat, note, length in notes:
                metric = 6 if beat in (0, 2) else -5 if beat % 1 else 0
                plucks.append((at + beat, string_of(note), note, loud + rise + metric, length, 0.006))
            if bar >= len(harmony) or at == 100:  # the last bar is the close's
                continue
            for beat in range(4):
                chord = names[harmony[bar][beat]]
                first = beat == 0 or harmony[bar][beat] != harmony[bar][beat - 1]
                if beat in (0, 2) or first:
                    bass = low[chord][0 if beat < 2 else 1]
                    plucks.append((at + beat, 0 if bass < 60 else 1, bass, loud - 18 + (4 if beat == 0 else 0),
                                   2.0, -0.01))
                if beat in (1, 3) and not any(s - 0.25 <= at + beat < e for s, e in tune_on_e):
                    plucks.append((at + beat, 2, inner[chord], loud - 30, 1.0, 0.004))
    # The close: C from the low G up under the tune's held C.
    for index, (string, note) in enumerate(((0, 55), (1, 60), (2, 64))):
        plucks.append((100, string, note, 66 - 4 * index, 6.0, 0.03 * index))

    # A string sounds one note at a time: each lets go before the next pluck on it.
    p.voice = "tune"
    p.message(0.0, (0xB0, 126, 4), 0)
    for string in range(4):
        mine = sorted(item for item in plucks if item[1] == string)
        for index, (beat, _, note, velocity, held, lean) in enumerate(mine):
            if index + 1 < len(mine):
                held = min(held, mine[index + 1][0] - beat - 0.04)
            p.play(p.at(beat, lean=lean), held, beat, note, velocity, string)

    p.voice = "strum"
    p.message(0.0, (0xB0, 126, 4), 0)

    def stroke(beat, chord, direction, velocity, held):
        onset = p.at(beat, scale=0.8)
        p.message(onset - 0.002, (0x90, DOWN if direction == "D" else UP, 100))
        p.message(onset + 0.01, (0x80, DOWN if direction == "D" else UP, 0), 0)
        strings = range(4)
        if direction == "U":
            roll = p.random.random()
            strings = (1, 2, 3) if roll < 0.55 else (2, 3) if roll < 0.85 else range(4)
        for string in strings:
            p.play(onset, held, beat, STANDARD[string] + SHAPES[chord][string], velocity, string)

    soft = [(0, "D", 0), (4, "D", -4), (6, "U", -14)]
    island = [(0, "D", 0), (2, "D", -8), (3, "U", -26), (5, "U", -22), (6, "D", -10), (7, "U", -26)]
    for tune, harmony, start, loud, strum in sections:
        if strum is None:
            continue
        for bar in range(8):
            at = start + 4 * bar
            last = strum == "island" and bar == 7
            pattern = [(0, "D", 0)] if last else soft if strum == "soft" else island
            for index, (eighth, direction, accent) in enumerate(pattern):
                following = pattern[index + 1][0] if index + 1 < len(pattern) else 8
                held = 6.0 if last else 0.5 * (following - eighth) - 0.05
                chord = names[harmony[bar][int(eighth // 2)]]
                stroke(at + 0.5 * eighth, chord, direction, loud - 26 + accent + (6 if strum == "island" else 0), held)
    return p


def tarantella():
    """Tarantella napoletana: the tarantella that closes Luigi Ricci's comic
    opera La festa di Piedigrotta (Naples, 1852), from the piano
    transcription in La ricreazione per tutti (ed. Domenico Ghinassi, 1859),
    whose two pages are leaves n180 and n181 of the Internet Archive's copy
    (bub_gb_Fo65eQm5t9YC, Public Domain Mark; Wikimedia Commons' "Tarantella
    di Piedigrotta.jpg" is a small scan of the first page). It is in E minor.

    The arrangement plays the print's first page and the first two systems
    of its second, abridged where the harmony joins: the call (B5 and C6
    over drummed B3s with their grace notes), the scale of neighbour figures
    falling three octaves to B3, the song the print doubles in octaves, the
    riff the tune is known by and the same riff an octave down, both over
    the print's broken chords, then the strain in thirds in G major over a
    D pedal. The call's last fragments and the rest of page two are left
    out. The riff comes back once more for the whole band, with its last
    three bars as a tag.

    Seven factory ukuleles share the print's two hands. Where a hand goes
    below a ukulele's G3 the line moves up an octave: the broken chords are
    voiced across the low-G tenor's strings, an octave or so above the
    print. The small note before each C5-E5 third in the G-major strain is
    read as F#5; the scan leaves its pitch uncertain. The song's second half, which the print doubles below a
    ukulele's range, has a strum and chord roots instead (Am, Em, B7, Em),
    and the closing riff a soprano thumb a third below the tune; those
    chords, the strums and the piezo concert's dead strokes, played as a
    tambourine, are this arrangement's, not the print's.

    Each player keeps a clock of their own, a lean and a slow wander
    (Performance.feel); the second and third eighths of every beat come a
    little late, the lilt of the dance; up-strokes catch two or three
    strings; quarters ring and eighths are clipped by varying amounts; the
    grace notes fall just before their beat, never by the same amount.

    The opening is played freely, as a call is: the first call starts under
    tempo, pushes through its turn and leans on its held B5, the drum comes
    in softly and gathers the dance's speed, the second call answers louder
    and nearly in time, and the third pushes into the scale, which hurries
    down and broadens onto its last note. The hands are looser together
    there and settle by the song."""
    # Each bar is a list of (eighth, note or (notes), eighths[, grace note]);
    # a bar is two beats, a beat a dotted quarter. Pitches are the print's.
    figure = [(3, 83, 1), (4, 84, 1), (5, 83, 1)]                 # B5 C6 B5
    turn = [(0, 83, 1), (1, 84, 1), (2, 83, 1)] + figure
    call_rh = {0: figure, 1: turn, 2: [(0, 83, 2)], 4: figure, 5: turn, 6: [(0, 83, 2)], 8: figure}
    call_lh = {0: figure, 1: turn, 2: [(0, 83, 2)], 4: figure, 5: turn, 6: [(0, 83, 2)]}  # an octave down
    drum_a = [(2, 59, 1), (3, 59, 2, 58), (5, 59, 1)]                # B3, A#3 grace notes
    drum_b = [(0, 59, 2, 58), (2, 59, 1), (3, 59, 2, 58), (5, 59, 1)]
    drum = {2: drum_a, 3: drum_b, 4: [(0, 59, 2, 58)], 6: drum_a, 7: drum_b, 8: [(0, 59, 2)]}
    # The scale: a turn on each half bar from A5 down to C4, landing on B3.
    tops = [81, 79, 78, 76, 74, 72, 71, 71, 69, 67, 66, 64, 62, 60]
    step_up = {81: 83, 79: 81, 78: 79, 76: 78, 74: 76, 72: 74, 71: 72, 69: 71, 67: 69, 66: 67, 64: 66,
               62: 64, 60: 62}
    song = [
        [(0, 86, 3), (3, 84, 1), (4, 86, 1), (5, 84, 1)],   # D6 . C6 D6 C6
        [(0, 83, 2), (2, 76, 1), (3, 83, 2), (5, 76, 1)],   # B5 E5 B5 E5
        [(0, 79, 3), (3, 78, 1), (4, 79, 1), (5, 78, 1)],   # G5 . F#5 G5 F#5
        [(0, 76, 3), (3, 76, 3)],                           # E5 E5
        [(0, 74, 3), (3, 72, 1), (4, 74, 1), (5, 72, 1)],   # the same an octave down
        [(0, 71, 2), (2, 64, 1), (3, 71, 2), (5, 64, 1)],
        [(0, 67, 3), (3, 66, 1), (4, 67, 1), (5, 66, 1)],
        [(0, 64, 2)],
    ]
    riff = [
        [(0, 83, 2), (2, 83, 1), (3, 88, 2), (5, 88, 1)],              # B5 B5 E6 E6
        [(0, 83, 2), (2, 83, 1), (3, 83, 2), (5, 83, 1)],              # B5 B5 B5 B5
        [(0, 84, 2), (2, 84, 1), (3, 84, 1), (4, 86, 1), (5, 84, 1)],  # C6 C6 C6 D6 C6
        [(0, 83, 2), (3, 83, 2), (5, 83, 1)],                          # B5 - B5 B5
        [(0, 83, 2), (2, 81, 1), (3, 81, 2), (5, 81, 1)],              # B5 A5 A5 A5
        [(0, 81, 2), (2, 79, 1), (3, 79, 2), (5, 79, 1)],              # A5 G5 G5 G5
        [(0, 79, 2), (2, 78, 1), (3, 78, 2), (5, 78, 1)],              # G5 F#5 F#5 F#5
        [(0, 76, 2)],                                                  # E5
    ]
    riff_pickup = [(3, 88, 2), (5, 88, 1)]                             # E6 E6, in the bar before
    BD, CE, AC, GB = (71, 74), (72, 76), (69, 72), (67, 71)            # the strain's thirds
    thirds = [
        [(0, BD, 2), (2, BD, 1), (3, CE, 2, 78), (5, BD, 1)],
        [(0, AC, 2), (2, 62, 1), (3, AC, 2), (5, 62, 1)],              # over D4, the pedal
        [(0, AC, 2), (2, 62, 1), (3, AC, 2), (5, 62, 1)],
        [(0, GB, 2), (3, GB, 2), (5, AC, 1)],
        [(0, BD, 2), (2, BD, 1), (3, CE, 2, 78), (5, BD, 1)],
        [(0, AC, 2), (2, 62, 1), (3, AC, 2), (5, 62, 1)],
        [(0, AC, 2), (2, 62, 1), (3, AC, 2), (5, 60, 1)],
        [(0, GB, 2)],
    ]
    # The print's left hand under the riff and the strain, one chord a bar.
    riff_chords = ["Em", "Em", "Am", "Em", "B", "Em", "B7", "Em"]
    thirds_chords = ["G", "D7", "D7", "G", "G", "D7", "D7", "G"]
    song_chords = ["Am", "Em", "B7", "Em"]  # this arrangement's, under the song's second half
    song_roots = {"Am": ((0, 57), (2, 64)), "Em": ((1, 64), (0, 59)), "B7": ((0, 59), (2, 66))}  # root, fifth
    # The broken chords on the low-G tenor's strings (G3 C4 E4 A4): (string, note), low to high.
    arps = {"Em": ((0, 59), (1, 64), (2, 67)), "Am": ((0, 57), (1, 60), (2, 64)),
            "B": ((0, 59), (1, 63), (2, 66)), "B7": ((0, 59), (1, 63), (2, 69)),
            "G": ((0, 55), (1, 62), (2, 67)), "D7": ((1, 62), (2, 66), (3, 69))}
    tones = {"Em": (4, 7, 11), "Am": (9, 0, 4), "B": (11, 3, 6), "B7": (11, 3, 6, 9)}

    # Where each section's first bar starts, in beats.
    INTRO, SCALE, SONG, RIFF, LOW, THIRDS, FINAL, TAG, CLOSE = 0, 18, 34, 50, 66, 82, 98, 114, 120
    # The calls' held B5s are beats 4 and 12; the drum enters at 4.67.
    tempo = ([(0, 100), (2.5, 112), (3.4, 110), (4.25, 90), (4.7, 104), (8, 118), (11.3, 122), (12.2, 110),
              (12.7, 120), (16, 124), (17.9, 128)]
             + breathe(127, [(SCALE, 8), (SCALE + 8, 8)], swell=0.035, last=0.09)
             + breathe(124, [(SONG, 8), (SONG + 8, 8)], swell=0.025, settle=0.03, last=0.01)
             + breathe(132, [(RIFF, 8), (RIFF + 8, 8), (LOW, 8), (LOW + 8, 8)], swell=0.02, settle=0.02,
                       last=0.04)
             + breathe(126, [(THIRDS, 8), (THIRDS + 8, 8)], swell=0.02, settle=0.03, last=0.06)
             + [(97.3, 124), (FINAL, 134), (106, 138), (TAG, 142), (118.5, 145), (119.4, 132),
                (CLOSE, 112), (CLOSE + 2, 88)])
    feel = {"high": (0.002, 0.007), "lead": (0.0, 0.006), "thumb": (0.006, 0.007), "fingers": (0.003, 0.007),
            "strum": (0.004, 0.006), "piezo": (0.0, 0.005), "bass": (-0.004, 0.005)}
    p = Performance(tempo, seed=1852, timing=0.005, dynamics=4.5, feel=feel)

    def beat_of(start, bar, eighth):
        return start + 2 * bar + eighth / 3

    def loose(beat):
        # How much wider each hand's scatter is than the dance's: the
        # players have not settled in together until the song.
        return interpolate([(INTRO, 2.2), (SCALE, 1.6), (SONG, 1.0)], beat)

    # Each call swells to its held B5, the second answering louder; the drum
    # enters softly and builds under each call's held note.
    call_shape = [(1, -9), (4, 1), (9, -5), (12, 4), (17, 2)]
    drum_shape = [(4.6, -14), (8, -2), (12.6, -7), (16, 2)]

    def lilt(eighth):
        # The first eighth of each three a little long, as the dance is played.
        return (0.0, 0.011, 0.006)[eighth % 3]

    def accent(eighth):
        return 7 if eighth == 0 else 3 if eighth == 3 else -4

    def grace(onset, note, velocity, channel=0):
        ahead = min(0.07, max(0.035, p.random.gauss(0.05, 0.008)))
        p.message(onset - ahead, (0x90 | channel, note, p.velocity(velocity - 14)))
        p.message(onset - 0.012, (0x80 | channel, note, 0), 0)

    def melody(part, start, bars, loud, shift=0, clip=0.68, shape=None):
        """Play {bar: notes} from `start`, `shift` semitones from the print,
        `shape` adding velocity by beat."""
        p.voice = part
        for bar, notes in sorted(bars.items()):
            for item in notes:
                eighth, notes_, length = item[:3]
                chord = notes_ if isinstance(notes_, tuple) else (notes_,)
                beat = beat_of(start, bar, eighth)
                arch = 3.0 * math.sin(math.pi * ((bar % 4) + eighth / 6 + 0.5) / 4)
                velocity = loud + accent(eighth % 6) + arch + (interpolate(shape, beat) if shape else 0)
                onset = p.at(beat, lean=lilt(eighth), scale=loose(beat))
                # A quarter rings nearly its length, an eighth is clipped,
                # and no two the same.
                held = length / 3 * min(0.95, (0.88 if length >= 2 else clip) + p.random.gauss(0.0, 0.06))
                if len(item) > 3:
                    grace(onset, item[3] + shift, velocity)
                for index, note in enumerate(sorted(chord, reverse=True)):
                    p.play(onset, held, beat, note + shift, velocity - 5 * index)

    def strum(part, beat, chord, direction, velocity, held, chuck=False):
        p.voice = part
        onset = p.at(beat, lean=lilt(round(3 * (beat % 1))), scale=0.8 * loose(beat))
        if chuck:
            p.message(onset - 0.004, (0x90, CHUCK, 100))
            p.message(onset + 0.04, (0x80, CHUCK, 0), 0)
        key = DOWN if direction == "D" else UP
        p.message(onset - 0.002, (0x90, key, 100))
        p.message(onset + 0.01, (0x80, key, 0), 0)
        strings = range(4)
        if direction == "U":  # an up-stroke rarely reaches the G string, sometimes not the C string either
            roll = p.random.random()
            strings = (1, 2, 3) if roll < 0.55 else (2, 3) if roll < 0.85 else range(4)
        for string in strings:
            p.play(onset, held, beat, STANDARD[string] + SHAPES[chord][string], velocity, string)

    # Down, a light down, up; the same on the second beat (the hand keeps
    # moving through the light strokes). "soft" keeps the quarter-eighth lilt.
    full = [(0, "D", 0), (1, "D", -24), (2, "U", -12), (3, "D", -5), (4, "D", -24), (5, "U", -12)]
    soft = [(0, "D", 0), (2, "U", -16), (3, "D", -6), (5, "U", -18)]
    tambourine = [(0, "D", 8), (2, "U", -8), (3, "D", 1), (5, "U", -10)]

    def strum_bars(part, start, chords, pattern, loud, chuck=False, bars=None):
        for bar in (range(len(chords)) if bars is None else bars):
            for index, (eighth, direction, lift) in enumerate(pattern):
                following = pattern[index + 1][0] if index + 1 < len(pattern) else 6
                held = 0.06 if chuck else (following - eighth) / 3 - 0.05
                strum(part, beat_of(start, bar, eighth), chords[bar], direction,
                      loud + lift + 2 * math.sin(math.pi * (bar % 4 + 0.5) / 4), held, chuck)

    # ---- the tunes, by player
    melody("high", INTRO, call_rh, 78, shape=call_shape)
    melody("fingers", INTRO, call_lh, 68, shift=-12, shape=call_shape)
    scale = {}
    for h, top in enumerate(tops):
        scale.setdefault(9 + h // 2, []).extend(
            (3 * (h % 2) + k, note, 1) for k, note in enumerate((top, step_up[top], top)))
    melody("high", INTRO, {bar: notes for bar, notes in scale.items() if bar <= 11}, 84)
    melody("lead", INTRO, {bar: notes for bar, notes in scale.items() if bar >= 12}, 80)
    melody("lead", INTRO, {16: [(0, 59, 2)]}, 86)
    # The print doubles the scale an octave down from bar 12's second half:
    # the low-G tenor while it can reach, then the fingerstyle tenor an
    # octave up instead.
    lower = {bar: [n for n in notes if bar > 12 or n[0] >= 3] for bar, notes in scale.items() if 12 <= bar <= 13}
    upper = {bar: notes for bar, notes in scale.items() if bar >= 14}
    melody("fingers", INTRO, upper, 66, shift=12)
    melody("high", SONG, {-1: [(5, 88, 1)], **{bar: song[bar] for bar in range(4)}}, 82)
    melody("fingers", SONG, {-1: [(5, 88, 1)], **{bar: song[bar] for bar in range(4)}}, 74, shift=-12)
    melody("lead", SONG, {bar: song[bar] for bar in range(4, 8)}, 84)
    melody("high", SONG, {7: riff_pickup}, 88)
    melody("high", RIFF, dict(enumerate(riff)), 92)
    melody("lead", RIFF, {7: riff_pickup}, 84, shift=-12)
    melody("lead", LOW, dict(enumerate(riff)), 88, shift=-12)
    melody("fingers", LOW, {7: [(3, GB, 2), (5, AC, 1)]}, 74)
    melody("fingers", THIRDS, dict(enumerate(thirds)), 76, clip=0.62)
    final = dict(enumerate(riff))
    tag = dict(enumerate(riff[4:7] + [[(0, 76, 7)]]))
    for part, shift, loud in (("high", 0, 100), ("lead", -12, 86)):
        melody(part, THIRDS, {7: riff_pickup}, loud - 6, shift=shift)
        melody(part, FINAL, final, loud, shift=shift)
        melody(part, TAG, tag, loud + 4, shift=shift)
    # A soprano thumb a third (or a fourth) below the tune in the closing riff.
    p.voice = "thumb"
    for start, bars, chords in ((FINAL, final, riff_chords), (TAG, tag, riff_chords[4:7] + ["Em"])):
        for bar, notes in bars.items():
            for eighth, note, length in notes:
                chord = chords[bar] if bar < len(chords) else "Em"
                below = [n for n in range(note - 5, note - 2) if n % 12 in tones[chord]]
                beat = beat_of(start, bar, eighth)
                held = length / 3 * min(0.95, (0.88 if length >= 2 else 0.66) + p.random.gauss(0.0, 0.06))
                p.play(p.at(beat, lean=lilt(eighth)), held, beat, below[-1] if below else note - 3,
                       84 + accent(eighth % 6))

    # ---- the strumming hands
    for part in ("strum", "piezo", "bass"):
        p.voice = part
        p.message(0.0, (0xB0, 126, 4), 0)
    for bar in range(5, 9):  # the tambourine fades in under the second call
        strum_bars("piezo", INTRO, ["Em"] * 9, tambourine, 44 + 4 * (bar - 5), chuck=True, bars=[bar])
    strum_bars("piezo", INTRO, ["Em"] * 17, tambourine, 58, chuck=True, bars=range(9, 17))
    strum_bars("piezo", SONG, ["Em"] * 8, tambourine, 64, chuck=True)
    strum_bars("piezo", RIFF, riff_chords, tambourine, 70, chuck=True)
    strum_bars("piezo", LOW, riff_chords, tambourine, 64, chuck=True)
    strum_bars("piezo", FINAL, riff_chords, tambourine, 76, chuck=True)
    strum_bars("piezo", TAG, riff_chords[4:7], tambourine, 80, chuck=True)
    strum_bars("strum", SONG, ["Em"] * 4 + song_chords, soft, 58, bars=range(4, 8))
    strum_bars("strum", LOW, riff_chords, soft, 66)
    strum_bars("strum", THIRDS, thirds_chords, soft, 52)
    strum_bars("strum", FINAL, riff_chords, full, 78)
    strum_bars("strum", TAG, riff_chords[4:7], full, 82)
    for part, velocity in (("strum", 102), ("piezo", 90)):
        strum(part, CLOSE, "Em", "D", velocity, 2.6)

    # ---- the low-G tenor: the call's drum, its share of the scale, the
    # song's roots and the print's broken chords, each string sounding one
    # note at a time
    plucks = []  # (beat, string, note, velocity, beats held, grace note)
    for bar, notes in drum.items():
        for eighth, note, length, *ornament in notes:
            beat = beat_of(INTRO, bar, eighth)
            plucks.append((beat, 0, note, 84 + accent(eighth) + interpolate(drum_shape, beat), length / 3 * 0.8,
                           ornament[0] if ornament else None))
    for bar, notes in lower.items():
        for eighth, note, length in notes:
            plucks.append((beat_of(INTRO, bar, eighth), 0, note - 12, 70 + accent(eighth), 0.3, None))
    for bar, chord in enumerate(song_chords[:3], 4):
        (root_string, root), (fifth_string, fifth) = song_roots[chord]
        plucks.append((beat_of(SONG, bar, 0), root_string, root, 74, 0.95, None))
        plucks.append((beat_of(SONG, bar, 3), fifth_string, fifth, 64, 0.95, None))

    def broken(start, chords, loud, bars=None):
        for bar in (range(len(chords)) if bars is None else bars):
            for half in (0, 1):
                for k, (string, note) in enumerate(arps[chords[bar]]):
                    lift = (6 if half == 0 else 2) if k == 0 else -3
                    plucks.append((beat_of(start, bar, 3 * half + k), string, note, loud + lift, 1.0, None))

    broken(SONG, ["Em"] * 8, 58, bars=[7])
    broken(RIFF, riff_chords, 58)
    broken(LOW, riff_chords, 56, bars=range(7))
    plucks.append((beat_of(LOW, 7, 0), 1, 64, 70, 0.9, None))  # the print's E under the riff's last E
    broken(THIRDS, thirds_chords, 52)
    broken(FINAL, riff_chords, 66)
    broken(TAG, riff_chords[4:7], 70)
    for k, (string, note) in enumerate(arps["Em"]):
        plucks.append((CLOSE + 0.04 * k, string, note, 96 - 4 * k, 2.6, None))

    p.voice = "bass"
    for string in range(4):
        mine = sorted(item for item in plucks if item[1] == string)
        for index, (beat, _, note, velocity, held, ornament) in enumerate(mine):
            if index + 1 < len(mine):
                # Let go before the next pluck on this string, or its grace note.
                held = min(held, mine[index + 1][0] - beat - (0.16 if mine[index + 1][5] else 0.04))
            onset = p.at(beat, lean=lilt(round(3 * (beat % 1))), scale=loose(beat))
            if ornament is not None:
                grace(onset, ornament, velocity, string)
            p.play(onset, held, beat, note, velocity, string)
    return p


# Each part of a song is its own file, songs/<song>-<part>.mid, played by
# its own ukulele: a factory preset with the construction AculeleProbe's
# options change, set in the mix by its pan (-1 left to 1 right) and trim
# in dB. Tools/render_songs.py renders and mixes them.
SONGS = {
    "aloha-oe": (aloha_oe, {
        "tune": ("low-g-tenor", [], 0.15, 0.0),
        "strum": ("concert-strum", [], -0.35, -6.0),
    }),
    "tarantella": (tarantella, {
        "lead": ("tenor-picked-melody", [], 0.12, -2.0),
        "high": ("soprano-d-tuning", [], 0.35, -2.0),
        "thumb": ("soprano-thumb", [], -0.3, -3.0),
        "fingers": ("tenor-fluorocarbon-fingerstyle", [], -0.1, 0.0),
        "strum": ("concert-strum", [], -0.5, -7.0),
        "piezo": ("concert-piezo-stage", [], 0.55, 2.0),
        "bass": ("low-g-tenor", [], -0.12, -4.0),
    }),
    "minuet-in-g": (minuet_in_g, {
        "tune": ("tenor-fluorocarbon-fingerstyle", ["--shape", "0", "--wood", "2", "--strings", "0"], 0.3, 0.0),
        "bass": ("low-g-tenor", [], -0.3, -1.0),
    }),
    "ode-to-joy": (ode_to_joy, {
        "tune": ("tenor-fluorocarbon-fingerstyle", ["--shape", "1", "--wood", "0"], 0.25, 0.0),
        "strum": ("concert-strum", ["--shape", "2", "--wood", "1"], -0.35, -7.0),
        "harmony": ("soprano-thumb", ["--wood", "3"], -0.05, -3.0),
    }),
}

PHRASES = {
    "concert-strum": concert_strum,
    "soprano-d-tuning": soprano_d_tuning,
    "tenor-fluorocarbon-fingerstyle": tenor_fluorocarbon_fingerstyle,
    "low-g-tenor": low_g_tenor,
    "soprano-thumb": soprano_thumb,
    "tenor-picked-melody": tenor_picked_melody,
    "concert-piezo-stage": concert_piezo_stage,
}


def preset_slugs():
    header = (ROOT / "Source/DSP/AculelePresets.h").read_text(encoding="utf-8")
    return re.findall(r'\{ "[^"]+", "([a-z0-9-]+)",', header)


def last_event_seconds(phrase):
    return max(tick for tick, _, _ in phrase.events) / PPQ * 60 / phrase.bpm


def files():
    """{file name: bytes} of every source, after checking each phrase's limits."""
    slugs = preset_slugs()
    if sorted(slugs) != sorted(PHRASES):
        sys.exit(f"error: phrases {sorted(PHRASES)} do not match the bank {sorted(slugs)}")
    written = {}
    for number, slug in enumerate(slugs, 1):
        phrase = PHRASES[slug]()
        # The renderer adds a 2.5 s tail, and a demo is 8-12 s long.
        last = last_event_seconds(phrase)
        if not 5.5 <= last <= 9.5:
            sys.exit(f"error: {slug}'s last event is at {last:.2f} s, outside 5.5-9.5 s")
        for _, _, data in phrase.events:
            if data[0] & 0xF0 == 0x90 and data[1] not in KEYSWITCHES and data[1] not in STRING_NOTES:
                sys.exit(f"error: {slug} plays note {data[1]}, which no string or keyswitch has")
        written[f"{number:02d}-{slug}.mid"] = phrase.smf()
    return written


def song_files():
    slugs = preset_slugs()
    written = {}
    for song, (build, parts) in SONGS.items():
        performance = build()
        for part, (preset, _, _, _) in parts.items():
            if preset not in slugs:
                sys.exit(f"error: {song}'s {part} names {preset}, which is not in the bank")
            written[f"{song}-{part}.mid"] = performance.smf(part)
    return written


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--check", action="store_true", help="fail unless the committed files match")
    args = parser.parse_args()
    stale = []
    for directory, expected in ((MIDI_DIR, files()), (SONG_DIR, song_files())):
        found = {path.name: path.read_bytes() for path in directory.glob("*.mid")}
        if args.check:
            stale += sorted(set(found) ^ set(expected) | {name for name in expected if found.get(name) != expected[name]})
            continue
        directory.mkdir(parents=True, exist_ok=True)
        for name in set(found) - set(expected):
            (directory / name).unlink()
        for name, data in expected.items():
            (directory / name).write_bytes(data)
            print(directory / name)
    if stale:
        sys.exit(f"Docs/audio/midi differs from {pathlib.Path(__file__).name} in: {', '.join(stale)}")


if __name__ == "__main__":
    main()
