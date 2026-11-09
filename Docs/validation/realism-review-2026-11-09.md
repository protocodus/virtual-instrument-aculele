# Physical-model review, 2026-11-09

The review baseline is `8fd6d61` (0.4.54), the current `origin/main` fetched
at the beginning of the session. The environment's older `b7360c3` snapshot
was superseded before a candidate was retained. Its preliminary results are
not used as before/after evidence. The selected changes become 0.4.55.

This review separates physical/numerical corrections from listening choices.
Existing decisions in `Docs/decisions.md`, factory trims, physical hand limits
and transient acceptance gates remain in force. No listener preference or
market-leading realism is claimed by these measurements.

## Selected corrections

### Physical pluck position

The hand chooses a point on the speaking string. Dispersion consumes part of
its round-trip delay, so the written line is shorter than the full physical
period. Its triangle coordinate is therefore
`p_line = p_string * physical_period / written_line_length`.
The old code clamped this transformed coordinate to the hand's physical
0.46 limit again. It also clipped the normal plane's bridgeward 0.044 point
to 0.05. Both moved the emitted kink, increasingly on stiff upper frets.

The hand still spans 0.05–0.46 of the physical string. Only the internal
triangle and reference-grid domains now use the numerical 0.01–0.95 bounds.
`PluckPhysics` recovers the actual written kink from its curvature rather
than reading back the stored coordinate. Across three rates, three shapes,
four strings and representative frets/positions, its worst position error
is 0.00000591 of the string. More than 120 cases cross the old upper bound,
and 30 cross the old lower bound. The old upper bound missed by as much as
0.150 of the string; the old lower bound missed by 0.00260.

This oracle isolates the held triangle with zero release-velocity share and
zero slip. It does not claim to identify a kink after the finite plectrum
release has mixed and smoothed the two wave components. The production
finite-release and reference-grid bounds are corrected consistently.

### Bent release speed

A held transverse force releases two waves at velocity `F/(2Z)`, with
`F = T*y*(1/a + 1/(L-a))` and `c = T/Z`. The bend model stretches the
string's linear density, so its wave-speed and impedance ratios differ:

- `c/c0 = sqrt((T/T0)*(1+strain))`;
- `Z/Z0 = sqrt((T/T0)/(1+strain))`.

The release formerly used the impedance ratio as wave speed. A cached
`(T/T0)/(Z/Z0)` now gives the release its consistent speed without adding
sample work. The unbent branch retains its exact arithmetic. This corrects
force/impedance consistency within the existing bend model; it does not add
a new bent-string geometry. The independent force/impedance check spans
44.1/48/96 kHz, both string sets and −2/0/+2/+6/+12-semitone MPE bends.
Maximum relative error is 1.41e−7, versus 1.78% on the baseline.

### Quiet reference and fret-crown audit

A common offset in the folded travelling potentials is not physical string
displacement: local displacement is their difference. The former
`fretCrownGrazing` clip limited one absolute outgoing bridge potential at
−0.052. It therefore manufactured motion from a silent common offset and
could not represent local clearance at a fret. That clip is removed. Its
public calibration flag remains a documented no-op for source compatibility.

A causal passive local contact was researched with an independent velocity
reference, but moving-contact geometry and state transfer were not proven.
No incomplete replacement is shipped.

The second-order allpass recurrence now pairs cancelling terms:
`y = x2 + a1*(x1-y1) + a2*(x-y2)`.
This is the same transfer function with the same history-update order, but
preserves `H(1)=1` exactly in float. The live fractional delay, its optional
stage handoff, the release warm-up and ordinary/optional dispersion stages
all use the matching form. The baseline changed 20,194 constant samples in
the moving-delay probe, and 33,406 in the dispersion-coefficient probe;
the corrected probes change none. A small AC signal on a ±4 reference
changes by at most 9.18e−7, within input quantization at that offset.
A closed delay/dispersion ring checks the connected constant-field behavior
while crossing integer anchors and enabling/retiring optional stages.
These numerical corrections are not claimed to be bit-identical ordinary
audio or proof of arbitrary time-varying filter passivity. With every other
selected change held fixed, expanded-versus-difference recurrences produce
RMS residuals 91.77 dB below Concert Strum and 97.14 dB below Tenor Picked
Melody through the shipping Performer. Their whole-file level differences
are below 0.000006 dB. These ordinary renders are separate from the extreme
constant-field oracle.

### Cleanup and ownership

The monofilament ukulele has no wound strings: `woundStringCount` was
compile-time zero. Unreachable winding-ridge/squeak synthesis, its unused
longitudinal arrival queue and associated state are removed. The separate
physical `StringAxialForce` model remains. Public ablation fields keep their
layout and documented no-op behavior. Other removed private state had no
readers, including the always-true release-step switch.

A cross-instrument audit prompted a delayed-strike ownership check. Aculele
already stores its countdown on the current physical voice and overwrites
it when a new pitch takes that string. Twelve public-API cases cover an
initially silent/ringing string, old single/strum scheduling, replacements
before/after the old deadline and the old note's eventual key-up. Exactly
one replacement attack occurs at its own deadline; no scheduling production
change was needed.

## Sustain experiment rejected

A direct fit to the viscous-air square-root decay law improved the isolated
loss approximation while keeping the same filter topology. It nevertheless
failed the unchanged construction-retune transient gate: 54.7 dB below the
preceding chord where at least 55 dB is required. The old approximation
remains. The [research record](../research/square-root-air-loss-2026-11-09.md)
contains the derivation, numerical results and rejected transition attempts.
No test threshold, output mask or fitted level table was changed to admit it.

