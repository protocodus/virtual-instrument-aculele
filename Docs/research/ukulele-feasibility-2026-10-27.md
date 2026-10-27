# Aculele: can Acustra's model make a ukulele?

Experiment, 2026-10-27, made as `Experiments/Aculele` on a work branch of `virtual-instrument-acustra` (commit `a42cb24`) and moved unchanged into this repository; the demos re-render bit-identical here. It is DSP only, with no UI.

Everything here is measured; nobody had listened to the renders when it was written.

## Short answers

1. **Close to a real instrument?** Partly. The string side is physics from published nylon and fluorocarbon measurements, and it holds up:
   - open strings are within 3 cents;
   - the inharmonicity matches the model's own prediction;
   - a real G4's fundamental decays in 3.1 s against 3.4-3.8 s here;
   - the bridge-force spectrum sits close to a real recording.

   The body is the weak half. No ukulele has been measured, so it is Acustra's flamenca-guitar bank shrunk by plate scaling. Its air and top resonances land where builders report them, but it is still 7-12 dB too bright at 1-2 kHz. The result reads as a ukulele, not yet as a particular real one.
2. **Does it sound good?** That is a listening question. The review pack put the preset demos beside a real ukulele recording and Aculele playing the same notes on the same strings with the same timing, matched in loudness. Listen for:
   - a "plinky" 1-2 kHz edge, the known gap;
   - whether the strums feel like a finger, not a pick;
   - whether the C string's slightly stretched overtones read as a real ukulele.
3. **Is a full instrument feasible?** Yes. The engine carried over with about 600 changed lines out of 13,900. It builds as C++17 with `-Werror` for the Rack, and plays at about 9% of one CPU core. What is missing is data, not code (see the last section).

## What it is

Acustra 1.9.0's engine and Performer, with:

- four re-entrant strings in Standard (G4 C4 E4 A4), Low G and D tuning;
- soprano, concert and tenor bodies from Kala's dimensions;
- nylon or fluorocarbon strings from real set gauges;
- the 13.3 mm saddle spacing;
- a new finger Strum technique (the index finger swept over the neck joint);
- tab input, with one MIDI channel per string, so chords like G (0232) and Am (2000) can double a pitch.

Guitar-specific voicing is switched off. The README's How it works lists every change.

## Measurements

| Check | Result |
| --- | --- |
| Open-string tuning | within 3 cents, all strings and sets |
| Inharmonicity, concert nylon C4 | 8th partial +35 cents. The thick unwound C string is genuinely stretched; the model predicts +37. |
| Body A0 / T1 (spruce) | 266/653 Hz soprano, 215/485 concert, 190/414 tenor. Builders report tenors at about 211-214 Hz. |
| Harmonic balance vs a real ukulele (H2-H8, first 150 ms) | 7 dB average difference for the concert, 11 dB for the soprano. With Acustra's guitar recording EQ left in it was 16 dB, so that EQ is now off. |
| Third-octave spectrum vs the same recording | within about 4 dB from 250-800 Hz (soprano); 7-12 dB too bright at 1-2 kHz |
| CPU | an 11.5 s demo renders in 1.03 s on one 2.8 GHz Xeon core |

The real-ukulele reference is one recording of an unidentified instrument, recorded on a budget USB microphone. It is evidence, but weak evidence.

## Review pack

The demos and comparison files were handed over for listening, not committed:

- one demo per preset, 48 kHz 24-bit, 10-12 s, at -18 LUFS, rendered by `RenderAculelePresetDemos` from `Docs/audio/midi/`;
- "Ukulele Arpeggio" by David Baker, Duke Sonic Dictionary, licensed CC BY-NC 4.0, for listening only (see `THIRD_PARTY_NOTICES.md`);
- Aculele's soprano and concert playing the reference's notes, matched to its loudness (-24.4 LUFS; trims +2.5 and +0.7 dB).

## What a full instrument needs

In order of how much they matter:

1. **A measured ukulele body.** This is the single biggest item and needs a real instrument. Measure bridge admittance and radiation on one or two ukuleles, the way g21 was measured for Acustra: an impact hammer at the bridge, plus a microphone or accelerometer. Without it, the remaining brightness can only be guessed at.
2. **Real ukulele recordings** for calibration, comparable to GuitarSet for Acustra: strums and fingerstyle on known instruments.
3. **Strumming calibration:** finger-strum speed, nail against flesh, chucks, triplet rolls. Today these reuse guitar-pick data.
4. **Playing on one MIDI channel.** Acustra's hand planner already places C and G7 correctly, but one channel cannot ask for a doubled pitch. Either keep tab input or add a "full strum" option that fills the unison.
5. **Product shell, mostly reuse:** Acustra's JUCE plug-in and editor, presets, a test suite adapted from Acustra's, then a Rack adapter, in this repository.

The recommendation was to decide by ear on the demos and the comparison, and, if it is worth pursuing, to measure a body (item 1): that is the step that turns this from plausible into accurate.
