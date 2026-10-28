# Third-party notices

Aculele's original source code is covered by the project's `LICENSE` file.
The adapted measurement data and derived values below are not relicensed by
Aculele.

Aculele's engine is forked from Acustra 1.9.0
(<https://github.com/protocodus/virtual-instrument-acustra>, commit
`0e20192`), and its measured tables came with it unchanged. The entries for
them are Acustra's; `Tools/` and `Docs/` paths in them name files in Acustra's
repository at that commit, where the tables were made.

## Robert Mores guitar-measurement archive

Creator: Robert Mores, Hamburg University of Applied Sciences

Title: *Archive for the acoustical documentation of classical Spanish
guitars, flamenco guitars and romantic guitars from private and public
collections – bridge mobility* (2021)

Source: <https://doi.org/10.5281/zenodo.4604577>

Licence: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/)

`Source/DSP/MeasuredBodyData.h` is adapted from the archive's
`qualified_selected_impulses.mat` (MD5
`733cb10baf5ce36d8bf333610ffbb260`). Acustra selects g21, the archive's 2018
Lester DeVoe flamenca blanca with spruce top and cypress back and sides. It
uses the guitar's first and third one-second segments,
containing bass-side and treble-side normal bridge impacts. After applying the
archive's full-record half-cosine taper and SI calibration scales, it forms
complex H1 force-to-pressure responses to the upper, treble-side and bass-side
microphones, each 10 cm above the top plate. Their common time origin and
relative phase are retained.

`Source/DSP/StudioCaptureData.h` adapts four complex pressure modes from
these fitted tables and `MeasuredJointBodyData.h` into constrained and free
plate/hole source strengths. `Tools/GenerateStudioCapture.py` records the
source hashes and finite-radiator geometry assumptions. Its projected neck
and off-hole observations are derived models, not additional measurements
from the archive. This adaptation retains the CC BY 4.0 attribution above.

The archive's physical-measures table identifies g21's strings as Savarez
Tomatito, with nylon/KF trebles and wound multifilament basses. Acustra adapts
this flamenco measurement for its steel strings; it is not a measured
steel-strung body. Both `MeasuredBodyData.h` and
`MeasuredBridgeData.h` derive from these records; the bridge adaptation fits
positive-semidefinite heave/rocking residues to the calibrated bass/treble
acceleration-to-force measurements.

For each g21 microphone/impact path, Acustra inverse-transforms the H1 response,
retains 3000 samples, leaves the first 2700 unchanged and applies an authored
300-sample raised-cosine fade. These common causal windows preserve inter-path phase; no independent
minimum-phase reconstruction or delay alignment is applied.

Acustra jointly selects shared frequencies and Q values over 80 Hz–10 kHz
from all six paths and fits regularised complex endpoint residues.
The smallest passing prominence-ordered prefix has 127 poles for g21, with frequencies, Q values and residue components rounded to float32.
For bass/treble forces `Fb`/`Ft`, the transformed inputs are `F = Fb + Ft` and
`T = Ft - Fb = M/a`, where `a` is the assumed impact half-spacing. The pressure
paths are `(Ht + Hb)/2` and `(Ht - Hb)/2`, respectively. Separate force/moment
states drive all three microphone outputs. This basis reconstructs the two
measured inputs; extending it to arbitrary strings assumes a rigid saddle and
does not identify a horizontal-force response.

Apart from the plate-mode Q below, no corpus averaging, FIR remainder, per-path
peak/RMS normalisation or authored
left/right gain spread is used. The causal fades, modal reduction, force-pair
basis conversion, global gain and shape/material transformations are Acustra
changes; the selected body calibration has neutral frequency/Q scales, residue
tilt and low-mode gain. Runtime output has no RMS matching and is not calibrated
to absolute sound pressure. No DAFx-26 coefficient is used in this body model.

