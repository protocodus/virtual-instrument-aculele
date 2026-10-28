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

## 2026-10-28 — the thumb's upstroke is its nail's, 0.4.8 (from a source, not by ear)

Under Thumb, a chord's upstroke takes the Pick's hard contact, as a Strum's
downstroke does (`thumbnailUpstrokes`, `AculeleEngine::nailStroke`). Going
up, the back of the thumb leads into the strings, and Bruce Watson's ukulele
notes strum up with "the nail of your thumb on the upstroke" ("Strumming",
brucewatsonmusic.com). The same notes find that the thumb alone, down and up,
"tends to make the upstroke louder than the downstroke"; the model does not
reproduce that, its upstroke being 1-7 dB brighter at 3-8 kHz at a level
within 0.5 dB. No thumbnail stroke has been measured; this reuses the Pick's
fit, as the nail downstroke does. A before/after preview went to the owner;
no listener has compared them.

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

## 2026-10-27 — preset levels leave headroom for hard playing (owner's direction, measured)

Playing 0.3.0 in Reason, the owner heard Tenor Picked Melody overdrive and
the other presets come out too loud with several notes at high velocity, and
asked for headroom without the presets sounding quiet. The presets had been
trimmed so each demo played at -18 LUFS. Measured before the -1 dBFS safety
limiter, four-note chords at velocity 127 in eighths peaked at +2.4 to
+4.9 dBFS, so every preset's hard playing was being limited.

The Output trims now bring the louder of two eighth-note phrases at velocity
127, the open triads and four-note chords up the neck, to -3 dBFS
(`Tests/HeadroomTests.cpp`). -3 dBFS is the highest whole-decibel level at
which the same chords as a sixteenth-note tremolo also stay under the limiter
(the closest, Tenor Picked Melody, peaks at -1.3 dBFS). A strum at velocity
100 now plays at -19 to -23 LUFS; three of Acustra's presets play the same
phrase at -16 to -18 LUFS. No listener has judged the new levels.

## 2026-10-27 — typical playing at -9 VU behind the shared output stage (owner's choice, measured)

The owner asked for typical playing (across the range, octaves and
velocities) to read about -7 dB on Reason's VU meter, for velocity-127
chords on Pick never to reach the red, and for every microphone and the
piezo to be calibrated alike. On Reason's channel meter as
`Tools/ReasonMeter.h` defines it, plucked chords read 7 to 9 dB above typical
playing and peak 11 to 15 dB above their own reading, so at -7 VU the hardest
chords either reach the red or clip; 0.3.1's peak rule held typical playing
near -13 VU. Offered -7 VU with more limiting, -9 VU with a clean peak
limiter, or the peak rule alone, the owner chose -9 VU with a clean peak
limiter.

What it licensed, each settled by measurement (`AculeleLevels --report`):

- `LevelMatchData.h` brings every shape, wood, Picking and Capture to -9 VU at
  0 dB Output; before, the cells spread over about 10 dB and Pick was 3 to
  4 dB hotter. The preset trims are 0.0 to -1.3 dB.
- Piezo Mix crossfades at equal power. Laid over the microphones at full
  level it added 1.5 to 5.7 dB; it now moves the level by -1.6 to +2.7 dB, the
  rest being how far the two sensors are correlated.
- The output ends in the portfolio's shared output stage
  (`Source/DSP/OutputStage.h`), after the owner asked for the same limiter in
  every new instrument. With its limiter alone the loudest chords read -3.5 to
  -0.1 VU; with the plucked instruments' guard (`kVuGuard`, acting only above
  -4 VU), every typical reading is unchanged to 0.01 dB and the loudest read
  -4.2 to -2.5 VU. Aculyre needs the guard to keep the lyre out of the red;
  Aculele adopts it so the stage is the same.

No listener has judged the new levels.
