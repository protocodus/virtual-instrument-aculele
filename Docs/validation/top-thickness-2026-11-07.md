# A thinner top, a candidate not taken, 2026-11-07

A ukulele top thinner than the anchor guitar's, one ratio h = 0.848 pinned so
Concert Mahogany's T1 reads a measured concert's 395 Hz, was built and
measured on 2026-11-07 and not taken; Aculele stays at 0.4.46 with the
guitar's thickness. The measurements decided it: four unit tests went red at
their limits, the concert's T1 coincidence moved from the open A to the open
G (0.27 s/kg at 394 Hz) instead of going away, both Low G tenors played less
evenly (level SD 2.3 to 2.7 and 2.0 to 2.5 dB, steps up to 10 dB) and less in
tune (worst fundamental 10-12 cents against 6-8), and A0 moved away from the
three measured tenors. Nobody listened. The area-scaled plate-band count
measured with it was not taken either.

So the three measured ukuleles still sit below the law's T1 (Sunta's concert
at 395 Hz, the Labraña-Espinoza printed soprano at 324, the Elie et al.
soprano's mobility peak near 500, against the shipping 453-485 Hz concert and
610-653 Hz soprano), and the concert's T1 on its open A string, whose
fundamental drains at about 78 dB/s and reads 4-5 cents flat, remains a
known gap. A future attempt must first solve two things: the coupled
polarisation tuning and vibrato near T1 (the flat normal member that
`Aculele.Tuning` and `Aculele.Vibrato` caught), and that a lower T1 lands on
another open string (here the concert's G4) unless the box or the string set
moves with it.

## What was built

The body banks are a flamenco guitar's measured modes scaled to the three
ukulele boxes by Christensen and Vistisen's coupled plate-cavity pair and the
plate law (`bodyShapeMorph` in `Source/DSP/AculeleEngine.cpp`), which keeps
the guitar's top thickness: a plate's frequencies go as 1/A_p and its mass as
A_p. The candidate gave the top one relative thickness h
(`ukuleleTopThicknessRatio`): the plate alone rings at h/A_p, the cavity
spring on it goes as A_p/(V h), the low pair's bridge residues and the plate
modes' bridge residues as 1/(A_p h) (a separate `plateMobility` in the morph),
and the plate modes' radiation residues stay at 1/A_p (peak pressure per force
goes as 1/h, frequency as h). The woods' graduation multiplied on top.

## The pin

h = 0.848 put Concert Mahogany's T1 at 395 Hz (394.9; 0.849 gives 395.3),
Sunta's concert, the Kala KA-C the concert box is taken from being mahogany.
`Tools/AculeleBodyModes.cpp`, every wood (wood 0-3: spruce, mahogany, koa,
cedar):

| Shape | Wood | A0 before → after, Hz | T1 before → after, Hz |
| --- | --- | ---: | ---: |
| Soprano | Spruce | 266 → 262 | 653.4 → 562.3 |
| Soprano | Mahogany | 266 → 261 | 610.5 → 525.8 |
| Soprano | Koa | 265 → 261 | 583.9 → 503.3 |
| Soprano | Cedar | 266 → 262 | 697.7 → 600.0 |
| Concert | Spruce | 215 → 210 | 485.0 → 421.6 |
| Concert | Mahogany | 215 → 209 | 453.5 → 394.9 |
| Concert | Koa | 215 → 209 | 434.0 → 378.5 |
| Concert | Cedar | 216 → 211 | 517.7 → 449.3 |
| Tenor | Spruce | 212 → 205 | 415.2 → 364.7 |
| Tenor | Mahogany | 211 → 203 | 388.7 → 342.7 |
| Tenor | Koa | 211 → 202 | 372.3 → 329.2 |
| Tenor | Cedar | 212 → 206 | 442.7 → 387.8 |

The Elie soprano's ~500 Hz peak sits 5% under the mahogany soprano's 526 Hz
and on the koa soprano's 503. The printed soprano's 324 Hz stays 38% under
the mahogany soprano, a top built otherwise. One thickness for every Shape is
an assumption: only the concert is pinned. The lighter top lowers A0 4-9 Hz:
Concert Mahogany's 209 Hz was 13 Hz under Sunta's 222, and the tenors'
202-206 Hz sit under the three measured tenors' 211-214.

## What it needed re-pinned (reverted)

- `ukuleleTopMobilityRatio` 0.983 → 0.818: at the old ratio the lighter plate
  modes left Concert Mahogany's C string at 0.0108 s/kg over 1-6 kHz; at
  0.818 0.00900, the soprano's 0.00871 and the tenor's 0.00852
  (`AculeleBridgeMobility --check`).
