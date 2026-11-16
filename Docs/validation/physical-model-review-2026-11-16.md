# Physical model review, 2026-11-16

The engine reviewed is 0.4.67 (`3f4d8e3`); the candidate is 0.4.68. Both are
Release builds from the same compiler and flags (GCC 13.3, Ninja), and every
render is 48 kHz in 256-sample blocks unless a rate is named. The review read
the pluck's initial condition, the bridge junction, the room and the loss
laws against the model's own physics, and measured each candidate before
deciding. One issue held up and is fixed. The rest are listed with what was
measured and why nothing changed.

## Fixed: the saddle's held force was let go in one sample

**The issue.** Before a pluck lets go, the hand holds the string aside and
the saddle carries the static force of the string's slope. The release gives
that force back to the bridge (`initialisePluck`, the whole force since
0.4.56), and the string's wave carries the rest of the change. Since 0.4.35 a
slow release (a thumb's pad, a soft nail or pick) is smoothed into the wave
over the whole period at the contact's own slip time. The force on the saddle
is let go by the same contact in the same time, but it was shed in one
sample, so a soft note began with a step in force that its own string did not
have.

**Evidence.** The engine stepped one sample at a time, an E4 with Picking
Thumb at velocity 32, reading the bridge's force on the body
(`getLastBridgeBodyForce`): after one sample it stood at 0.99 of its value 16
samples on. The repository's own click measure
(`Aculele.ThumbDynamics`, the loudest millisecond above 6 kHz against the
note's mean power) read -21.9 and -22.3 dB for a soft G5 and A5.

**The fix.** The force comes off the saddle through one pole at the stroke's
slip: the pad's actual contact time for Thumb, the edge's for the nails and
the pick (`releaseStepSlipPole`). What is still held is kept as a force, not
as a filter's state, so a re-pluck with another slip sheds all of it and the
total given back is unchanged. A hard edge's pole is near zero and sheds it
at once, as before: a full-velocity pick's first sample still carries more
than the sixteenth (1.61 of it).

**After.** The same E4 reads 0.17 after one sample and rises smoothly. The
soft G5 and A5 read -37.5 and -34.3 dB. `Aculele.ThumbDynamics` now holds
both: a soft thumb's first sample under a quarter of the sixteenth, and a
hard pick's over half. 0.4.67 fails the first.

**What changes audibly.** B (0.4.68) minus A (0.4.67) in dB, the median over
C4, E4, A4, E5 and A5, each note alone on Concert Strum's controls with only
Picking changed, at velocities 20, 50, 90 and 127:

| Picking | Band and span | 20 | 50 | 90 | 127 |
| --- | --- | ---: | ---: | ---: | ---: |
| Thumb | 3-8 kHz, first 5 ms | -10.0 | -4.9 | -3.3 | -1.4 |
| Thumb | 8-16 kHz, first 5 ms | -12.1 | -8.8 | -4.5 | -1.0 |
| Thumb | 3-8 kHz, first 30 ms | -5.0 | -1.5 | -0.6 | -0.3 |
| Pick | 3-8 kHz, first 5 ms | -4.9 | -0.7 | -0.1 | -0.1 |
| Finger (nails) | 3-8 kHz, first 5 ms | -2.0 | -0.3 | 0.0 | -0.1 |

Strum's single notes take a nail and move as Finger does. The top's knock
(60-300 Hz, first 50 ms) stays within 0.3 dB in the median and 1.2 dB at most
(a velocity-20 thumb); whole notes stay within 0.2 dB. In words: soft thumb
notes lose a tick at their start, soft picked and nail notes lose a little of
it, and loud playing is as it was.

**Against a recording.** The FreePats tenor
(`Tools/compare_recorded_notes.py`) is one unstated dynamic and technique.
Against the velocity-90 render the change moves the click measure 0-2.5 dB,
inside what the unknown technique allows, so the recording neither confirms
nor rejects it. The ground for the fix is that one contact cannot let the
string go slowly and the saddle go at once. Acustra made the same change for
its pads in 1.9.46, there measured against its recordings.

**Levels.** `AculeleLevels --write` re-pinned the level match (factors moved
at most 0.2%). Low G Tenor's piezo read -10.50 VU before the change, on the
1.5 dB Capture bound, and just past it after; its Output moved from -1.5 to
-1.4 dB, which reads -8.74 VU with the piezo at -10.40. `Aculele.Levels`
passes.

**Listening.** An A/B set of three soft phrases (thumb, fingerstyle, strum)
is in the project files under
`aculele/listening/2026-11-16-release-step-slip/`. The owner has not heard it
yet ([decision](../decisions.md)).

## Real but too small to change: the parallel plane's held force

The string is held aside in two planes. Only the normal plane's held force
has a release step; the parallel plane's, a moment on the saddle's crown, has
none, though its wave has a rocking port at the bridge. Given back the same
way in a trial build, it moved the 60-300 Hz knock by under 0.2 dB and the
first 5 ms at 3-8 kHz by 0.2-2 dB, up on the G and C strings and down on the
E and A. That is under what a listening set could decide and its sign is not
one direction, so it is left out, and the comment in `initialisePluck` says
so.

## Examined and left as they are

- **The first period of an unslipped pluck.** For a release with no slip the
  loop's loss, dispersion and Thiran sections start empty, so the wave's
  share of the onset's force (11-43% of it, by the pluck's place) reaches the
  bridge about 16 samples after the step (0.33 ms on E4), and the first
  arrival carries a whole loop's dispersion. It touches one period, and every
  pluck calibration since 0.4.35 was made with it. Not changed.
- **Block size.** Renders in 64, 17 and 512-sample blocks are bit-identical.
- **The room across sample rates.** The room is the same at 48 and 96 kHz. At
  44.1 and 88.2 kHz its line lengths round to other primes: octave bands of a
  two-note tail differ by up to 4.5 dB in fine structure, with T60 within
  about 10% and the whole level within 0.2 dB. That is a delay network's
  nature, not an error in the model.
- **The top end against the FreePats tenor.** The render stands 9-14 dB over
  the recording at 6-16 kHz over 30-330 ms, and 15-22 dB over in the first
  30 ms. Thumb matches the recording's click measure (-8.9 against -9.3 dB)
  where the nails stand 9.6 dB over it. The recording's technique and
  microphone are unknown
  ([comparison](recording-comparison-2026-10-28.md)), so closing the gap
  would take an invented number. Not changed.
- **Wood and the top's modes.** `AculeleBodyModes` gives Concert's T1 at
  485.0, 453.5, 434.0 and 517.7 Hz for Spruce, Mahogany, Koa and Cedar, with
  A0 within 0.2%. The engine's comment beside the wood factors gave other
  percentages and is corrected (-6.5%, -10.5%, +6.7%); the model is unchanged.
- **Air loss and the strings' arithmetic.** The square-root-of-frequency air
  loss and the string and wood figures were checked against their sources in
  the earlier review and stand.

## Outside the physical model

Acustra 1.9.48 fixed several MIDI behaviours in its Performer. Two of them
were read against Aculele's: CC121 resetting MPE timbre (CC74), and an MPE
zone resize clearing vibrato ownership. Aculele's Performer has the older
behaviour in both. Neither is part of the physical model, and they are not
ported here.

The README demos are 0.4.67's renders; the main build renders them again.
