# Consolidation audit, 2026-11-09

An end-to-end review of the engine ahead of release: what was checked, what
was found and what each fix measured. It starts from 0.4.58 (`0bfe1a1`).
48 kHz and 256-sample blocks unless a rate is named; timings are from a
4-core Intel Xeon VM at 2.10 GHz shared with other work, GCC 13.3 `-O3`,
each block's fastest of three runs, and are development-machine figures,
never Reason's CPU meter.

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
