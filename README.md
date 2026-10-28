# Aculele

Aculele is a physically modelled ukulele: four re-entrant nylon or fluorocarbon strings on a soprano, concert or tenor body, played by finger, thumb, plectrum or a finger strum. Its engine is Acustra's, forked from Acustra 1.9.0 and re-strung, re-sized and re-voiced as a ukulele. It is DSP only for now, a JUCE-free C++17 engine with a demo renderer and analysis tools; there is no VST3, AU, CLAP or standalone build yet.

## Audio demos

The main build renders these demos once Aculele has a plug-in to build. Until then, `RenderAculelePresetDemos` renders them locally from the phrases in `Docs/audio/midi/` (see Build).

<!-- audio-demos-begin -->
<!-- audio-demos-end -->

## Version history

### 0.4.16 — 2026-10-28 — A key struck twice on one sample keeps its stroke's force

A pluck on the same sample as the last stroke on its string is that stroke played again: it keeps that stroke's force variation, faded out as the force changes, as one a sample later already did. Before, it drew a new variation in full, so a key struck twice on one sample at a much softer velocity could play up to 1 dB away from where it played one sample later. The demos and the levels are unchanged.

### 0.4.15 — 2026-10-28 — No two plucks alike, and every string let go at one force

Every pluck now draws its own force, within ±1 dB, and its own release angle, turned by about 2° from pluck to pluck. Before, the force varied only when the same note repeated, so a melody of different notes at one velocity varied by 0.2 dB and every pluck sounded alike. Now the spread is 0.8-0.9 dB, near a real player's 0.65 dB. An accent within two seconds on the same string still plays as written. The finger also lets every string go at one force, so a tighter string is displaced less and no string drives the bridge harder than the others. Against the other strings, the A string plays 1.1-2.6 dB quieter (1.8 dB on the nylon sets) and the low G 2.3-2.6 dB louder. Both were the owner's choices by ear. Four preset trims moved by 0.6-2.1 dB to keep typical playing at -9 VU.

### 0.4.14 — 2026-10-28 — The soprano is strung with a soprano set

The soprano now takes D'Addario's soprano set, EJ65S, whose C and A strings are thinner than the concert set's (.034 and .024 in against .0403 and .028), so they sit at 20.5 and 29.0 N instead of 28.9 and 39.4 N. The tenor on fluorocarbon takes Worth's tenor set, CT, whose A is 9% thicker than CM's. The finger lets go of each string at the force it holds on the concert set's string, so a thinner string is pulled further and plays about as loud as before: the soprano's open C is 0.3 dB louder against its G and its A unchanged, and the fluorocarbon tenor's A is 0.3 dB quieter. This was the owner's choice by ear of three candidates. The concert is unchanged, and the nylon tenor keeps the concert set (Known gaps). Three preset trims moved by 0.1-0.2 dB to keep typical playing at -9 VU.

### 0.4.13 — 2026-10-28 — No lone resonances between 1 and 4.4 kHz

The scaled guitar bank left a few strong, lone resonances between 1 and 4.4 kHz. On the soprano they sat at 1.7-2.2 kHz, where its high notes' second and third partials land, and made its upper register harsh. Each of them is now a cluster of resonances at the density of the one ukulele top that has been counted (Elie et al. 2011), and the band's level follows its own mean rather than where the bank's few modes fall. This applies to every Shape and Wood, and was the owner's choice by ear of four candidates. The band's sounding modes go from 18-25 to 37-47, and its peaks, smoothed over a third of an octave, from 4.1-5.2 dB over its mean to 1.0-2.0 dB. Against a real ukulele recording, the soprano's 1.6 and 2 kHz bands come down 4-5 dB. Four preset trims moved by 0.4-0.5 dB to keep typical playing at -9 VU.

### 0.4.12 — 2026-10-28 — The hardest playing gets louder again

The output stage's guard compressor, now the one every plucked Protocodus instrument shares, reads power over the 300 ms Reason's meter averages over, and eases in over a knee 16 dB wide, from -11 to +5 VU at 10:1. Before, it followed each pluck's attack over 80 ms, so chords at velocity 110, 120 and 127 came out alike: Concert Strum's loudest read -3.7, -3.5 and -3.6 VU. Now each step in velocity plays louder (-3.6, -2.9 and -2.5 VU), and the loudest chord on any preset and Picking reads -1.5 VU or lower, out of the red. Typical playing reads -9 VU, as before.

