# Aculele — decision log

Directions chosen by ear, recorded per the A–Z listening-test convention in
the repository's `CLAUDE.md`, and the owner's product decisions. A choice made
by ear is recorded as made by ear, never written up as though a measurement
had settled it; an entry settled by a measurement says so and quotes it.

## 2026-10-28 — each Shape takes its maker's strings, released at the concert string's force, 0.4.14 (chosen by ear)

**What was decided.** Every Shape was strung with the concert sets
(D'Addario EJ65C, Worth CM). The makers sell other sets for the other sizes:
EJ65S for the soprano, with a thinner C and A (.034 and .024 in against .0403
and .028), EJ65T for the tenor, 1.5-3.6% thicker than EJ65C, and Worth CT for
the tenor, whose A is 9% thicker than CM's. At the same pitch and scale a
thinner string sits at lower tension (the soprano's C and A at 20.5 and 29.0
N instead of 28.9 and 39.4 N). The pluck released every string at the same
displacement, so the thinner strings would play quieter. Releasing at a fixed
displacement and releasing at a fixed force are both defensible readings of a
finger, and no measurement of a ukulele player says which holds.

**The letters:**

- A: the shipping engine (0.4.13), the concert sets on every Shape, released
  at a fixed displacement.
- B: each Shape takes its maker's set, released at a fixed displacement.
- C: B's sets, with the finger letting go at the force it holds on the
  concert set's string: the released amplitude takes mu_concert / mu_own.

The set was one phrase (the four open strings let ring, a melody between the
C and A strings and the G and E, then C, Am down and up, and F) on Soprano
Thumb and on Concert Strum played on the soprano body, rendered through
`AculeleProbe` at 48 kHz in 256-sample blocks and matched to A by whole-file
RMS (trims: B +1.65 and +1.63 dB, C -0.14 dB on both). Before trim, B's open C
and A played 2.4-2.8 dB under A's, and C's within 0.3 dB of A's.

**The owner listened and chose C on both.**

**What it licensed.** C ships in 0.4.14 on the soprano (EJ65S, CM), the
concert (unchanged) and the fluorocarbon tenor (CT). On Soprano Thumb the open
C plays 0.3 dB louder against its G and the A unchanged; on Tenor
Fluorocarbon Fingerstyle the A plays 0.3 dB quieter. `LevelMatchData.h` was
re-pinned and three preset trims moved by 0.1-0.2 dB. The soprano's thinner C
string is less stiff, so fewer notes miss an upper partial by more than 10
cents (about 110 of 1,368 against 118 on 0.4.13, counted the same way).

**What it did not license.** The nylon tenor keeps EJ65C. With EJ65T, its
open F#4 in D tuning, just below the top's main resonance, read 5.5 cents
flat in `Aculele.Tuning` (4.0 on EJ65C), past the 5-cent tuner tolerance. With
either set the note plays 5-7 cents flat over its first half second; the test
passes on EJ65C only because its window takes in the in-tune tail. That is
the bridge tuning estimate's known limit beside a strong lossy mode, not
something the strings decide, so the tenor's nylon set waits for it and the
test was not changed. The force reading is chosen by ear; no ukulele
player's pluck has been measured.

## 2026-10-28 — the 1-4.4 kHz band takes a ukulele's mode density at its own mean level, 0.4.13 (chosen by ear)

**What was decided.** The body bank is the flamenco guitar g21's, scaled to
ukulele size. Its strongest plate modes above T1 land at 1.7-2.2 kHz on the
soprano (1.27-1.6 kHz on the concert, 1.07-1.35 kHz on the tenor), where the
soprano's high notes' second and third partials meet a few strong, isolated
resonances. On the same notes its high register had 8-13 dB more energy at
1.2-2.5 kHz, relative to the fundamental, than the concert's, and a split
render traced that to the radiation bank, not the strings or the bridge. The
one counted ukulele top (Elie, Gautier, David and Curtit, Forum Acusticum
2011) has about 57 modes between 1 and 4.4 kHz and a flat mean mobility. The
physics gave three ways to bring the bank to that density, each defensible,
and no measurement of a ukulele body says which level the band should take.

**The letters** (A-D, on every Shape and Wood):

- A: the shipping engine, unchanged.
- B: each scaled mode in the band becomes a cluster at the measured density
  within a sixth of an octave of it, keeping its energy, microphone balance
  and residue sum.
- C: B, then each cluster's power is scaled so the band's smoothed level
  follows its own mean across 1-4.4 kHz. A plate's response is flat in the
  mean above its separate low modes, and a point-driven plate radiates a
  power independent of frequency below coincidence.
- D: B, then the band is brought to the level the measured guitar top
  radiates in the same band, lower than C on every Shape.

The set was a soprano phrase (Soprano D Tuning: let-ring notes on the A and E
strings at frets 5-12, then three downstrokes up the neck) and the same phrase
on Concert Strum, rendered through `AculeleProbe` at 48 kHz and matched to A
by whole-file RMS (trims: soprano -0.02 dB for B and C, +0.04 dB for D;
concert +0.33 dB for C, +0.54 dB for D).

**The owner listened and chose C.**

**What it licensed.** C ships in 0.4.13 on every Shape and Wood
(`buildBodyConfiguration`), with `Aculele.PlateBand` checking that at least 35
modes sound in the band (37-47 do; the bank alone had 18-25) and that its
level, smoothed over a sixth of an octave either side, rises less than 3 dB
over its mean (1.0-2.0 dB; the bank alone 4.1-5.2 dB). The band's level
moved typical playing, so `LevelMatchData.h` was re-pinned and four preset
trims moved by 0.4-0.5 dB. The development machine's demo renders take about
6% more CPU. Against the Duke arpeggio recording, the soprano's 1.6 and 2 kHz
third-octave bands come down 4 and 5 dB.

**What it did not license.** The density and the flat mean come from one
measured ukulele of unstated size, so this is not a ukulele body
measurement. The band's level is its own mean, chosen by ear over D's guitar
level; neither is a measured ukulele's.

**Two corrections to the set's key.** First, the set was rendered on 0.4.11
(`10d0428`), before 0.4.12's output stage merged, so its A was 0.4.11. The
shipped code, applied to that base, renders the two C files byte for byte; on
0.4.12 it differs from them only by 0.4.12's own change. Second, the key said C
keeps the band's total. Measured after the choice, C leaves the band's mean
power 0.5-1.1 dB lower at 48 kHz, because the lone peaks held more of it than
the gaps they now fill. What the owner heard is what ships; only that
description was wrong.

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

## 2026-10-28 — open strings lead a chord's shape only in first position, 0.4.11 (owner's choice, checked against a chord book)

The owner's direction for the fretting hand, "prefer empty strings, then
prefer lower frets", voiced one-channel chords up the neck as no chord book
does: G (B4 D5 G5) as 0-11-10-10, where the books fret 7-7-7-10, and D7
(D4 F#4 A4 C5) as 5220, where they fret 2223. Three options were offered:
keep the rule; open strings first only in first position, with no fretted
note above the 4th fret, and the lowest hand position above it; or another
rule. The owner chose the second, on the understanding that it would be
refined until no voicing got worse.

What it licensed: `firstPositionFret` = 4. Above it, of two shapes the one
with the lower highest fret wins, then the one with fewer open strings, and
a three-note stroke's free string is fretted, the open string tried last.
Where the hand already is still comes first, as in the original direction,
so D7 straight after a G at the 7th fret stays 5220.

The check is a measurement, for evaluation only: every voicing the
MIT-licensed chords-db (David Rubert, `@tombatossals/chords-db` 0.5.1,
`ukulele.json`, not distributed) lists with four distinct pitches for 13
chord types, and every close triad, strummed from a fresh hand on Concert
Strum. On Standard, voicings landing on a shape the book lists for that
chord rose from 276 to 391 of 401, and close triads from 120 to 180 of 288
(Low G 573 to 623 of 623 and 183 to 253 of 378; D tuning as Standard).
None that landed on a book shape moved off one. The rule alone, without the
fretted fill and the fewer-open tie-break, reached 385 and 146. E4 G4 B4
stays 0402, which the books fret 0432; both are in first position, where the
open strings still lead. No one has listened to the change.

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
