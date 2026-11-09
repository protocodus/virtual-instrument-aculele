# Consolidation audit, 2026-11-09

A review of the engine ahead of release, by what a player and a host meet:
notes around a construction change, the heaviest blocks, a held chord and
an idle instrument, every key at three velocities, decay against the
sample rate, library calls in Reason's build, and Acustra's changes since
the fork. Each section says what was checked, what was found and what a
fix measured; what was not examined is listed at the end. It starts from
0.4.58 (`0bfe1a1`). 48 kHz and 256-sample blocks unless a rate is named;
timings are from a 4-core Intel Xeon VM at 2.10 GHz shared with other
work, GCC 13.3 `-O3` unless the Reason-like build is named, each block's
fastest of three or five runs, and are development-machine figures, never
Reason's CPU meter.

## Notes during a body, strings, tuning or age change (0.4.59)

**Found.** 0.4.58 ignored note-ons while the strings' dispersion designs
were made again after a construction change, 0.6-2.4 s on the factory
presets, so that no note ran its design's search in its own block. A note
played in that window was lost.

**Why the designs take that long.** It is pacing, not work. A factory
preset has 76 designs at 48 kHz (four strings, frets 0-18). Searching all of
them from nothing takes 2-39 ms on this machine. 59-71 are within the
search's 3-cent tolerance after its first fit; the other 5-17, stiff notes
from the 12th-14th fret up, run all sixteen fallback starts, 1.5-3.9 ms
each. The designs are made one start every 5.3 ms of audio so that no block
carries more than one, which spreads those few milliseconds over seconds.
A whole search does not fit a note-on block, so the finished designs cannot
simply be made at the note.

**What a note can sound on meanwhile.** Over every body, string set and
tuning at String Age 0, 0.15, 0.5 and 1 (72 constructions, 5,472 designs at
48 kHz), 740 designs need the fallback, none below the 5th fret and none on
an open string at 44.1, 48 or 96 kHz. For those 740, the worst of the first
twelve resonances against the stiff string's, in cents:

| Design | Mean | 90th | 99th | Worst |
| --- | --- | --- | --- | --- |
| The collocated single section | | | | 573-1446 on the factory presets |
| The previous construction's design for the slot, one control moved | 11-57 | 21-163 | 39-269 | 51-297 |
| The search's first fit | 13.0 | 16.7 | 164 | 405 |
| First fit and one fallback start | 7.3 | 14.1 | 29.0 | 43.8 |
| First fit and ten fallback starts | 7.1 | 14.0 | 28.5 | 32.0 |
| Finished (sixteen starts) | 4.5 | 8.2 | 13.0 | 13.8 |

The first fallback start removes the bad cases and the next nine add almost
nothing, so the interim design is the first fit and one start
(`interimDispersionStarts`). Every other design is finished by its first
fit, and its interim design is its finished one.

**Fix.** Notes always sound. While the engine is not ready
(`AculeleEngine::ready`), a design nothing made matches is the interim one,
marked not complete, at the note (`solvedDispersion`). The designs made
ahead (`prepareDispersion`) give every slot its interim design first, one a
step, then finish the stiff slots' searches a start a step, as a search
from nothing runs them. A string holding an interim design takes the
finished one at its next fresh pluck, when its waves are cleared
(`configureVoice`); a ringing note is never re-designed under way. The
Performer's note-on gate is gone.

**Measured.**

- The seven demos (24-bit WAV) and the stress phrase through all seven
  presets (stereo float, every sample) render bit for bit as 0.4.58.
- The designs made while playing after a Shape change equal a freshly
  prepared instrument's, bit for bit, on every preset (`Aculele.Ready`),
  which equal a search from nothing (`Aculele.PreparedDispersion`).
- Ready again 0.62-2.45 s after a Shape change (0.59-2.43 s in 0.4.58).
- A chord rings, the body changes size under it, and four-note chords
  follow in the next block and every 0.2 s, alternately high on the neck:
  0.4.58 plays none of the chords for 0.6-2.4 s; 0.4.59 plays them all. The
  worst block is the change's own or the one after it: 19-34% of the
  period in 0.4.58, 19-29% in 0.4.59.
