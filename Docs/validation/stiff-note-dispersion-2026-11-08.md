# Stiff-note dispersion — 2026-11-08

This local review candidate extends the allpass approximation to the same
stiff-string law. It changes no tension, gauge, loss modulus or target partial.
The public baseline is `b7360c301a5c41347e3bde0cfcba6b0abe61e207` (0.4.47).
Listening approval and final serial performance measurements are pending.

## Bounded fit and state

The existing one/two-section fit is retained for flexible strings. A fallback
has the same maximum sixteen three-variable refinements: ten original starts
in the two-section family and six coordinate steps through a third section.
Each retains the best complete design after exact fundamental retuning. The
third section runs only when it improves that error. A failed proposal keeps
the previous best. Its initial decay/pole ratios are solver starting guesses,
not new physical string parameters. Two-section-only coordinate refinement
was investigated first and barely changed the error; extra model capacity was
needed.

A prepared design and a synchronous cold solve run exactly the same search.
The incremental job stores the third-section trial independently of the best
accepted design. Nearby pitch/stiffness changes continue from all the previous
sections; no preparation-readiness or cache-history switch selects the model.
The extra section is after the Thiran read and before the existing second
section. At zero coefficients it is a two-sample delay exchanged with the
waveguide read. Activation is second then third; retirement is third then
second. Repluck state addition, warming, release scaling, reset, reference
Pick tuning and resting-string loading carry all three sections.

## Independent frequency-root check

An off-tree tool compiled unchanged against baseline and candidate gathers
all 1,368 design requests (3 shapes × 2 string sets × 3 tunings × 4 strings ×
19 frets) per rate. For each it constructs the full round-trip complex transfer
from the integer tap, float Thiran/allpass coefficients and loss sections,
then solves its resonance frequencies by bisection around the physical
stiff-string targets. It does not report the optimizer's phase residual as a
frequency error. H2–H12 below 0.42 of the host rate are included; the bridge is
excluded so coupled body frequency shifts are not mistaken for fit error.

| Rate | Largest error before/after, cents | RMS of per-note worst errors before/after, cents | Notes over 10 cents before/after |
| --- | --- | --- | --- |
| 44.1 kHz | 33.448 / 16.343 | 4.073 / 2.346 | 47 / 17 |
| 48 kHz | 33.132 / 16.020 | 4.031 / 2.286 | 48 / 17 |
| 96 kHz | 33.320 / 15.973 | 3.991 / 2.216 | 44 / 14 |

No note worsens at 48 or 96 kHz. Three notes worsen slightly at 44.1 kHz, by
at most 0.104 cents, because six old two-section starts were exchanged for
third-section coordinate steps. This keeps the search budget bounded. The
fundamental error is below 0.005 cents. Each float allpass's magnitude differs
from unity by less than 4e-14 in this double-precision transfer evaluation;
this proves the stationary section is lossless, not that arbitrary parameter
motion conserves energy. Remaining high-partial error is documented rather
than hidden by changing the physical target.

The earlier README estimate of 110 notes over 10 cents predates the current
fluorocarbon C string. The current baseline above is measured afresh.

## Verification and review

`Aculele.PreparedDispersion` verifies bit-identical cold and prepared designs.
`Aculele.DispersionState` exercises real loop histories, insertion/removal,
deferred retirement, warming, tail addition and scaling. Existing construction
change tests check audible transition artifacts. Focused prepared/construction/state checks, the 54-note permanent physical
root regression and C++17 compilation pass. The state handover differs from
its reference by exactly zero in the exercised cases; before the two-sample
flush fix retirement differed by 0.000787854. The complete unit/audio suite and serial
performance gate are run again after the independent piezo change is merged.

The one-mechanism listening fixture is retained outside the repository under
`review/aculele/stiff-note-dispersion`, with shipping A and baseline plus only
this mechanism B. Whole-file RMS matching preserves raw native renders and
separate keys. Listening approval is pending; no by-ear verdict is claimed.
