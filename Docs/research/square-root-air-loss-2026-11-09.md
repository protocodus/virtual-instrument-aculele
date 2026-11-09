# Square-root air loss: evaluated, not shipped

The 2026-11-09 review tested replacing the existing `0.284 * eta_air(f0)`
constant-angle approximation with a fit to the monofilament's actual
square-root air-decay law. The candidate is **not part of the production
change**: it failed the existing construction-change transient contract.

## Derivation

Woodhouse and Lynch-Aird, *Acta Acustica* 105 (2019), 516–529, equations
7, 13 and 14, supply the string material and viscous-air model already used
by `stringAirLossFactor` and `ukuleleStringLoss`. Since
`eta_air(f) = eta_air(f0) / sqrt(f/f0)`, the decay rate is
`sigma(f) = pi * f0 * eta_air(f0) * sqrt(f/f0)`.

The loop gain already sets the fundamental's complete decay. Therefore the
filter must approximate the excess round-trip loss
`Delta L_n = pi * eta_air(f0) * (sqrt(f_n/f0) - 1)`, including the existing
stiff-string partial frequencies. Fitting the absolute loss first, then
compensating its fundamental, fitted the wrong quantity and was rejected.

The existing passive two-pole/two-zero topology can fit that excess without
adding a filter or sample work. With `u = 4 sin²(omega/2)` and

`|H|^-2 = 1 + G(u)`,
`G(u) = (a*u + b*u²) / ((1 + u/u1) * (1 + u/u2))`,

the passivity constraints remain `a >= 0` and `a + 4*b >= 0`. For
`E_n = exp(2*Delta L_n)`, the correct fitting equation is still linear:

`G(u_n) - E_n * G(u_1) = E_n - 1`.

The experiment retained the existing twenty-corner search and spectral
factorization. It rejected any design whose fundamental filter loss exceeded
the physical fundamental air loss. Twelve fit points covered the audible
band through 300 dB/s of combined bending and air loss, followed by four
lightly weighted points through 20 kHz or 0.45 of the sample rate. This band
choice affects approximation accuracy; it does not change the target law.

## Numerical result

A matrix of 1,296 physical construction/rate fixtures covered three shapes,
two string sets, three tunings, four strings, frets 0/9/18, ages 0/1 and
44.1/48/96 kHz. The air-fit error normalized by the total physical excess
loss over partials 2–8 fell from a worst 21.4% to 4.0%; its median worst
partial fell from 7.94% to 0.61%.

An independent check of the production float coefficients covered 144
sections at 8/16/22.05/44.1/48/96 kHz, both string sets, ages 0/1, low G
and A, and frets 0/9/18. All remained stable and passive on the sampled
digital band. The 504 ordinary-rate partial checks had a maximum normalized
excess-loss error of 3.87%, versus 19.87% on the unchanged engine.
Tuning, prepared-design matching and the pluck physics checks passed.

## Why it was rejected

`Aculele.ConstructionChange` requires a Shape change's high-frequency
transient to remain at least 55 dB below the preceding chord, unless it is
within 6 dB of the following steady window. Returning from Tenor to Concert
through the piezo measured 54.7 dB below the chord and a 16.0 dB rise. The
unchanged engine measured 57.7 dB below the chord. The test and its limits
were not changed.

Sample traces placed the strongest transient six samples after the C
string's fractional-delay anchor crossing, consistent with the pickup
filter's latency. Neither intrinsic-loss section was bypassed and the
dispersion-section topology did not switch there. A DC-exact fractional
delay recurrence fixed a separate constant-potential rounding error but did
not resolve this transient. A C1 coefficient ramp over the same existing
30 ms transition also did not resolve it and was discarded.

The existing `delayAnchor` deliberately keeps the second-order Thiran
residual in [1.1, 2.1), where it remains stable. Its documented finite-
frequency phase step at an integer anchor change is about 0.037 radians at
half Nyquist. DC-exact arithmetic cannot remove that transfer-function
step. A changed loss section's phase can place the crossing on a different
part of the remaining wave; this is a plausible mechanism, not an isolated
proof of the entire observed transient. A phase-continuous delay redesign
would be a separate change requiring its own state and tuning evidence.

The original air approximation therefore remains. A future attempt needs a
verified solution for the construction-retune state/phase transition, not a
different acceptance threshold or an output mask. The prototype, numerical
scripts, wave traces and rejected test results are review artifacts under
`/workspace/scratch/aculele-20261109/air-loss-research/` and its parent folder;
they are not production scaffolding or a claimed listening preference.
