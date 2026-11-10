# Fix-only pass, 2026-11-10

A second read of the model after the
[2026-11-09 consolidation audit](consolidation-audit-2026-11-09.md), for
defects only: no control, mechanism or preset is new, and nothing is
re-voiced. It closes that audit's open reading on the koa tenor's F4 and
three of its "Not examined" items (the Performer, the rates other than 48 and
96 kHz, the block partition), and it found one defect, in how the body bank
is carried to other sample rates. Engine 0.4.63 at the start, 0.4.64 at the
end. The CPU work of the same day is in
[cpu-axial-sum-2026-11-10.md](../benchmarks/cpu-axial-sum-2026-11-10.md).

Renders in this document come from the build nearest the Rack device's
(Clang 18, C++17, `-O2 -fno-builtin`, SSE3, `ACULELE_EXTENDED_SAMPLE_RATES`),
on the development container.

## The body's radiation at other sample rates (0.4.64)

**Found.** The body bank was fitted at 48 kHz, and the instrument was voiced
and level-matched there. At any other rate it radiated differently. Each
body's response from the bridge to the two microphones (the bank's impulse
response through `BodyBank::render`, its transform at twelve points an
octave, the power of a third of an octave against the same third at 48 kHz;
all twelve bodies, both microphones):

| 0.4.63, worst third octave against 48 kHz | below 5 kHz | 5-10 kHz |
| --- | --- | --- |
| bridge force, 44.1 to 192 kHz | 18.6 dB | 4.4 dB |
| bridge moment, 44.1 to 192 kHz | 27.6 dB | 3.7 dB |

On the left microphone alone the force input stood up to 6.9 dB off under
1 kHz at 44.1 kHz and 12.8 dB at 88.2 kHz and above (RMS over the bodies and
thirds 1.2 and 3.1 dB), and 0.6-1.5 dB off from 1 to 10 kHz. Played phrases
showed it: the Soprano D Tuning demo's whole-file RMS was 0.63 dB lower at
96 kHz than at 48 kHz and the Soprano Thumb's 0.39 dB lower. The earlier
surveys across rates read each note's K-weighted level and decay, which this
moves little, and the piezo preset, which does not use the bank, agreed
within 0.1 dB at every rate then as now.

**Cause.** Three things, all in `AculeleEngine::buildBodyConfiguration`:

1. The bank sounded every measured mode below 0.46 fs. The ukulele's scale
   carries Acustra's modes to about 30 kHz, so at 48 kHz those above 22.08 kHz
   are silent, and 88.2 kHz and above sounded some twenty modes the 48 kHz
   instrument never plays. Their tails reach down: the sum of the Concert
   mahogany's modes at 0 Hz was +0.234 at 48 kHz and +0.023 at 96 kHz.
2. At 44.1 kHz the modes between 20.29 and 22.08 kHz were dropped, tails and
   all.
3. Each mode's conversion keeps its 48 kHz level at its resonance (the hold
   droop), which leaves its value far below the resonance off by
   that factor, 0.75 for a mode at 22 kHz played at 96 kHz. The high modes'
   tails nearly cancel under 1 kHz, and their errors do not: with the same
   modes sounding at every rate, a third octave under 1 kHz still stood
   4.4-5.6 dB off at 88.2 kHz and above.

**Fixed.** The same modes sound at every rate (those below 0.46 x 48 kHz). A
mode of the 48 kHz bank that the rate cannot reach leaves its static part, its
48 kHz value at 0 Hz. Each sounding mode's shortfall at 0 Hz and those static
parts gather in one last mode with its pole at zero, which passes the input
straight to the microphones. The 48 kHz bank has no such mode and is
untouched.

| 0.4.64, worst third octave against 48 kHz | below 5 kHz | 5-10 kHz |
| --- | --- | --- |
| bridge force, 44.1 to 192 kHz | 0.46 dB | 0.89 dB |
| bridge moment, 44.1 to 192 kHz | 1.21 dB | 2.09 dB |

