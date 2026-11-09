# CPU in the consolidation pass, 2026-11-09

What the engine costs at 0.4.58 (`0bfe1a1`) and after each change of the
pass. 48 kHz, 256-sample blocks, single-threaded, the host's denormal mode
left as it starts (no flush to zero). A 4-core Intel Xeon VM at 2.10 GHz,
GCC 13.3 `-O3`. The machine ran four other sessions' builds and renders
throughout (load average 3-7), so the timings are each block's fastest of
five repeats, baseline and candidate alternated, and still move by a point
between runs: instruction counts decide, timings illustrate.
Development-machine figures, never Reason's CPU meter.

## Method

- **Instruction counts.** `valgrind --tool=callgrind --collect-atstart=no`
  with collection toggled round one 256-sample block of the stress phrase
  (`cpu-2026-11-09.md` describes the phrase), and `--cache-sim=yes` for the
  first-level data misses.
- **Timings.** An off-tree harness plays the seven demos and the stress
  phrase through the Performer as `RenderAculelePresetDemos` does, five
  times, keeps each block's fastest time and reports the mean, the 99th
  percentile and the worst block as a share of the 5.33 ms period. It
  writes the floats, which the null tests compare.
- **Null tests.** Every change here renders the seven demos, the stress
  phrase, a body change under a ringing chord and a String Age sweep under
  re-struck chords (`consolidation-audit-2026-11-09.md`) through all seven
  presets bit for bit as the version before it: stereo float, every sample.

## Where a heavy block goes

The stress phrase's heaviest block is its second chord: eight notes struck
at once at 3.5 s over eight ringing under the pedal (block 656). On Concert
Strum at 0.4.59 it runs 10.14 million instructions, where the blocks either
side run 4.76 and 6.06 million.

| Part of block 656 | Instructions |
| --- | --- |
| The eight note starts (`startNote`), inclusive | 4.10 M |
| of which keeping each string's old sound as a tail (`captureTail`) | 2.31 M |
| `memset` and `memcpy` under them | 1.54 M and 1.40 M |
| Configuring strings (`configureVoice`, 62 calls) | about 1.6 M |
| The eight plucks (`firePluck`) | 0.88 M |

The clears and copies were the re-pluck arrival rings: two planes of 16,384
floats a string, 128 KB, cleared whole at every note start and copied whole
into the tail a re-plucked note leaves ringing, though a ring holds nothing
but zeros outside the arrivals still pending (at most a few hundred).

## 0.4.60: the arrival rings clear and copy their pending slots

| Block 656, Concert Strum | 0.4.59 | 0.4.60 |
| --- | --- | --- |
| Instructions | 10,143,524 | 8,308,209 |
| First-level data misses | 139,798 | 96,671 |
| Share of the period, seven presets, fastest of five | 12.6-15.6% | 10.7-12.9% |

What is left of the block's memory traffic is the strings' own delay lines,
32 KB each: 18 cleared and about 14 copied in that block, some 0.6 and
0.5 MB. A line has no such pending window (a ringing string has written
all of it), so those stay.

The seven demos, each block's fastest of five:

| | Mean | 99th percentile | Worst block |
| --- | --- | --- | --- |
| 0.4.58 | 5.4-6.6% | 7.7-11.5% | 11.7-15.5% |
| 0.4.60 | 4.8-7.1% | 7.2-10.5% | 11.2-14.1% |

The demos do not strike eight strings at once, so their blocks move inside
the run-to-run spread. Their worst blocks are four-note strums over ringing
strings (the first block of a run, when it is the worst, is the cold
start).

## 0.4.61: a held chord comes to rest

Acustra's pass found that a chord which stays held never reaches the idle
flush, and that its waves then run down into subnormal numbers
(`consolidation-audit-2026-11-09.md`). Aculele shares the loop. The open
chord G4 C4 E4 A4 at velocity 110, its keys held for 130 s, flush-to-zero
and denormals-are-zero off, the mean block of each one-second window as a
share of the period:

| Preset | 0.4.60, 10-20 s | 0.4.60, 110-130 s | 0.4.61, 110-130 s | 0.4.61 at rest from |
| --- | --- | --- | --- | --- |
| Concert Strum | 5.0% | 30.2% | 5.9% | 24 s |
| Soprano D Tuning | 4.7% | 33.3% | 7.6% | 13 s |
| Tenor Fluorocarbon Fingerstyle | 4.9% | 31.1% | 5.7% | 27 s |
| Low G Tenor | 4.8% | 24.4% | 7.5% | 15 s |
| Soprano Thumb | 4.6% | 32.5% | 7.6% | 24 s |
| Tenor Picked Melody | 4.7% | 29.8% | 7.1% | 18 s |
| Concert Piezo Stage | 4.2% | 30.1% | 7.1% | 26 s |

At 0.4.60 the denormal-operand flag is first raised for good at 15-32 s,
the mean passes 20% at 50-110 s and stays there for as long as the chord is
held; the pedal in place of the keys gives the same (30.0% and 25.4% on the
two presets tried). At 0.4.61 the output is exact zero from the time in the
last column and no window after it raises the flag. Its 110-130 s column
was taken on a busier machine than the 0.4.60 columns (the same runs' 10-20 s
windows read 3.8-6.8%), so it shows the slowdown gone, not what silence
costs.

## Subnormal numbers in ordinary playing

A tracer unmasked the denormal-operand and underflow exceptions for each
8-sample block and counted the faulting instruction (Acustra's method). Over
three demos at 0.4.61:

| Demo | Blocks of 8 that hit | Sites, by count |
| --- | --- | --- |
| 01 Concert Strum | 4.2% | the contact travel's filter memories 1,808, the key-up touch's envelope 516, the fret motion's drain 448 |
| 03 Tenor Fluorocarbon Fingerstyle | 6.4% | key-up touch 2,908, fret motion 917, contact travel 557 |
| 06 Tenor Picked Melody | 9.1% | key-up touch 4,840, contact travel 500, fret motion 460 |

Each is a tail of bounded length that ends in an exact zero: some 8 ms
of a finished contact's fractional-delay memories
(`ContactTravel::process`), the last of a key-up touch's two envelopes
(`renderReleaseNoise`), 75 ms of the fret motion's filters draining
(`renderFretMotion`). The seven demos with flush-to-zero and
denormals-are-zero off and on, alternated twice, fastest of five: means
5.4-6.9% off and 5.5-7.1% on, worst blocks 10.3-14.5% and 10.5-15.0%, no
difference outside the run-to-run spread. They are left as they are.