Since 2026-09-27 the g21 bank's plate modes carry the damping of five flamenca
blancas the same archive measured anechoically, g37, g38, g39, g42 and g43,
each fitted by the same generator and gates at its own converged window (g41
fails them): each g21 mode's Q is scaled by the population's median Q over
g21's own median in the octave round it, and never raised
(`Tools/GenerateBodyForcePair.py --plate-q median`). Until 2026-09-27 the rule
covered 300 Hz-10 kHz; since 2026-09-28 it reaches down to 150 Hz, so T1 (178.5
Hz), the 208.7 Hz rocking mode, the 229.0 Hz mode and the 287 Hz broad mode are
damped by it too. The air group below 150 Hz and every frequency and residue
are as fitted from g21. This Q scaling is an Acustra change to the archive's
data.

Since 2026-09-28 steel's own bridge, the g21 bridge bank, plays on its
radiation bank's poles (since 2026-09-29 it is the Original model's only
bridge):
`MeasuredBridgeData.h` records, for each of its 47 modes, the radiation mode
that is the same resonance (18 of them: the nearest, inside that mode's
as-fitted half-power band, the bridge mode narrower than the radiation bank's
local spacing), and the engine gives such a mode that radiation mode's
frequency and Q after the anchor, Shape and Wood transforms and the plate-Q
rule above; the other modes keep their measured frequency under the same
transforms and take the plate-Q rule's octave factor on their Q, inside the
same 150 Hz-10 kHz band. Every bridge residue is scaled by 0.274
(`steelTopMobilityRatio`), the Fylde Falstaff's measured mobility over g21's
(below), so the flamenca's bridge
drains at a steel-string guitar's level. These are Acustra changes to the
archive's data (`Tools/GenerateMeasuredBridge.py`).

In the steel blend (`Source/DSP/SteelBodyBlend.h`) two further
transformations of the same archive ship. (1) The four g21 modes between 150
and 300 Hz take their Q 0.85 of the way, in log Q, from the as-fitted value
to the plate-Q rule's (`GenerateBodyForcePair.py --plate-q-t1-weight`). (2) A
jointly fitted body, `MeasuredJointBodyData.h` (`Tools/GenerateJointBody.py`):
131 frequency/Q pairs fitted jointly to g21's six microphone paths and its
four accelerometer paths (velocity/force over the whole record, the hammer
differentiated, a 2-sample alignment), refined by variable projection below
the bridge's 2245 Hz cross-side corner, 56 modes carrying
positive-semidefinite heave/rocking mobility residues, the plate-Q rule from
300 Hz on the common Q, and the mobility scaled by 0.32275
(`steelJointTopMobilityRatio`), the RMS ratio of
the Fylde Falstaff's measured mobility (Carcagno et al., below) to g21's at
the bass impact over 80 Hz-4 kHz. The header carries all 131 modes; the
engine plays the 9 below 550 Hz, radiation and bridge, at 0.075 in parallel
with g21's modes, which play at 0.925 below 550 Hz and at 0.985 (radiation)
and 1 (bridge) above it. The weights, the band, these fits and the level
ratio are Acustra changes to the archive's data.

Aculele, like Acustra, ships only those transformed numerical coefficients. It does not ship
the source MAT file, recorded impulses, photographs or documentation from the
archive. The attribution, source link, licence link and description of changes
above must accompany distributions containing the coefficient table.

The archive's other records are no longer shipped: g35 (a 1978 classical, which
the Classical 78 Model played with steel strings until 2026-10-10) was
removed with that Model; g34 (a 1971 Manuel Contreras classical, which the nylon
strings played until 2026-09-29) and the unused g36 candidate were removed with
the nylon model.
g37, g38, g39, g42 and g43 contribute only the plate-Q statistic above; none
of their coefficients ship.

