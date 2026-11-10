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

## A moving bridge hand

`callgrind` over a 4.5 s phrase on Soprano D Tuning in which the bridge
hand (CC2) moves under ringing strings: 6.09 G instructions, 265 M of them
under the hand's `configureVoice` (the pair's tuning solve 110 M, the loop
delay 46 M, the dispersion's continuation 32 M). That is about 1.1 million
instructions a block more while the hand moves (+18%), and no whole
dispersion search. Acustra's 1.9.37 tunes the pair on the hand's target and
so solves it once a gesture; that changes the sound while the hand moves
and was not taken here, where the heaviest blocks are note starts.

## The build Reason makes, before and after the pass

Reason compiles the engine with Clang at `-O2 -std=c++17 -fno-builtin` for
x86-64 with SSE3 (no AVX, no FMA) and for arm64. The nearest build this
machine makes: Clang 18, `-std=c++17 -O2 -fno-builtin -march=x86-64 -msse3`,
linked against glibc (Reason's own maths library and `-ffreestanding` are
not reproduced). It renders the stress phrase through all seven presets
bit for bit as the GCC build. The device also sets
`ACULELE_EXTENDED_SAMPLE_RATES=1`, which the figures of this section lack:
the 0.4.63 section below has both. Each block's fastest of five, 0.4.58 and
0.4.61 alternated twice and the lower reading kept, load average 6-12:

| Reason-like build | Mean | 99th percentile | Worst block |
| --- | --- | --- | --- |
| Seven demos, 0.4.58 | 5.0-6.4% | 7.4-11.0% | 11.2-15.7% |
| Seven demos, 0.4.61 | 4.8-6.3% | 7.6-11.2% | 10.5-12.9% |
| Stress phrase, seven presets, 0.4.58 | 4.6-5.2% | 6.8-8.3% | 12.0-14.1% |
| Stress phrase, seven presets, 0.4.61 | 4.5-5.4% | 6.9-8.7% | 10.5-11.6% |

| Instructions, Concert Strum, Reason-like build | 0.4.58 | 0.4.61 | 0.4.62 | 0.4.63 |
| --- | --- | --- | --- | --- |
| Block 656 (eight note starts over eight ringing waves) | 8,630,644 | 7,637,446 | 7,621,359 | 7,144,894 |
| Block 700 (the same waves ringing, 0.23 s on) | 5,720,085 | 5,723,435 | 5,710,463 | 5,707,184 |