- `radiationTiltShelfDb` -2.88/-1.36/-1.16 → -1.95/-1.13/-1.36 dB: the 0.4.46
  gains left the spruce bodies falling -4.05, -3.41 and -3.02 dB per octave
  over 1-6.3 kHz; re-pinned, -3.201, -3.200 and -3.202 against the measured
  soprano's -3.204 (`AculeleRadiation --check`) at the new gains.
- `Aculele.PlateBand` passed unchanged: its lower edge still takes modes
  whose cluster reaches 1 kHz. The plate modes sat 15% lower, so the bank
  puts 18-31 modes in the band or within a sixth of an octave under it
  (18-26 before); the band holds 50-61 sounding modes (51-62), and its highest
  sixth-octave average stands 1.07-1.83 dB over its mean (0.96-1.80). Every
  cluster still fit the 232 body slots at 44.1, 48 and 96 kHz.
- `LevelMatchData.h` rewritten by `AculeleLevels --write`; preset Output would
  have moved:
  Soprano D Tuning +1.2 → +1.95, Tenor Fluorocarbon Fingerstyle -0.5 → -0.7,
  Low G Tenor -1.6 → -2.1, Soprano Thumb -0.2 → -0.4, Tenor Picked Melody
  -1.1 → -1.5 dB. The report asked +2.4 and -0.2 for the two sopranos; at
  +2.4 Soprano D Tuning's piezo read -7.09 VU, 1.9 dB over the target, and at
  -0.2 Soprano Thumb's loudest chords reached -0.89 VU. The two mahogany
  sopranos' T1 lay on C5 (526 Hz), which the Standard-tuning level
  calibration plays and D tuning plays less. Their trims sit inside
  `Aculele.Levels`'s own bounds (typical playing -9.46 and -9.19 VU).

## Also not taken: the band's count by area

Elie et al.'s Eq. 4 gives a thin plate's modal density as (S/2) sqrt(rho h / D),
so the band's 57 (a soprano's count) would scale with the top's area: about
78 modes on the concert and 92 on the tenor. With h in place it does not fit:
the concert bodies would need 244-251 and the tenors 265-271 of the 232 body
slots, and the clusters above 3.45-4.24 kHz would be dropped
(`bandEnd + members > bodyModeCount`). It needs `ACULELE_BODY_MODE_COUNT`
raised to about 272 and `plateClusterLimit` to 28, a CPU cost not measured
here. Measured with the slots raised (a scratch build, not shipped), its
single-microphone fluctuation does not move clearly toward the measured
soprano's 2.2-3.4 dB:

| Body | 1/12-octave SD (largest), dB: 0.4.46 | h = 0.848 | h = 0.848 and the band by area |
| --- | --- | --- | --- |
| Soprano Spruce | 3.06 (9.2) | 2.87 (7.9) | 2.87 (7.9) |
| Soprano Mahogany | 2.85 (5.5) | 3.33 (9.8) | 3.33 (9.8) |
| Soprano Koa | 2.74 (7.8) | 3.61 (11.6) | 3.61 (11.6) |
| Soprano Cedar | 2.60 (5.8) | 2.61 (7.9) | 2.61 (7.9) |
| Concert Spruce | 4.21 (11.8) | 3.77 (11.3) | 3.91 (10.0) |
| Concert Mahogany | 4.52 (12.6) | 3.65 (9.8) | 4.17 (11.0) |
| Concert Koa | 4.21 (11.2) | 3.52 (8.9) | 3.61 (8.4) |
| Concert Cedar | 3.74 (12.1) | 4.50 (13.8) | 4.03 (10.5) |
| Tenor Spruce | 3.78 (10.9) | 4.22 (11.2) | 4.00 (11.4) |
| Tenor Mahogany | 3.59 (9.4) | 4.65 (11.7) | 4.05 (12.6) |
| Tenor Koa | 3.58 (9.1) | 2.71 (5.9) | 3.94 (9.1) |
| Tenor Cedar | 4.40 (13.3) | 3.82 (12.3) | 3.73 (12.2) |

Statistic b of `tools/fluct.py` (the run of the note survey): each
1/12-octave band's power level over 1-4.4 kHz about its third octave's, from
`AculeleRadiation --curve`. The thinner top alone moves the concert's from
3.74-4.52 to 3.52-4.50 dB and the tenor's from 3.58-4.40 to 2.71-4.65; the
soprano's from 2.60-3.06 to 2.61-3.61.

