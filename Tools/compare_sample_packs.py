#!/usr/bin/env python3
"""Measure DecentSampler ukulele packs and Aculele renders alike.

    compare_sample_packs.py --write-midi notes.mid
    AculeleProbe 0 notes.mid concert.wav 101 --float --picking 0 --capture 7 --shape 1 --wood 1
    compare_sample_packs.py <pack-dir> [...] --render concert=concert.wav [...]

A pack directory holds a .dspreset and the samples it names; nothing from
any pack is in this repository. Every attack sample whose pitch lands in
C4-C6 is measured, after shifting each pack's root notes by the octave or
semitone its samples actually sound at (some packs map their samples away
from their pitch). The render plays C4 to C6 one note at a time, velocity
90, 4 s apart and held 3.5 s, as --write-midi writes them.

Only measures a microphone, a DI or a room hardly change are printed, so
packs recorded through an SM58, a phone or a pickup can stand together:
  - inharmonicity B, fitted to partials 1-12 as (f_n/n)^2 = f0^2 (1 + B n^2),
    the median within registers;
  - each partial's T60, fitted from 0.1 s to 40 dB down or the noise floor,
    the median within bands;
  - the onset pitch glide, the fundamental over 15-115 ms against 0.8-1.6 s;
  - the 10-90% rise time of the broadband envelope;
  - beating: each of partials 1-4's envelope in dB, from 0.1 s to 2 s while
    it stays 20 dB above the file's floor, less a quadratic fit; the share
    of notes whose residual spans more than 6 dB.
Partial levels against each other follow the microphone and are left out.

Needs NumPy, SciPy and soundfile (the packs hold AIFF and 16-bit WAV).
"""

import math
import struct
import sys
import xml.etree.ElementTree as ET
from collections import Counter
from pathlib import Path

import numpy as np
import soundfile
from scipy import signal

FIRST, LAST, SPACING, HOLD, VELOCITY = 60, 84, 4.0, 3.5, 90
REGISTERS = [(60, 63), (64, 68), (69, 74), (75, 79), (80, 84)]
BANDS = [250, 400, 700, 1000, 1500, 2200, 3200, 4700, 7000]


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
    for index, note in enumerate(range(FIRST, LAST + 1)):
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


def spectrum(segment, rate, size):
    window = np.hanning(len(segment))
    return np.abs(np.fft.rfft(segment * window, size)), np.fft.rfftfreq(size, 1 / rate)


def peak_near(magnitude, bins, frequency, cents):
    index = np.where((bins >= frequency * 2 ** (-cents / 1200))
                     & (bins <= frequency * 2 ** (cents / 1200)))[0]
    if len(index) < 3:
        return frequency, -300.0
    k = index[np.argmax(magnitude[index])]
    a, b, c = np.log(magnitude[k - 1:k + 2] + 1e-30)
    curvature = a - 2 * b + c
    shift = 0.5 * (a - c) / curvature if curvature else 0.0
    return bins[k] + shift * (bins[1] - bins[0]), 20 * math.log10(magnitude[k] + 1e-30)


def sounding_midi(x, rate):
    """The MIDI note whose first eight harmonics hold the most of the
    spectrum's square-rooted magnitude, at 10-cent steps."""
    magnitude, bins = spectrum(x[int(0.1 * rate):int(0.9 * rate)], rate, 1 << 18)
    magnitude = np.sqrt(magnitude)
    best, score = FIRST, -1.0
    for midi in np.arange(40, 100.01, 0.1):
        f0 = 440 * 2 ** ((midi - 69) / 12)
        total = 0.0
        for n in range(1, 9):
            if n * f0 > 0.45 * rate:
                break
            k = int(round(n * f0 / bins[1]))
            width = max(2, int(n * f0 * 0.015 / bins[1]))
            total += magnitude[k - width:k + width + 1].max() * 0.84 ** (n - 1)
        if total > score:
            best, score = midi, total
    return int(round(best))


def onset(x, rate):
    envelope = np.abs(signal.hilbert(x))
    width = int(0.001 * rate)
    smooth = np.convolve(envelope, np.ones(width) / width, mode="same")
    k = np.argmax(smooth > 0.1 * smooth.max())
    while k > 0 and smooth[k] > 0.02 * smooth.max():
        k -= 1
    return k


def partial_envelope_db(x, rate, frequency, n, hop):
    width = min(1 / 12, 0.4 / n)
    sos = signal.butter(4, [frequency * 2 ** -width, min(frequency * 2 ** width, 0.45 * rate)],
                        btype="bandpass", fs=rate, output="sos")
    envelope = np.abs(signal.hilbert(signal.sosfiltfilt(sos, x)))[::int(hop * rate)]
    return 20 * np.log10(envelope + 1e-12)


