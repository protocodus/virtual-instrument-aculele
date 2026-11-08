# Note color and decay across all constructions

The comparison uses the previous engine at `3f511c9` and the updated engine
with the same survey tool, capture gain tables and preset trims. This isolates
the changes in mechanical behavior. The final output calibration is measured
separately. All twelve combinations of soprano, concert and
tenor with spruce, mahogany, koa and cedar use Concert Strum's other
controls, including Standard tuning, nylon strings and stereo microphones.
Each isolated note starts after a reset. The tab sweep plays every string
from open to fret 18 at velocities 60 and 110, for 1,824 renders per engine.
The ordinary one-channel sweep covers MIDI 60–87 at the same velocities,
for another 672 renders per engine. The factory bank adds separate coverage
of Low G, D tuning, fluorocarbon and the other picking and capture modes.

The renders use 48 kHz and 256-sample blocks. A note is held for 1.6 s and
observed until 2.5 s. The survey measures stereo K-weighted power, the
fraction of onset power above 2.5 kHz and the early spectral centroid. Late
windows measure the note at 1.4–1.6 s while held, and 100–300 ms and
700–900 ms after MIDI key-up. The late windows include sympathetic strings,
body and room. The held slope is the first 200 ms less the 900–1,100 ms
window, divided by 0.85 s. It is a whole-signal comparison of early and late
energy, which can differ from an individual partial's exponential decay.

```sh
AculeleNoteSurvey --bank constructions --mode strings --out constructions-strings.csv
AculeleNoteSurvey --bank constructions --mode keys --out constructions-keys.csv
AculeleNoteSurvey --mode strings --out factory-strings.csv
```

## Held notes and key-up tails

Open C had the slowest held decay. Its nut now takes 0.0074 dB per round
trip, the former Acustra support loss. The fretted endpoint remains 0.022 dB,
as calibrated against the [sampled ukuleles](sample-pack-benchmark-2026-10-29.md).
The existing contact weight blends those endpoints for a moving stop near
the nut. The same loss applies to the nominal resting strings used in the
tuning calculation and the live open strings, including sympathetic ones.

The picking hand's authored open-string key-up catch now has a nominal
0.64 s T60, down from 1.25 s. The fretting hand's catch remains 0.32 s.
Release velocity still scales those times, and the sustain controls
continue to defer a normal key-up catch.

| Tab sweep result | Before | After |
| --- | ---: | ---: |
| Median held decay rate, all notes | 41.46 dB/s | 42.35 dB/s |
| Median held decay rate, open strings | 19.24 dB/s | 22.20 dB/s |
| Median held decay rate, open C | 11.25 dB/s | 13.36 dB/s |
| Loudest late held note, 1.4–1.6 s | −32.34 dB | −35.37 dB |
| Loudest key-up tail at 100–300 ms | −40.42 dB | −46.98 dB |
| Loudest key-up tail at 700–900 ms | −55.54 dB | −60.15 dB |
| Median open C tail at 700–900 ms | −84.05 dB | −98.70 dB |

The median paired change 700–900 ms after release was −5.31 dB across
all 1,824 tab renders. A few late tails rise as the strings and body trade
energy differently. The largest rise was 6.68 dB, from −91.75 to −85.07 dB
on Concert Mahogany's G-string fret 5 at velocity 60.

For ordinary keys, the median paired change in the 700–900 ms release
window was −5.23 dB. The additional 1,064 factory-preset renders showed a
−5.25 dB median change in the same window. The factory bank's strongest
late tail was Tenor Picked Melody's C4 played on the low G string at
fret 5. It fell from −40.24 to −44.25 dB. Its unplayed open C string still
rings sympathetically after key-up, and the full-string mute can stop it.

## Sound color

Concert Spruce's B4 on the A string at fret 2 sits near a strong top
resonance. At 493.88 Hz the original A-string bridge conductance is
0.138 s/kg, with 0.123 from the shifted 485.35 Hz mode and another 0.013
from its counterpart in the joint layer. In a velocity-110 note, the true
fundamental falls below a sympathetic component near C5. The survey's
fundamental search can then pick that component and report about +105 cents.
B4's second partial is stronger than both and still carries the played pitch.

