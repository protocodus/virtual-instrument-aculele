# Axial-source sampling, 2026-11-08

Local candidate 0.4.48 against frozen 0.4.47 (`b7360c3`). No listening
preference has been recorded, and this candidate has not been published.

`designStringAxialForce` chooses a spatial cell width of
`floor(fs / (4 * min(f_longitudinal, 5000)))`. Previously the squared-slope
integral was evaluated only once per cell width in time as well, and its
change spread over the following interval. A squared source contains sum
tones: components above that reduced sampling rate's Nyquist folded before
the output lowpass. Filtering the folded result could not undo that error.

The candidate keeps the spatial quadrature and the existing longitudinal
ring and output filter. It evaluates the integral every host sample and
passes its one-sample difference to the ring. No new fitted constant or
cutoff was introduced. Spatial approximation and host-Nyquist quadratic
alias remain; this is specifically a correction of the additional temporal
decimation, not a complete oversampled nonlinear-string solution. It also
does not change the one-way axial-force/body observation route.

## Independent regression checks

`Aculele.StringAxialForce` injects a travelling sinusoid forming a standing
transverse shape. At 6 kHz its squared slope contains only DC and 12 kHz.
The check reads the source before the longitudinal ring or output filter.
The old reduced-rate alias relative to the correct sum tone was +1.21 dB
at 44.1 kHz, +8.73 dB at 48 kHz, +9.34 dB at 96 kHz and +9.49 dB at
192 kHz. The candidate's corresponding line is below the -75 dB test
bound at every rate. The new test fails the unchanged baseline at all four
rates. Existing slow-limit, longitudinal-frequency, loss and retirement
checks pass without changing their tolerances.

`Aculele.BridgeWork` adds fixed-body cumulative-work and transverse force
balance checks for all three shapes at 44.1, 48 and 96 kHz, plus an
independent reconstruction of the port-work observer across initial,
released and repeated contacts. Work includes heave and normalized rocking.
These are normalized mechanical port checks, not a complete string/body
storage ledger or an acoustic-power measurement; microphone RMS and the
output limiter do not prove passivity. The suite passes.

## Listening and higher-rate evidence

The review package outside the repository contains identical eight-second
hard Pick performances through the production Performer, native floats,
the renderer, score and parameter provenance, hashes, and A/B WAVs. A is
the frozen baseline. B is matched to A's whole-file stereo RMS with a
-0.022321 dB trim; native levels are retained separately. No demo phrase or
committed MIDI was changed.

The candidate also renders that score at 96 and 192 kHz, then resamples to
48 kHz for comparison. Below 5 kHz the isolated axial microphone output's
band level differs by -0.057 and -0.048 dB respectively; from 5 to 10 kHz
the 48 kHz version is 2.14 and 2.81 dB higher. The unaligned isolated-output
waveform residuals are -7.83 and -5.13 dB relative to those references.
Different host rates change string, bridge and radiation discretization,
onset and modal phase as well as spatial quadrature, so those residuals
must not be described as isolated alias power. They do not establish full
broadband convergence to the higher-rate physical reference.

Validation uses a Release GCC 14.2 build. All 20 unit/audio tests pass, including
the new source and bridge-work checks, and the C++17 compatibility target
builds with warnings treated as errors. The serial CPU gate remains pending
until the coordinated quiet window; its result is recorded with the review
manifest before promotion.
