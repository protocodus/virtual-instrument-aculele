# Live piezo saddle load, 2026-11-08

The pickup's existing analog seat model contains the summed normal string
impedance `Z`: `H(s,Z) = Zk Q / (1 + Zk Q)`, with `Zk = k/s + c` and
`Q = 1/(sM + Z) + G`. Previously the discrete filter was designed only in
prepare/reset, using Standard notes even for another tuning. Shape, string
material, tuning, master tune, MPE tension bends and retained tails changed
the actual junction ports without changing this sensor parameter.

The selected load now equals `drive.impedance0` after the junction's held-load
fallback. It is the unweighted normal-port sum, including the retained tail
branches that already participate in that junction. Per-string pickup
sensitivity weights belong to the measured force and do not change this
mechanical load. The parallel polarization contributes to rocking inertia,
not a second normal port. The measured force, reciprocal junction solution,
axial source, electrical circuit, and all bus delays are unchanged.

## Prepared domain and interpolation

Each port has `Z0 = mu * 2 L f`. Enumerating the supported three Shapes,
two string sets and three tunings gives per-string maxima 0.19785199,
0.19585437, 0.19700821 and 0.19521981 N s/m. These occur in different
constructions; summing the individual maxima covers mixed retained tails.
There is at most one main and one tail branch per string. The existing bend
law bounds the physical impedance multiplier by `sqrt(2)` after accounting
for stretched linear mass, and master tuning is bounded to +100 cents.
Thus the conservative upper bound is
`2 * sqrt(2) * 2^(100/1200) * 0.78593438 = 2.3551421 N s/m`.
The storage domain is rounded outward to 2.356 N s/m for floating-point
rebase/follower/sum error. That numerical envelope is not a new physical
parameter. A retune can make an individual applied scale exceed `sqrt(2)`;
it is the product of base impedance and applied scale that is bounded.

Prepare builds 65 filters, 32 intervals below and 32 above the exact default
Concert/Nylon/Standard load, 0.55709017 N s/m. The bank occupies 5,720 bytes
of coefficients. Its rate-specific saddle designs are unchanged. Every
changed live load selects two neighboring filters and linearly interpolates
their twenty numerator and two denominator coefficients. An unchanged load
is cached. No QR solve, allocation or history clearing occurs in reset,
parameter changes or processing. The filter follows the junction's existing
two-stage 6 ms impedance follower; it adds no independent smoothing lag.

The real second-order Schur inequalities `1 +/- a1 + a2 > 0` and
`1 - a2 > 0` describe a convex region, so interpolating stable denominators
preserves stability. Unity DC transmission is a linear coefficient identity
and is preserved up to float rounding. This does not prove arbitrary
time-varying filter passivity or close the instrument's mechanical energy
ledger: the saddle remains a one-way observation approximation, as described
in [the sensor validation](piezo-sensor-2026-11-08.md).

## Checks

`Aculele.PiezoLoad` sweeps every grid point and quarter interval at 8, 16,
44.1, 48, 96, 192, 384 and 768 kHz, with 257 logarithmic frequencies from
20 Hz through `min(20 kHz, .45 fs)`. The extended 768 kHz bank is exercised
independently of the default build's delay-storage cap. It checks the
independent analog component relation, direct-design interpolation error,
strict pole stability, DC precision and the exact default anchor.

The largest interpolation discrepancy is 0.000467 dB / 0.00265 degrees,
at 768 kHz. The analog error remains below 0.377 dB / 0.85 degrees at
44.1 kHz, 0.031 dB / 0.16 degrees at 48 kHz, and 0.087 dB / 1.26 degrees
at 96 kHz. The intentionally approximate 8 kHz FIR retains its existing
behavior (2.41 dB / 8.76 degrees over that limited band).

At up to 192 kHz the existing relative DC tolerance of 1e-5 holds. At 384
and 768 kHz, cancellation in the stored float denominator amplifies the
original direct fit's coefficient roundoff; maximum relative DC errors are
1.74e-5 and 7.03e-5. The test bounds the absolute DC coefficient residual
by twice float epsilon times the coefficient norm before that cancellation.
No circuit parameter or physical gain is adjusted to hide this precision
limit.

Live tests require the selected load to equal the actual held junction load
sample for sample. They cover all eighteen construction/tuning combinations,
master tuning, manager slides as a negative control, member tension bends
and their six-semitone tension cap, a different-pitch refret's retained tail
and retirement, and the held-load fallback. Settled live/cold comparisons
disable the independent fretting-pitch draw and allow only the existing
two float followers' quantization residue. Histories survive coefficient
changes, invalid loads cannot enter float-to-index conversion, and replacing
only the sensor bank leaves bridge states and stereo microphones bit-identical.

The isolated listening pair lives outside the checkout at
`review/aculele/piezo-live-load/`. A is the frozen shipping engine; B adds
only this live-load bank to that same source, retaining its original saddle
approximation and latency. Both use identical production Performer MIDI,
live controls, seeds, rate and blocks, with whole-file stereo RMS matching.
Raw float audio, element force, circuit voltages, junction impedance and
filter poles are retained. No listening verdict or push is implied.
