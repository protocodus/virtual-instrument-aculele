# Every note of every preset, 2026-11-07

The owner asked for every playable key at two velocities to be checked for
outliers in level, tone and resonance, so that each preset plays smoothly
across its range. This is what was measured at 0.4.45, what 0.4.46 changed,
what was measured and not taken, and what was checked and kept.

`Tools/AculeleNoteSurvey.cpp` plays every key of each factory preset, from its
tuning's lowest open string to its highest string's 18th fret, each note alone
after a reset at velocities 110 and 60, through the preset exactly as shipped
(room, release noise, capture and Output included), on one MIDI channel so the
Performer chooses the string; `--mode strings` plays every string from open to
the 18th fret as tab instead. Its header gives the protocol and every column.
The figures below take each note's K-weighted level over its first second,
over C4-C6 (MIDI 60-84), less a straight line through them: the SD of what is
left, the largest step between neighbouring keys, and the most negative
residual and its key. Across the strings, `Tools/AculeleEvenness.cpp` (nylon,
velocity 90, the 48 shapes, woods and Pickings) gives each string's mean level
against the four strings' mean, each cell's level SD and how far apart one
pitch plays on different strings.

## What 0.4.46 changed

The plate band's hard lower edge at 1 kHz left a mode just under it alone
while the modes above it were clustered and brought to the band's mean. On
the koa tenor a plate mode at 992.6 Hz stood 3.7 dB under T1 with every
clustered neighbour 10-25 dB lower, which put B5 on the A string (string 3,
fret 14) +8.6 dB (v110) and +9.2 dB (v60) over the line in Tenor Picked
Melody. A mode now belongs to the band when its cluster, a sixth of an octave
either side, reaches 1 kHz (`buildBodyConfiguration`). The band then holds
51-62 modes between 1 and 4.4 kHz across the twelve bodies (50-62 before).

The cluster factor brings each source mode's power to the band's mean before
the members are drawn, but their random places and phases left the realised
band's local power a draw again. Taking the two modes under 1 kHz into the
band reshuffled every later draw, and Soprano Mahogany's highest
sixth-octave average rose from 2.41 to 3.04 dB over the band's mean, past
`Aculele.PlateBand`'s 3 dB. One pass after the draw now brings every sounding
mode in the band to the mean over a sixth of an octave either side of it; it
is not iterated, so the fluctuation between neighbouring modes stays. The
highest sixth-octave average now reads, at 44.1, 48 and 96 kHz alike:

| Shape | Spruce | Mahogany | Koa | Cedar |
| --- | --- | --- | --- | --- |
| Soprano | 1.80 | 1.79 | 0.96 | 1.50 |
| Concert | 1.55 | 1.48 | 1.47 | 1.59 |
| Tenor | 1.41 | 1.33 | 1.27 | 1.45 |

The three radiation shelves (`radiationTiltShelfDb`) were re-pinned to
`Aculele.Radiation` from -2.09/-1.75/-1.44 to -2.88/-1.36/-1.16 dB, which
reads -3.208, -3.202 and -3.207 dB per octave against the measured -3.204.
`LevelMatchData.h` was rewritten and four presets' Output moved: Soprano D
Tuning 1.4 to 1.2 dB, Tenor Fluorocarbon Fingerstyle -0.4 to -0.5, Soprano
Thumb -0.1 to -0.2 and Tenor Picked Melody -2.4 to -1.1. The bridge level is
unchanged.

## Before and after, C4-C6

