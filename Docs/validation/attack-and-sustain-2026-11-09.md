# Attack, sustain and CPU, 2026-11-09

The baseline is 0.4.55 (`00f5deb`). The candidate is 0.4.56. Both are Release
builds from the same compiler and flags (GCC 13.3, Ninja). Every render is
48 kHz in 256-sample blocks. Two changes are audible and have their own A/B
sets; the third is a CPU change that leaves the sound in place. Nothing here
was fitted to a recording, and nobody has listened to the sets yet.

## The reference

The reference is FreePats' "Ukulele" of 2026-08-11, CC0 1.0: a Flight Fireball
tenor (solid mango, Worth Brown fluorocarbon), thirteen single notes C4-C6.
`Tools/compare_recorded_notes.py` gives its source, hash and the commands. Its
render is Tenor Fluorocarbon Fingerstyle with Mahogany on the mono microphone,
each note alone at velocity 90. The recording is high-passed at 190 Hz and
level-matched note by note, so only decays and relative levels compare.

## The release gives back the whole held force

Before a pluck lets go, the hand holds the string aside, and the saddle
carries the static force of the string's slope, `T y / a`. The release gives
that force back to the bridge as a step, and the top springs back with a knock
(`initialisePluck`). Acustra gave back 0.45 of it: 0.8 chosen by ear on a
guitar, then 5 dB less once its saddle anchors were retired. Aculele kept that
guitar value. 0.4.56 gives back the whole force, the physical value.

- **The knock's size.** The 2026-11-05 comparison against the FreePats tenor
  found the release knock 25-30 dB under the note and weak against the
  recording's. At its full physical size, it would match the recorded
  tenor's air-mode knock within 0.2 dB. At 0.45 it stood about 7 dB
  (20 log10 0.45) under that.
- **Continuity in force.** Swept from 0 to 1.6 of the force, the 1-2 kHz
  energy within 4 ms of each onset is least at 1.0. The values are in dB
  against the same band's 30-330 ms level per 4 ms, median over the 13 notes:

  | Share | 0 | 0.45 | 0.7 | 1.0 | 1.3 | 1.6 |
  | --- | ---: | ---: | ---: | ---: | ---: | ---: |
  | 1-2 kHz | +5.1 | +4.5 | +4.1 | +4.0 | +4.7 | +5.8 |
  | 0.5-1 kHz, between partials, first 20 ms | -10.1 | -11.4 | -11.5 | -15.3 | -12.0 | -8.4 |

  The second row is the energy between the partials over the first 20 ms,
  against the recording's, in dB. The full step cancels the most of it.

- **One side effect moves away from the recording.** The real tenor's first
  20 ms carry a woody, noisy layer between the partials at 0.5-1 kHz. The
  render stood 11.4 dB under it and now stands 15.8 dB under. That layer is
  not the release step's: even with no step the render is 10 dB under it.
  Its source, probably the finger's contact and the body's response to it,
  is still missing from the model.
- **Partial levels over 30-330 ms** are unchanged within 0.1 dB in every
  third octave.

## Open strings ring as long as 0.4.33 fitted

0.4.33 fitted a loss of 0.022 dB per round trip at a fretted note's stop
against five sampled ukuleles. In them, open strings ring 1.15-1.46 times as
long as fretted notes. That fit replaced Acustra's 0.0074 dB, which every
string had lost alike (Docs/decisions.md, 2026-10-30). 0.4.54 put the
0.0074 dB back on the open strings because open C had the slowest held decay
in a survey. But open C rings longest on a real ukulele too. No recording or
decision was given for the change. 0.4.56 takes it off again; fretted notes
are unchanged.

Partial T60s against the FreePats tenor, the median within each band, in
seconds:

| Band | Recording | 0.4.55 | 0.4.56 |
| --- | ---: | ---: | ---: |
| 200-400 Hz | 4.89 | 3.71 | 4.13 |
| 400-700 Hz | 3.85 | 2.35 | 2.35 |
| 700-1000 Hz | 2.77 | 2.69 | 2.69 |
| 1-1.5 kHz | 1.90 | 1.95 | 2.04 |
| 1.5-2.2 kHz | 1.62 | 1.50 | 1.54 |
| 2.2-3.2 kHz | 1.31 | 1.16 | 1.22 |
| 3.2-4.7 kHz | 0.72 | 0.88 | 0.89 |
| 4.7-7 kHz | 0.50 | 0.61 | 0.61 |

