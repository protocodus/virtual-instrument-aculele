# The same stroke up the neck, 2026-11-08

A cross-instrument check of the bugs found in the 2026-11-07 reviews. Acustra
1.9.25 found that its pluck release held the hand at a fixed distance from the
bridge while the Finger easing and the knee moved it bridgeward on stopped
strings, so the same stroke grew louder the higher it was fretted. Aculele,
forked from Acustra 1.9.0, carried the same law:
`releaseScale = (1 - p) / (1 - a / L0)`, with `a = p L` the hand's distance
on the stopped string. Its high-register easing (non-Strum Picking, strings
shorter than the 4th fret's length) and its knee at 0.36 both move `p`, so
the open string the stroke is calibrated on was taken as plucked where the
stopped one was. 0.4.47 uses the compliance law Acustra 1.9.25 adopted, the
release scaling by `L p (1 - p)` against `L0 p0 (1 - p0)`, `p0` the point the
same hand meets the open string at.

## Measurement

`AculeleNoteSurvey --mode strings` (every string, frets 0-18, velocities 60
and 110, 48 kHz, each preset as shipped). Level (K-weighted, first second) of
each fret against the same string open, averaged over the four strings and
both velocities:

| Preset | 5th fret (0.4.46 → 0.4.47) | 12th | 18th |
| --- | --- | --- | --- |
| concert-piezo-stage | -2.1 → -2.4 dB | -0.0 → -2.2 dB | +2.1 → -2.4 dB |
| concert-strum | -2.4 → -2.7 dB | +0.6 → -1.7 dB | +3.2 → -1.3 dB |
| low-g-tenor | +0.8 → +0.6 dB | +1.1 → -1.1 dB | +0.9 → -3.7 dB |
| soprano-d-tuning | -1.3 → -2.0 dB | +1.6 → -1.1 dB | +1.9 → -3.2 dB |
| soprano-thumb | -0.7 → -1.0 dB | +0.5 → -1.7 dB | +1.3 → -3.0 dB |
| tenor-fluorocarbon-fingerstyle | -1.9 → -2.1 dB | -0.9 → -3.2 dB | +0.9 → -3.8 dB |
| tenor-picked-melody | -1.7 → -2.0 dB | -1.6 → -4.1 dB | -1.4 → -5.7 dB |

At 0.4.46 every preset but the picked tenor grew louder up the neck by 0.9-3.2
dB at the 18th fret. At 0.4.47 every preset falls with pitch, 1.3-5.7 dB at the
18th fret (about an octave and a half), which is the direction real
instruments take: the CC0 Eastman E1D guitar recordings Acustra measured fall
4.2 dB per octave finger-plucked. No ukulele recording here was measured for
this. Open strings are unchanged. `Aculele.Levels` was re-pinned with
`AculeleLevels --write` and four presets' Output moved 0.1-0.4 dB.

`Aculele.StringAxialForce`'s slow-limit check read a 12th-fret pluck at
velocity 0.9 whose stretch now stands at 0.0078 N, under the 0.01 N the check
needs; it now plays a full-velocity 5th-fret pluck (0.014 N), with the reason
beside it.

## Also checked, from the same round

- **The coupled-polarisation tuning** weighs each pole by its energy left
  after 3 s, the rule Acustra 1.9.26 replaced. Acustra's replacement (pole
  family by loudness over life, wide seeds) was ported and measured on the same
  sweep: settled pitch RMS 2.19 → 2.76 cents, notes more than 5 cents off 23 →
  85 (open G strings at the 2nd fret +9 cents, the fluorocarbon tenor's C at
  the 7th +11). With the step disabled the sweep reads 3.70 cents RMS, so on
  the ukulele the 3 s rule helps and stays. Not ported.
- **The sympathetic switch** keyed the tuning solve and dropped the resting
  strings from it when off. The solve now always includes them, so the switch
  changes only what is heard. No front end turns it off, and the survey is
  bit-identical.
- **Re-solving under the vibrato**: the mod-wheel vibrato does not re-run the
  tuning solve here (counted: one solve per note with and without it); a
  pitch-bend sweep re-solves per step, cheaply, as before.
- **Treble damping gated by string number** (Acustra 1.9.15's shelf): not in
  Aculele, whose losses are designed per note from its sounding length.
- **Loss filters fitted from the 2nd partial**: checked clean by the Aculyre
  review (2026-11-07).