Aculele plays these banks through Acustra's coupled A0/T1 Shape morph, which
scales a body by the equal-thickness plate law (f proportional to 1/A_p) to
the soprano, concert and tenor boxes of Kala's published dimensions
(`ukuleleBodies` in `Source/DSP/AculeleEngine.cpp`), at steel's bridge level
(`ukuleleTopMobilityRatio`, equal to `steelTopMobilityRatio` below), with the
13.3 mm saddle spacing setting the bridge lever arms. These are Aculele
changes to the archive's data.

## Measured Fylde steel-string bridge: derived constants only

Samuele Carcagno, Roger Bucknall, Jim Woodhouse, Claudia Fritz and Christopher
J. Plack, *Effect of back wood choice on the perceived quality of steel-string
acoustic guitars*, JASA 144(6), 3533–3547 (2018),
<https://doi.org/10.1121/1.5084735>.

Source data: <https://osf.io/f4pqa/>,
`guitar_back_wood_code_data_v1.0.1.zip`,
SHA-256 `1d35dd28ece660eadc165be34995255fae6f56c9cb3d8d6d96bd00fe2902e582`.

Licence: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/).
The dataset's licence is supplied in its `LICENCE.txt` and OSF record.

Acustra uses the first `specSet` column in `bridge_admittance_all.mat`: the
commissioned Fylde Falstaff with Sitka spruce top, Brazilian rosewood back
and sides, ebony bridge and Elixir Nanoweb Light 80/20 Bronze steel strings.
The manufacture year is unspecified. The measurement is normal bridge
velocity/force between the fifth and sixth strings, with strings damped.

No bank of this measurement ships. Until 2026-09-29 Acustra shipped a 44-mode
modal fit of it (`MeasuredSteelBridgeData.h`) as an optional bridge; that
choice and its coefficients were removed. The measurement still underlies
three derived constants, fitted by `Tools/FyldeBridgeReference.py` (which
selects measured peaks, pins the three lowest body frequencies and Q values
to Table I, infers a polarity and phase alignment from a constrained fit, and
fits nonnegative scalar residues while retaining measured SI magnitude):
`steelTopMobilityRatio` in `MeasuredBridgeData.h`, the geometric mean over
80 Hz–4 kHz of the fit's |Y| over the Mores g21 bridge bank's at the matching
(bass-side) position, 0.274, which scales the flamenca's bridge to a
steel-string guitar's level; `steelJointTopMobilityRatio` in
`MeasuredJointBodyData.h`, the corresponding RMS ratio for the jointly fitted
body, 0.32275; and the piezo's bridge conductance (`PiezoDesign` in
`AculeleEngine.h`), below. The source MAT, recorded/synthesized audio and
experimental participant data are not distributed. This attribution, licence
link, source link and description of changes must accompany distributions of
these derived values.

## GuitarSet: derived values only

Qingyang Xi, Rachel M. Bittner, Johan Pauwels, Xuzhou Ye and Juan P. Bello,
*GuitarSet: A Dataset for Guitar Transcription*, ISMIR 2018. Dataset:
GuitarSet v1.1.0, <https://zenodo.org/records/3371780>.

Licence: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/)

No GuitarSet audio or annotation is included in this repository or any
Aculele binary. Values Acustra fitted to it are compiled into Aculele
too:

- The capture voicing, the six filter sections in
  `Source/DSP/CaptureVoicingData.h`, written by `Tools/FitCaptureVoicing.py`.
  It is fitted, in third octaves, to the consensus of five sets of
  steel-string recordings against the engine's renders of the same notes;
  one set is GuitarSet's six microphone performances (`audio_mono-mic`,
  one 12-second excerpt per player, rendered from their annotations by
  `Tools/BenchmarkPerformances.py`).