On the left microphone the force input now reads within 0.08 dB under 1 kHz,
0.19 dB to 5 kHz and 0.34 dB to 10 kHz at every one of those rates (RMS
0.01-0.03, 0.02-0.03 and 0.08-0.11 dB). `Aculele.RadiationRates` holds the
table's figures with limits of 0.6 and 1.2 dB on the force and 1.5 and 2.5 dB
on the moment; built against 0.4.63's engine it fails 5,680 of its 15,360
readings.

- **48 kHz is unchanged, bit for bit:** the seven demos and the stress,
  body-change, String Age and bridge-hand phrases through every preset, 35
  renders, hash for hash as 0.4.63. `LevelMatchData.h` and the presets' trims
  were fitted at 48 kHz and stand.
- **Other rates change audibly.** Against 0.4.63 the seven demos differ by a
  residual 15-29 dB under the signal at 44.1 kHz and 16-26 dB under it at
  96 kHz (the piezo preset by nothing). Their whole-file RMS now sits within
  0.03 dB of the 48 kHz render at 44.1 kHz and within 0.06 dB at 96 kHz
  (from up to 0.13 and 0.63 dB).
- **The demos' spectra follow.** Each demo's third octaves from 100 Hz to
  10 kHz (both channels' energy over the whole render, bands within 50 dB of
  its loudest) against its 48 kHz render: at 0.4.63 the worst band stood
  0.3-4.4 dB off at 44.1 kHz (Concert Strum's 125 Hz) and 3.2-6.4 dB off at
  96 kHz on the six microphone presets, all between 100 and 200 Hz; at
  0.4.64 the worst is 0.4-0.8 dB at 44.1 kHz and 0.4-1.8 dB at 96 kHz, now
  in the upper bands. The piezo demo reads 0.3 and 1.0 dB in both versions.
- **Rates under 44.1 kHz** take the same construction. At 32 and 22.05 kHz
  the left microphone's force response stood up to 12.2 and 19.2 dB off under
  1 kHz; it now reads within 0.27 and 0.59 dB there and within 0.26 and
  0.72 dB to 5 kHz. Near those rates' band edges a dropped mode's static
  part is a poor stand-in for its resonance, and the 5-10 kHz thirds stay
  about as far off as they were: 2.0 and 5.7 dB, from 2.7 and 6.9.
- **The mid-band pair** the capture filter dips (`locateMidBand`) is now the
  48 kHz pair on every body at every rate from 22.05 to 192 kHz, except the
  koa Soprano's hole at 88.2 kHz and above, one search step higher (740.6
  for 735.3 Hz).

**For the ear.** The change is a direction only in the sense that 48 kHz is
taken as the instrument. A level-matched set, `A` 0.4.63 and `B` 0.4.64 at
44.1 kHz and at 96 kHz with the 48 kHz render beside them, went to the owner
on 2026-11-10 (not committed; its `key.md` names the letters and the trims).
No verdict is recorded here: the candidate is on the branch under the owner's
standing rule that it ships unless they say otherwise.

**Left.** From 5 kHz up a third octave still reads up to 0.9 dB off on the
force input (the right microphone; 0.34 dB on the left): a mode's tail is
right at 0 Hz and at its resonance and approximate between, and for the
modes above 10 kHz "between" is the top of the band. Closing it needs a zero
beside each high mode's pole.

## Six rates, every string

112 single-string notes (seven presets, four strings, frets 0, 5, 10 and 12),
2 s each, at 44.1, 48, 88.2, 96, 176.4 and 192 kHz at 0.4.63, and again at
44.1 and 96 kHz at 0.4.64. No output sample is non-finite at any rate. Each
note's reading less its 48 kHz reading, as mean, standard deviation and
range over the 112:

| | 44.1 kHz, 0.4.63 | 44.1 kHz, 0.4.64 | 96 kHz, 0.4.63 | 96 kHz, 0.4.64 |
| --- | --- | --- | --- | --- |
| Level, first 0.2 s (dB) | +0.05, 0.37, -1.51 to +1.73 | +0.01, 0.09, -0.20 to +0.33 | -0.10, 0.43, -1.41 to +1.39 | -0.03, 0.11, -0.39 to +0.24 |
| Level, 0.2-0.6 s (dB) | -0.08, 0.65, -2.61 to +2.00 | +0.01, 0.11, -0.29 to +0.43 | -0.15, 0.69, -1.92 to +2.39 | -0.04, 0.14, -0.94 to +0.39 |
| Fall from the first 0.2 s to 0.6-1.0 s (dB) | +0.21, 0.84, -2.52 to +4.84 | +0.01, 0.11, -0.39 to +0.43 | +0.28, 0.67, -1.36 to +2.86 | +0.02, 0.18, -0.57 to +1.35 |
| Pitch (cents) | +0.04, 0.12, -0.19 to +0.63 | 0.00, 0.01, -0.07 to +0.05 | +0.01, 0.21, -0.67 to +0.97 | -0.01, 0.05, -0.32 to +0.10 |