The open notes' fundamentals, T60 in seconds:

| Note | Recording | 0.4.55 | 0.4.56 |
| --- | ---: | ---: | ---: |
| C4 | 7.62 | 5.92 | 7.32 |
| E4 | 5.87 | 3.24 | 4.13 |
| G4 | 1.86 | 4.09 | 5.12 |
| A4 | 4.15 | 1.43 | 1.54 |

G4 and A4 are each held by a body mode near their pitch, on one instrument or
the other. The recorded mango tenor drains F#4 and G4. The model's tenor
drains A4, whose fundamental dies in 1.5 s against the recording's 4.2.
That is the 400-700 Hz band's gap, and it is the body's, not the string's.
No measured tenor body is available, so it is left.

## The whole keyboard

`AculeleNoteSurvey --mode keys --velocities 20,50,80,100,115,127` covers
every key of the seven presets at six velocities, 1,236 notes per engine.
Each paired difference is 0.4.56 minus 0.4.55:

| Measure | Median | Range |
| --- | ---: | ---: |
| Level over the note | +0.05 dB | 0 to +2.8 |
| Brightness (share above 2.5 kHz) | -0.01 dB | -0.8 to +0.5 |
| Held decay, open strings | 22.1 to 18.7 dB/s | |
| Held decay, fretted notes | 42.3 to 41.8 dB/s | |
| 100 ms after key-up | +4.7 dB | 0 to +9.8 |