- Three strum statistics that `Tools/MeasureStrums.py` measures from the
  comping tracks' hex-pickup audio (`audio_hex-pickup_debleeded`) and
  their note annotations: the pick's traversal speed, 0.51 to 2.46 m/s
  (`AculeleEngine::strumDelaySamples` in `Source/DSP/AculeleEngine.cpp`);
  a stroke's span varying with a 36.47 % standard deviation
  (`strumSpeedJitterHalfWidth` in `Source/DSP/AculeleEngine.h`); and a
  repeated string's level varying with a 4.47 dB standard deviation (the
  7.74 dB half-width in `AculeleEngine::initialisePluck`).
- An upstroke's attack against a downstroke's on the same string, which
  `Tools/MeasureStrumDirection.py` measures from the same hex-pickup audio
  and annotations: 0.9 dB more per string toward the high E and 1.2 dB
  under the downstroke between the D and G strings (`upstrokeTiltDbPerString`
  and `upstrokeMidNeckDb` in `AculeleEngine::initialisePluck`).
- How fingers time a chord they pluck together, which
  `Tools/MeasureFingerChords.py` measures from the bossa nova comping's
  hex-pickup audio and annotations: each string's median lag behind the
  chord's highest string (0, 2.27 and 4.85 ms for three strings; 0, 2.36,
  6.98 and 3.06 ms for four) and a 4.47 ms standard deviation about it
  (`AculeleEngine::fingerChordOffsetSamples`).
- What a strum does to a string it crosses that the chord leaves out, which
  `Tools/MeasureMutedStrokes.py` measures from the comping's hex-pickup
  audio and annotations: a strike 4.6 dB under the string's own played
  attacks and a 20 dB fall in 13.8 to 14.8 periods, taken as a 43-period
  T60 (`AculeleEngine::muteStroke` and `beginMutedStroke`).
- How far a released fretted note's pitch falls as its level drops, which
  `Tools/MeasureReleaseFall.py` measures from the comping's pitch contours
  and hex-pickup audio: the median fall at 1.5, 4.5, 8, 12.5, 17.5, 22.5,
  27.5 and 35 dB under the sustain, 0.4 to 80.5 cents
  (`releaseFallCurve` in `Source/DSP/AculeleEngine.cpp`).
- How far each string of a picked stroke sounds from the stroke's own
  sweep, which `Tools/MeasureStrumScatter.py` measures from the comping's
  hex-pickup audio and annotations: a robust 4.53 ms, of which 4.35 ms is
  taken as each string's own moment (`AculeleEngine::strumScatterSamples`).
- How far apart a pick sounds the strings of a two-note group, which
  `Tools/MeasureTwoStringStrokes.py` measures from the comping's hex-pickup
  audio and annotations: 5% of adjacent pairs within 2 ms, a median 13.0 ms
  apart (`Performer::flushNoteGroup`).
- How a picked stroke's time a string falls with the strings it crosses,
  which `Tools/MeasureStrokeSpan.py` measures within the comping's takes
  from their hex-pickup audio and annotations: the spacings crossed to the
  -0.36 power once each string's scatter is allowed for
  (`AculeleEngine::strumDelaySamples`).
- How hard a pick strikes each string of a stroke against the rest of the
  same stroke, which `Tools/MeasureStrokeEdges.py` measures from the
  comping's hex-pickup audio and annotations: the entry and exit strings
  1.8 and 2.0 dB under the strings inside, and an upstroke tilt of 0.30 dB
  a string (`AculeleEngine::strumEdgeGain`, `initialisePluck`).

These are Acustra's measurements and fits from the dataset, not part of
it; the source comments and tools above describe how each was derived. This
attribution, licence link, source link and description of changes must
accompany distributions of these derived values.

The capture voicing's other recordings are CC0 and are listed for
provenance only: the Eastman E1D takes above (both whole takes,
`Tools/PrepareEastmanCorpus.py`), the reference bank's eight finger-plucked
Eastman notes, and 11 notes of Jeff Learman's 2017 Martin HD28 samples
(program 026 of the Discord SFZ GM Bank,
<https://github.com/sfzinstruments/Discord-SFZ-GM-Bank>, commit
`7a9c478fe331f94f246d33332f0adedb25bbbe27`, `Tools/PrepareMartinCorpus.py`).

