# Aculele

Aculele is a physically modelled ukulele: four re-entrant nylon or fluorocarbon strings on a soprano, concert or tenor body, played by finger, thumb, plectrum or a finger strum. Its engine is Acustra's, forked from Acustra 1.9.0 and re-strung, re-sized and re-voiced as a ukulele. It is DSP only for now, a JUCE-free C++17 engine with a demo renderer and analysis tools; there is no VST3, AU, CLAP or standalone build yet.

## Audio demos

The main build renders these demos once Aculele has a plug-in to build. Until then, `RenderAculelePresetDemos` renders them locally from the phrases in `Docs/audio/midi/` (see Build).

<!-- audio-demos-begin -->
<!-- audio-demos-end -->

## Version history

### 0.4.6 — 2026-10-28 — Melodies around the open strings stay in first position

A single note played with no fretted note in the last two seconds now goes where a hand in first position plays it, at its lowest fret on a string no key holds. Before, it went to whichever string had not sounded yet, however high up the neck: after G4 A4 G4 E4 G4 on the open strings, C5 sounded at the C string's twelfth fret instead of the A string's third, and a Low G scale from C4 put D4 at the low G string's seventh fret and G4 at the C string's seventh, where it now plays C2 and E3. MPE notes are placed as before. The demos are unchanged.

### 0.4.5 — 2026-10-28 — Sequenced notes take the strings a hand would

A note that ends on the sample the next begins, as notes drawn end to end in Reason's sequencer do, now lets go of its string before the next note chooses one. Before, the ending note still held its string, so the next went elsewhere: a one-channel strum after a chord lost its fourth string (C then Am played x000 instead of 2000), and a scale hopped to strings still held (F4 on the C string's fifth fret, D5 on the G string's seventh) where it now plays in first position and on up the A string. A zero-length note, a key struck again and a hammer-on's source keep their order. The demos are unchanged.

### 0.4.4 — 2026-10-28 — Chords below the ukulele's range are voiced in it

A chord on one MIDI channel with notes below the tuning's lowest string now plays them octaves up, where a ukulele player reads a chord chart, instead of dropping them. A keyboard part often sits there: Reason calls MIDI note 60, the C string's open C4, C3. So A3 C4 E4 now strums Am as 2000 where it sounded only C4 and E4, and C3 E3 G3 strums C as 0003 where it was silent. A single note below the range still sounds nothing. The demos are unchanged.

### 0.4.3 — 2026-10-28 — Chucks stop what the sustain lets ring

A chuck now stops every string its stroke crosses, as the fretting hand lying across them does, including those the sustain pedal, a front end's Sustain switch or the D#0 keyswitch lets ring. Before, those strings rang on through the chuck: a C chord under the sustain, chucked, still sounded at -44 dBFS RMS 0.3 s later, where it now falls silent as an unsustained chuck does. A key still held keeps its string. The demos are unchanged.

### 0.4.2 — 2026-10-28 — Chords keep their notes on one channel

A chord on one MIDI channel with two notes between C4 and D#4, which only the C string reaches, no longer drops one of them: its lowest note moves up an octave, as a player voices it, so C4 Eb4 G4 plays Cm as 0333, C4 Eb4 F4 A4 plays F7 as 2313 and C4 Eb4 F#4 G#4 plays Ab7 as 1323. Before, Cm sounded 00xx without its third. The moved key's key-up lets go of the string it sounds on. The demos are unchanged.

### 0.4.1 — 2026-10-28 — A smoother piezo

The piezo's saddle now sits on the ukulele's own bridge, the measured one the strings already play into, instead of the far stiffer guitar bridge Acustra's piezo was designed on. That bridge soaks up the pickup element's resonance, so the piezo loses most of its nasal edge: against the microphones, a strum or a fingerpicked phrase is 3-8 dB less bright at 4-6.3 kHz, and 3-7 dB less dull above 10 kHz. Preset levels are unchanged.