- String Age automated every block from 0 to 1 over 2 s under chords high
  on the neck re-struck every 0.2 s: 0.4.58 loses every chord struck from
  the sweep's start until the designs are made again after it, and its
  worst block takes 19-132% of the period (each age step searches every
  ringing stiff string's design in full); 0.4.59 plays every chord and its
  worst block takes 24-45%.

**Left.** An age step still designs each sounding string again, now with
the interim design's two steps rather than the whole search. A ready
engine still searches in full at the note for a design nothing made
matches: a stiff note re-plucked off its fret's pitch whose continued
design misses by more than a cent beyond the last search (README, Known
gaps).

## The heaviest note-on block (0.4.60)

**Found.** The stress phrase's heaviest block, eight notes struck at once
over ringing strings, ran 10.1 million instructions against 4.8-6.1 million
in the blocks either side, and 2.9 million of them were `memset` and
`memcpy`: each string's re-pluck arrival ring (128 KB at 48 kHz) was cleared
whole at every note start and copied whole into the tail a re-plucked note
leaves ringing.

**Fix.** A ring holds zeros outside its pending arrivals: `add()` writes
only inside them and `process()` zeroes each slot it reads. `clear()` and a
copy now touch the pending slots alone (`RepluckArrivals`).

**Measured.** 8.3 million instructions (-18%), first-level data misses
139,798 to 96,671, the block 10.7-12.9% of the period from 12.6-15.6%
(`Docs/benchmarks/cpu-consolidation-2026-11-09.md`). The demos, the stress
phrase and the body-change and String Age phrases render bit for bit as
0.4.59. `Aculele.RepluckArrivals` holds the invariant: 20,000 seeded adds,
reads, clears and copies against a ring that clears and copies whole
planes, and no slot set outside the pending ones after any block of a
rapidly re-plucked phrase on every preset.

**Left.** The strings' own delay lines, 32 KB each, are still cleared and
copied whole at a note start (18 and about 14 in that block); a ringing
line has no pending window to bound them by.

## A held chord and an idle instrument (0.4.61)

**Found.** Acustra's pass found it in the string loop the two engines
share (its 1.9.38, `65648c0`): the idle flush, which clears the instrument
once everything before the output gain has been under 1e-11 for 80 ms,
waited for every string to be handed back, so a chord kept under its keys
or the pedal never reached it. Aculele's 1e-30 floors (the line write, the
longitudinal ring of 0.4.57) do not cover the loop's filter memories. With
flush-to-zero off, the open chord G4 C4 E4 A4 held for 130 s on each
factory preset: the denormal-operand flag raised for good from 15-32 s, a
block's mean 24-33% of the period from 50-110 s on where it was 4.2-5.0%,
for as long as the chord was held.

A second site showed in the trace of a chord let go: each string's level
observer (`finishVoice`) decays from the flushed residue to the smallest
subnormal float and stays there, a subnormal operand in every sample for as
long as the engine is awake. `suspend()` zeroes the observers, but nothing
in this repository calls it: the Performer and the demo renderer leave the
engine awake.

**Fix.** The flush runs with strings held. A held string's line, loop
filter memories and slope observers are zeroed in place
(`StringLoop::settle`) and it stays held; the others are handed back as
before; every string's level observer is zeroed. Once nothing is held the
instrument is flushed again as one, so a suspension still keeps exactly
what an awake engine holds.

**Measured.** At rest 13-27 s into the held open chord on the seven
presets, exact zero output from then on, no window after it raising the
flag, and the mean block at its playing level
(`Docs/benchmarks/cpu-consolidation-2026-11-09.md`). The demos, the stress
phrase and the body-change and String Age phrases render bit for bit as
0.4.60. `Aculele.Rest` plays, on every preset as a host with neither flag
set: a chord of which one string is let go, two are kept by the pedal and
one by its key, to rest (10.9-15.2 s); a body change under it; a pedal-held
note struck again; everything let go, to rest again (5.9-12.4 s). At each
rest every line, loop memory and level is zero, the output is exact
silence and twelve blocks raise neither the denormal-operand nor the
underflow flag. Against 0.4.58 it fails at the first step on every preset.

**Left.** Ordinary playing still passes through subnormal numbers in three
bounded tails (a finished contact's fractional-delay memories, a key-up
touch's envelopes, the fret motion's 75 ms drain): 4-9% of a demo's
8-sample blocks take at least one, and turning flush-to-zero on moves no
demo's mean or worst block outside the timing spread, so they are not
floored. A body change under a resting held chord sounds the fingers'
shift along the new scale at -81 to -94 dBFS; that is the fret motion
model doing what it does under a sounding chord.

