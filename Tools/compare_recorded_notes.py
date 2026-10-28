#!/usr/bin/env python3
"""Compare single notes of the FreePats tenor ukulele against Aculele.

    compare_recorded_notes.py --write-midi notes.mid
    AculeleProbe 2 notes.mid render.wav 53 --float --wood 1 --capture 7
    compare_recorded_notes.py <samples-dir> render.wav

The reference is FreePats' "Ukulele" set of 2026-08-11 by Mateusz Dabrowski,
CC0 1.0 (https://freepats.zenvoid.org/GuitarFamily/ukulele.html; archive
https://github.com/freepats/ukulele1/releases/download/2026-08-11/Ukulele-SFZ+WAV-20260811.7z,
SHA-256 cb857464cc847c261d954a4bfe3610380f96ff4d2603d94573ab3529aa574775).
Its README gives a Flight Fireball tenor; the maker's listings give solid
mango and Worth Brown fluorocarbon strings, in Standard tuning. Thirteen
single notes from C4 to C6, one dynamic, a RODE Podcaster at an unstated
distance, 6th-order high-pass at 190 Hz, each note level-matched over its
first 300 ms and trimmed at twice the noise floor with a 150 ms fade.
Technique, string and fret are not stated. Its samples directory is the
first argument; nothing from it is in this repository.

The render plays the same pitches through the probe: Tenor Fluorocarbon
Fingerstyle with Mahogany, the nearest Wood to mango, on the mono
microphone, each note alone at velocity 90, 4 s apart and held 3.5 s.

Printed, for the 13 notes together:
  - each partial's T60 (analyse_probe.py's fit over each note's own span,
    to 0.2 s before the recording's end), the median within frequency bands;
  - each partial's level against its note's summed partial power over
    30-330 ms, render minus recording, the median within third octaves.
    Pluck position combs follow the partial number, not its frequency, so
    across 13 notes they average out, and a difference fixed in frequency
    (body, capture, excitation) remains. The level match and the
    microphone make only these relative levels comparable.
"""

import math
import struct
import sys
from pathlib import Path

import numpy as np

from analyse_probe import measure, read_wav

NAMES = ["C4", "D4", "E4", "F#4", "G4", "A4", "B4", "C#5", "D#5", "F5", "G5", "A5", "C6"]
MIDI = [60, 62, 64, 66, 67, 69, 71, 73, 75, 77, 79, 81, 84]
SPACING, HOLD, VELOCITY = 4.0, 3.5, 90


def write_midi(path):
    def vlq(value):
        out = [value & 0x7F]
        value >>= 7
        while value:
            out.insert(0, (value & 0x7F) | 0x80)
            value >>= 7
        return bytes(out)

    ticks = 960  # per second: 480 per quarter at 120 bpm
    events = []
    for index, note in enumerate(MIDI):
        events.append((round(index * SPACING * ticks), 1, bytes([0x90, note, VELOCITY])))
        events.append((round((index * SPACING + HOLD) * ticks), 0, bytes([0x80, note, 0])))
    track = bytearray(b"\x00\xff\x51\x03" + (500000).to_bytes(3, "big"))
    last = 0
    for time, _, message in sorted(events):
        track += vlq(time - last) + message
        last = time
    track += b"\x00\xff\x2f\x00"
    Path(path).write_bytes(b"MThd" + struct.pack(">IHHH", 6, 0, 1, 480)
                           + b"MTrk" + struct.pack(">I", len(track)) + track)


def partials(segment, rate, f0):
    size = 1 << 18
    spectrum = np.abs(np.fft.rfft(segment * np.hanning(len(segment)), size))
    bins = np.fft.rfftfreq(size, 1 / rate)
    found = []
    for n in range(1, 17):
        if n * f0 > 9000:
            break
        # Half a semitone either side, widened for the string's stiffness.
        mask = (bins > n * f0 * 2 ** (-0.5 / 12)) & (bins < n * f0 * 2 ** (0.5 / 12) * (1 + 4e-4 * n * n))
        k = np.argmax(spectrum * mask)
        found.append((bins[k], spectrum[k] ** 2))
    found = np.array(found)
    found[:, 1] /= found[:, 1].sum()
    return found


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--write-midi":
        write_midi(sys.argv[2])
        return
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    samples, render = Path(sys.argv[1]), sys.argv[2]
    rate, frames = read_wav(render)
    model = frames.mean(axis=1)
    decays = {"recording": [], "render": []}
    levels = []
    for index, (name, note) in enumerate(zip(NAMES, MIDI)):
        _, reference = read_wav(str(samples / f"{name}.wav"))
        reference = reference.mean(axis=1)
        span = min(len(reference) / rate - 0.2, 3.4)
        window = model[int(index * SPACING * rate):int((index * SPACING + 0.5) * rate)]
        onset = index * SPACING + np.argmax(np.abs(window) > 0.01 * np.abs(window).max()) / rate
        for key, signal, start in (("recording", reference, 0.0), ("render", model, onset)):
            _, _, rows, _ = measure(signal, rate, note, start, span)
            decays[key] += [(fn, t60) for _, fn, _, _, t60 in rows if math.isfinite(t60) and t60 < 20]
        f0 = 440 * 2 ** ((note - 69) / 12)
        first = int(onset * rate)
        a = partials(reference[int(0.03 * rate):int(0.33 * rate)], rate, f0)
        b = partials(model[first + int(0.03 * rate):first + int(0.33 * rate)], rate, f0)
        for j in range(min(len(a), len(b))):
            levels.append((a[j, 0], 10 * math.log10(b[j, 1] / a[j, 1])))

    print("Partial T60, median within each band, seconds")
    edges = [200, 400, 700, 1000, 1500, 2200, 3200, 4700, 7000]
    for low, high in zip(edges[:-1], edges[1:]):
        cells = []
        for key in ("recording", "render"):
            values = [t for f, t in decays[key] if low <= f < high]
            cells.append(f"{key} {np.median(values):5.2f} (n {len(values):2d})" if values else f"{key}    -")
        print(f"  {low:5d}-{high:5d} Hz  " + "   ".join(cells))
    print("Partial level, render minus recording, median within each third octave, dB")
    levels = np.array(levels)
    for centre in 1000 * 2 ** (np.arange(-6, 10) / 3):
        values = levels[(levels[:, 0] >= centre * 2 ** (-1 / 6)) & (levels[:, 0] < centre * 2 ** (1 / 6)), 1]
        if len(values):
            print(f"  {centre:6.0f} Hz  {np.median(values):+6.1f}  (n {len(values):2d}, "
                  f"quartiles {np.percentile(values, 25):+6.1f} to {np.percentile(values, 75):+6.1f})")


if __name__ == "__main__":
    main()