- **Evenness.** The level's spread about a line in note number moved by
  -0.23 to +0.03 dB on every preset and velocity. Its worst notes are the
  same ones, the radiation holes the 0.4.46 survey kept, except on Tenor
  Picked Melody at velocities 20-80. There its A-string fret 1 (A#4) rose
  0.9 dB toward the line, so the low G's fret 2 (A3), which already stood
  6.5 dB over it, is now the largest residual.
- **Key-up tails.** The unplayed open strings, which the bridge drives,
  ring on longer, so they are louder 100 ms after key-up. They still sit a
  median 50 dB under the note, against 55. At 0.3-1.0 s the strongest bin
  off the played note's harmonics, mostly a sympathetic string, rises a
  median 2.6 dB.
- **Highest frets.** On Tenor Picked Melody's A-string fret 18 that bin was
  already over the note's own harmonics. It now stands +5.9 dB over them at
  velocity 50, against -2.0.
- **Velocity.** Against each key's velocity-127 level, velocities 50-115 sit
  a median 0.11 dB under the 20 log10(v) law on both engines (10-90%: -1.0
  to +0.1 dB). Velocity 20 sits 2.6 dB under it, where the plectrum's slip
  makes soft notes quieter. From 127 to 50 a note's share above 2.5 kHz
  falls a median 0.49 dB for every dB it is played softer, on both (0.48 on
  0.4.55; the Kai pack's velocity layers fall 0.29, and 0.43 over C5-C6;
  [dynamics](dynamics-2026-10-29.md)). Neither change moves the keyboard's
  response to velocity.

## CPU: quiet strings skip the stretch sum

A callgrind profile of Concert Strum's demo, 4 s on 0.4.55, by self cost:

| Function | Share |
| --- | ---: |
| `renderStringAxialForce`, the string's stretch | 32.2% |
| `BodyBank::render` | 13.1% |
| `process` | 11.1% |
| `BridgeLoad::advanceModes` | 7.4% |
| `RoomAmbience::step` | 4.8% |
| `atan2` in the dispersion designs made ahead of the notes | 4.4% |

The designs made ahead are a warm-up, one search step a block, that ends
once every fret of every string is designed ([CPU at 0.4.46](../benchmarks/cpu-2026-11-07.md));
they are not a note-on cost.

The stretch sums every string's squared slopes at every sample, across two
lines of about a hundred cells each. Its force goes as the square of the
motion. Once a string's own bridge force has fallen 40 dB from its peak, the
stretch force stands 80 dB under its own peak. The sum then stops. The
longitudinal ring rings out on what it holds, and a re-pluck or a string the
bridge drives back up starts the sum again. Measured with `stringAxialForce`
off on the 13 tenor notes, the stretch force is 29-49 dB under the note over
its first 300 ms. At the gate it is 69 dB or more under it.

- **Instructions (callgrind).** The 13 held notes take 37.56 G against
  40.21 G, 6.6% fewer. Concert Strum's 4 s demo is unchanged at 5.84 G: no
  strummed string falls 40 dB while it is held.
- **Wall clock.** `RenderAculelePresetDemos --cpu`, alternated twice
  between builds, reads 0.053-0.079x real time for both builds on this
  shared machine. The difference is within the run-to-run spread.
- **Sound.** Against the same source without the gate, the seven demos
  differ by 105-125 dB under their RMS and peak at -97 dBFS or less. The 13
  held notes differ by 82 dB or more under each note's RMS.
- **A 30 dB gate.** It would save 8.0% and leave 72 dB under the note. It was
  not taken.

### In Reason's build

The stretch sum's four-wide path was compiled only for x86-64. The Jukebox
SDK's compiler vectorises no loop itself, so in Reason the sum ran one cell
at a time on every machine. It now takes the four-wide path under the SDK
too, adding the squares in the scalar order, as Acustra's does (`d86e59d`).
The SDK is not in this session, so two stand-ins were built from the same
source before and after, each with `JUKEBOX_SDK` defined, no
auto-vectorisation, no FP contraction and `-fno-builtin`:

- **Clang 18, x86-64, `__SSE2__` undefined.** The output is the same bit for
  bit on Concert Strum's demo (6 s) and the 13 tenor notes (53 s). Concert
  Strum's 4 s take 5.69 G instructions against 6.70 G, 15% fewer; the stretch
  sum takes 1.96 G against 2.97 G. The 13 held notes take 35.57 G against
  36.65 G, 2.9% fewer, since the quiet-string gate already skips most of
  their sum.
- **Clang 18, ARM64, under qemu.** The output is the same bit for bit on both
  renders, and the stretch sum now runs on NEON four-lane arithmetic
  (`fmul`, `fsub`, `fadd` and `fcvtzs` on `.4s`), where before it had none.

Native builds take the path they took before, and their samples are
unchanged bit for bit. `build45.py compile-only` and a CPU reading in Recon,
with the SDK, are still to do.

## Clean-up

These were removed. None was read by the engine, its tools or the Rack
adapter:

- the no-op `PerformanceRealism` flags `woundRidges`, `longitudinalSqueak` and
  `fretCrownGrazing`;
- `AculeleEngine::bodyWoodProperties`.

The comment on the pick's half-strength contact burst said Finger kept the
whole burst. Since 0.4.17 every Finger stroke is a nail stroke and takes the
half. Only the pad strokes, a thumb's downstroke and a strum's upstroke, keep
the whole burst. The comment now says so.

## Levels

Together the two audible changes raise typical playing by 0.04-0.54 dB VU. `LevelMatchData.h`
was re-pinned with `AculeleLevels --write`. Tenor Picked Melody's Output moves
from -1.0 to -1.2 dB, as `--report` suggests. The other presets read within
0.2 dB of their trims, and `Aculele.Levels` passes.

## Checks

- **Unit:** all 25 tests pass.
- **Audio:** `Aculele.AudioDemos` and `Aculele.Levels` pass.
- **Perf:** `Aculele.CpuBudget` passes.
- **C++17:** `AculeleDSPCxx17` builds.
- **ARM64:** `AculeleEngine.cpp` compiles with Clang 18 for aarch64 under
  `-std=c++17 -Wall -Wextra -Werror -fno-builtin`, with and without
  `JUKEBOX_SDK`.
- **Listening sets.** Rendered, not committed:
  - `aculele/listening/2026-11-09-release-knock/`
  - `aculele/listening/2026-11-09-open-string-ring/`

  Each has A (0.4.55), B (0.4.55 with that one change) and a separate
  `key.md`.