## Every key at three velocities

**Checked.** `AculeleNoteSurvey --velocities 40,80,120` plays every key of
each factory preset alone after a reset, 206 keys and 618 notes, through
the preset as shipped. Each note's K-weighted level over its first second,
over C4-C6, less a straight line through them:

| Preset | SD at 40, 80, 120 | Largest step between neighbours | Furthest from the line |
| --- | --- | --- | --- |
| Concert Strum | 2.76, 2.54, 2.42 | 4.8-5.0 | -6.8 to -7.5 at D5 |
| Soprano D Tuning | 3.50, 3.42, 3.35 | 4.0-4.3 | -7.9 to -8.4 at F#5 |
| Tenor Fluorocarbon Fingerstyle | 3.00, 2.71, 2.55 | 9.4-10.0 | -6.2 to -6.7 at C6 |
| Low G Tenor | 2.59, 2.29, 2.16 | 6.3-6.4 | -5.8 to -6.7 at B4 |
| Soprano Thumb | 3.16, 3.09, 3.07 | 3.8-4.5 | -8.1 to -8.4 at F#5 |
| Tenor Picked Melody | 1.83, 1.65, 1.57 | 5.3-6.1 | -4.4 to -5.2 at A#4 |
| Concert Piezo Stage | 2.31, 2.04, 1.88 | 3.2-3.7 | -4.2 to -5.5 at B4 |

dB. Every preset spreads least at velocity 120 and 0.1-0.45 dB more at 40.
Four sampled ukulele packs spread 2.2-3.7 dB over the same keys with
largest steps of 5.4-12 dB
([benchmark](sample-pack-benchmark-2026-10-29.md)).

- **Velocity.** No note's distance from its preset's line moves more than
  2 dB between velocity 40 and 120, so no key is an outlier at one
  velocity only. From 40 to 120 a note gains a median 9.7-11.9 dB by
  preset (SD 0.3-1.75 dB over the keys); least 7.1-7.75 dB at G#3 on the
  two Low G presets, most 12.0-14.5 dB at the top keys of six presets
  (Soprano Thumb's keys differ least: 9.2-10.7 dB).
- **Pitch.** The fundamental reads a mean +1.1 cents (SD 1.9) and the
  pitch the partials carry +1.3 to +2.0 cents by preset and velocity; the
  largest is +5.8 cents (Concert Piezo Stage, G5).
- **The furthest keys are the ones the 2026-11-07 survey found**
  ([note survey](note-survey-2026-11-07.md)), the same key on six presets:
  A#4-F#5, most of them on the A string's first nine frets, where the top
  radiates little between and above its first modes (README, Known gaps).
  On the spruce tenor (Tenor Fluorocarbon Fingerstyle) C5 was the furthest
  and reads -5.9 to -6.5 dB now; C6 reads -6.2 to -6.7 dB and stands
  9.4-10.0 dB under B5 beside it, the largest step in the bank (9.3-9.6 dB
  at 0.4.46). The strings survey places C6 in the body, not the string: on
  the G string's 17th fret it reads 6-8 dB under its neighbours too, with
  a peak within 2 dB of theirs and a decay rate between theirs, and the
  koa tenor has no such key. It lies in the plate band, 17 cents over a
  mode the tenor's bank holds at 1,036 Hz (`AculeleBodyModes`); the band's
  level within a third of an octave is a draw (largest 9-13 dB, Known
  gaps), and whether a spruce tenor has such a key would take a measured
  body.
- **Above the line:** G#3 and A3 on Tenor Picked Melody, +5.2 to +6.9 dB
  over the whole range's line, either side of the tenor's air mode at
  211 Hz. Its top key, D#6, has a component off its harmonics 3.9-6.4 dB
  over its strongest partial (`spike_db`), the only key in the bank where
  one stands above the partials at every velocity.

**Found.** No key whose level, decay, brightness or pitch the model gets
wrong by a cause this pass could name. Each outlier follows a pitch on its
body across strings, as a body resonance does; none is trimmed.

**Before and after.** The survey's 618 rows at 0.4.61 are those of 0.4.58,
byte for byte: a single note after a reset meets none of this pass's three
changes.

## Decay up the neck against the sample rate

**Checked.** Acukoto's pass found its loss sections over-damping the
fundamental of a short loop, more at a lower rate. Aculele's loop gain
already divides both loss sections' magnitude at the playing frequency out
of the period's gain (`loopGain = desiredPeriodGain / filterGain`,
`configureVoice`), and the 0.4.46 survey at 44.1, 48 and 96 kHz read no
direction in decay ([rates](note-survey-rates-2026-11-07.md)). Measured
again at 0.4.61 where it would show most: `AculeleNoteSurvey --mode
strings`, every string of every preset from open to the 18th fret at
velocity 80 (532 notes), at 48 and at 96 kHz. Each note's 96 kHz reading
less its 48 kHz reading:

| Frets | Notes | Decay, dB/s: mean, SD | Level over 200 ms, dB: mean, SD | Partials' pitch, cents: mean, SD |
| --- | --- | --- | --- | --- |
| Open | 28 | +0.22, 0.59 | -0.19, 0.37 | +0.03, 0.11 |
| 1-5 | 140 | +0.45, 1.03 | -0.29, 0.41 | +0.07, 0.19 |
| 6-11 | 168 | +0.46, 1.27 | -0.07, 0.49 | +0.11, 0.32 |
| 12-14 | 84 | +0.49, 1.17 | +0.05, 0.39 | +0.04, 0.62 |
| 15-18 | 112 | +0.16, 0.94 | +0.09, 0.43 | +0.04, 0.21 |

At the 15th-18th frets the notes decay at a mean 49.0 dB/s at 48 kHz and
their 96 kHz decay is 1.003 times that (median 1.001, 0.91-1.07 over the
112 notes; by preset the mean difference is -0.3 to +0.6 dB/s). The decay
does not depend on the rate, up the neck or anywhere else; nothing to fix.

One reading moved further: the fundamental's peak of F4 on the koa
tenor's low G string (10th fret) reads -9.9 cents at 48 kHz and +3.1 at
96 kHz, while the pitch its partials carry reads +2.9 and +5.1. That key
sits where the note's two polarisations part (Known gaps, "Notes beside
the top's main resonance start out of tune"); which of the pair the peak
reading lands on was not traced.

## Library calls in Reason's build (0.4.62)

**Checked.** Reason compiles the engine at `-fno-builtin` for x86-64 with
SSE3, so a `std::` maths function on the sample path is a call into its
library, and so is `floor`, which has no SSE3 instruction. The engine's
square roots, absolute values and finiteness tests on that path already
go through compiler builtins (`exact::`). A Clang build with those flags,
profiled over one ringing block
(`Docs/benchmarks/cpu-consolidation-2026-11-09.md`), made about 1,200
calls to `floor` and 400 to `expf`: one each whenever a string's delay
moved, which in that block, 0.23 s after an eight-note strike, was on
about a third of the loops' delay reads.

**Fix.** `delayAnchor` takes the floor by truncation and a step down for a
negative fraction, the same integer for every delay a `double` holds
within an `int`'s range. The `expf` (the slope observer's coefficient,
`finishVoice`) has no exact replacement and stays.

**Measured.** The seven demos and the stress, body-change, String Age and
bridge-hand phrases through seven presets (35 float renders) are those of
0.4.61 bit for bit. No `floor` call is left in the ringing block; on glibc
that is 0.2% of its instructions, and what Reason's own library saves is
not known here.

**Left.** A note start's design work is library maths for about 0.7 of its
7.6 million instructions (`atan2`, `log1p`, complex division); the control
period's `configureVoice` makes some 50,000 instructions of it a ringing
block.

## The bridge hand

**Checked.** A moving bridge hand (CC2) reconfigures every sounding string
each control period, and Acustra's 1.9.37 found its own doing a whole
tuning solve each time. Profiled over a 4.5 s phrase on Soprano D Tuning:
265 million of 6.09 billion instructions under the hand's `configureVoice`,
about 1.1 million a block (+18%) while the hand moves, and no whole
dispersion search in any block. **Not changed:** Acustra's remedy tunes
the pair on the hand's target, which moves the sound while the hand moves,
for a block that is not this instrument's heaviest.

## Acustra since the fork

Aculele forked from Acustra 1.9.0. Its later versions, against this
engine:

