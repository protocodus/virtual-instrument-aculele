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
person's timing (Performance, below). They are not preset demos: render one
through the preset SONGS names, by its index in the bank, with

    build-dsp/AculeleProbe 3 Docs/audio/midi/songs/minuet-in-g.mid minuet-in-g.wav

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
    "C7": (0, 0, 0, 1), "E7": (1, 2, 0, 2),
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
    def __init__(self, tempo, seed, timing=0.007, dynamics=4.0):
        self.tempo = sorted(tempo)  # [(beat, bpm)], linear between, held past the ends
        self.random = random.Random(seed)
        self.timing = timing        # seconds, the onset scatter's standard deviation
        self.dynamics = dynamics    # velocity steps, likewise
        self.events = []            # (seconds, order, bytes); note-offs sort first

    def bpm(self, beat):
        points = self.tempo
        if beat <= points[0][0]:
            return points[0][1]
        for (b0, t0), (b1, t1) in zip(points, points[1:]):
            if beat <= b1:
                return t0 + (t1 - t0) * (beat - b0) / (b1 - b0)
        return points[-1][1]

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
        return max(0.0, self.seconds(beat) + lean + self.scatter(scale))

    def message(self, seconds, data, order=1):
        self.events.append((seconds, order, bytes(data)))

    def play(self, onset, beats_held, beat, note, velocity, channel=0):
        """One note struck at `onset` seconds and let go `beats_held` written
        beats after `beat`, with the release scattered too."""
        release = max(onset + 0.04, self.seconds(beat + beats_held) + self.scatter(0.6))
        self.message(onset, (0x90 | channel, note, self.velocity(velocity)))
        self.message(release, (0x80 | channel, note, 0), 0)

    def smf(self):
        """A format 0 file at the piece's opening tempo, so it sits roughly on
        a DAW's bars; the performance's own time is in the ticks."""
        bpm = self.tempo[0][1]
        events = sorted(self.events, key=lambda e: (e[0], e[1]))
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
    """Beethoven's Ode to Joy (Symphony No. 9, 1824) as a chord melody on
    the concert ukulele, tab-written: the tune on the top string the hand is
    at, the chord on the strings below it. The first time through the
    fingers pick the tune and brush the chord on the half bars; the second
    time every note is a strum, with light upstrokes between them, a little
    faster and louder, before the last phrase broadens into a slow C."""
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
    tones = {"C": (0, 4, 7), "G": (7, 11, 2), "Am": (9, 0, 4), "D7": (2, 6, 9, 0)}

    def voicing(melody, chord):
        """{string: note}: the tune on the A string from A4 up, else on the E
        string with the A string left out; each string below it takes the
        chord tone under the tune nearest the hand, open where the hand is low."""
        top = 3 if melody >= STANDARD[3] else 2
        fret = melody - STANDARD[top]
        notes = {top: melody}
        for string in range(top):
            options = [f for f in range(8) if (STANDARD[string] + f) % 12 in tones[chord]
                       and STANDARD[string] + f < melody]
            if options:  # the re-entrant G string sits above a low tune: left out
                notes[string] = STANDARD[string] + min(
                    options, key=lambda f: 0 if f == 0 and fret <= 5 else abs(f - fret) + 0.5)
        return notes

    passes = [(0, 96, 74, False), (64, 108, 92, True)]
    tempo = breathe(96, [(0, 16), (16, 16), (32, 16), (48, 16)]) + breathe(
        108, [(64, 16), (80, 16), (96, 16), (112, 16)], last=0.35)
    p = Performance(tempo, seed=1824, timing=0.008)
    p.message(0.0, (0xB0, 126, 4), 0)
    for start, _, loud, strummed in passes:
        for bar, notes in enumerate(bars):
            phrase_rise = 6 * math.sin(math.pi * ((bar % 4) + 0.5) / 4)
            for index, (beat, melody, length) in enumerate(notes):
                at = start + 4 * bar + beat
                chord = harmony[bar][int(beat >= 2)]
                accent = 8 if beat in (0, 2) else 0
                velocity = loud + accent + phrase_rise
                last = start == 64 and bar == 15 and index == len(notes) - 1
                if strummed or beat in (0, 2):
                    notes_of = voicing(melody, chord)
                    onset = p.at(at)
                    p.message(onset - 0.002, (0x90, DOWN, 100))
                    p.message(onset + 0.01, (0x80, DOWN, 0), 0)
                    held = 3.5 if last else length - 0.05
                    for string, note in notes_of.items():
                        p.play(onset, held, at, note, velocity - (0 if string == max(notes_of) else 10), string)
                    # A light upstroke on the off-beat of a held note keeps
                    # the hand moving.
                    if strummed and length >= 1 and not last:
                        up = p.at(at + 0.5, scale=1.3)
                        p.message(up - 0.002, (0x90, UP, 100))
                        p.message(up + 0.01, (0x80, UP, 0), 0)
                        for string, note in notes_of.items():
                            if string != max(notes_of):
                                p.play(up, length - 0.55, at + 0.5, note, velocity - 30, string)
                        p.play(up, length - 0.55, at + 0.5, melody, velocity - 22, max(notes_of))
                else:
                    top = 3 if melody >= STANDARD[3] else 2
                    p.play(p.at(at), length - 0.05, at, melody, velocity - 4, top)
    return p


def minuet_in_g():
    """The Minuet in G from the 1725 notebook for Anna Magdalena Bach, now
    credited to Christian Petzold, fingerpicked on the low-G tenor: the
    thumb takes the bass on the low G string and the fingers the tune, and
    the hand chooses the strings (one channel). Played twice, the repeat
    softer, as an echo, with the bass a touch ahead of the tune."""
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
            for beat, note, length in tune[bar]:
                metric = 8 if beat == 0 else -6 if beat % 1 else 0
                p.play(p.at(start + beat), 4.5 if final else length * 0.95, start + beat, note,
                       loud + metric + rise)
            for beat, note, length in bass[bar]:
                p.play(p.at(start + beat, lean=-0.012), 4.5 if final else length * 0.9, start + beat, note,
                       loud - 14 + (4 if beat == 0 else 0) + rise / 2)
            if final:
                # The last G, filled out under the tune: D4 and B4 with it.
                for offset, note in enumerate((62, 71)):
                    p.play(p.at(start, lean=0.012 * (offset + 1)), 4.5, start, note, loud - 16)
    return p


SONGS = {
    # file in Docs/audio/midi/songs/: (phrase, the preset it is written for)
    "minuet-in-g.mid": (minuet_in_g, "low-g-tenor"),
    "ode-to-joy.mid": (ode_to_joy, "concert-strum"),
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
    for name, (build, preset) in SONGS.items():
        if preset not in slugs:
            sys.exit(f"error: song {name} names {preset}, which is not in the bank")
        written[name] = build().smf()
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