| Preset | velocity | SD 0.4.45 | SD 0.4.46 | largest step 0.4.45 | 0.4.46 | worst 0.4.45 | 0.4.46 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Concert Piezo Stage | 110 | 2.20 | 2.20 | 3.2 | 3.2 | -5.0 at 71 | -5.0 at 71 |
| | 60 | 2.48 | 2.48 | 3.3 | 3.3 | -5.9 at 71 | -5.9 at 71 |
| Concert Strum | 110 | 2.68 | 2.62 | 5.2 | 5.3 | -7.7 at 74 | -7.7 at 74 |
| | 60 | 2.87 | 2.82 | 5.1 | 5.2 | -8.2 at 74 | -8.3 at 74 |
| Low G Tenor | 110 | 2.36 | 2.28 | 6.6 | 6.0 | -6.8 at 71 | -6.6 at 71 |
| | 60 | 2.57 | 2.50 | 6.6 | 6.0 | -7.4 at 71 | -7.2 at 71 |
| Soprano D Tuning | 110 | 3.18 | 3.44 | 5.6 | 4.1 | -8.5 at 79 | -8.2 at 78 |
| | 60 | 3.38 | 3.58 | 6.2 | 4.6 | -8.9 at 79 | -8.7 at 78 |
| Soprano Thumb | 110 | 3.12 | 3.31 | 5.2 | 3.9 | -8.7 at 78 | -8.6 at 78 |
| | 60 | 3.19 | 3.38 | 4.9 | 4.5 | -9.0 at 78 | -8.8 at 78 |
| Tenor Fluorocarbon Fingerstyle | 110 | 2.71 | 2.64 | 9.4 | 9.3 | -6.8 at 72 | -6.7 at 72 |
| | 60 | 2.96 | 2.90 | 9.7 | 9.6 | -7.2 at 72 | -7.2 at 72 |
| Tenor Picked Melody | 110 | 2.35 | 1.95 | 7.2 | 5.9 | -3.9 at 72 | -5.4 at 70 |
| | 60 | 2.48 | 2.14 | 8.0 | 6.4 | -4.3 at 72 | -6.1 at 70 |

SD, steps and residuals in dB. On Tenor Picked Melody B5 now reads +2.9 dB
(v110) and +3.5 dB (v60) over the line fitted to the whole range; its largest
residual above C4 is +5.4-5.8 dB at D#6, the A string's 18th fret and the
preset's top key. The two sopranos spread 0.2-0.3 dB more, though their
largest steps are smaller; Concert Piezo Stage is unchanged, and the other
three spread 0.05-0.1 dB less. Four sampled ukulele packs spread 2.2-3.7 dB
(SD) over C4-C6 with largest steps of 5.4-12 dB
([benchmark](sample-pack-benchmark-2026-10-29.md)); every preset lies inside
those bounds or under them.

Across the strings 0.4.46 leaves the balance where it was: the G string's
lead over the four strings' mean, averaged by Picking, is 2.0-3.6 dB (1.9-3.5
at 0.4.45; 0.6-4.3 over the cells against 0.5-4.3), the cells' mean level SD
2.67-2.90 dB (2.65-2.88), and one pitch on different strings 3.6-5.5 dB apart
(3.6-5.5).

## The band's fine fluctuation

Measured on one microphone, the left force response of `AculeleRadiation
--curve` on its 1/48-octave grid, and the same grid read from Labraña et
al.'s 18 single positions (below): over 1-4.4 kHz, (a) the SD of the level
about a least-squares line in log frequency, and its largest excursion; (b)
the SD of 1/12-octave power levels about their third octave's, and its
largest. Before is 0.4.46 without the pass after the draw (with its own
shelves re-pinned); after is what ships.

| Body | (a) before | (a) after | (b) before | (b) after |
| --- | --- | --- | --- | --- |
| Soprano spruce | 4.73, 17.4 | 4.85, 22.2 | 3.29, 9.2 | 3.06, 9.2 |
| Soprano mahogany | 4.21, 13.4 | 3.80, 13.8 | 3.44, 8.0 | 2.85, 5.5 |
| Soprano koa | 4.82, 17.3 | 4.37, 13.4 | 2.93, 8.2 | 2.74, 7.8 |
| Soprano cedar | 3.44, 10.3 | 3.38, 14.5 | 2.57, 4.9 | 2.60, 5.8 |
| Concert spruce | 5.71, 19.8 | 5.52, 15.8 | 4.23, 12.9 | 4.21, 11.8 |
| Concert mahogany | 5.94, 19.1 | 6.08, 21.4 | 4.46, 13.2 | 4.52, 12.6 |
| Concert koa | 6.03, 19.9 | 6.26, 22.1 | 4.04, 11.4 | 4.21, 11.2 |
| Concert cedar | 5.57, 15.9 | 5.79, 23.7 | 3.87, 11.5 | 3.74, 12.1 |
| Tenor spruce | 6.79, 29.7 | 6.09, 20.3 | 3.84, 9.9 | 3.78, 10.9 |
| Tenor mahogany | 6.22, 23.3 | 5.53, 22.0 | 3.65, 10.0 | 3.59, 9.4 |
| Tenor koa | 6.76, 29.2 | 6.05, 26.0 | 3.60, 10.7 | 3.58, 9.1 |
| Tenor cedar | 6.25, 22.0 | 6.38, 22.4 | 4.13, 12.6 | 4.40, 13.3 |
| Measured soprano, 18 positions | 5.2-6.8, 12.8-34.1 | | 2.2-3.4, 5.1-11.0 | |