| Acustra | What | Here |
| --- | --- | --- |
| 1.9.38-1.9.39 | A held chord and the level observers come to rest | Taken: 0.4.61 |
| 1.9.40 | The re-pluck arrival rings | 0.4.60's change, taken there |
| 1.9.37 | The palm's tuning solve | Measured (above), not taken |
| 1.9.33 | The axial ring's loss stays passive | Here since 0.4.43. Acustra also tests the single-precision coefficients the loop runs on; over 45,144 designs (every shape, string set, tuning, string and fret at String Age 0, 0.5 and 1, eleven rates from 8 to 192 kHz) none stored here reaches one, the nearest 5.6e-5 under it, so the second test is not added |
| 1.9.28 | A string remembers the tunings of its last eight stops | Not taken: the tuning solve is 0.14 of the heaviest block's 7.64 million instructions here |
| 1.9.27 | The tuning solve's arithmetic, a sixth off a note start | Not taken: Aculele's solve has since parted from Acustra's (it reads the port at the bent impedance), so it is a re-derivation, not a copy |
| 1.9.26 | Which member of the coupled pair the string is tuned to | Not taken: audible, and Aculele's rule is its own (Known gaps, "Notes beside the top's main resonance start out of tune") |
| 1.9.36 | A picked string's release direction | Not compared in this pass |
| 1.9.29-1.9.35 | The axial force's units and rate, the pickup's load, output headroom, pluck coordinates | Here as 0.4.48-0.4.55, by their entries; not diffed |
| 1.9.25 and earlier | | Not compared again in this pass |

Acukoto's fundamental-exact loop gain is not needed: see the decay check
above.

## Every control reaches held and new notes

`Aculele.ParameterReach` is another branch's test
(`claude/project-thread-ad7las`, `84ac9b3`), not yet on this one. At
0.4.58 it failed on Body shape, String age and Tuning, whose notes were
lost while the designs were made again, and on Strings, whose held notes
moved 0.47 dB against the 0.5 dB it asks. This branch's four commits
apply on top of that commit without conflict, and there the test passes:
20 parameters, 42 changes, none failed.

| Change | New chord apart | Held notes moved |
| --- | --- | --- |
| Body shape, tenor to soprano | 7.04 dB | 8.00 dB |
| Body shape, tenor to concert | 4.77 dB | 5.65 dB |
| Tuning, Low G to Standard | 7.87 dB | 2.05 dB |
| Tuning, Low G to D | 9.47 dB | 4.17 dB |
| String age, 0.2 to 1 | 2.53 dB | 1.70 dB |
| Strings, nylon to fluorocarbon | 2.40 dB | 0.55 dB |

Strings passes its held-note threshold by 0.05 dB. Restringing under a
ringing chord changes little of a wave already on the string, so the
margin is thin and a later change could cross it.

## Known gaps, checked

Each of the README's gaps was read against 0.4.62. All still hold. "A
stiff note can still overrun a small buffer now and then" was rewritten at
0.4.59 and stands as written. One is added: the heaviest note-start
blocks stand over a tenth of the period on the development machine.

## Found and not fixed

- **The heaviest blocks are over 10% of a 256-sample period.** Eight notes
  struck at once over ringing strings take 10.5-11.6% on the Reason-like
  build (12.0-14.1% at 0.4.58), the demos' worst strums 10.5-12.9%
  (11.2-15.7%). What is left, with what each reduction would cost, is in
  the benchmark document; the one that costs nothing in sound (bounding
  the delay lines' clears and copies) needs a proof over some 35 read
  sites.
- **C6 on the spruce tenor** stands 9.4-10.0 dB under B5 (above). It was
  there at 0.4.46. Moving it means redrawing the plate band, which every
  note above 1 kHz on that body would hear.
- **A ready engine still searches in full at the note** for about one
  re-pluck in 36 of a stiff note (Known gaps), and a String Age sweep
  still designs each sounding string again at every step.
- **Subnormal numbers in three bounded tails** of ordinary playing, with
  no measurable cost (above).
- **F4 on the koa tenor's low G string** reads 13 cents apart at 48 and
  96 kHz in its fundamental's peak; not traced.
- **Acustra's 1.9.36** (the pick's release direction) was not compared
  with Aculele's pick.

## Not examined

This pass did not read the engine line by line. Not examined: the units
and signs of each block of How it works beyond what its existing tests
hold; the Performer's voicing and strum scheduling; the room and the
output stage; rates other than 48 and 96 kHz in the surveys (the Rest and
Ready tests run at 48 kHz); MPE; and Acustra's changes before 1.9.26
against this engine's own versions of them.

