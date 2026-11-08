# Piezo sensor, 2026-11-08

The saddle approximation now follows its stated analog model at ordinary
sample rates. This is a correction to discretization, not a new pickup
measurement or a change to the physical component values.

The analog force transfer remains `H = Zk Q / (1 + Zk Q)`, where
`Zk = k/s + c`, `Q = 1/(sM + sum(Z_string)) + G`, `M = 3.8 g`,
`k = M (2 pi 6000)^2`, `c = k/(18 * 2 pi 6000)` and
`G = 0.009 s/kg`. The higher conductance of the ukulele bridge changes the
phase and damping enough that Acustra's four-tap undelayed numerator fit
no longer approximates it closely: at 44.1 kHz its magnitude is wrong by
9.60 dB at 19.845 kHz, and at 48 kHz by 5.87 dB at 20 kHz. The discrepancy
is in the frozen 0.4.47 source as well as the 0.4.48 parent of this change.

The new low-rate design keeps the exact mapped poles and DC gain, and fits
twenty numerator taps to `H(jw) exp(-jw*7/fs)`. The existing 256-point
log-frequency grid, 20 Hz to `min(20 kHz, .45 fs)`, and relative complex
response weighting are unchanged. A fixed-size streaming Givens QR solve
avoids the longer normal equations' squared condition number. The seven
samples are numerical support, not a physical propagation delay. At
88.2 kHz and above the existing five-tap approximation already meets
0.1 dB and 2 degrees through 20 kHz; it is retained with seven explicit
zero taps. Below 16 kHz the existing four-tap FIR is delayed likewise.

`Aculele.PiezoCircuit` compares the implemented filter directly with the
independent analog formula, removing exactly the seven-sample delay.
The load sweep is 0.05, 0.25, 1 and 3 kg/s summed string impedance, at
16, 22.05, 32, 44.1, 48, 64, 80, 88.2, 96 and 192 kHz, with 1,025
frequency points including both endpoints. At 44.1 kHz the full-band
magnitude error is below 0.38 dB and phase error below 0.86 degrees;
at 48 kHz below 0.031 dB and 0.18 degrees. At 96 kHz the retained fit
stays below 0.087 dB and 1.25 degrees, and at 192 kHz below 0.011 dB
and 0.041 degrees. The test also checks unity DC force transmission
and the bounded low-rate FIR.

## Electrical circuit and units

The electrical design is unchanged: the element's charge sensitivity is
`d = Cp * S = 1.45 nF * 0.2 V/N = 290 pC/N`; the element capacitance,
390 pF cable, bootstrapped ESP Project 202 OPA2134 preamp, coupling and
feedback capacitors, 1N4148 protection diodes, loaded output rails and
BLAMP are retained. `Tools/PiezoCircuitReference.py` independently solves
the electrical node equations, finite-bandwidth op-amp model and diodes.
Its input is force on the element; it does not include a second saddle.
The C++ suite bypasses the saddle only for electrical tests, then tests
the actual saddle separately as above.

DC/AC, force-to-charge-to-voltage scaling, clipping harmonics, aliasing,
overload recovery, reset and silence are checked against its fixtures.
Small-signal error is below 0.003 dB; phase error below 0.54 degrees at
44.1–192 kHz. In-band harmonic/THD error is below 0.40 dB for 100 Hz,
1 kHz and 5 kHz inputs at +1, +3, +6 and +12 dB above clipping.
The worst tested alias floor is -45.19 dB for a 3 kHz sine 6 dB above
clipping at 48 kHz (the strong U1A/diode overload is not alias-free).
Overload waveforms at +3/+10/+20 dB differ from the continuous-time
reference by -56.1/-54.9/-44.6 dB RMS relative to that reference;
recovery trajectories remain within the existing 10% circuit-reduction
bound. The reference's self-test cross-checks Radau against BDF and
checks DC, capacitive loading and the generated BLAMP table.

## Timing and limits

All core audio buses now have 15 samples of sensor/transport latency:
seven from the saddle approximation, seven from the preamp BLAMP, and
one explicit transport sample. The microphone pair, mono microphone and
room wait fifteen samples; the pickup's extra sample precedes the split
to Main and the separate Piezo bus. The output-stage delay follows this.
Performer reports the sum plus its optional Gather/Finger Lookahead holds.
Tests require Main in Piezo capture to equal the separate bus sample for
sample, and require Main to be unchanged by requesting that separate
bus or changing block partition.

The final transport sample makes phase-zero half-band decimation integral
at 1x/2x/4x: sensor and converter delays are 15, 39 and 51 host samples,
so a wrapper can pad 36, 12 or 0 samples to 51. The wrapper must use this
new core latency; these files do not update a host adapter.

The mechanics remain an observation approximation: the saddle/element
filter is driven by the rigid bridge solution and does not react back on
the string/body junction. Adding it to that junction without removing
corresponding mass/compliance from the measured mobility would risk
counting the same load twice. The electrical backreaction alone is small:
`d^2*k/Cp = 0.00031324`, about 0.0313% open/short stiffness change or
0.27 cents resonance shift. This is a bound for the assumed element-seat
parameters, not a proof of reciprocal mechanical energy accounting.
The 3.8 g saddle, 6 kHz resonance and sensor electrical parts remain a
guitar-derived construction; no ukulele pickup measurement is claimed.

Listening evidence is frozen outside the checkout in
`review/aculele/piezo-seat/` and `review/aculele/piezo-seat-extreme/`.
Each pair isolates this saddle/timing change against the frozen shipping
engine, through Performer and the production output stage. Native float
signals and whole-file RMS trims are retained. No listening verdict or
push is implied by these tests.