At 0.4.63 the piezo preset's notes agreed within 0.1 dB at every rate while
the microphone presets' did not, which pointed at the radiation. With the
radiation the same at every rate, a note's level, its fall and its pitch
follow: the fall, up to 4.8 dB from its 48 kHz reading on single notes, is
within 0.4 dB at 44.1 kHz and 1.4 dB at 96 kHz. The 2026-11-09 audit's decay
table (96 kHz less 48 kHz, standard deviation 0.6-1.3 dB/s about no mean)
carried that spread and read it, rightly as far as it went, as no direction.

**F4 on the koa tenor's low G string**, which the 2026-11-09 audit left
reading "13 cents apart at 48 and 96 kHz", is not a tuning difference. That
reading took the fundamental's peak over the note's first half second, where
the two polarisations are still parting (Known gaps, notes beside the top's
main resonance). Over 1-2 s the note reads -0.21 to -0.15 cents at all six
rates, and -0.73 to -0.63 on the mahogany Low G Tenor.

## The block partition

Each preset's demo and the stress phrase, every event on its own sample,
rendered in blocks of 1, 64 and 480 samples and in blocks of random length
up to 700, against the 256-sample render: bit-identical in all 56 cases at
each of 44.1, 48 and 96 kHz.

## The Performer

`AculelePerformer.cpp` and `.h` read end to end: voice and string
allocation, the chord gathering window and its held-back events, strum
planning and scatter, the direction and chuck keyswitches, sustain, panic,
string-per-channel mode, pitch bend, the RPN and NRPN parser, MPE timbre and
pressure.
No functional defect found. Five comments still described the guitar it was
forked from (a six-string allocator, strums of five or six strings, six
scatter draws, CC126 = 6); the code has four strings throughout, and the
comments now say what it does.

## Reason's layout, built and played here

The Rack compiles the engine with `ACULELE_EXTENDED_SAMPLE_RATES`, which
makes every delay line four times as long and changes how reads wrap. This
repository's build never set it, so no test or budget here ran the layout
the device ships, and the CPU defect of this pass lived only there
([benchmark](../benchmarks/cpu-axial-sum-2026-11-10.md)). `AculeleDSPCxx17`,
the C++17 `-Werror` build of the Rack's sources, now sets it, and
`Aculele.SampleRateLayoutParity` (after Acustra's) plays chords, re-struck
strokes, a bend, the wheel, All Notes Off and a reset through both layouts at
44.1, 48, 96 and 192 kHz and requires the same bytes. They are the same; the
35 renders of the null tests are too, at 48 kHz.

## Found and not fixed

- The radiation from 5 kHz up, 0.9 dB (above).
- **The top of the band at 96 kHz.** The piezo demo's 10 kHz third octave
  reads 1.0 dB from its 48 kHz render at 96 kHz (0.3 dB at 44.1 kHz), and
  the microphone demos' 8-10 kHz thirds 1.4-1.8 dB on three presets. The
  pickup does not pass through the body bank, so part of that lies in the
  strings, the pluck or the pickup's chain. Not traced.
- The 2026-11-09 audit's other open items stand: the heaviest blocks over
  10% of a period (now lower, see the benchmark), C6 on the spruce tenor,
  the search a ready engine still makes for one re-pluck in 36 of a stiff
  note, the subnormal tails, and Acustra 1.9.36's pick release, not
  compared.

## Not examined

The room and the output stage across rates; MPE beyond reading its handlers;
the units and signs of the string, bridge and pickup blocks beyond what their
tests hold; Acustra's changes before 1.9.26.