`CaptureVoicingData.h` ships in the source, but Aculele's signal path leaves
the capture voicing out (`guitarCaptureContour` in
`Source/DSP/AculeleEngine.cpp`) unless it is built with
`ACULELE_GUITAR_CAPTURE_CONTOUR=1`. The strum statistics apply to the
ukulele's own 13.3 mm string spacing.

## DAFx-26 nonlinear modal synthesis paper: no licensed material

Michele Ducceschi, Riccardo Russo and Craig J. Webb, *Measurement-Informed
Nonlinear Modal Synthesis of 65 Classical Guitars*, Proceedings of the 29th
International Conference on Digital Audio Effects (DAFx-26), 2026,
<https://dafx26.mit.edu/assets/papers/DAFx26_paper_40.pdf>, CC BY 4.0.

Until 2026-09-29 `Source/DSP/AculeleEngine.cpp` reproduced the paper's Table 1
(the EJ45 nylon strings' diameters, effective densities and Young's moduli)
for Acustra's nylon strings. That table was removed with the nylon strings,
and no coefficient table, figure, text or audio from the paper is included
now. The engine still cites the paper for methods and single published
values: its fixed-end attachment (the 3.25 mm stub that its xi_b = 0.995
leaves on a 650 mm scale) and its longitudinal tension-increase expression.
The paper's measurement-to-modal procedure also informs the Mores-data
adaptation described above.

## Published figures behind the piezo chain: no licensed material

The under-saddle piezo chain (`AculeleEngine::PiezoDesign` in
`Source/DSP/AculeleEngine.h`, `Tools/PiezoReference.py`, Docs/decisions.md
2026-09-29) takes individual published values: part values, datasheet
limits and single measured figures. None comes from a dataset, table or file
under a licence, and no figure, schematic, table, text or recording from any
of these sources is reproduced or distributed; each value is cited beside it
in the source.

- Rod Elliott, Elliott Sound Products, "Project 202 - Piezo Preamps", Fig. 1,
  <https://sound-au.com/project202.htm>: the preamp circuit's topology and
  its resistor and capacitor values, read off the published figure. The
  circuit is modelled from those values; the figure itself is not copied.
- Texas Instruments, OPAx134 datasheet (SBOS058B),
  <https://www.ti.com/lit/ds/symlink/opa2134.pdf>: the OPA2134's open-loop
  gain, gain-bandwidth, slew rate, input capacitance, bias current, input
  common-mode range and output swing.
- The 1N4148's saturation current and emission coefficient (2.52 nA, 1.752)
  and 4 pF junction capacitance, from the widely reposted SPICE model.
- Manfred Zollner, *Physics of the Electric Guitar* (2005), chapter 6,
  <https://www.gitec-forum-eng.de/wp-content/uploads/2019/03/poteg-6-piezo-pickups.pdf>:
  a bridge piezo's sensitivity of about 0.2 V/N at 1.45 nF (the Ovation
  EA-68), its 1-2 V output when played loudly (section 6.7), which bounds
  `Tools/CalibratePiezo.py`'s check, and the measurement rig's Q of 18, taken
  as the bound on the element's loss. Figs 6.24/6.25 are cited as the reason
  loudness, not RMS, is matched; nothing is read off them.
- Radial Engineering, PZ-DI product page: the DI's 1 MOhm input setting.
- Mogami 2524 cable's capacitance (130 pF/m), from the manufacturer's
  published specification.
