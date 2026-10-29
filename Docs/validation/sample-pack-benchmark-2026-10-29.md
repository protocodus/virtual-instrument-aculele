# Aculele against five sampled ukuleles, 2026-10-29

The owner supplied six free DecentSampler ukulele packs from the Pianobook
community as a realism benchmark. None of them is in this repository, and
nothing is fitted from them. `Tools/compare_sample_packs.py` measures them and
Aculele 0.4.26 the same way; its header gives the commands.

## The packs

| Pack | Instrument | Capture | Notes used |
| --- | --- | --- | --- |
| Guffland Ukulele (Mark Lowrie, 2022) | a Peavey with a second sound hole on its top | unstated | 100: C4-C6, four round robins |
| James' Ukulele (James Blunsdon) | a Brunswick | Shure SM58 | 25: C4-C6 |
| Kai Ukulele (Seb Overell) | a Kai KT-10 tenor kit, two sound holes | RØDE NT-USB | 50: C4-C6, loud and quiet layers |
| Soprano Plucked Ukulele (Manas Kunder) | a cheap soprano | a phone's microphone | 14: C#4-D5 |
| Leolani Ukulele (David May, 2023) | a Leolani | DI, no amp | 96: C4-B5, four round robins |
| Uncle Bill's Serenader (Rob Asselstine) | a Serenader baritone | unstated | not used: one sample stretched over the keyboard |

The packs' own README files give the instrument and capture; string sets,
sizes (but the Kai's and the soprano's), players and pluck points are not
stated. Two packs map their samples away from their pitch: Leolani's sound
two octaves under their root notes and the soprano's a semitone above. The
tool finds each pack's shift from the samples' own pitch.

Aculele played the same pitches, C4 to C6 alone at velocity 90, through
Concert Strum's settings with Finger, Mahogany and the mono microphone, on
each Shape (`AculeleProbe 0 notes.mid out.wav 101 --float --picking 0
--capture 7 --wood 1 --shape N`).

## The measures

Only measures that a microphone, a DI or a room hardly change are compared,
so an SM58, a phone and a pickup can stand together. Partial levels against
each other follow the capture: across these packs, the 3rd harmonic stands
12-31 dB under the 1st, and Aculele's 22 dB sits inside that, so they decide
nothing here.

## Results

`compare_sample_packs.py` on the five packs and on Aculele 0.4.26, and the
beating on 0.4.27, which took the 0.2% split below:

| | B ×1e-4, C4-D#4 | E4-G#4 | A4-D5 | D#5-G5 | G#5-C6 |
| --- | --- | --- | --- | --- | --- |
| Guffland | 1.25 | 1.84 | 0.82 | 1.75 | 2.52 |
| James | 2.18 | 2.51 | 2.45 | 1.78 | 5.88 |
| Kai | 2.47 | 1.86 | 1.11 | 1.80 | 3.31 |
| Soprano | 4.55 | 3.02 | 2.82 | - | - |
| Leolani | 1.65 | 1.36 | 0.86 | 2.58 | 3.99 |
| Aculele soprano, nylon | 8.16 | 4.12 | 2.40 | 4.04 | 7.68 |
| Aculele concert, nylon | 7.94 | 3.29 | 2.23 | 4.29 | 8.09 |
| Aculele tenor, nylon | 5.03 | 2.27 | 1.57 | 2.94 | 5.29 |
| Aculele concert, fluorocarbon | 2.55 | 1.39 | 0.85 | 1.84 | 3.07 |

| Partial T60 (s), band from | 250 | 400 | 700 | 1000 | 1500 | 2200 | 3200 | 4700 Hz |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Guffland | 1.66 | 2.38 | 2.54 | 1.50 | 1.16 | 0.89 | 0.76 | 0.58 |
| James | 2.86 | 2.63 | 1.92 | 1.30 | 1.11 | 0.66 | 0.62 | - |
| Kai | 2.00 | 3.50 | 2.76 | 1.44 | 1.25 | 0.93 | 0.72 | 0.53 |
| Soprano | 4.08 | 2.10 | 2.13 | 1.67 | 1.33 | 0.86 | 0.71 | - |
| Leolani | 3.11 | 2.71 | 2.09 | 1.53 | 1.05 | 0.84 | 0.63 | 0.61 |
| Aculele soprano, nylon | 3.93 | 2.18 | 2.01 | 1.50 | 1.08 | 0.84 | 0.55 | - |
| Aculele concert, nylon | 3.58 | 2.51 | 2.13 | 1.48 | 1.27 | 0.82 | 0.64 | - |
| Aculele tenor, nylon | 3.28 | 2.73 | 2.04 | 1.62 | 1.32 | 0.95 | 0.63 | 0.51 |
| Aculele concert, fluorocarbon | 5.07 | 3.55 | 3.14 | 2.20 | 1.96 | 1.23 | 1.08 | 0.67 |
| the same at String Age 1.0 | 4.74 | 3.27 | 2.89 | 1.87 | 1.36 | 0.78 | 0.59 | - |