SD, then largest, in dB. By (a) every body already lies at or under the
measured soprano's range. By (b) the sopranos lie inside it and the concert
and tenor stay 0.2-1.1 dB over it; the pass evens a third of an octave and
leaves the fluctuation within it, so it does not close that gap, and nothing
measured says how a concert's or a tenor's band should fluctuate.

## Consistent residue scaling, measured and rejected

`scalePlateBridgeResidues` scales a plate mode's heave, cross and rocking
residues all by 1/A_p. The lever arms are physical lengths (`saddleLeverArm`,
13.3 mm string spacing over the archive's 21.6 mm half-separation), and on
the equal-thickness law a mode shape's slope goes as A_p^-1/2, so the
scaling consistent with the law is cross 1/A_p^1.5 and rocking 1/A_p^2, with
the moment radiation residues taking the same extra A_p^-1/2. That candidate
was built together with the band change above, its bridge level re-pinned (`ukuleleTopMobilityRatio`
0.983 to 0.917) and its shelves re-pinned (-1.79/-1.42/-0.85 dB), and measured:

- Over C4-C6 the key survey's SD was 1.77-3.31 dB at v110 and 2.04-3.43 at
  v60, against 1.95-3.44 and 2.14-3.58 for what ships. It was better on
  Concert Strum, Low G Tenor and the fluorocarbon tenor by 0.15-0.3 dB.
- Across the strings it was worse: the G string's lead 2.7-4.2 dB against
  1.9-3.5 at 0.4.45 (0.9-5.1 over the cells against 0.5-4.3), the cells' mean
  level SD 2.92-3.17 against 2.65-2.88, one pitch on different strings
  4.5-6.0 dB apart against 3.6-5.5.
- Concert Mahogany's bridge-rocking mode near 638 Hz went back to 0.34 s/kg
  at the A string and 0.21 at the G (0.13 and 0.05 as shipped), the drain
  0.4.38 removed.
- `Aculele.Vibrato` failed: the full wheel on D5 at the G string's 7th fret,
  587 Hz, 1.4 semitones under that mode, swung 84.3 cents at 5.64 Hz against
  57.3 cents at 5.06 Hz. `Aculele.StringAxialForce` failed: on the A string's
  5th fret, also 587 Hz, the waves' force at 4 f0 sat 19.7 degrees from the
  uniform tension's, against 6.8 and the test's 15. Both pass with the
  uniform scaling.
- `Aculele.ConstructionChange` failed on the piezo's Concert to Tenor change
  with the bridge ratio at 0.917 alone: 54.3 dB under the chord against the
  55 dB bound (57.8 at 0.983).

The bank is a guitar's bridge stretched to a ukulele's top, and these
measurements decide against the consistent scaling, so the uniform 1/A_p
stays as a pinned choice (`Docs/decisions.md`, 2026-11-07).

## The holes above T1 were checked and kept

The A string's quietest notes, C5-D#5 on the concert (72-75) and F#5-G5 on
the soprano (78-79), lie where the body radiates little between T1 and the
next mode, 15-20 dB under T1. Labraña, Gajardo, Correa and Espinoza's hybrid
soprano (Zenodo, doi:10.5281/zenodo.22692775, `data/mean_rad_hybrid_XZ.txt`
and `_YZ.txt`, the 18-position mean radiativity in 1 Hz steps) has its A0 at
200 Hz and top modes at 318 and 375 Hz, then a trough from 450 to 680 Hz
20-25 dB under the top modes' peaks and another at 750-850 Hz. A real
soprano falls as far there, so the model's fall was kept.

Between 1 and 4.4 kHz the concert's and tenor's bands fluctuate more than
the measured soprano's (table above). Halving the plate band's Q only partly
closed that gap, measured at 0.4.45, and broke the radiation slope pin
(`Aculele.Radiation`), so it was not changed.