## Validation and range review

Release builds use GCC 14.2, CMake 3.31 and Ninja. Baseline executables and
matching headers are immutable in the working evidence directory
`/workspace/scratch/aculele-20261109/baseline-8fd6d61/`.
The candidate passes all 25 unit suites, including Tuning, PreparedDispersion,
DispersionState, ConstructionChange, ThumbDynamics, StringAxialForce,
BridgeWork, Reset, the fuzz replay and the new pluck invariants. The seven
shipping MIDI demo checks pass. Existing output tables and factory trims
pass the level checks without regeneration.
The native C++17 warning-as-error build passes. A separate Clang 19 Release
build with ASan/UBSan passes NoteChange, ConstructionChange, PluckPhysics
and the 300-input fuzz replay, including leak detection.

The complete factory matrix covers 1,030 ordinary-key renders at velocities
20/60/90/110/127 and 1,064 string/fret renders at 60/110. Each reset note is
held 1.6 seconds in a 2.5-second slot through the production Performer at
48 kHz, 256-sample blocks. K-weighted onset/held/release windows, peak,
brightness, spectral centroid and pitch/resonance estimates are compared
with the baseline. Quiet velocity 1/2 and finite-range checks are recorded
separately; late FFT pitch estimates near the noise floor are not treated as
reliable tuning measurements.

Both final factory matrices have no nonfinite values or velocity reversals.
Median paired onset-level and decay changes are zero at the survey's printed
precision. The comparisons are:

| Absolute paired change | Ordinary keys | Every string/fret |
| --- | ---: | ---: |
| Onset level, 95th percentile | 0.10 dB | 0.27 dB |
| Largest onset-level change | 0.36 dB | 0.99 dB |
| Held decay, 95th percentile | 0.20 dB/s | 0.30 dB/s |
| Largest amplitude-weighted pitch change | 0.04 cents | 0.47 cents |
| Brightness, 95th percentile | 1.67 dB | 4.17 dB |

The largest tab brightness change is +15.13 dB on Soprano D Tuning's
E-string fret 17 at velocity 110: the power share above 2.5 kHz moves from
−47.29 to −32.16 dB. This is a weak high-frequency notch moving with the
corrected contact, not a 15 dB increase of the note's total level. The
largest key brightness change is −9.45 dB on a soft Soprano Thumb MIDI 85,
from −46.92 to −56.37 dB in that same weak spectral band. Median onset level
is unchanged while the intended harmonic notches and initial phase move.

Another 412 factory-key renders cover MIDI velocities 1 and 2. All are
finite and each velocity-2 onset exceeds its velocity-1 counterpart; no
onset is silent. Their early levels span −88.87 to −47.49 dB, with the
baseline's same −79.81 dB median. At this floor the late spectral search can
prefer a sympathetic note (including the baseline's +105-cent B4 result);
that is not evidence of a newly mistuned fundamental.

A separate 864-case stress probe checks all MIDI 0–127 in each tuning,
including exactly silent unsupported engine notes, plus the open/top frets
of every string at 8/16/22.05/48/96 kHz, all three shapes, both string sets
and 0/+12-semitone bends. All cases pass; maximum observed internal state
is 0.510008, versus 0.510008 on the baseline. This is a 0.35-second extreme-
range stability/support probe, separate from the longer factory decay
renders. Performer keyswitches intentionally trigger hand gestures and are
covered by the existing HandGestures suite.

The twelve shape/wood combinations add 672 ordinary-key renders at
velocities 60/110, keeping Concert Strum's other controls fixed. All are
finite and velocity-monotonic. The 95th-percentile onset-level change is
0.13 dB; the largest is −1.53 dB on Soprano Spruce's MIDI 80 at velocity
110. The largest weighted-pitch change is 0.12 cents, and the largest
nonharmonic-peak ratio change is 1.23 dB. This brings the paired keyboard
review to 3,178 renders per engine, in addition to the extreme-range probe.

### A retained body-radiation notch

Concert Strum's MIDI 85→86 drop is real in the model and already present on
the baseline. Both notes use the A string (frets 16/17), with approximately
+2.1/+1.7 cents fundamental error. At velocity 110, early K-weighted level
falls 8.18 dB, but peak falls only 1.56 dB. An isolated body-force-to-mic
response falls 9.79 dB at their fundamental frequencies, while the actual
early body-force RMS changes only 0.37 dB. The output notch is predominantly
the construction's radiation response, not an allocator or string-loss jump.
It is retained rather than flattened with per-note gain. MIDI 74's smaller
level dip likewise follows the force/moment radiation mixture. No claim of
perfectly even keyboard volume is made.

## Profiling and review audio

Profiling is the final implementation stage, after the sound changes and
range checks are frozen. The numerical allpass multiplication reduction is
part of that acoustic/numerical baseline, not a later exact-optimization
claim. Final CPU, exactness and matched A/B evidence will be recorded here.
Generated WAVs and large CSVs remain outside the checkout; no listening
preference is invented from numerical differences.

The acoustic A/B is `/workspace/scratch/aculele-20261109/final-ab/A.wav`
versus `B.wav`, with a separate `key.md` and raw-level/hash metadata. It is
a 6.25-second upper-register Pick sequence through the production Performer
at 48 kHz and 256-sample blocks. The same MIDI, seeds, controls and factory
trim drive both versions. Whole-file RMS is matched; B's adjustment is only
−0.00577 dB and both true peaks are below −9.7 dBFS. The ordinary difference
has an RMS 29.20 dB below A. This is review material, not a chosen sound
direction or a claimed listening verdict.