def measure(x, rate, midi):
    x = x[onset(x, rate):][:int(3.0 * rate)]
    span = len(x) / rate
    nominal = 440 * 2 ** ((midi - 69) / 12)
    long_magnitude, long_bins = spectrum(x[int(0.08 * rate):int(min(span, 1.0) * rate)], rate, 1 << 20)
    early_magnitude, early_bins = spectrum(x[int(0.03 * rate):int(0.33 * rate)], rate, 1 << 19)
    f1 = peak_near(*spectrum(x[int(0.2 * rate):int(min(span, 1.2) * rate)], rate, 1 << 20), nominal, 60)[0]
    partials = []
    for n in range(1, 25):
        guess = n * f1 * math.sqrt(1 + 2e-5 * n * n)
        if guess > min(10000, 0.45 * rate):
            break
        fn = peak_near(long_magnitude, long_bins, guess, 40)[0]
        partials.append((n, fn, peak_near(early_magnitude, early_bins, fn, 25)[1]))
    partials = np.array(partials)
    strongest = partials[:, 2].max()

    use = (partials[:, 0] <= 12) & (partials[:, 2] > strongest - 45)
    n, fn = partials[use, 0], partials[use, 1]
    coefficients = np.linalg.lstsq(np.vstack([np.ones(len(n)), n ** 2]).T, (fn / n) ** 2, rcond=None)[0]
    inharmonicity = coefficients[1] / coefficients[0]

    hop = 0.005
    decays = []
    for n, fn, level in partials:
        if level < strongest - 50 or fn > 9000:
            continue
        db = partial_envelope_db(x, rate, fn, n, hop)
        t = np.arange(len(db)) * hop
        first, end = int(0.1 / hop), len(db) - int(0.1 / hop)
        floor = np.median(db[-int(0.2 / hop):])
        below = np.where(db[first:end] < max(db[first] - 40, floor + 8))[0]
        if len(below):
            end = first + below[0]
        if end - first >= int(0.3 / hop):
            slope = np.polyfit(t[first:end], db[first:end], 1)[0]
            if slope < 0:
                decays.append((fn, -60 / slope))

    beating = []
    for n in range(1, 5):
        db = partial_envelope_db(x[:int(2.2 * rate)], rate, partials[n - 1, 1], n, 0.01)
        t = np.arange(len(db)) * 0.01
        part, times = db[10:200], t[10:200]
        audible = np.where(part > np.percentile(db, 5) + 20)[0]
        if len(audible) < 40:
            beating.append(math.nan)
            continue
        part, times = part[:audible[-1]], times[:audible[-1]]
        residual = part - np.polyval(np.polyfit(times, part, 2), times)
        beating.append(residual.max() - residual.min())

    def pitch(start, stop):
        return peak_near(*spectrum(x[int(start * rate):int(stop * rate)], rate, 1 << 20), f1, 60)[0]

    glide = 1200 * math.log2(pitch(0.015, 0.115) / pitch(0.8, min(span, 1.6))) if span > 1.0 else math.nan
    envelope = np.abs(signal.hilbert(x[:int(0.2 * rate)]))
    width = max(1, int(0.0005 * rate))
    smooth = np.convolve(envelope, np.ones(width) / width, mode="same")
    top = np.argmax(smooth)
    rise = (top - np.argmax(smooth > 0.1 * smooth[top])) / rate * 1000
    return dict(midi=midi, inharmonicity=inharmonicity, decays=decays, beating=beating,
                glide=glide, rise=rise)


def pack_notes(directory):
    preset = next(p for p in sorted(Path(directory).glob("*.dspreset")))
    samples = []
    for group in ET.fromstring(preset.read_text(errors="replace")).iter("group"):
        attributes = {k.lower(): v for k, v in group.attrib.items()}
        if attributes.get("trigger", "attack") != "attack":
            continue
        for sample in group.iter("sample"):
            fields = {k.lower(): v for k, v in sample.attrib.items()}
            x, rate = soundfile.read(str(preset.parent / fields["path"]), always_2d=True)
            samples.append((int(fields["rootnote"]), x.mean(axis=1), rate))
    shift = Counter(sounding_midi(x, rate) - root for root, x, rate in samples).most_common(1)[0][0]
    return [(root + shift, x, rate) for root, x, rate in samples if FIRST <= root + shift <= LAST]


def render_notes(path):
    x, rate = soundfile.read(path, always_2d=True)
    x = x.mean(axis=1)
    return [(FIRST + k, x[int(k * SPACING * rate):int((k * SPACING + HOLD + 0.1) * rate)], rate)
            for k in range(LAST - FIRST + 1)]


def report(label, notes):
    rows = [measure(x, rate, midi) for midi, x, rate in notes]
    registers = []
    for low, high in REGISTERS:
        values = [r["inharmonicity"] for r in rows if low <= r["midi"] <= high and abs(r["inharmonicity"]) < 0.01]
        registers.append(f"{np.median(values) * 1e4:5.2f}" if values else "    -")
    decays = []
    for low, high in zip(BANDS[:-1], BANDS[1:]):
        values = [t for r in rows for f, t in r["decays"] if low <= f < high and t < 30]
        decays.append(f"{np.median(values):5.2f}" if len(values) >= 3 else "    -")
    beating = np.array([r["beating"] for r in rows])
    shares = [np.nanmean(beating[:, n] > 6) * 100 for n in range(4)]
    print(f"{label:12s} {len(rows):3d} | " + " ".join(registers)
          + " | " + " ".join(decays)
          + f" | {np.nanmedian([r['glide'] for r in rows]):+5.1f} {np.nanmedian([r['rise'] for r in rows]):5.1f}"
          + " | " + " ".join(f"{s:3.0f}" for s in shares))


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--write-midi":
        write_midi(sys.argv[2])
        return
    packs = [a for a in sys.argv[1:] if "=" not in a and a != "--render"]
    renders = [a.split("=", 1) for a in sys.argv[1:] if "=" in a]
    if not packs and not renders:
        sys.exit(__doc__)
    print("                 | B x1e-4 by register" + " " * 10 + "| partial T60 (s) by band from"
          + " " * 18 + "| glide rise | beating %")
    print(f"{'':12s}   n | " + " ".join(f"{low}-{high}"[:5].rjust(5) for low, high in REGISTERS)
          + " | " + " ".join(f"{b:5d}" for b in BANDS[:-1])
          + " | cents   ms | H1  H2  H3  H4")
    for directory in packs:
        report(Path(directory).name[:12], pack_notes(directory))
    for label, path in renders:
        report(label[:12], render_notes(path))


if __name__ == "__main__":
    main()
