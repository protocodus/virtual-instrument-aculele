# CPU on 2026-11-10: the strings' stretch in Reason's layout

Two changes to the sum that reads each ringing string's stretch
(`sumAxialCells`, `renderStringAxialForce`), both exact: every render is the
bytes of 0.4.63. Engine 0.4.63 (`8583471`) against 0.4.64. Method, phrase and
machine as in [cpu-consolidation-2026-11-09.md](cpu-consolidation-2026-11-09.md):
48 kHz, 256-sample blocks, the stress phrase on Concert Strum, a 4-core Xeon
VM. The machine ran five other sessions' builds throughout (load average
20-32), so instruction counts decide here (`valgrind --tool=callgrind`,
collection toggled round the named blocks) and the timings at the end only
illustrate. Development-machine figures, never Reason's CPU meter.

Two builds are counted:

- **Reason-like**: Clang 18, C++17, `-O2 -fno-builtin`, SSE3, with the
  device's `ACULELE_EXTENDED_SAMPLE_RATES=1` unless a row says otherwise.
- **Native**: this repository's Release build, GCC 13.3, C++20, `-O3`,
  without the define.

## What was wrong

The sum reads about 38 cells along each of a driven string's two loops,
every sample, and is 38-44% of a ringing block. Each cell interpolates the
loop's delay line at a fractional age. Where none of a sample's reads cross
the line's end, the sum takes them unwrapped, four cells to a vector;
otherwise it masks every index, a cell at a time.

`renderStringAxialForce` decided that from the newest sample's index,
`writeIndex - 1`, compared with the ring's mask. In the plug-in layout the
line and the ring are both 8,192 samples and the test does what it says. In
the device's layout `writeIndex` counts round a ring of 32,768 while the
line a 48 kHz session reads is still its first 8,192, read through the mask.
The unmasked index was above the mask three samples in four, so three sums
in four took the slow path with nothing to wrap: in blocks 690-709 the fast
path ran 18 times in 389,102. The 2026-11-09 benchmark measured the result
without finding the cause: "a ringing block runs 12% more instructions there
than without the define (6.40 M against 5.71 M)".

## The two changes

1. **The newest sample's place is its low bits** (`& mask`), in both
   layouts. The decision is then the same in both, and the reads were
   already the same samples either way.
2. **A lane's two adjacent samples come in one eight-byte read**, two lanes
   to a vector, in place of eight scalar reads and their inserts. The
   products, sums and their order are untouched.

| Instructions, Reason-like build with extended rates | 0.4.63 | Change 1 | Changes 1 and 2 (0.4.64) |
| --- | --- | --- | --- |
| Blocks 690-709, strings ringing | 127,218,536 | 115,015,050 | 108,290,172 (-14.9%) |
| Blocks 650-669, eight note starts over ringing strings | 117,929,417 | 117,372,148 | 112,022,117 (-5.0%) |
| Block 700 alone | 6,397,510 | | 5,446,214 (-14.9%) |
| Block 656 alone, the phrase's heaviest | 7,264,960 | | 7,145,906 (-1.6%) |

| Instructions, other builds | 0.4.63 | 0.4.64 |
| --- | --- | --- |
| Reason-like without the define, block 700 | 5,707,184 | 5,385,648 (-5.6%) |
| Reason-like without the define, block 656 | 7,144,894 | 7,055,617 (-1.2%) |
| Reason-like without the define, blocks 690-709 | | 107,088,640 |
| Reason-like without the define, blocks 650-669 | | 110,674,123 |
| Native, blocks 690-709 | 117,868,126 | 113,906,380 (-3.4%) |
| Native, blocks 650-669 | 119,693,748 | 116,567,215 (-2.6%) |

The device's layout now costs a ringing block 1.1% more instructions than
the plug-in's, from 12%. The native build, which never had the first
fault, gains the second change only.

**Grade: exact.** The seven demos and the stress, body-change, String Age
and bridge-hand phrases through all seven presets, 35 stereo float renders,
are the bytes of 0.4.63 after each change, in the Reason-like build with and
without the define. `Aculele.StringAxialForce` holds the sum's value, and
`Aculele.SampleRateLayoutParity`, new in 0.4.64, builds the device's layout
in this repository and requires its bytes to be the other layout's at 44.1,
48, 96 and 192 kHz. Before 0.4.64 nothing here built that layout, which is
how a fault that lived only in it went unseen.

**The second change and the clock.** Acustra's pass tried the same paired
read on 2026-11-09, found its demos' per-block times unmoved beyond noise
(a point either way in ten) and reverted it. The count here says a ringing
block does 5.8% fewer instructions for it on Clang and 3.4% on GCC, which
that noise would hide. The clock here follows as far as this machine could
show today: the phrase's mean block falls from 4.9-5.0% to 4.8% of a period
with it (below). It is exact and no slower by any reading, so it stays.

## Timings

The stress phrase through all seven presets, Reason-like build with extended
rates, each block's fastest of five passes, as a share of the block's period
on one core. The three builds ran in turn, twice over, with the load average
at 27-32:

| Over the seven presets, first and second round | 0.4.63 | Change 1 | Changes 1 and 2 (0.4.64) |
| --- | --- | --- | --- |
| Mean block | 5.1%, 5.2% | 5.0%, 4.9% | 4.8%, 4.8% |
| 99th percentile block | 8.2%, 8.6% | 8.2%, 8.1% | 7.6%, 7.7% |

Per preset the mean is 4.8-5.5% at 0.4.63 and 4.4-5.1% at 0.4.64. The worst
block read 9.8-17% in every build, with single readings of 85% and 141% where
the scheduler took the core away: at this load it is not a measurement, and
block 656's instruction count (-1.6%) is the figure to keep for the heaviest
note start. The 2026-11-09 tables were taken on a quiet machine and are not
redone here.

## Looked at and left

- **The body bank** (`BodyBank::render`): already four modes to a vector,
  about 85 instructions a group; its flush of tiny states is a fifth of
  that, and the exact zeros it makes are what lets a silent group rest.
- **The bridge's load** and **`process` itself**: no single part worth a
  rewrite; their instructions are spread over the per-sample bookkeeping.
- **Clearing only the part of a fresh string's line a reader can reach**
  (the 2026-11-09 candidate): its gain is in cache misses, which instruction
  counts do not show, and it still needs the proof over every read site.
  Not attempted.
- Every other candidate of the 2026-11-09 table changes the sound (the
  stretch at every second sample, fewer body modes, designs at the fret's
  pitch). None was reopened: this pass ships exact changes only.
- **Acustra's and Acukoto's passes** (2026-11-09 and 2026-11-10): Acustra's
  coarser axial quadrature was put to the owner's ear and declined, which
  holds here too. Acustra's engine has the same unmasked index beside the
  same sum (`renderStringAxialForce`) and its Rack build sets the same
  define, so it very likely pays the same penalty; not measured here, noted
  for its pass. Acukoto's changes are in its own string and wall models,
  which Aculele does not share.
