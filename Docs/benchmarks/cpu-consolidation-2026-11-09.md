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
