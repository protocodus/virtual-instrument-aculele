# Aculele — decision log

Directions chosen by ear, recorded per the A–Z listening-test convention in
the repository's `CLAUDE.md`, and the owner's product decisions. A choice made
by ear is recorded as made by ear, never written up as though a measurement
had settled it; an entry settled by a measurement says so and quotes it.

## 2026-10-27 — the guitar recording contour is off (measured, not by ear)

Acustra's capture voicing (`CaptureVoicingData.h`) corrects its close
microphones toward recorded steel-string flat-tops: -6 dB at 250 and 500 Hz
and +6 dB at 1.4 kHz. Against one real ukulele recording ("Ukulele Arpeggio",
Duke Sonic Dictionary, CC BY-NC 4.0, not distributed), with Aculele playing
the same notes on the same strings, the mean H2-H8 level difference in the
first 150 ms was about 16 dB with the contour and 7 dB (concert) and 11 dB
(soprano) without it. The contour is off (`guitarCaptureContour`), and
`ACULELE_GUITAR_CAPTURE_CONTOUR=1` restores it for comparison. The reference
is one recording of an unidentified instrument on a budget USB microphone,
so this is weak evidence; no listener has compared the two.

## 2026-10-27 — the name is Aculele (owner)

The owner suggested "aculele" and "ukurele" for the fork. Aculele was
recommended as fitting the portfolio's naming (repository
`virtual-instrument-aculele`, product `Aculele`, bundle and CLAP id
`cz.protocodus.aculele`), and the owner took it by creating the repository.

## 2026-10-27 — the bridge level comes from the one measured ukulele (measured; its reading chosen by ear)

Aculele inherited Acustra's `steelTopMobilityRatio`, 0.274, which brings the
flamenco guitar g21's bridge to a steel-string guitar's. That corrects
Acustra's proxy and says nothing about a ukulele. Elie, Gautier, David and
Curtit (Forum Acusticum 2011, Fig. 5) measured one ukulele at 0.009 s/kg
characteristic mobility, 1-6 kHz, with its usual rosewood bridge. With the
inherited ratio, Concert Mahogany's G string read 0.0051 s/kg
(`Tools/AculeleBridgeMobility.cpp`), and its C string 0.0025.

The level is the measurement's. Where it applies is not: the accelerometer
sat "at the base of the lowest string". On the paper's guitars that is the
bass-side edge, the G string's place. By pitch it is the C string. Each
reading is defensible and neither needs an invented number, so they went to
the owner as a lettered set (`bridge-listening-2026-10-27`). The set used
tab input of open C, open G, C5 on the A string, then a C downstroke,
through Concert Strum, level-matched on whole-file RMS:

- A: the shipping engine, ratio 0.274.
- B: the G reading, ratio 0.485 (G string 0.009 s/kg).
- C: the C reading, ratio 0.982 (C string 0.009 s/kg).

The owner liked B and C and chose C as a bit better, by ear. The ratio is
now 0.982, and `Aculele.BridgeMobility` holds Concert Mahogany's C string
at 0.009 s/kg within 3%. This licenses the C reading of one measurement,
not a fitted level: a bridge measured at both strings would settle it.

Against the one comparison recording (Duke Sonic Dictionary, CC BY-NC 4.0,
compared only), 1-1.6 kHz stays +5.6 to +10.1 dB too strong at every level
tried (0.274, 0.50 and 1.0). So that excess is not the bridge's, and no
bridge level was chosen to reduce it.

## 2026-10-27 — the strumming hand for 0.3.0 (from sources, not by ear)

Five changes, each from a published description or measurement, and each a
`PerformanceRealism` switch except the new F0 key. With every switch off the
seven demos render bit-identically to 0.2.0. Per-fix before/after previews,
level-matched on whole-file RMS, went to the owner; no listener has yet
compared them, so none of this is a by-ear verdict.

- **Nail downstrokes** (`nailStrokes`). MasterClass's "How to Strum a
  Ukulele" teaches the downstroke on the index finger's nail and the
  upstroke on its pad. A nail is a hard edge the string slides off, so a
  downstroke takes the Pick's whole contact, which is GuitarSet's plectrum
  fit. No nail stroke has been measured; this reuses an existing fit and
  draws no new constant.
- **The hand's swing** (`strumPendulum`). The same lesson keeps the hand
  moving down on the beat and up between, through the strokes it skips. The
  restart after 4 beats or 2 s keeps the earlier rest rule's 2 s bound.
- **Chucks** (F0, held). A string the fretting hand lies on takes GuitarSet's
  measured muted stroke (0.589 level, 43-period T60), as the dead strings
  inside a strum already did; a chuck gives it to every string crossed.
- **Nylon fretting pitch** (`nylonFrettingPitch`). Varieschi and Gower's
  stretch law, Δf/f = ½(EA/T)ΔL/L, scales Acustra's steel finger spread by
  each string's own axial stiffness over tension and by the scale length.
- **Fingerstyle thumb** (`fingerstyleThumb`). In p-i-m-a fingerpicking the
  thumb plays the G string, so under Finger that string takes the Thumb's
  pad footprint.