### 0.4.11 — 2026-10-28 — Chords up the neck take the chord books' shapes

A one-channel chord now leads with open strings only in first position, with no fretted note above the 4th fret (the owner's choice). Above it, the shape that keeps the hand lowest wins, and the hand frets the strings rather than leaving them open: a piano roll's G (B4 D5 G5) strums 7-7-7-10, not 0-11-10-10; D7 (D4 F#4 A4 C5) strums 2223, not 5220; C (G4 C5 E5) strums 5-7-8-7, not 0-0-8-7. Of 401 four-note voicings a ukulele chord book lists, 391 now strum as the book frets them (276 before), and 180 of 288 close triads take one of its shapes (120 before); none that did lost it. The hand's reach still comes first, so D7 straight after a G at the 7th fret stays 5220. Tab input and the demos are unchanged.

### 0.4.10 — 2026-10-28 — No search at a stiff note's first pluck

A stiff note's first pluck no longer runs its whole partial search in the note-on batch: 3.5-9 ms at 48 kHz on the development machine, where a 256-sample batch lasts 5.3 ms. Each string's first design at every fret is now made ahead of the note, one step of its search every 5.3 ms of audio (a median 0.3 ms, at most about 2 ms), and all of them are ready 0.8-3.5 s after a factory preset loads at 48 kHz, or its body, strings, tuning or rate change (up to 7 s at 44.1 kHz with the Pick, Strum or Thumb, which need a 48 kHz design too). Meanwhile the engine takes about 10% more of real time than it does at rest. A melody climbing the nylon C string from the 6th fret fell to 0.5-1.3 ms a note-on. The designs are the ones the notes made themselves, so the sound is unchanged: every demo renders bit for bit as in 0.4.9.

### 0.4.9 — 2026-10-28 — Stiff high notes are lighter on the CPU

About one note in five, most of them on the C string high up the neck and on nylon, needs a slower search to place its upper partials, and that search ran again at every pluck and every 40 ms or so under a vibrato or a bend. Such a note now continues from its last design while its pitch stays within 1%, and searches again only when continuing would leave its worst partial more than a cent further out than the last search did. One such note under full vibrato fell from about 22% to 9% of real time at 48 kHz on the development machine. Its first pluck still runs the whole search, up to 12 ms. Partials move by at most 0.3 cents: Soprano Thumb's E4 on the C string is the one demo note that changes.

### 0.4.8 — 2026-10-28 — The thumb strums back up on its nail

Under Thumb, a chord's upstroke now meets the strings with the thumbnail, as the back of the thumb leads into them going up, where it used the thumb's pad both ways. An upstroke is 1-7 dB brighter at 3-8 kHz, its level within 0.5 dB. Downstrokes and single notes keep the pad. The demos are unchanged: Soprano Thumb's ballad is all downstrokes.

### 0.4.7 — 2026-10-28 — Three-note strums take the shape their four strings make

A strum or pick sweep of three notes on one channel also sounds the free string, with a chord tone. The shape is now chosen with that note, by the four strings the stroke sounds, so it is the shape those four pitches take as a chord of four. Before, it was chosen for the three notes alone, and the added string followed: F4 A4 C5 played 5550, doubling A4, where it now plays 2013 with the open C, as C4 F4 A4 C5 does; on Low G, E4 G4 C5 played 12-12-0-10 and now plays 0433 with the open low G. Some higher chords now take an open string further up the neck, as their chords of four already did: B4 D5 G5 plays 0-11-10-10 where it played 7-7-7-10. Finger chords are unchanged. The demos are unchanged.

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
- **Strings from measurement, not fitting.** Plain nylon or fluorocarbon monofilaments use the set each maker sells for the body: D'Addario EJ65S on the soprano and EJ65C on the concert and tenor; Worth CM on the soprano and concert and CT on the tenor; and Worth's unwound low G. The finger lets every string go at one force, the force that displaces a string at the concert nylon set's mean tension (about 37.7 N) as far as the stroke was calibrated, so a string's displacement goes as 1/T against it. Tension comes from T = μ(2Lf)². Stiffness, bending loss and air loss come from Woodhouse and Lynch-Aird, Acta Acustica 105 (2019) 516, with their stress-dependent E′ and E″ for each material. See `ukuleleStringLoss` in `Source/DSP/AculeleEngine.cpp`.
- **Bodies.** The soprano, concert and tenor bodies use Kala's published dimensions and scale lengths, through Acustra's own coupled-oscillator morph. Measured with `Tools/AculeleBodyModes.cpp`, A0/T1 sit at 266/653 Hz (soprano), 215/485 Hz (concert) and 190/414 Hz (tenor) in spruce.
- **A ukulele's mode density between 1 and 4.4 kHz.** Elie et al. (Forum Acusticum 2011) counted about 57 modes there on one ukulele's top, against the 18-25 the scaled bank places, and measured its mean mobility flat. Each bank mode in the band with a Q above 4.3 becomes a cluster at that density, spread over at most a sixth of an octave either side and up to halfway to its neighbours, with random places and phases from a fixed seed. Each cluster's power is then scaled by the band's mean over its own power within a sixth of an octave, so the band follows its mean (`buildBodyConfiguration` in `Source/DSP/AculeleEngine.cpp`, `Aculele.PlateBand`).
- **Bridge level from a measured ukulele.** Elie, Gautier, David and Curtit (Forum Acusticum 2011) measured one ukulele's bridge at about 0.009 s/kg characteristic mobility from 1 to 6 kHz, with its usual rosewood bridge. Concert Mahogany's C string reads that between 1 and 6 kHz, and the other bodies are within 10% of it (`Tools/AculeleBridgeMobility.cpp`, `Aculele.BridgeMobility`). It replaces Acustra's steel-string correction, which left the bridge about a quarter as mobile. The piezo's saddle sits on the same bridge: its conductance, 0.009 s/kg, damps the pickup element's 6 kHz resonance to a +2 dB rise, where the guitar bridge Acustra's piezo assumed left a +11 dB peak (`PiezoDesign` in `Source/DSP/AculeleEngine.h`).
- **Geometry.** The saddle spacing is 13.3 mm, which sets the bridge lever arms and the strum's travel. Pluck distances and the finger's contact width keep their fraction of the shorter string. The hand's band reaches 0.46 of the string, where a ukulele is strummed over the neck joint.
- **Strum technique.** `PickingTechnique::Strum` is the index finger swept down and up over the neck joint, with the plectrum's sweep timing. A downstroke strikes with the nail, which the string slides off as it does a plectrum, so it takes the Pick's hard contact; an upstroke strikes with the pad, Finger's soft one (MasterClass, "How to Strum a Ukulele"). A nail stroke puts about 7 dB more of its energy at 3-8 kHz than the pad's.
- **Thumb strokes.** Under `PickingTechnique::Thumb` a chord's downstroke meets the strings with the thumb's pad and its upstroke with the thumbnail, the Pick's hard contact, as a ukulele thumb strum leads with the back of the thumb going up (Bruce Watson, "Strumming"). Single notes are downstrokes and keep the pad.
- **The hand's swing.** With no direction keyswitch, the hand moves down on each eighth and up between, and keeps that time through the strokes it skips: a stroke an odd number of eighths after the last is the other direction, an even number the same. A gap of an odd number of sixteenths puts the hand in sixteenths for a bar. After 4 beats or 2 s without a stroke, it starts again with a downstroke (`Performer::pendulumUpstroke`).
- **Chucks.** While F0 is held, every swept stroke is a chuck: the fretting hand lies across the strings instead of fretting them, and each string the stroke crosses takes GuitarSet's muted-string stroke, 4.6 dB down and dying in 43 periods. The stroke's keys own no string. A chord whose keys come up as the chuck's go down is let go first, so back-to-back chords in a sequencer chuck too, and a string the sustain lets ring takes the dead stroke as well; only a key still held keeps its string.
- **Fretting pitch.** Each contact's pitch differs a little within GuitarSet's bounds. A fretted string is stretched by the finger, and the pitch it gains scales with the string's axial stiffness over its tension, and inversely with the scale length (Varieschi and Gower, Am. J. Phys. 78 (2010) 47). Acustra's steel figure scaled this way gives nylon 2.0-3.4 cents on the concert, 4.2 on the soprano's C string and 1.6-2.2 on fluorocarbon (`AculeleEngine::frettingPitchHalfWidth`).
- **No two plucks alike.** Every pluck but a strummed chord's, which has its own variation, draws its own force within ±1 dB (0.58 dB SD), below the 0.65 dB median spread of a real guitarist's takes of one note (the CC0 Shinyguitar archtop's round robins). It also draws its own release angle, about 2° apart from pluck to pluck, which is one harpist's spread in Chadefaux et al., JASA 131 (2012) 844, Table VI. Within the hand's two-second memory on a string, a changed velocity keeps its accent exact.
- **Fingerstyle thumb.** Under Finger the thumb plucks the G string with its broader pad, as the Thumb technique does, and the fingers the other three.
- **Tab input.** In string-per-channel mode (Mono Mode On, CC126 = 4, on channel 1), channels 1-4 are the G, C, E and A strings. A group of notes on those channels is one strummed or pinched stroke, so a chord can double a pitch on two strings, as G (0232) and Am (2000) do. The demos are written this way.
- **Full strums on one channel.** A strum or pick sweep of three notes on one ordinary channel leaves one string free. The fretting hand gives that string a chord tone at the fret it reaches most cheaply with the shape, so the stroke sounds all four strings:
  - C4 E4 G4 plays 0003;
  - C4 E4 A4 plays 2000;
  - D4 G4 B4 plays 0232;
  - D4 F#4 A4 plays 2220.

  The shape is chosen with the added note, by the open strings and frets of all four, so it is the shape those four pitches take as a chord of four: F4 A4 C5 plays 2013, as C4 F4 A4 C5 does (`AculeleEngine::planChord`). No key owns the added note. It is held with the chord and let go with the last of its keys, under the pedal as they are. Finger chords, pairs and tab input are unchanged.
- **Notes end to end.** A sequencer ends a note on the sample the next one starts. The ending key comes up first, as the player's finger lifts, so the next note or chord is placed as if the key had come up a moment before (`Performer::flushNoteGroup`). The key-ups of a zero-length note, of a key struck again on that sample and of a hammer-on's or pull-off's source still come after the Note Ons.
- **First position.** A single note with no fretted note in the hand's two-second memory goes to its lowest fret on a string no key holds, as a hand in first position plays it; after that, the frets the hand last held place each note, as in Acustra (`AculeleEngine::chooseString`).
- **Open strings in first position only.** A chord's shape takes the most open strings only when no fretted note is above the 4th fret (`AculeleEngine::firstPositionFret`). Above it, the shape whose highest fret is lowest wins, then the one fretting more strings, and a three-note stroke's free string is fretted where the hand reaches a chord tone. So B4 D5 G5 plays 7-7-7-10 and G4 C5 E5 5-7-8-7, as the chord books fret them, while E4 G4 B4 keeps 0402. Where the hand already is still comes first (`AculeleEngine::betterShape`).
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

Levels are set on Reason's channel meter, a VU meter read with 0 VU at -12 dBFS RMS (`Tools/ReasonMeter.h`). `Source/DSP/LevelMatchData.h`, which `AculeleLevels --write` measures, brings typical playing on every shape, wood, Picking and Capture to -9 VU at 0 dB Output, and each preset's Output trim brings its own there. Main and the Piezo output end in the portfolio's shared output stage, `Source/DSP/OutputStage.h`: a guard compressor on the louder channel's power over 300 ms, its knee 16 dB wide from -11 to +5 VU at 10:1, then a look-ahead limiter that holds every sample under -1.5 dBFS and passes everything below it bit for bit. `Aculele.Levels` checks the readings and that velocity-127 chords stay under -1 VU. The demo phrases in `Docs/audio/midi/` come from `Tools/write_preset_midi.py`.

## Known gaps

- **No ukulele body has been measured.** The radiation and bridge banks are a nylon flamenca guitar scaled down by the equal-thickness plate law. Playing a real ukulele recording's arpeggio (an unidentified instrument) with the Finger and the guitar contour off, the soprano's third-octave spectrum lies within 2 dB of the recording's from 250 to 630 Hz and 7 dB under it at 800 Hz. It is still 2-10 dB too bright at 1-2 kHz, against 3-15 dB before 0.4.13. Between 1 and 4.4 kHz the bank's modes are spread at the one measured ukulele's density, but only within a sixth of an octave of each. So the band has 37-47 of that ukulele's 57 modes, and keeps gaps where the bank's modes lie further apart. Its level follows its own mean, not a measured ukulele's. A top thinned to the thicknesses builders give would add at most 1.4 times as many modes (`Docs/plans/ukulele-realism-plan-2026-10-27.md`).
- **Some notes are weaker than their neighbours, and the banks decide which.** Played alone at velocity 90, the 52 notes from the open strings to the twelfth fret span 7.2-10.7 dB in level over their first second on four presets (K-weighted). On Concert Strum the span is 9.5 dB, with a standard deviation of 2.5 dB. The largest step between neighbouring frets is 6.2 dB, from the A string's seventh fret to its eighth. The largest gaps come from the scaled guitar bank itself:
  - between the air resonance and T1, the top radiates little, so the C string's first five frets play 5-8 dB under the open A;
  - just above T1 it radiates little again, and on Concert Mahogany a mode near 659 Hz rocks the bridge. That mode's conductance reaches 0.32 s/kg at the A string and 0.19 s/kg at the G, against 0.02 s/kg at the C, and it radiates little. The A string's third to seventh frets play 3-7 dB under the open A, and E5's fundamental on the A and G strings starts about 12 dB under the fret above.

  A small top under a ukulele's wide saddle rocks more readily than a guitar's, and real tops have such notes. How deep a ukulele's are would take a measured body.
- **One channel chooses the voicing.** On one MIDI channel, a three-note strum's added string is the hand's cheapest chord tone, and the shape follows the notes given and where the hand already is. That is not always the voicing a player would choose. From a fresh hand on Standard, 10 of the 401 four-note voicings a ukulele chord book lists, and 108 of 288 close triads, strum a shape the book does not list for that chord (`Docs/decisions.md`, 2026-10-28):
  - E4 G4 B4 plays 0402, where most players fret Em as 0432, since both are in first position, where open strings lead;
  - D4 F#4 A4 C5 straight after a G at the seventh fret plays D7 as 5220, near where the hand was, where chord books show 2223.

  A Finger chord of three plays three strings. Tab input names every string; every demo but the Low G picked melody uses it.
- **The stiffest notes' upper partials are placed loosely.** Two allpass sections place a string's partials, and they cannot follow the stiffest notes: on nylon the C string misses one of its first twelve partials by 10 cents from the tenth fret and by about 30 from the fifteenth (the concert's, the worst of the three bodies, at 48 kHz), the low G from the eleventh fret and the E string from the fourteenth. About 110 of the 1,368 notes of every body, string set, tuning, string and fret miss by more than 10 cents. The fundamental is exact.
- **A stiff note can still overrun a small buffer now and then.** The search for a stiff note's upper partials takes 3.5-9 ms in one 256-sample batch, which lasts 5.3 ms at 48 kHz, on the development machine. Its first pluck finds the design made ahead (`AculeleEngine::prepareDispersion`), and a re-pluck continues from the last design in about 0.5 ms. The search still runs at the note in two cases: a note played before its design is ready, within 0.8-3.5 s of a preset loading or its body, strings, tuning or rate changing (up to 7 s at 44.1 kHz with the Pick, Strum or Thumb), while each batch can also carry up to about 2 ms of a design being made; and about one re-pluck in 36 of such a note, whose continued design would miss by more than a cent beyond the last search. Every shorter search tried so far left some notes several cents further out.
- **The nylon tenor keeps the concert set, and notes beside the top's main resonance start out of tune.** Near that resonance a note's two polarisations part. The one normal to the top is pulled by the body; the one parallel to it hardly is. The string has one length for both, and it is tuned to the one that lasts, the parallel one, as fixed frets would leave it. From the open strings to the twelfth fret on every body and string set (312 notes, peak pitch as `Aculele.Tuning` reads it), notes just below the resonance start up to 13 cents flat and notes just above it up to 10 cents sharp over their first 0.4 s. That onset follows the normal polarisation's pull, with a correlation of -0.6 on the concert and tenor. After a second, all but 10 notes lie within 4 cents; the worst, the nylon tenor's E4 and F4 on the C string, are 6 cents flat. The bridge tuning estimate's own error there, from the body's loss changing with frequency, is under 0.5 cents except at E5 beside the 659 Hz rocking mode. D'Addario's tenor set, EJ65T, is 1.5-3.6% thicker than EJ65C. Its thicker E string is pulled further, and the tenor's open F#4 in D tuning read 5.5 cents flat in `Aculele.Tuning` (4.0 with EJ65C), past the test's 5-cent tuner tolerance. With either set that note's first half second plays 5-7 cents flat and beats against its in-tune tail about once a second. A player would tune such an open string to the tuner's reading instead; whether the open strings should follow that reading is undecided.
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