The existing player-contact damping now follows the dominant top mode's
participation on each construction. The pressure observation and its
passive bridge twin share the loaded pole, with the same modal frequencies
and residues as before. In 100 ms Hann windows from the Concert Spruce
render, the change shifts the late fundamental balance without flattening
the body response.

| B4 spectral peak, 400–500 ms | Before | After |
| --- | ---: | ---: |
| B4 fundamental | −73.1 dBFS | −66.6 dBFS |
| Near-C5 sympathetic component | −64.9 dBFS | −68.0 dBFS |
| B4 second partial | −49.4 dBFS | −49.7 dBFS |

Across the tab sweep, the median paired change in the first 200 ms was
−0.05 dB and 95% of renders changed by at most 0.76 dB. The median paired
change in brightness was +0.08 dB and 95% changed by at most 1.04 dB in
absolute value. The median absolute held-decay deviation from the mean of
each note's two fret neighbors fell from 3.47 to 3.04 dB/s. The analogous
brightness deviation moved from 3.45 to 3.41 dB. There are still individual
color differences at the top of the neck. For example, the tenor cedar's
A-string fret 16 is brighter and quieter than its neighbors at both
velocities. The [earlier note survey](note-survey-2026-11-07.md) compares
these kinds of peaks and troughs with measured ukulele radiativity. This
change retains the construction-specific color.

| Shape and wood | Median brightness, dB | Median held decay, dB/s | Open C at 1.4–1.6 s, dB |
| --- | ---: | ---: | ---: |
| Soprano spruce | −28.28 → −27.71 | 41.97 → 43.89 | −33.08 → −36.10 |
| Soprano mahogany | −29.07 → −29.00 | 43.71 → 44.30 | −32.34 → −35.37 |
| Soprano koa | −28.25 → −27.72 | 45.67 → 46.55 | −32.60 → −35.60 |
| Soprano cedar | −27.61 → −26.83 | 43.48 → 45.64 | −33.82 → −36.84 |
| Concert spruce | −23.66 → −23.11 | 41.84 → 42.19 | −38.92 → −42.02 |
| Concert mahogany | −24.50 → −24.02 | 42.48 → 43.20 | −37.88 → −41.02 |
| Concert koa | −26.55 → −26.48 | 41.52 → 41.53 | −37.92 → −41.00 |
| Concert cedar | −23.64 → −23.37 | 44.05 → 44.08 | −39.51 → −42.76 |
| Tenor spruce | −24.51 → −24.23 | 40.89 → 41.80 | −35.53 → −38.68 |
| Tenor mahogany | −25.93 → −25.80 | 39.26 → 39.67 | −35.26 → −38.38 |
| Tenor koa | −24.68 → −24.49 | 38.31 → 39.23 | −34.44 → −37.57 |
| Tenor cedar | −22.18 → −22.14 | 42.53 → 43.14 | −37.32 → −40.56 |

The table shows velocity 110. PlateBand, ConstructionChange, Vibrato,
Tuning, StringAxialForce, BridgeMobility and Radiation pass on the updated
engine.

## Factory output calibration

`AculeleLevels --write` regenerated the capture reference table for the
updated engine. Tenor Fluorocarbon Fingerstyle's Output trim moved from
−0.5 to −0.8 dB and Tenor Picked Melody's from −0.9 to −1.0 dB. With those
trims, `AculeleLevels --check` passes. Its offline Reason meter model uses
a 300 ms window and −12 dB VU Offset. Typical factory playing reads
−9.05 to −8.83 VU. The hardest test chords peak at −1.50 dBFS. The furthest
alternate capture from the −9 VU target is Low G Tenor's piezo at −10.39 VU.

A repeat of the 912 velocity-110 tab cases with the new gain table also
verified every played string assignment. In each of the twelve
constructions, the scatter of first-second note levels about a straight
line in pitch was smaller. The mean scatter across constructions fell from
2.62 to 2.54 dB, and the worst construction fell from 2.99 to 2.83 dB.