| | glide (cents) | rise (ms) | beating H1 | H2 | H3 | H4 |
| --- | --- | --- | --- | --- | --- | --- |
| Guffland | +0.8 | 6.0 | 19% | 46% | 70% | 72% |
| James | +4.2 | 4.8 | 40% | 84% | 76% | 80% |
| Kai | +1.8 | 18.0 | 44% | 38% | 40% | 22% |
| Soprano | +2.1 | 9.6 | 50% | 79% | 86% | 100% |
| Leolani | +3.9 | 6.1 | 8% | 53% | 76% | 59% |
| Aculele soprano | +3.3 | 5.2 | 24% | 24% | 0% | 4% |
| Aculele concert | +1.8 | 4.8 | 28% | 4% | 8% | 4% |
| Aculele tenor | +0.7 | 5.8 | 36% | 8% | 4% | 12% |
| 0.4.27 soprano | +4.7 | 4.8 | 32% | 36% | 40% | 48% |
| 0.4.27 concert | +1.6 | 4.9 | 28% | 44% | 60% | 76% |
| 0.4.27 tenor | +0.6 | 6.1 | 40% | 52% | 68% | 100% |

Beating is the share of notes in which a partial's envelope, less a
quadratic fit to its decay, spans more than 6 dB within 2 s. Kai's rise time
and lower beating come from its quiet layer, recorded near its noise floor;
its loud layer alone reads 32%, 60% and 60% at H2-H4.

## What the comparison shows

- **Decay matches.** Aculele's nylon partials ring within the five
  instruments' spread in every band, and close to their median from 400 Hz
  up. Below 400 Hz the instruments themselves differ by a factor of 2.5.
- **Pitch glide and attack match.** Every instrument's notes start a median
  1-4 cents sharp and settle, as Aculele's do, and rise in 5-10 ms (Kai's
  quiet layer aside).
- **The strings are as stiff as Aculele's fluorocarbon, not its nylon.**
  Register by register, the five instruments' inharmonicity lies with the
  fluorocarbon concert's, and at a third to a half of the nylon's. The nylon
  figure is right for the strings it models: Woodhouse and Lynch-Aird's
  Table I (E' = 4.5 + 39σ GPa) on D'Addario's EJ65C C string, 1.02 mm at
  35 N, gives 6.5e-4, which the concert's C string reads. The recorded
  ukuleles are not strung with a nylon that thick; their sets are not stated.
- **Their partials still die as fast as the nylon's.** Fluorocarbon rings
  1.4-1.6 times longer than the instruments above 1 kHz, as the FreePats
  tenor's Worth Brown strings did against it
  ([recording comparison](recording-comparison-2026-10-28.md)). String Age
  1.0 brings fluorocarbon's 1.5-4.7 kHz into the instruments' range, but
  leaves 0.25-1 kHz about a third longer than their median, and String Age's
  shelf is Acustra's authored one. No string set Aculele has matches both
  the stiffness and the decay.
- **Real upper partials beat; Aculele's do not.** In every pack, partials 2-4
  swell and fade within the first two seconds: a dip over 6 dB in 38-84% of
  notes at H2, 40-86% at H3 and 22-100% at H4, the DI as much as the
  microphones. Aculele's do so in 0-12% at H3 and H4. Its fundamentals beat
  about as often as the instruments'.

### Why Aculele's upper partials do not beat

A partial beats when the microphone hears two members of it at slightly
different frequencies. A string's two planes, normal and parallel to the
top, are such a pair (Woodhouse, Acta Acustica 90 (2004) 945-965). Aculele
gives both planes one loop delay, so only the bridge tells them apart: on a
light nylon string its reactance moves a partial by a fraction of a cent.

Two changes were tried on the concert, rendering the same notes:
- Doubling the saddle's height over the top, which raises the parallel
  plane's share of the bridge's rocking, and releasing half of every pluck
  normal to the top, left the beating unchanged (0-12% at H2-H4).
- A loop 0.2% shorter in the normal plane than in the parallel one, about
  3.5 cents, beat in 44%, 60% and 76% of notes at H2-H4; 0.4%, about 7
  cents, in 52%, 68% and 80%. Over the three bodies, both splits fall about
  within the instruments' range, and the statistic cannot choose between them.

What makes a real ukulele string's two planes differ by a few cents (the
saddle's or the fret's sideways hold, the string's own asymmetry) has not
been identified or measured, so a split's size would be chosen to match
these recordings rather than derived. 0.4.27 takes the 0.2% split under the
owner's standing rule to accept candidates, with the choice still open by
ear, and the strings are left for the owner's ear (`Docs/decisions.md`).
0.4.27's partial decays stay within the five instruments' spread.