## Every key, before and after

`Tools/AculeleNoteSurvey.cpp`, velocities 60 and 110, every preset as shipped
(its header has the protocol). Levels over C4-C6 (MIDI 60-84): the SD of each
note's level about a line through them, the largest step between
neighbouring keys and the largest residual. Tuning over the whole range:
f0_cents_error (the strongest peak near the note) and pitch_cents_error (the
partials-weighted pitch). Before is 0.4.46, the shipping engine (identical to
the earlier survey-keys-final2 run); after is the candidate.

Keys (one MIDI channel, the Performer chooses the string):

| Preset | Velocity | Level SD / largest step / worst residual (key), dB | f0 error mean / SD / max, cents | Partials-weighted pitch error mean / SD / max, cents |
| --- | ---: | --- | --- | --- |
| concert-piezo-stage | 110 | 2.20/3.2/-5.0(71) → 1.75/2.7/+3.4(64) | +1.35/2.05/6.9 → +1.55/2.04/8.8 | +1.89/1.50/5.8 → +2.11/1.19/5.3 |
| concert-piezo-stage | 60 | 2.48/3.3/-5.9(71) → 1.98/2.6/-3.9(69) | +1.30/2.16/7.4 → +1.48/2.08/8.6 | +1.80/1.55/5.6 → +2.02/1.24/5.3 |
| concert-strum | 110 | 2.62/5.3/-7.7(74) → 2.43/6.2/-6.6(71) | +1.35/1.78/6.2 → +1.37/2.06/6.5 | +1.98/1.31/4.6 → +2.00/1.40/5.2 |
| concert-strum | 60 | 2.82/5.2/-8.3(74) → 2.61/6.2/-7.2(71) | +1.26/1.85/6.1 → +1.02/2.48/6.4 | +1.88/1.34/4.7 → +1.87/1.31/4.8 |
| low-g-tenor | 110 | 2.28/6.0/-6.6(71) → 2.65/10.0/-7.8(83) | +1.33/2.31/8.0 → +1.42/3.09/12.0 | +1.66/1.53/5.2 → +1.85/1.85/7.4 |
| low-g-tenor | 60 | 2.50/6.0/-7.2(71) → 2.71/10.3/-7.9(83) | +1.20/2.36/8.0 → +1.33/3.19/11.9 | +1.56/1.66/5.2 → +1.76/1.97/7.4 |
| soprano-d-tuning | 110 | 3.44/4.1/-8.2(78) → 2.88/5.1/-7.3(76) | +1.38/1.58/5.4 → +1.30/1.38/5.2 | +1.85/1.31/5.2 → +1.93/1.10/5.0 |
| soprano-d-tuning | 60 | 3.58/4.6/-8.7(78) → 3.02/5.2/-7.8(76) | +1.32/1.66/5.5 → +1.18/1.46/5.3 | +1.77/1.38/5.3 → +1.80/1.17/5.2 |
| soprano-thumb | 110 | 3.31/3.9/-8.6(78) → 2.96/5.4/-7.3(76) | +1.23/1.38/4.7 → +1.33/1.52/6.2 | +1.70/1.34/4.6 → +1.91/1.26/6.2 |
| soprano-thumb | 60 | 3.38/4.5/-8.8(78) → 3.04/5.6/-7.7(76) | +0.97/1.31/3.6 → +1.16/1.59/5.9 | +1.43/1.28/3.7 → +1.68/1.31/5.9 |
| tenor-fluorocarbon-fingerstyle | 110 | 2.64/9.3/-6.7(72) → 2.05/5.9/+3.9(80) | +1.28/1.92/6.3 → +1.55/3.02/11.1 | +1.82/1.10/4.5 → +1.84/1.39/6.0 |
| tenor-fluorocarbon-fingerstyle | 60 | 2.90/9.6/-7.2(72) → 2.22/6.2/+4.0(80) | +1.26/1.95/6.3 → +1.53/3.01/10.9 | +1.81/1.14/4.6 → +1.83/1.42/6.2 |
| tenor-picked-melody | 110 | 1.95/5.9/-5.4(70) → 2.54/6.2/+7.0(80) | +0.76/2.13/8.0 → +1.74/2.85/9.9 | +1.59/1.18/4.0 → +1.89/1.27/4.5 |
| tenor-picked-melody | 60 | 2.14/6.4/-6.1(70) → 2.70/6.9/+7.5(80) | +0.56/2.28/7.8 → +1.56/3.05/9.8 | +1.39/1.29/4.1 → +1.70/1.41/4.4 |

