# Output levels and low-rate stability, 2026-11-08

The existing factory calibration remains authoritative: event-median louder-channel 300 ms RMS, with 0 VU = −12 dBFS RMS and normal playing near −9 VU. No preset Output or `LevelMatchData.h` value changed. At 48 kHz the seven factory normals remain −8.995 to −8.968 VU; the existing hard-chord gate's largest result is −1.72 VU. Dense artificial maximum-control stress can read above 0 VU without digital overload; that diagnostic did not justify stronger VU compression.

## Output-only reserve

A sampled ceiling does not bound the reconstructed waveform. Native 8/16/22.05 kHz and Reason 22.05 kHz at 1× exposed intersample overload despite the existing −1.5 dBFS sampled ceiling. The final engine reserves 3.3 dB below 44.1 kHz, making the effective sampled ceiling −4.8 dBFS. Rates of 44.1 kHz and above keep the exact old path.

Let `g = 10^(3.3/20)`. The engine multiplies each output-stage input by `g`, raises the preceding compressor's threshold by 3.3 dB, then multiplies its output by `1/g`. The detector power and threshold power both increase by `g²`, preserving the compressor gain trajectory and quiet gain to float precision. Only the limiter reaches its threshold earlier. The common `OutputStage.h`, physical forces, pickup volts and nonlinear circuit headroom are unchanged. No delay is added. Main stereo linkage and the independent direct-pickup stage are preserved.

`OutputCeilingTests` compares the real helper with a separately prepared unchanged stage while compression is active and the limiter is inactive. A deliberately omitted threshold shift fails the fixture. Separate loud-signal tests check the effective ceiling, exact Main/Piezo parity, reset and unchanged quiet-impulse latency.

The initial 2.5 dB candidate was rejected as too close to full scale: an expanded 8 kHz all-preset survey reached −0.0558 dBTP with a long reconstruction kernel. The adopted 3.3 dB reserve adds margin to that measured case. This is an output-headroom choice, not a fitted physical parameter.

## Stable observation contour

The authored radiation shelves are 1414.2, 2828.4 and 5656.9 Hz. At 8 kHz the last shelf exceeds Nyquist; inserting that frequency into the RBJ equations makes `sin(omega)` negative. Its poles had radii 2.323–2.355 across the three body shapes. Microphone and room histories overflowed, and even pure-Piezo Main could become silent through a zero-weight NaN while the separate pickup continued.

Each observation-filter corner is now bounded by `min(authoredHz, 0.45*sampleRate)`. This retains the shelf direction, DC gain and Nyquist gain while providing a stable, explicitly bandwidth-limited approximation. It is not a new measurement of body radiation or a change to mechanical body modes. All authored sections remain unchanged from 16 kHz upward. The 0.45-rate boundary avoids the numerically degenerate exactly-Nyquist design.

`LowRateCaptureTests` checks actual configured coefficients for all three shapes at 8, 11.025, 12, 16, 44.1, 48 and 96 kHz using independent Jury inequalities and denominator roots. It also checks shelf endpoints, half-dB corners, monotonic cut response, impulse energy/decay and reset. Sustained hard retriggers at 8 kHz inspect internal capture, microphone-delay and room histories and require nonzero exact Main/direct-Piezo parity. Checking only sanitized final output would miss the original fault.

## Measurement and limits

Production Performer audio is preserved as raw three-bus float samples. Reason cases are measured after its actual converter and final host guard. The primary independent reconstruction uses an explicit unity-DC Kaiser(8.6) sinc at 8×, with 128 host samples of support on each side. Worst cases repeat at 16×/32× and through 2048 host samples of support. Real silent pre-roll is retained or explicitly prepended; scored samples exclude filter boundaries. Kernel hashes and original-grid error are recorded. Padded FFT interpolation supplies a second numerical cross-check.

The final low-rate survey contains 49 cases: three representative profiles at native 8/16/22.05/32 kHz and Reason 22.05/32 kHz at 1×/2×/4×, plus all seven presets and three captures under maximum controls at 8 kHz. The hardest 8 kHz case reaches −0.859584 dBTP with 32×/2048-sample FIR support and −0.897987 dBTP with padded FFT interpolation. A second near-worst case reaches −1.091919 and −1.084829 dBTP respectively. These are finite musical-stress measurements, not a certified BS.1770 meter or an arbitrary-signal true-peak guarantee.

The unchanged 44.1/48 kHz survey contains 147 cases and reaches −0.507924 dBTP after long-kernel refinement. Its maximum host sample is −1.5 dBFS after Reason's separate final ceiling-only guard. That wrapper guard follows oversampling conversion, uses no second compressor, and adds its existing 2 ms output-stage delay. The core reserve adds none. A 192 kHz/4× production extended-rate spot remains below full scale. Raw evidence, convergence results, exact high-rate comparisons and separately RMS-matched shipping A/Bs are retained in the local review package.

## Final checks

All 25 core unit/audio checks pass: 23 registered unit tests, AudioDemos and the complete Levels check. The two audio commands used one worker. The C++17 build with warnings as errors passes. The final Levels report is byte-identical to 0.4.52. All 23 selected production high-rate rerenders are byte-identical on all three buses, covering every preset/capture at 44.1 kHz and additional 48 kHz/2× and 44.1 kHz/4× cases.

An off-tree coefficient sweep also checks the existing axial-ring passivity guard, independently of final output limiting: all 27,360 combinations across ten rates from 8 to 192 kHz, three shapes, two materials, three tunings, both age endpoints, four strings and nineteen fret positions satisfy `abs(a_float)+abs(b_float)<1`. The largest stored-filter loop gain is 0.999960. Aculele already applied the all-frequency loss bound introduced in 0.4.43; this audit required no axial-loss change.