### 0.4.0 — 2026-10-27 — Calibrated to Reason's meter, behind the shared output stage

Typical playing, single notes across the neck and chords at velocity 70 to 110, now reads -9 VU on Reason's channel meter on every preset, and every body, wood, Picking and Capture reads within 1.5 dB of it, so switching one no longer jumps the level; before, they spread over about 10 dB and Pick was 3 to 4 dB hotter. Piezo Mix now crossfades from the microphones to an even blend with the piezo, instead of laying the piezo over them at full level: turning it up moves the level by -1.6 to +2.7 dB instead of adding 1.5 to 5.7. Main and the Piezo output end in the portfolio's shared output stage, a guard compressor that acts only above -4 VU and a 2 ms look-ahead limiter at -1.5 dBFS: velocity-127 chords on any Picking read -2.5 VU or lower, out of the red, and typical playing passes untouched. Main and the Piezo output are 2 ms later (96 samples at 48 kHz), which a front end reports as latency.

### 0.3.1 — 2026-10-27 — Headroom for hard playing

Played hard, every preset drove its output into the -1 dBFS safety limiter, which is heard as overdrive: four-note chords at velocity 127 peaked at +2.4 to +4.9 dBFS before it, Tenor Picked Melody's at +3.9. The presets' Output levels now bring those chords, struck in eighths, to -3 dBFS, and the same chords as a sixteenth-note tremolo stay under the limiter. The presets are 5.4 to 7.9 dB quieter than before: a strum at velocity 100 plays at -19 to -23 LUFS, and the demos at -23 to -26 LUFS instead of -18. Nothing else about the sound changes.

### 0.3.0 — 2026-10-27 — The strumming hand: nail and pad, its swing, chucks

A Strum downstroke now meets the strings with the index finger's nail, as hard as a plectrum, and an upstroke with the pad. Strokes with no direction keyswitch follow the hand's down-up swing in eighths, or sixteenths, so the island strum D, D-U, U-D-U plays as written; before, a stroke alternated with the last within a dotted eighth and restarted after it. Holding F0 makes each stroke a chuck, every string muted under the fretting hand. A fretted note's pitch spread now follows each nylon string's own stiffness: about 4 cents on the soprano's C string, 1.6-2.6 on the tenor. Under Finger the thumb plucks the G string with its broader pad. The demos latch their directions and play no chucks. The engine also takes about 18% less CPU, rendering the same samples ([measurement](Docs/benchmarks/cpu-2026-10-27.md)).

### 0.2.0 — 2026-10-27 — A ukulele's bridge, and full strums on one channel

The bridge is now as mobile as the one published ukulele measurement, so the strings give more of their energy to the top. Before, it was Acustra's steel-string level, about a quarter of that. A strum of three notes on one MIDI channel now sounds all four strings, the free one fretted with a chord tone, as a ukulele chord does. The presets' Output levels are re-trimmed so every demo stays at -18 LUFS. See [the research plan](Docs/plans/ukulele-realism-plan-2026-10-27.md).

### 0.1.0 — 2026-10-27 — A ukulele on Acustra's engine

The first engine: four re-entrant strings in Standard, Low G and D tuning, nylon and fluorocarbon sets, soprano, concert and tenor bodies, a finger Strum technique, tab input with one MIDI channel per string, and seven factory presets. Guitar-specific voicing is switched off. See [the feasibility study](Docs/research/ukulele-feasibility-2026-10-27.md).

## Downloads

<!-- build-download-begin -->
No downloads yet: Aculele is DSP only, with no plug-in to build.
<!-- build-download-end -->

---

## How it works

Most of the engine and Performer are Acustra's, unchanged: dual-polarisation stiff-string waveguides, the two-way bridge junction, the measured g21 bridge and radiation banks under the coupled A0/T1 Shape morph, finger and plectrum contact, release joins, room and piezo. What makes it a ukulele:

- **Four strings, re-entrant.** The tunings are Standard G4 C4 E4 A4, Low G (G3, on its own unwound fluorocarbon string) and D tuning A4 D4 F#4 B4. Strings are in physical order G, C, E, A, and the fretboard has 18 frets.
- **Strings from measurement, not fitting.** Plain nylon or fluorocarbon monofilaments use real set gauges (D'Addario EJ65C, Worth CM and CM-LG). Tension comes from T = μ(2Lf)². Stiffness, bending loss and air loss come from Woodhouse and Lynch-Aird, Acta Acustica 105 (2019) 516, with their stress-dependent E′ and E″ for each material. See `ukuleleStringLoss` in `Source/DSP/AculeleEngine.cpp`.
- **Bodies.** The soprano, concert and tenor bodies use Kala's published dimensions and scale lengths, through Acustra's own coupled-oscillator morph. Measured with `Tools/AculeleBodyModes.cpp`, A0/T1 sit at 266/653 Hz (soprano), 215/485 Hz (concert) and 190/414 Hz (tenor) in spruce.
- **Bridge level from a measured ukulele.** Elie, Gautier, David and Curtit (Forum Acusticum 2011) measured one ukulele's bridge at about 0.009 s/kg characteristic mobility from 1 to 6 kHz, with its usual rosewood bridge. Concert Mahogany's C string reads that between 1 and 6 kHz, and the other bodies are within 10% of it (`Tools/AculeleBridgeMobility.cpp`, `Aculele.BridgeMobility`). It replaces Acustra's steel-string correction, which left the bridge about a quarter as mobile. The piezo's saddle sits on the same bridge: its conductance, 0.009 s/kg, damps the pickup element's 6 kHz resonance to a +2 dB rise, where the guitar bridge Acustra's piezo assumed left a +11 dB peak (`PiezoDesign` in `Source/DSP/AculeleEngine.h`).
- **Geometry.** The saddle spacing is 13.3 mm, which sets the bridge lever arms and the strum's travel. Pluck distances and the finger's contact width keep their fraction of the shorter string. The hand's band reaches 0.46 of the string, where a ukulele is strummed over the neck joint.
- **Strum technique.** `PickingTechnique::Strum` is the index finger swept down and up over the neck joint, with the plectrum's sweep timing. A downstroke strikes with the nail, which the string slides off as it does a plectrum, so it takes the Pick's hard contact; an upstroke strikes with the pad, Finger's soft one (MasterClass, "How to Strum a Ukulele"). A nail stroke puts about 7 dB more of its energy at 3-8 kHz than the pad's.
- **The hand's swing.** With no direction keyswitch, the hand moves down on each eighth and up between, and keeps that time through the strokes it skips: a stroke an odd number of eighths after the last is the other direction, an even number the same. A gap of an odd number of sixteenths puts the hand in sixteenths for a bar. After 4 beats or 2 s without a stroke, it starts again with a downstroke (`Performer::pendulumUpstroke`).
- **Chucks.** While F0 is held, every swept stroke is a chuck: the fretting hand lies across the strings instead of fretting them, and each string the stroke crosses takes GuitarSet's muted-string stroke, 4.6 dB down and dying in 43 periods. The stroke's keys own no string. A chord whose keys come up as the chuck's go down is let go first, so back-to-back chords in a sequencer chuck too, and a string the sustain lets ring takes the dead stroke as well; only a key still held keeps its string.
- **Fretting pitch.** Each contact's pitch differs a little within GuitarSet's bounds. A fretted string is stretched by the finger, and the pitch it gains scales with the string's axial stiffness over its tension, and inversely with the scale length (Varieschi and Gower, Am. J. Phys. 78 (2010) 47). Acustra's steel figure scaled this way gives nylon 2.0-3.4 cents on the concert, 4.2 on the soprano's C string and 1.6-2.2 on fluorocarbon (`AculeleEngine::frettingPitchHalfWidth`).
- **Fingerstyle thumb.** Under Finger the thumb plucks the G string with its broader pad, as the Thumb technique does, and the fingers the other three.
- **Tab input.** In string-per-channel mode (Mono Mode On, CC126 = 4, on channel 1), channels 1-4 are the G, C, E and A strings. A group of notes on those channels is one strummed or pinched stroke, so a chord can double a pitch on two strings, as G (0232) and Am (2000) do. The demos are written this way.
- **Full strums on one channel.** A strum or pick sweep of three notes on one ordinary channel leaves one string free. The fretting hand gives that string a chord tone at the fret it reaches most cheaply with the shape, so the stroke sounds all four strings:
  - C4 E4 G4 plays 0003;
  - C4 E4 A4 plays 2000;
  - D4 G4 B4 plays 0232;
  - D4 F#4 A4 plays 2220.

  No key owns the added note. It is held with the chord and let go with the last of its keys, under the pedal as they are. Finger chords, pairs and tab input are unchanged.
- **Notes end to end.** A sequencer ends a note on the sample the next one starts. The ending key comes up first, as the player's finger lifts, so the next note or chord is placed as if the key had come up a moment before (`Performer::flushNoteGroup`). The key-ups of a zero-length note, of a key struck again on that sample and of a hammer-on's or pull-off's source still come after the Note Ons.
- **First position.** A single note with no fretted note in the hand's two-second memory goes to its lowest fret on a string no key holds, as a hand in first position plays it; after that, the frets the hand last held place each note, as in Acustra (`AculeleEngine::chooseString`).
- **Chords lifted into reach.** A one-channel chord's notes below the tuning's lowest string move up by whole octaves into its range, so A3 C4 E4 plays Am as 2000 (`Performer::foldChord`); a single low note still sounds nothing. Only the C string reaches below E4, so a chord with two notes in C4-D#4 has no shape: the fewest notes move up an octave, the lowest first, to octaves the chord does not already hold, until the hand can fret it. C4 Eb4 G4 plays Eb4 G4 C5, Cm as 0333 with the fill. A key already down stays on its string, and each moved key's key-ups let go of the string it sounds on (`Performer::liftChord`).
- **Switched off because they were measured or voiced on guitars:**
  - the wound-string noises;
  - the studio low-mode observation, a fit to guitar boxes below 400 Hz;
  - the +12 dB air-mode gain;
  - the per-construction loudness tables;
  - the mid-band capture dip;
  - the guitar recording contour (`CaptureVoicingData.h`), which is -6 dB at 250 and 500 Hz and +6 dB at 1.4 kHz. Against a real ukulele recording, that contour made the model 20-30 dB too bright above the 2nd harmonic.

### Presets

| # | Preset | Body, strings, tuning | Playing |
| --- | --- | --- | --- |
| 1 | Concert Strum | Concert, mahogany, nylon, Standard | Strum |
| 2 | Soprano D Tuning | Soprano, mahogany, nylon, D tuning | Strum |
| 3 | Tenor Fluorocarbon Fingerstyle | Tenor, spruce, fluorocarbon, Standard | Finger |
| 4 | Low G Tenor | Tenor, mahogany, nylon + low G, Low G | Finger |
| 5 | Soprano Thumb | Soprano, mahogany, nylon, Standard | Thumb |
| 6 | Tenor Picked Melody | Tenor, maple, fluorocarbon, Low G, mono mic | Pick |
| 7 | Concert Piezo Stage | Concert, spruce, nylon, Standard, piezo | Strum |

Levels are set on Reason's channel meter, a VU meter read with 0 VU at -12 dBFS RMS (`Tools/ReasonMeter.h`). `Source/DSP/LevelMatchData.h`, which `AculeleLevels --write` measures, brings typical playing on every shape, wood, Picking and Capture to -9 VU at 0 dB Output, and each preset's Output trim brings its own there. Main and the Piezo output end in the portfolio's shared output stage, `Source/DSP/OutputStage.h`: a guard compressor on the louder channel's power, its knee centred at -2 VU, then a look-ahead limiter that holds every sample under -1.5 dBFS and passes everything below it bit for bit. `Aculele.Levels` checks the readings and that velocity-127 chords stay under -1 VU. The demo phrases in `Docs/audio/midi/` come from `Tools/write_preset_midi.py`.

## Known gaps

- **No ukulele body has been measured.** The radiation and bridge banks are a nylon flamenca guitar scaled down by the equal-thickness plate law. With the guitar contour off, the soprano's third-octave spectrum lies within about 4 dB of a real ukulele recording (an unidentified instrument) from 250 to 800 Hz, but it is still 7-12 dB too bright at 1-2 kHz. A real ukulele top is thinner and more lightly braced than the law assumes.
- **One channel chooses the voicing.** On one MIDI channel, a three-note strum's added string is the hand's cheapest chord tone, and the shape follows the notes given and where the hand already is. That is not always the voicing a player would choose:
  - C4 F4 A4 plays F as 2010;
  - F4 A4 C5 plays 5550 from a fresh hand, or 2013 after a C;
  - E4 G4 B4 plays 0402, where most players fret Em as 0432;
  - D4 F#4 A4 C5, or C4 D4 F#4 A4 lifted to it, plays D7 as 5220, where chord books show 2223.

  A Finger chord of three plays three strings. Tab input names every string; every demo but the Low G picked melody uses it.
- **The bridge level rests on one measured instrument.** The paper puts its accelerometer "at the base of the lowest string". That is the C string by pitch, and the G string's place by the guitar convention, which would halve the level. The C reading was chosen by ear (`Docs/decisions.md`); no measurement has decided between them.
- Pluck release, contact noise, strum speeds and finger-chord timing are Acustra's fits to guitar recordings (GuitarSet, the flat-top bank), reused unchanged. The nail's contact is GuitarSet's plectrum, and a chuck is GuitarSet's muted guitar string: no ukulele nail or chuck has been measured. The nylon fretting spread scales a steel-string estimate by stiffness; it is not a nylon measurement.
- The piezo's saddle and element are a guitar's: a 3.8 g bone saddle, its element resonance chosen at 6 kHz within a guitar pickup's documented 5-7 kHz. No ukulele undersaddle pickup has been measured.
- There is no koa wood; mahogany stands in for it.
- There are no rasgueado or triplet-roll strokes, and no palm chuck: a chuck is the fretting hand's.
- There is no plug-in, editor, screenshot, main build or release yet.

## Build

```sh
cmake -S . -B build-dsp -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-dsp
ctest --test-dir build-dsp -L '^(unit|audio|perf)$' --output-on-failure
./build-dsp/RenderAculelePresetDemos --jobs 4 Docs/audio/midi <out-dir>
./build-dsp/AculeleProbe <preset> <in.mid> <out.wav> [seconds] [--float] [--shape N] [--wood N] [--strings N] [--picking N] [--position X] [--capture N] [--piezo-mix X]
python3 Tools/analyse_probe.py <out.wav> <midi>:<start-seconds> ...
./build-dsp/AculeleBodyModes
```

CMake 3.22 and a C++20 compiler build the engine; `AculeleDSPCxx17` compiles it as C++17 with `-Werror`, as the Rack toolchain does. `Tools/analyse_probe.py` needs NumPy and SciPy. A demo renders at about 10x real time on one 2.8 GHz Xeon core, about 10% of that core; these are development-machine figures, not a DAW's.

## Licensing

Aculele is source-available under the PolyForm Noncommercial License 1.0.0 with an additional permission for official builds; see [LICENSE](LICENSE). The measurement-derived tables it carries from Acustra are adapted from CC BY 4.0 datasets, attributed with their changes in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