Strings (`--mode strings`, every string open to the 18th fret; a pitch two
strings play is averaged for the level line):

| Preset | Velocity | Level SD / largest step / worst residual (key), dB | f0 error mean / SD / max, cents | Partials-weighted pitch error mean / SD / max, cents |
| --- | ---: | --- | --- | --- |
| concert-piezo-stage | 110 | 1.47/2.2/-4.2(71) → 1.32/2.2/-3.5(68) | +0.95/2.46/8.8 → +1.20/2.58/8.8 | +1.70/2.04/6.2 → +1.74/2.12/6.5 |
| concert-piezo-stage | 60 | 1.69/2.5/-4.8(71) → 1.54/2.5/-4.0(69) | +0.73/2.33/7.4 → +1.01/2.56/8.6 | +1.52/1.97/6.1 → +1.54/2.10/6.4 |
| concert-strum | 110 | 1.44/2.9/+2.9(77) → 1.75/8.1/-4.9(83) | +0.45/2.38/8.0 → +2.03/12.26/105.4 | +1.22/2.06/6.4 → +2.43/12.09/105.3 |
| concert-strum | 60 | 1.58/3.1/+3.3(77) → 1.86/8.6/-5.3(83) | +0.31/2.25/7.9 → +1.89/12.30/105.4 | +1.07/1.93/5.3 → +2.33/12.10/105.3 |
| low-g-tenor | 110 | 1.56/7.7/-4.2(83) → 2.75/8.9/-9.0(83) | +0.55/3.23/9.8 → +0.71/3.52/12.0 | +0.97/2.02/6.2 → +1.11/2.09/7.4 |
| low-g-tenor | 60 | 1.67/8.1/-4.5(83) → 2.88/9.3/-9.5(83) | +0.42/3.26/9.8 → +0.50/3.37/11.9 | +0.84/2.16/6.3 → +0.95/2.14/7.4 |
| soprano-d-tuning | 110 | 2.34/3.8/+5.4(83) → 1.52/4.8/+3.6(79) | +0.45/2.06/5.4 → +0.59/2.03/5.7 | +1.24/1.82/5.2 → +1.33/1.93/5.8 |
| soprano-d-tuning | 60 | 2.52/4.1/+6.0(83) → 1.62/5.1/+3.8(79) | +0.35/2.06/5.5 → +0.41/2.04/6.0 | +1.10/1.82/5.3 → +1.14/1.89/6.0 |
| soprano-thumb | 110 | 2.44/3.9/+5.4(83) → 2.21/5.4/+7.1(60) | +0.46/2.17/6.7 → +0.57/2.31/6.6 | +1.29/1.97/6.7 → +1.33/2.18/6.6 |
| soprano-thumb | 60 | 2.51/4.1/+5.8(83) → 2.23/5.6/+7.3(60) | +0.15/1.91/6.1 → +0.19/2.43/6.6 | +0.91/1.85/5.0 → +0.97/2.15/6.1 |
| tenor-fluorocarbon-fingerstyle | 110 | 1.84/8.7/-5.7(84) → 1.55/4.6/+4.0(80) | +0.72/2.30/6.8 → +0.80/3.10/11.1 | +1.09/1.72/4.6 → +1.03/1.99/6.3 |
| tenor-fluorocarbon-fingerstyle | 60 | 1.98/9.2/-6.0(84) → 1.65/4.9/+4.3(80) | +0.71/2.30/6.7 → +0.77/3.12/10.9 | +1.07/1.74/4.6 → +1.02/1.99/6.4 |
| tenor-picked-melody | 110 | 1.76/5.3/+4.6(81) → 2.48/5.9/+7.7(80) | +0.57/2.95/9.5 → +1.23/2.97/11.7 | +1.42/1.56/5.2 → +1.55/1.50/4.5 |
| tenor-picked-melody | 60 | 1.85/5.8/+4.9(81) → 2.65/6.3/+8.4(80) | +0.21/3.21/10.0 → +1.02/3.09/11.5 | +1.18/1.46/4.9 → +1.31/1.53/4.4 |