The stress phrase's worst block is block 656 on every preset. The demos'
worst blocks are four-note strums over ringing strings, or the first block
of the run where that reads worst. The mean does not move: the pass
changed what a note start and a long hold cost, not what a ringing string
costs. The 10% ceiling is met by every mean and 99th percentile but one
(Concert Piezo Stage's demo, 11.2%) and by no worst block.

### Library calls (0.4.62)

Under `-fno-builtin` every `std::` maths function is a call into the
host's library, and without SSE4.1 so is `floor`. In the ringing block
(700) the Reason-like build made these on glibc: `expf` 17,668
instructions (`finishVoice`, one each time a string's delay moves, about
420 a block), `floor` 3,765 (`delayAnchor` under `readDelay`, about 1,200
a block), complex division 16,452 (`bridgePortMobility`, once a control
period a string), and `atan2`, `log1p`, `cos`, `sin`, `pow` and `exp2f`
together 48,000 in the control period's `configureVoice`: 1.6% of the
block. Square roots and absolute values on the sample path are compiler
builtins already (`exact::`).

0.4.62 takes the `floor` out: `delayAnchor` truncates and steps down for a
negative fraction, the same integer for every delay, so the 35 float
renders (seven demos; the stress, body-change, String Age and bridge-hand
phrases through seven presets) are those of 0.4.61 bit for bit. On glibc
that is 13,000 instructions a ringing block (0.2%); what Reason's own
`floor` costs is not known here.

In the note-start block (656) about 0.70 of the 7.64 million instructions
are library maths, nearly all in the notes' own design work: `atan2`
0.30 M (the dispersion fit and the loop's tuning), `log1p` 0.11 M (the
loss section), complex division 0.08 M, `pow`, `cos` and `sin` 0.16 M. If
Reason's library is slower than glibc's, a note start costs that much
more there; not measured.

## 0.4.63: a retained wave takes only the line it can read

The one candidate of the table below that costs nothing in sound was "clear
and copy only the part of a delay line a reader can reach". 0.4.63 makes it
for the copies. The clears stay whole.

**What the 0.52 M was.** In block 656 the note starts keep seven old waves
(`captureTail`), two loops each: 14 copies of a 32 KB line, 467,282
instructions of `memcpy`. glibc copies a block that size with `rep movsb`,
which valgrind counts once a byte, so that figure is the bytes copied and
overstates the copy's share of the time: what it costs is 448 KB read and
448 KB written. The 18 clears (`StringLoop::reset`, 16 in `configureVoice`
for the strings plucked afresh and 2 in `initialisePluck`) are 50,850
instructions and 9,216 first-level write misses, one for each cache line of
their 576 KB.

**The bound.** A retained wave's two loops are touched by `advance`,
`write`, `applyGestureContact`, `bridgeVelocity`, `setLoopGain` and `reset`
and by nothing else. No `configureVoice`, bend, slide, retune, release fall
or construction change reaches them, and a rate change resets them:
nothing sets a retained wave's delay or its target again. Its reads of the
line are these. `readDelay`'s deepest tap lies at most 0.9 of a sample
beyond the delay. The delay follows its target and never passes it. Each of
the two dispersion sections that may still retire lengthens both by two
samples, once. The damping hand reads two samples beyond the two ages its
landing froze. So the wave reads no further behind its write position than

    max(floor(the longer of its delay and its target) + 5, the hand's older age + 2)

samples, and every sample it writes extends what holds. The derivation
stands beside the bound in `StringLoop::captureFrom`. A wave whose bound
reaches the length of the line, or whose delay is not a number, takes the
whole line, and so does one whose line has another ring size than its
string's (the extended-rate build).

**What keeps its whole line.** A voice's own loops. `configureVoice` floors
a string's frequency at the rate over the line's length less three and sets
no other bound on what a bend, a slide, a retune or a body change may ask
for, so no reach short of the line can be proven for them. That covers the
18 clears and the scratch copies of a same-pitch re-pluck
(`repluckOldLoops_`, none in this block). A clear cannot follow what was
written either: a string that has rung 0.17 s has written all of its line.

**The test.** `Aculele.TailReach` (unit tier) turns on an engine switch that
writes 3.0e6, a value no string takes, over every sample of a retained line
that `captureFrom` did not copy. It plays six phrases through all seven
presets twice, plain and poisoned: the preset's demo; the stress phrase; a
body change under ringing chords; re-plucks under the pedal while the wheel,
set to two octaves, swings and the bridge hand moves; chords bent down,
damped there and bent up before the next; and a String Age sweep under
re-struck chords. The two renders must be finite and equal bit for bit, and
each phrase must show poisoned lines beside ringing retained waves, or it
would prove nothing. Those phrases need not meet the bound's two side
terms, so `Aculele.DispersionState` checks them on the loop itself: a wave
taken on the sample before the first of two sections retires, and a wave
whose hand landed when its string was longer.

Trials with the bound shortened on purpose, each rebuilt and run:

| Bound | Result |
| --- | --- |
| `floor(longest)`, no margin | Every `TailReach` run fails (28 of 28 at the time): "a retained wave read past its reach" |
| `floor(longest) + 2` | The loop's retirement check fails on its audio |
| The hand's age without its 2 | The loop's hand check fails on its audio |
| `floor(longest) + 1`; the hand's age `+ 1`; no hand term at all | `TailReach` reads no poison: only the loop's checks reach the side terms |

By the derivation the worst case needs `+ 3`: the first section can retire
on the very next sample, the second not before two more are written. The
shipped `+ 5` does not credit those two samples.

| Block 656, Concert Strum, Reason-like build | 0.4.62 | 0.4.63 |
| --- | --- | --- |
| Instructions | 7,621,359 | 7,144,894 |
| of which `memcpy` | 467,282 | 2,542 |
| Data reads | 2,207,238 | 1,744,743 |
| Data writes | 1,012,981 | 546,995 |
| First-level data misses, reads and writes | 78,024 and 21,323 | 72,677 and 14,128 |

Block 700 moves by 3,279 instructions (5,710,463 to 5,707,184): nothing is
captured in it, and a loop's state now lies before its line in memory.

**The device's build.** The Rack device compiles the engine with
`ACULELE_EXTENDED_SAMPLE_RATES=1`: rates up to 768 kHz, with lines of 8,192
samples up to 192 kHz and 16,384 and 32,768 above, read through a ring
mask. The Reason-like build of this document did not set it until now.
With it, on the same flags:

| Instructions, Concert Strum, Reason-like build with extended rates | 0.4.62 | 0.4.63 |
| --- | --- | --- |
| Block 656 | 7,731,887 | 7,264,960 |
| Block 700 | 6,397,006 | 6,397,510 |

A ringing block runs 12% more instructions there than without the define
(6.40 M against 5.71 M), which the tables above this section do not show.
The 35 float renders are those of 0.4.62 bit for bit in that build too,
and `Aculele.TailReach` and `Aculele.DispersionState` pass compiled with it
(off-tree: the repository's own build does not set the define).

**Timings.** Each block's fastest of five, 0.4.62 and 0.4.63 alternated
three times and the lowest reading kept, load average 2-3.5:

| Reason-like build | Mean | 99th percentile | Worst block |
| --- | --- | --- | --- |
| Stress phrase, seven presets, 0.4.62 | 4.4-5.0% | 6.5-7.9% | 10.1-11.1% |
| Stress phrase, seven presets, 0.4.63 | 4.4-4.9% | 6.7-8.0% | 9.8-10.1% |
| Seven demos, 0.4.62 | 4.6-6.1% | 7.0-10.1% | 9.6-12.1% |
| Seven demos, 0.4.63 | 4.6-6.3% | 7.0-10.2% | 9.3-12.2% |
| With extended rates: stress phrase, 0.4.62 | 4.6-5.1% | 6.8-8.2% | 10.1-11.4% |
| With extended rates: stress phrase, 0.4.63 | 4.6-5.1% | 6.7-8.1% | 9.4-10.6% |
| With extended rates: seven demos, 0.4.62 | 4.9-6.5% | 7.5-10.6% | 10.2-12.1% |
| With extended rates: seven demos, 0.4.63 | 4.8-6.5% | 7.3-10.9% | 9.7-12.5% |

The stress phrase's heaviest block is 0.3-1.0 points lighter on every
preset (0-1.3 with extended rates) and stands at about the 10% line. The
demos, whose worst blocks are four-note strums or the first block of the
run, move inside the run-to-run spread. The machine was quieter than for
the tables above (load 6-12 there), so only the pairs alternated here
compare.

## What is left above 10%

Block 656 at 0.4.61 on the Reason-like build, 7,637,446 instructions
(10.5-11.6% of the period on this machine; about 6.6-7.3 million would be
10%). Inclusive counts; where one part calls another they overlap. At
0.4.63 the block runs 7,144,894 instructions (7,264,960 with the device's
extended rates, 9.4-10.6% of the period): the table's last row is gone and
the rest stands.

| Part | Instructions |
| --- | --- |
| Everything that rings (`process`) | 5.86 M |
| the strings' stretch (`renderStringAxialForce`) | 2.02 M |
| the body's modes (`BodyBank::render`) | 1.07 M |
| the string loops themselves | 0.87 M |
| the bridge's load (`BridgeLoad::process`) | 0.53 M |
| the room | 0.30 M |
| bridge reads and observers (`finishVoice`) | 0.30 M |
| the control period (`updateControlState`) | 0.22 M |
| the pickup's chain, heard or not (`renderPiezo`) | 0.09 M |
| The eight note starts (`noteOn`) | 1.77 M |
| configuring strings (`configureVoice`, every caller) | 1.20 M |
| of which the re-pluck's dispersion design | 0.37 M |
| of which the loss sections' designs | 0.37 M |
| of which the tuning solve, bridge port and loop delay | 0.32 M |
| the plucks (`firePluck`) | 0.73 M |
| old waves kept as tails (`captureTail`: line copies) | 0.47 M |

What could come out, and what it would cost:

| Candidate | Instructions in this block | Cost in sound |
| --- | --- | --- |
| Clear only as much of a fresh string's line as its delay reaches, and clear further whenever a bend, a slide or a retune lengthens it (18 clears of 32 KB; the 14 copies came out in 0.4.63) | 0.05 M and a tenth of the block's first-level data misses (9,216 of 86,233) | None, but every writer of a string's delay and every one of some 35 read sites would have to keep the cleared extent. Not proven here |
| Design a note's loss sections and continue its dispersion design at the fret's pitch, not at the few cents each contact draws, so both come from the designs made ahead | up to 0.74 M | Not measured: the partials above the fundamental follow the fret's design, a few cents of the fundamental off the contact's, on every fretted note |
| Read the strings' stretch every second sample | about 1.0 M, in every ringing block too | The aliases 0.4.48 removed come back in part: the squared motion's upper sum tones fold under half the reduced rate |
| Fewer body modes above 1 kHz (51-62 there now) | at least 4,000 a mode, some 0.25 M for all of them, in every block | The plate band thins: fewer, wider-spaced resonances above 1 kHz on every note |
| Let the pickup's chain rest while a microphone preset plays | 0.09 M, in every block | The pickup starts from rest when Capture changes under a ringing chord |
| Hold fewer old waves under the pedal, or let them go sooner | about 0.7 M a wave | A re-plucked string's earlier note ends sooner under the sustain pedal |

The first row's copies came out in 0.4.63 (above): the parts table's last
row, 0.47 M at 0.4.61, is 0.01 M now. Nothing else is made in this pass:
what is left of the first row has no proof, and each of the others changes
the sound and has more than one defensible size, so it is the owner's to
choose and to hear.