- Martin Keith, "Troubleshooting imbalance between strings on common acoustic
  guitar pickup systems", *Acoustic Guitar*, and US patent 6,822,156 B1: the
  string-to-string imbalance of an installed under-saddle pickup, which bounds
  the per-string weights (their pattern is this project's own choice).
- F. Esqueda, S. Bilbao and V. Valimaki, "Aliasing reduction in clipped
  signals", IEEE Transactions on Signal Processing 64(20), 2016: the
  band-limited ramp (BLAMP) method the clip's corners are corrected with. The engine's residual is its
  own Kaiser-windowed sinc, tabulated by `Tools/PiezoReference.py`; no code
  or table from the paper is included.

The bridge conductance under the saddle (1.59e-3 s/kg) is the mean of Re(Y)
over 5-7 kHz of the measured Fylde mobility (see its entry above); no new
data is added for it.

## fdlibm `scalbnf`: Sun Microsystems notice

Built with the Reason Rack Extension's Jukebox SDK only, the engine's
`std::complex<float>` division scales by `sdkScalbn()` in
`Source/DSP/AculeleEngine.cpp`, a transcription, bit for bit, of `scalbnf`
from newlib's `libm/common/sf_scalbn.c` (Ian Lance Taylor of Cygnus Support
converted it to float from `s_scalbn.c`), which the SDK's libc links. A
desktop build does not compile it. That file's notice, which it asks to be
preserved:

```
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
```

## Ukulele strings and bodies: published values only

The ukulele's strings and boxes take individual published values. None comes
from a dataset, table or file under a licence, and no figure, table, text or
recording from these sources is reproduced; each value is cited beside it in
`Source/DSP/AculeleEngine.cpp`.

- Jim Woodhouse and Nicolas Lynch-Aird, "Choosing strings for plucked musical
  instruments", *Acta Acustica united with Acustica* 105 (2019) 516-529: the
  stress-dependent storage and loss moduli of nylon and fluorocarbon
  monofilaments, and the air-damping law (`ukuleleStringLoss`).
- D'Addario EJ65S and EJ65C and Worth CM, CM-LG, CT and CT-LG string sets:
  their published gauges.
- Kala's published body and scale dimensions for its soprano, concert and
  tenor ukuleles (`ukuleleBodies`, `scaleLengthFor`).

## Labraña–Espinoza ukulele radiativity: one derived value

Creators: Sebastián Labraña, Ayelen Gajardo, Víctor Correa and Carolina
Espinoza, Universidad de Chile

Title: *Data and code for: Vibroacoustic reproducibility of 3D-printed PLA+
and wood soundboards for ukuleles* (2026)

Source: <https://doi.org/10.5281/zenodo.22692775>

Licence: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/)
for the data (MIT for its code, none of which is used)

Aculele derives one value from the deposit's `data/mean_rad_hybrid_XZ.txt`
and `data/mean_rad_hybrid_YZ.txt`, the soprano with the spruce top:
its radiation's third-octave slope from 1 to 6.3 kHz, -3.204 dB per octave.
`Source/DSP/AculeleEngine.cpp` (`radiationTiltShelfDb`) and
`Tools/AculeleRadiation.cpp` hold the body to it. The way the value is
derived is given at the head of that tool. No file, figure or table from the
deposit is reproduced.

## Reference ukulele recording: not distributed

"Ukulele Arpeggio" by David Baker, Duke Sonic Dictionary, licensed CC BY-NC
4.0 (<https://creativecommons.org/licenses/by-nc/4.0/>), was compared against
Aculele's renders of the same notes when the fork was evaluated. Neither the
recording nor anything fitted from it is included in this repository, and it
is not used in promotion.

FreePats' "Ukulele" set of 2026-08-11, by Mateusz Dąbrowski, CC0 1.0
(<https://freepats.zenvoid.org/GuitarFamily/ukulele.html>), was compared
against Aculele's renders by `Tools/compare_recorded_notes.py`. It is not
included in this repository, and nothing is fitted from it.

## Trademarks

Kala, D'Addario, Worth and Martin are trademarks of their owners. They are
named only to identify the instruments and strings whose published figures
the model uses, and imply no affiliation or endorsement.