The keys even out on five presets (level SD 0.2-0.6 dB lower) and spread on
the two tenors that play Low G (0.2-0.6 dB higher; Low G Tenor's largest step
6 → 10 dB, at B5). The tenors' worst f0 error grows from 6-8 to 10-12 cents,
on G4-F#4 (MIDI 65-67) over T1, where the normal polarisation is pulled sharp;
the partials-weighted pitch grows less (4.5-5.2 to 4.5-7.4). On Concert Strum
the C string's G#4 (fret 8) reads 105 cents sharp in the strings survey: its
fundamental, 5% over the candidate's T1, drained at 101 dB/s and by 0.3 s the open A
string's sympathetic 441 Hz is as strong, so the estimator takes it.

## Across the strings

`Tools/AculeleEvenness.cpp` (nylon, velocity 90, 48 shapes, woods and
Pickings), mean over each Picking:

| Picking | G string's lead over the four strings' mean, dB | Cell level SD, dB | One pitch on two strings, dB apart |
| --- | --- | --- | --- |
| Finger | 2.0 → 1.5 | 2.67 → 2.36 | 3.61 → 3.40 |
| Pick | 3.6 → 3.4 | 2.90 → 2.81 | 5.46 → 5.37 |
| Thumb | 2.1 → 1.5 | 2.90 → 2.56 | 4.01 → 3.80 |
| Strum | 2.0 → 1.4 | 2.67 → 2.37 | 3.57 → 3.37 |

## The notes that sat on T1

The fundamental alone (`AculeleProbe <preset> <mid> <wav> 2.5 --float`, one
note; its level over 0.05-0.5 s in 4096-sample Hann frames, its frequency the
median of their interpolated peaks):

| Note | Velocity | Decay, dB/s, before → after | Cents, before → after |
| --- | ---: | --- | --- |
| Concert Strum, open A (A4) | 110 | 77.9 → 36.4 | -4.4 → +5.5 |
| Concert Strum, open A (A4) | 60 | 79.6 → 36.6 | -4.7 → +5.3 |
| Low G Tenor, E string fret 4 (G#4) | 110 | 69.0 → 39.8 | +8.4 → +5.9 |
| Low G Tenor, E string fret 4 (G#4) | 60 | 69.1 → 39.3 | +8.1 → +5.6 |
| Concert Strum, open G (G4) | 110 | 20.6 → 43.3 | -2.0 → -0.1 (26.8 c wobble) |
| Concert Strum, C string fret 8 (G#4) | 110 | 38.4 → 101.2 | -2.2 → +12.1 |

On the candidate the open A and the tenor's G#4 no longer drained, but
Concert Mahogany's T1 lay on the open G (392 Hz) instead: its conductance at
the C string peaked at 0.27 s/kg at 394 Hz (0.21 at 455 at 0.4.46), the open G
decayed twice as fast and wobbled 27 cents across its first half second, and
G#4 a semitone above it drained at 101 dB/s. The pin moved the coincidence; it did not remove it.

## Tests

`ctest -L '^(unit|audio|perf)$'` on the candidate: 16 of 20 passed (`Levels`,
`BridgeMobility`, `Radiation`, `PlateBand`, `AudioDemos`, `CpuBudget` among them;
`AculeleDSPCxx17` builds). Four failed, each passing at 0.4.46. Scratch builds
separate the two re-pins: h = 1 with the new mobility ratio passes all four;
h = 0.848 with the old ratio fails `Vibrato` (worse: 9.05 Hz, a 32-cent
swing) and `FingerLookahead` (-2.6 dB, 8.3 dB together) but passes
`StringAxialForce` (5.5 degrees) and reads the D4 below at -3.2 cents. So
the thinner top fails the first two, and the two together the other two:

- `Aculele.Tuning`: the tenor's C string in D tuning (D4) reads -5.95 cents
  over 1-2 s (limit 5) on nylon and fluorocarbon. The tenor mahogany's T1 at
  343 Hz raises its C string's conductance at 294 Hz from 0.0035 to
  0.013 s/kg; the flat normal member lasts into the window.
- `Aculele.Vibrato`: on Concert Mahogany, F4 at the E string's 1st fret, 12%
  under T1, tracks at 6.24 Hz instead of 4.8-5.4 (swing 16.9 cents, within
  12-17); the two polarisation members beat in the pitch track.
- `Aculele.StringAxialForce`: on the 5th-fret A (D5), the waves' 4 f0 line
  agrees with the uniform tension in level (-0.05 dB) but not in phase, 22
  degrees against the 15 allowed (6.8 at 0.4.46).
- `Aculele.FingerLookahead`: the third finger shift's slide noise stands
  2.6 dB under the plain player's residual (4.3 over at 0.4.46), and the
  slides together 8.1 dB over it against the 12 required (14.1 at 0.4.46).

No limit was moved, and the candidate was reverted.
