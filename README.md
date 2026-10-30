# Aculele

Aculele is a physically modelled ukulele: four re-entrant nylon or fluorocarbon strings on a soprano, concert or tenor body, played by finger, thumb, plectrum or a finger strum. Its engine is Acustra's, forked from Acustra 1.9.0 and re-strung, re-sized and re-voiced as a ukulele. It is DSP only for now, a JUCE-free C++17 engine with a demo renderer and analysis tools; there is no VST3, AU, CLAP or standalone build yet.

## Audio demos

The main build renders these demos once Aculele has a plug-in to build. Until then, `RenderAculelePresetDemos` renders them locally from the phrases in `Docs/audio/midi/` (see Build).

<!-- audio-demos-begin -->
<!-- audio-demos-end -->

## Version history

### 0.4.33 — 2026-10-30 — Open strings ring longer than fretted notes

On a real ukulele an open string rings on, while a fretted note, stopped by a fingertip pressed just behind the fret, dies sooner. In all five sampled ukuleles the open strings' partials ring 1.15-1.46 times as long as fretted notes' at the same frequency, and C4, open on any ukulele, 1.17-1.86 times. Aculele's rang alike (1.01-1.06). A fretted note now loses a little of every wave at its stop, 0.022 dB per round trip, fitted to those recordings; this replaces a loss every string took alike, open or not, which Acustra had chosen by ear on a guitar. Open notes now ring 1.15-1.32 times as long as fretted ones on nylon and 1.34 times on fluorocarbon, whose fretted notes come closer to the recordings' shorter ring. Fretted notes' partials die 10-17% sooner between 400 Hz and 1.5 kHz, up to 12% under the five instruments' range there. Levels were re-pinned; three presets' Output moved 0.1-0.3 dB. Adopted under the owner's standing rule to accept candidates; the owner has not heard it yet ([benchmark](Docs/validation/sample-pack-benchmark-2026-10-29.md)).

### 0.4.32 — 2026-10-30 — Nail and flesh from pluck to pluck, and a gentler vibrato near the nut

No two nail plucks meet the string alike: with more nail the string is dragged off the edge at speed, with more flesh it rolls off the pad nearly at rest. Each nail pluck (Finger, a Strum downstroke, a Thumb upstroke) now draws how much of its release the nail carries, within 1.75 octaves either way of the fitted share. Across the round robins of two sampled ukuleles, one note's power above 2.5 kHz varies by 3.6-4.0 dB (SD) and not with the take's level; Aculele's varied by 1.3-1.8 dB and now by 1.6-2.7, still independent of level, and its partials' tilt from take to take by 0.8-1.0 dB per octave where it was 0.6-0.7 (the recordings' 1.1-1.2). A repeated note now changes colour as well as place. The demos' levels move under 0.1 dB and their share above 2.5 kHz by up to 0.6 dB; Pick, a plectrum, is unchanged.

The mod wheel's vibrato now swings as far as the fret allows: 16 cents at the 1st fret, rising to the full 60 at the 6th, as Rodrigues measured the same guitarists' vibratos at the 1st fret against the 6th and 11th. A vibrato on a low fret was as wide as one up the neck. With the wheel down, nothing changes.

Both were adopted under the owner's standing rule to accept candidates; the owner has not heard them yet. A third candidate, a wider force residual that brought a note's level spread from take to take (0.7-0.8 dB) to the Leolani DI's 1.6, was dropped: no Output trim then kept Tenor Picked Melody's and Low G Tenor's Pickings within the level check ([benchmark](Docs/validation/sample-pack-benchmark-2026-10-29.md)).

### 0.4.31 — 2026-10-30 — Less CPU while strumming

When a string is plucked again, the wave it was still carrying now lets go once it has died away, where the bridge's own motion used to keep it running for as long as anything sounded. Steady strumming costs about 11% less CPU, and the dispersion design that runs after a preset loads and on new notes about 16% less. The sound moves very slightly (levels within 0.1 dB over a demo); it was adopted under the owner's standing rule to accept candidates.

### 0.4.30 — 2026-10-29 — A vibrato you can hear, and ukulele woods

The mod wheel's vibrato now swings the note both ways about its own pitch, up to 60 cents from top to bottom at 5.07 Hz. Those are the widest swing and the median rate of three guitarists' fast vibratos along a nylon string (Rodrigues, UFMG 2014), and half the wheel gives their median swing, 31 cents. It used to rise only above the note, by up to 20 cents at 1.4 Hz, which was hard to hear and raised the note's mean pitch by up to 11 cents. While the wheel is up, a note the hand can stop is fretted instead of played on an open string, which takes no vibrato: E4 goes to the C string's 4th fret. The strings' dispersion now holds through the swing, so a vibrato costs less CPU than before. With the wheel down, nothing changes on spruce or mahogany.

Wood now offers the tops ukuleles are built with: spruce, cedar, mahogany and koa. Maple, a guitar's back wood, is gone; koa takes its place in saved settings, and Tenor Picked Melody is now koa at -2.5 dB Output. Cedar is western red cedar from the USDA Wood Handbook. The handbook does not list koa, so koa takes black walnut's figures: a USDA report on Hawaiian timbers gives koa as identical to walnut in weight and strength, and its own koa samples agree.

### 0.4.29 — 2026-10-29 — Each pluck lands further from the last

The plucking hand now lands anywhere within ±0.09 of the string's length of its place, where it landed within ±0.02, the spread Acustra took from an archtop guitar's takes. Across the round robins of two sampled ukuleles, one note's partials scatter 2.4 dB (microphone) and 3.0 dB (DI) from take to take once each take's level and tilt are taken out, the mark a moving pluck point leaves; Aculele's scattered 1.2-1.4 dB and now 2.6-2.8. A note played again at one velocity changes colour from pluck to pluck as the recorded ones do, while its level moves hardly more than before (0.7-0.8 dB SD). The hand's slow drift still carries most of the travel, so quick notes stay close together and a strum's strings share their stroke's place; every technique takes the wider spread. It was adopted under the owner's standing rule to accept candidates, with a gentler ±0.06 rendered beside it. Three preset trims moved by 0.1-0.2 dB to keep typical playing at -9 VU ([benchmark](Docs/validation/sample-pack-benchmark-2026-10-29.md)).

### 0.4.28 — 2026-10-29 — Upper partials beat, as real ukuleles' do

A string's two planes of vibration, normal and parallel to the top, now sound 2 cents apart: the normal plane's loop is that much shorter. Until now they shared one loop and only the bridge told them apart, so a partial's two members lay a fraction of a cent apart and the 2nd to 4th partials died away without beating. In five sampled ukuleles, those partials swell and fade within two seconds in 22-100% of notes, through a DI as much as through microphones; Aculele's did in 0-24%, and now do in 36-96% across the three bodies. The parallel plane, which lasts and which the tuning follows, keeps its pitch. What sets a real string's planes apart is not identified, so the split is not derived: 3.5 and 7 cents matched the recordings, and the owner chose 2 cents by ear, which still does. Four preset trims moved by 0.1-0.2 dB to keep typical playing at -9 VU ([benchmark](Docs/validation/sample-pack-benchmark-2026-10-29.md)).

### 0.4.27 — 2026-10-29 — A softer thumb lets go as a pad does

Under Thumb, the pad's edge was taken as smaller the harder the stroke, so a soft note let the string go far more slowly than the string's own speed asks: on Concert Strum an E4 at velocity 16 let go over 232 samples, 1.6 of its periods. That slow release put upper harmonics back rather than taking them away. From velocity 127 to 16 the E4's H2 rose 9 dB and its H5 10 dB against the fundamental, and soft G4, C5 and E5 notes came out 4.6-7.5 dB brighter at 2.5-6 kHz than loud ones. The thumb's pad now takes the fingertip's law, a contact that grows with force, and that E4 lets go over 18 samples. Its H2 and H5 rise 5 dB, its 2.5-6 kHz share falls at every step from velocity 127 to 32, and G4 and C5 end within 2.3 dB of their loud brightness. Velocity 127 renders the same samples, and every velocity's level stays within 0.2 dB. Only the pad changes: Thumb's downstrokes and single notes. Between soft and full, a note now darkens more gradually, so a medium stroke is slightly brighter than before: Soprano Thumb's demo, played at velocity 72-96, is 0.2 dB brighter at 2.5-6 kHz. It was adopted under the owner's standing rule to accept candidates, before the owner had heard it. The Thumb's typical playing read up to 0.3 dB louder; its level table was re-pinned, and no preset trim moved.

### 0.4.26 — 2026-10-29 — A retuned string's pull on the bridge eases in

When Shape, Strings, Tuning or the host's master tune retunes a ringing string, the force it presents to the bridge follows its new tension over the 6 ms its pitch takes. That follow set off at full speed, a corner in the force on the saddle, and the piezo and the body heard the corner as a click. It now eases in and out through two stages at the same rate. Under a held G4 on Concert Piezo Stage, a Shape change clicked 25 dB under the note's peak above 5 kHz and now stands 65 dB under it. At 48 kHz, a C chord on each of the seven presets, the loudest flash on any Shape, Strings or Tuning change went from 38 to 42 dB under the chord's peak, and the loudest Shape change from 38 to 50. A few got slightly nearer the chord, the most a Tuning change on Soprano D Tuning, by 3 dB, to 50 dB under. Bends and re-plucks follow the same two stages, so playing changes too, but only 80-92 dB under each demo's level.

### 0.4.25 — 2026-10-29 — Shape and Strings restring every string at once

Each string kept its length and tension in a cache that did not know the body or the string set. After a Shape or Strings change, a string not being played kept the old construction's tension until it was plucked again, so it rang along with the chord at the old stiffness and pulled on the bridge with the old impedance: from Concert to Tenor, the idle C string stayed at 34 N where a tenor's carries 47 N. A held note took the new tension a moment after the change, all at once, without the 30 ms glide the change itself gives, and its sound above 5 kHz flashed. Every string now takes the new construction with the change. At 48 kHz, a C chord on each of the seven presets, the loudest Shape flash stood 13 dB under the chord's peak and now stands 38 dB under it; every Shape change now stands 38-52 dB under. Strings changes stay within 3 dB of where they were, 41 dB or more under. Wood, Tuning and the host's master tune render the same bytes, and so do the seven demos.

### 0.4.24 — 2026-10-29 — Less of a flash when Strings or Shape change under a ringing chord

Changing Strings, Shape or Tuning restrings the ringing chord, and its sound above 5 kHz flashed for a moment. Two causes are gone. A change that drops the second of the two sections placing a stiff string's partials cut that section out at full strength, a step in the wave; it now fades to nothing first and then leaves without one. And a retuned string's sections now follow over 30 ms, the time its delay takes to settle, not one trip along the string, which was fast enough to ring its upper partials. At 48 kHz on three presets, the worst flash stood 25 dB under the chord's peak and now stands 31 dB under it. On Concert Strum, nylon to fluorocarbon went from 27 to 52 dB under, and on Concert Piezo Stage a Shape change from 25 to 41. Two changes back to a preset's own strings now stand 2-4 dB nearer the chord than before, 41 dB or more under it. Playing is unchanged: six demos render to the same bytes, and Tenor Picked Melody differs only 100 dB under its own level.

### 0.4.23 — 2026-10-29 — No tick when Wood or Master Tune changes under a ringing chord

Changing Wood, Tuning or the host's master tune while strings rang made a 1 ms tick above 5 kHz. The bridge's motion was carried across a step where the strings meet it, and nothing steps there: the strings' ports follow their new tension, and a rebuilt bridge fades its load. At 48 kHz on three presets, Wood's tick stood 39-53 dB under the chord; it is now 65 dB or more under it, as is Master Tune's, at 44.1, 48 and 96 kHz. Tuning's tick is up to 11 dB softer. Playing is unchanged.

### 0.4.22 — 2026-10-29 — A strum's added string counts in the hand's stretch

On one channel, a three-note strum frets its free string too, and that fret now counts in the stretch by which the shape is chosen. F4 G#4 B4 strums Fdim as 4-5-4-2, the chord books' shape, not 1-5-1-2, which spans four frets. Of 288 close triads, 181 now take a chord-book shape (180 before), and none that did lost it. Tab input and the demos are unchanged.

### 0.4.21 — 2026-10-29 — The nylon tenor takes tenor strings

On nylon the tenor now takes D'Addario's tenor set, EJ65T, in place of the concert set. That follows the owner's 0.4.14 choice that each Shape takes the set its maker sells for it. The nylon tenor had waited for the tuning check to read the settled pitch, which it has done since 0.4.20. EJ65T's strings are 1.5-3.6% thicker. Across the tenor's range, no note's level moves by more than 0.26 dB, and no note's pitch by more than 1.4 cents. Low G Tenor is the one preset on these strings.

The tenor's level match was re-pinned on the new strings, which also moves the fluorocarbon tenor. Four preset trims moved by 0.1 dB. Every preset except Tenor Picked Melody (-8.90) now reads within 0.04 dB of -9 VU.

### 0.4.20 — 2026-10-29 — The tenor's air resonance rings where real tenors' do

The tenor's soundhole is now 2 5/8 in (66.7 mm), the size a builder's published dimensions give a tenor, in place of an estimated 54 mm. Its air resonance moves from 190 to 212 Hz. Three real tenors ring at 211-214 Hz: a builder's two, tapped at the bridge, and the knock recorded on the CC0 FreePats tenor.

At 190 Hz the resonance sat 45 cents under Low G's open G3. On Low G Tenor that note played 5.8 dB over the median of the notes from G3 to C4, and died sooner than the notes above it. The resonance now lies between G#3 and A3, which it lifts by 3-4 dB, and G3 no longer dies early. In standard tuning every note lies above the resonance. The soprano and concert are unchanged.

This was adopted under the owner's standing rule to accept candidates, before the owner had heard it. Three tenor preset trims moved by 0.1-0.9 dB to keep typical playing at -9 VU.

### 0.4.19 — 2026-10-28 — The bodies fall above 1 kHz as a measured ukulele does

Above 1 kHz every body now loses level towards the treble at the rate measured on a real soprano ukulele: 3.2 dB per octave from 1 to 6.3 kHz. The measurement is sound pressure per newton at the bridge, averaged over 18 microphone positions (Labraña, Gajardo, Correa and Espinoza, Zenodo, 2026, CC BY 4.0). The scaled guitar bank fell 1.2 dB per octave on the soprano, 2.1 on the concert and 2.4 on the tenor, so the bodies sounded too bright.

Three gentle high shelves now cut, at 2 kHz, 4 kHz and from 8 kHz:

- the soprano by 2.3, 4.6 and 6.9 dB;
- the concert by 1.3, 2.5 and 3.8 dB;
- the tenor by 0.9, 1.8 and 2.7 dB.

Against a real ukulele recording's arpeggio, the soprano was 3-5 dB too bright from 3 to 6 kHz; it now lies within 3 dB of it. Below 1 kHz, and in the piezo's own signal, nothing changes. Mahogany and maple take their body's cut. Only the soprano is measured; the concert and tenor are assumed to fall as it does.

This was adopted under the owner's standing rule to accept candidates, before the owner had heard it. The level table was re-pinned and no preset trim moved. The measurement and a CC0 tenor recording are compared in [`Docs/validation/recording-comparison-2026-10-28.md`](Docs/validation/recording-comparison-2026-10-28.md).

### 0.4.18 — 2026-10-28 — Soft nail and pick notes keep their level

A nail or plectrum now keeps its own edge size at every velocity, so a softer stroke lets the string go a little more slowly only because the string moves more slowly. Before, the edge was taken as larger the softer the stroke, so the string slipped off it over most of a period at velocity 20, and soft nail and pick notes fell far under the level the velocity asks for. An E4 at velocity 20 played 30 dB under velocity 127 on the nail and 23 dB on the pick, against the 16 dB that half the force at half the velocity gives. It now plays 16-18 dB under, and soft notes still come out darker. Notes above velocity 60 change by under 1 dB. This affects Pick, Finger's nails, Strum's downstrokes and Thumb's upstrokes. It was adopted under the owner's standing rule to accept candidates, before the owner had heard it. Three preset trims moved by 0.1-0.3 dB to keep typical playing at -9 VU.

### 0.4.17 — 2026-10-28 — Fingerstyle plucks with the nails

Under Finger, the thumb and fingers now meet the strings with their nails, the hard contact a strum's downstroke and a thumb's upstroke already use, as a classical player's do. Over a note's first 10 ms, a real ukulele recording's 2-8 kHz sits about 10 dB under its fundamental. The fingertip left Aculele's 18-24 dB under on Low G Tenor and 13-21 dB on Tenor Fluorocarbon Fingerstyle, and nothing measured gives a fingertip more. The nail brings 5-13 dB of that back, so each note starts with a clearer, brighter pluck. This was adopted under the owner's standing rule to accept candidates, before the owner had heard it. Two preset trims moved by 0.2-0.3 dB to keep typical playing at -9 VU.

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
- **Strings from measurement, not fitting.** Plain nylon or fluorocarbon monofilaments use the set each maker sells for the body: D'Addario EJ65S on the soprano, EJ65C on the concert and EJ65T on the tenor; Worth CM on the soprano and concert and CT on the tenor; and Worth's unwound low G. The finger lets every string go at one force, the force that displaces a string at the concert nylon set's mean tension (about 37.7 N) as far as the stroke was calibrated, so a string's displacement goes as 1/T against it. Tension comes from T = μ(2Lf)². Stiffness, bending loss and air loss come from Woodhouse and Lynch-Aird, Acta Acustica 105 (2019) 516, with their stress-dependent E′ and E″ for each material. See `ukuleleStringLoss` in `Source/DSP/AculeleEngine.cpp`.
- **Bodies.** The soprano, concert and tenor bodies use Kala's published dimensions and scale lengths, through Acustra's own coupled-oscillator morph. Kala publishes no soundhole: the soprano's and concert's, 48 and 51 mm, are estimates, and the tenor's is the 2 5/8 in (66.7 mm) a builder's published dimensions give a tenor. Measured with `Tools/AculeleBodyModes.cpp`, A0/T1 sit at 266/653 Hz (soprano), 215/485 Hz (concert) and 212/415 Hz (tenor) in spruce. Three real tenors ring at 211-214 Hz.
- **Woods.** Wood sets the top: Sitka spruce, western red cedar, Honduras mahogany or Hawaiian koa, each a quarter-cut plate by its 12% density and elastic constants in the USDA Wood Handbook (FPL-GTR-282). Koa is not in the handbook; it takes black walnut's figures, which Skolmen (USDA PSW-8, 1974) gives as identical to koa's in weight and strength, and which match his koa measurements (12% specific gravity 0.55; bending stiffness 10.8 GPa against walnut's 11.6). Against spruce, mahogany lowers T1 4%, koa 3% and cedar 1-2%. The lighter cedar top radiates more and lets the bridge move more, the denser koa less (`woodSpecs` in `Source/DSP/AculeleEngine.cpp`).
- **A ukulele's mode density between 1 and 4.4 kHz.** Elie et al. (Forum Acusticum 2011) counted about 57 modes there on one ukulele's top, against the 18-25 the scaled bank places, and measured its mean mobility flat. Each bank mode in the band with a Q above 4.3 becomes a cluster at that density, spread over at most a sixth of an octave either side and up to halfway to its neighbours, with random places and phases from a fixed seed. Each cluster's power is then scaled by the band's mean over its own power within a sixth of an octave, so the band follows its mean (`buildBodyConfiguration` in `Source/DSP/AculeleEngine.cpp`, `Aculele.PlateBand`).
- **Bridge level from a measured ukulele.** Elie, Gautier, David and Curtit (Forum Acusticum 2011) measured one ukulele's bridge at about 0.009 s/kg characteristic mobility from 1 to 6 kHz, with its usual rosewood bridge. Concert Mahogany's C string reads that between 1 and 6 kHz, and the other bodies are within 10% of it (`Tools/AculeleBridgeMobility.cpp`, `Aculele.BridgeMobility`). It replaces Acustra's steel-string correction, which left the bridge about a quarter as mobile. The piezo's saddle sits on the same bridge: its conductance, 0.009 s/kg, damps the pickup element's 6 kHz resonance to a +2 dB rise, where the guitar bridge Acustra's piezo assumed left a +11 dB peak (`PiezoDesign` in `Source/DSP/AculeleEngine.h`).
- **Radiation's fall from a measured ukulele.** Labraña, Gajardo, Correa and Espinoza measured the sound pressure per newton at the bridge of a soprano ukulele: an Engelmann spruce top on a printed PLA+ body, at nine angles in each of two planes (Zenodo, doi:10.5281/zenodo.22692775, 2026, CC BY 4.0). From 1 to 6.3 kHz it falls 3.2 dB per octave. Three high shelves on the summed microphone pressure bring each spruce body to that slope. They sit at 1.41, 2.83 and 5.66 kHz and are flat again from 8 kHz (`radiationTiltShelfDb` in `Source/DSP/AculeleEngine.cpp`, `Tools/AculeleRadiation.cpp`, `Aculele.Radiation`). The other woods take their body's cut and fall 2.6-3.3 dB per octave.
- **Geometry.** The saddle spacing is 13.3 mm, which sets the bridge lever arms and the strum's travel. Pluck distances and the finger's contact width keep their fraction of the shorter string. The hand's band reaches 0.46 of the string, where a ukulele is strummed over the neck joint.
- **Strum technique.** `PickingTechnique::Strum` is the index finger swept down and up over the neck joint, with the plectrum's sweep timing. A downstroke strikes with the nail, which the string slides off as it does a plectrum, so it takes the Pick's hard contact; an upstroke strikes with the pad, the fingertip's soft contact (MasterClass, "How to Strum a Ukulele"). A nail stroke puts about 7 dB more of its energy at 3-8 kHz than the pad's.
- **Thumb strokes.** Under `PickingTechnique::Thumb` a chord's downstroke meets the strings with the thumb's pad and its upstroke with the thumbnail, the Pick's hard contact, as a ukulele thumb strum leads with the back of the thumb going up (Bruce Watson, "Strumming"). Single notes are downstrokes and keep the pad.
- **The hand's swing.** With no direction keyswitch, the hand moves down on each eighth and up between, and keeps that time through the strokes it skips: a stroke an odd number of eighths after the last is the other direction, an even number the same. A gap of an odd number of sixteenths puts the hand in sixteenths for a bar. After 4 beats or 2 s without a stroke, it starts again with a downstroke (`Performer::pendulumUpstroke`).
- **Chucks.** While F0 is held, every swept stroke is a chuck: the fretting hand lies across the strings instead of fretting them, and each string the stroke crosses takes GuitarSet's muted-string stroke, 4.6 dB down and dying in 43 periods. The stroke's keys own no string. A chord whose keys come up as the chuck's go down is let go first, so back-to-back chords in a sequencer chuck too, and a string the sustain lets ring takes the dead stroke as well; only a key still held keeps its string.
- **Vibrato.** The mod wheel is the fretting hand's vibrato along the string: it stretches the string, so it moves the pitch and the string's tension together (Erkut et al., AES 2000). It swings both ways about the note, as the axial vibratos three classical guitarists played on a nylon string did (Rodrigues, "O vibrato no violão", UFMG 2014), at their fast vibratos' median rate, 5.07 Hz. The wheel sets the depth: the full wheel is the widest swing they measured, 60 cents from top to bottom at the 11th fret, and half the wheel their median, 31 cents. It sets in over half a second. An open string takes none, so with the wheel up the hand frets a note it would otherwise play open (`vibratoSemitones` in `Source/DSP/AculeleEngine.cpp`).
- **Fretting pitch.** Each contact's pitch differs a little within GuitarSet's bounds. A fretted string is stretched by the finger, and the pitch it gains scales with the string's axial stiffness over its tension, and inversely with the scale length (Varieschi and Gower, Am. J. Phys. 78 (2010) 47). Acustra's steel figure scaled this way gives nylon 2.0-3.4 cents on the concert, 4.2 on the soprano's C string and 1.6-2.2 on fluorocarbon (`AculeleEngine::frettingPitchHalfWidth`).
- **No two plucks alike.** Every pluck but a strummed chord's, which has its own variation, draws its own force within ±1 dB (0.58 dB SD), below the 0.65 dB median spread of a real guitarist's takes of one note (the CC0 Shinyguitar archtop's round robins). It also draws its own release angle, about 2° apart from pluck to pluck, which is one harpist's spread in Chadefaux et al., JASA 131 (2012) 844, Table VI. Within the hand's two-second memory on a string, a changed velocity keeps its accent exact. Each pluck lands within ±0.09 of the string's length of where the hand is, which reproduces how much two sampled ukuleles' partials scatter between takes of one note (`takeSpread` in `Source/DSP/AculeleEngine.cpp`).
- **Fingerstyle.** Under Finger the thumb plucks the G string and the fingers the other three, all with their nails: the Pick's hard contact, which a nail's edge lets the string slide off as a plectrum does (`AculeleEngine::nailStroke`). Where each lands and how hard stay Finger's.
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
- **Open and stopped strings.** An open string ends on the nut; a fretted one ends at the fret with the fingertip pressed just behind it, and loses 0.022 dB of every wave there, so fretted notes die sooner than open ones as five sampled ukuleles' do. The loss is fitted to those recordings; no fingertip has been measured (`fretStopLossDb` in `Source/DSP/AculeleEngine.cpp`).
- **Two planes, two pitches.** A string's plane normal to the top takes a loop 2 cents shorter than the parallel one, so its partials beat in the first seconds as five sampled ukuleles' do. The owner chose the size by ear, and it matches those recordings' beating; what sets a real string's planes apart is not identified (`normalPlaneDelayRatio` in `Source/DSP/AculeleEngine.cpp`).
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
| 6 | Tenor Picked Melody | Tenor, koa, fluorocarbon, Low G, mono mic | Pick |
| 7 | Concert Piezo Stage | Concert, spruce, nylon, Standard, piezo | Strum |

Levels are set on Reason's channel meter, a VU meter read with 0 VU at -12 dBFS RMS (`Tools/ReasonMeter.h`). `Source/DSP/LevelMatchData.h`, which `AculeleLevels --write` measures, brings typical playing on every shape, wood, Picking and Capture to -9 VU at 0 dB Output, and each preset's Output trim brings its own there. Main and the Piezo output end in the portfolio's shared output stage, `Source/DSP/OutputStage.h`: a guard compressor on the louder channel's power over 300 ms, its knee 16 dB wide from -11 to +5 VU at 10:1, then a look-ahead limiter that holds every sample under -1.5 dBFS and passes everything below it bit for bit. `Aculele.Levels` checks the readings and that velocity-127 chords stay under -1 VU. The demo phrases in `Docs/audio/midi/` come from `Tools/write_preset_midi.py`.

## Known gaps

- **One ukulele body's radiation has been measured, and none of its modes are used.** The radiation and bridge banks are a nylon flamenca guitar scaled down by the equal-thickness plate law. Only their fall above 1 kHz follows a measured ukulele, a soprano; the concert and tenor are assumed to fall as it does. That soprano's printed body has its plate–cavity modes at 202, 324 and 374 Hz. The scaled spruce soprano has its A0 and T1 at 266 and 653 Hz. The measured body is not a commercial soprano, so its modes were not adopted. A mid-priced concert measured in a 2020 thesis rings at 222 Hz (air) and 395 Hz (its first top mode), against the scaled concert's 215 and 466-485 Hz: its top mode is 15-19% lower. Its first top mode rings higher than the measured soprano's 324 Hz on a larger box, so the two tops are built differently, and neither replaces the law.

  Playing a real ukulele recording's arpeggio (an unidentified instrument) with the Finger, the soprano's third-octave spectrum compares like this:
  - from 250 to 630 Hz it lies within 3 dB of the recording's;
  - at 800 Hz it is 6 dB under;
  - from 1 to 2.5 kHz it is 2-12 dB too bright;
  - from 3 to 6.3 kHz it is within 3 dB.

  Against the measured soprano's radiation itself, relative to each one's 1 kHz band, the model is within 2 dB from 1.6 to 4 kHz. So the recording's remaining excess there does not come from the measured body's radiation. It may lie in the excitation, the strings, or the recording's own instrument and microphone (`Docs/validation/recording-comparison-2026-10-28.md`).

  Between 1 and 4.4 kHz the bank's modes are spread at the one measured ukulele's density, but only within a sixth of an octave of each. So the band has 37-47 of that ukulele's 57 modes, and keeps gaps where the bank's modes lie further apart. Its level follows its own mean, not a measured ukulele's. A top thinned to the thicknesses builders give would add at most 1.4 times as many modes (`Docs/plans/ukulele-realism-plan-2026-10-27.md`).
- **Some notes are weaker than their neighbours, and the banks decide which.** Played alone at velocity 90 on 0.4.21, the 52 notes from the open strings to the twelfth fret span 9.4-10.6 dB in level over their first second (K-weighted) on Concert Strum, Soprano D Tuning, Tenor Fluorocarbon Fingerstyle and Soprano Thumb. On Concert Strum the span is 10.6 dB, with a standard deviation of 2.6 dB. The largest step between neighbouring frets is 6.2 dB, from the A string's seventh fret to its eighth. The largest gaps come from the scaled guitar bank itself:
  - between the air resonance and T1, the top radiates little, so the C string's first five frets play 3-5.5 dB under the open A;
  - just above T1 it radiates little again, and on Concert Mahogany a mode near 659 Hz rocks the bridge. That mode's conductance reaches 0.32 s/kg at the A string and 0.19 s/kg at the G, against 0.02 s/kg at the C, and it radiates little. The A string's third to seventh frets play 4.5-7.5 dB under the open A, and E5's fundamental on the A and G strings starts about 12 dB under the fret above.

  A small top under a ukulele's wide saddle rocks more readily than a guitar's, and real tops have such notes. How deep a ukulele's are would take a measured body.
- **One channel chooses the voicing.** On one MIDI channel, a three-note strum's added string is the hand's cheapest chord tone, and the shape follows the notes given and where the hand already is. That is not always the voicing a player would choose. From a fresh hand on Standard, 10 of the 401 four-note voicings a ukulele chord book lists, and 107 of 288 close triads, strum a shape the book does not list for that chord (`Docs/decisions.md`, 2026-10-28 and 2026-10-29). For 75 of those triads, no shape the book lists sounds those three pitches. Of the voicings a player would fret otherwise:
  - E4 G4 B4 plays 0402, where most players fret Em as 0432, since both are in first position, where open strings lead;
  - D4 F#4 A4 C5 straight after a G at the seventh fret plays D7 as 5220, near where the hand was, where chord books show 2223.

  A Finger chord of three plays three strings. Tab input names every string; every demo but the Low G picked melody uses it.
- **Changing Shape, Strings, Tuning or the host's master tune under a ringing chord still flashes briefly.** The strings are restrung under their waves. Above 5 kHz the sound then rises at once and dies away within about 0.2 s. At 48 kHz, a C chord on each of the seven presets, that flash stands 42-75 dB under the chord's peak (13-65 dB before 0.4.25, 38-66 before 0.4.26), and the master tune's 55-71 dB under. The loudest, Concert Piezo Stage changed back from fluorocarbon to nylon, is not traced. A Wood change flashes only 66 dB or more under the chord (`Aculele.ConstructionChange`).
- **A string let go slowly still keeps some brightness it should lose.** A slow release takes a note's upper harmonics away, but the released wave is then shifted so the string's line starts at rest, and the loop it feeds runs about 13% longer than that line. The shift puts some of those harmonics back, more the slower the release, and the pad's wide contact shape does the same. On Concert Strum's Thumb, from velocity 127 to 16, an E4's H2 and H5 still rise 5 dB against its fundamental, most notes' 2.5-6 kHz share rises again, by up to 2.3 dB, from velocity 32 to 16 (`Aculele.ThumbDynamics` checks the E4 down to 32), and an E5 grows brighter at every step below velocity 112, 5.8 dB in all. The nail shows it too: a Strum downstroke's E4 at velocity 16 has its H2 5 dB higher against its fundamental than at 127. Filling the loop's filters from the released wave would remove the shift. A release without the lag, its corner only, was tried and made Thumb and Pick worse.
- **The stiffest notes' upper partials are placed loosely.** Two allpass sections place a string's partials, and they cannot follow the stiffest notes: on nylon the C string misses one of its first twelve partials by 10 cents from the tenth fret and by about 30 from the fifteenth (the concert's, the worst of the three bodies, at 48 kHz), the low G from the eleventh fret and the E string from the fourteenth. About 110 of the 1,368 notes of every body, string set, tuning, string and fret miss by more than 10 cents. The fundamental is exact.
- **A stiff note can still overrun a small buffer now and then.** The search for a stiff note's upper partials takes 3.5-9 ms in one 256-sample batch, which lasts 5.3 ms at 48 kHz, on the development machine. Its first pluck finds the design made ahead (`AculeleEngine::prepareDispersion`), and a re-pluck continues from the last design in about 0.5 ms. The search still runs at the note in two cases: a note played before its design is ready, within 0.8-3.5 s of a preset loading or its body, strings, tuning or rate changing (up to 7 s at 44.1 kHz with the Pick, Strum or Thumb), while each batch can also carry up to about 2 ms of a design being made; and about one re-pluck in 36 of such a note, whose continued design would miss by more than a cent beyond the last search. Every shorter search tried so far left some notes several cents further out.
- **Notes beside the top's main resonance start out of tune.** Near that resonance a note's two polarisations part. The one normal to the top is pulled by the body; the one parallel to it hardly is. The string has one length for both, and it is tuned to the one that lasts, the parallel one, as fixed frets would leave it. From the open strings to the twelfth fret on every body and string set (312 notes, peak pitch as `Aculele.Tuning` reads it), notes just below the resonance start up to 13 cents flat and notes just above it up to 10 cents sharp over their first 0.4 s. That onset follows the normal polarisation's pull, with a correlation of -0.6 on the concert and tenor. After a second, all but 10 notes lie within 4 cents; the worst, the nylon tenor's E4 and F4 on the C string, are 6 cents flat. The bridge tuning estimate's own error there, from the body's loss changing with frequency, is under 0.5 cents except at E5 beside the 659 Hz rocking mode. The nylon tenor took the concert set, EJ65C, until 0.4.21. With D'Addario's tenor set, EJ65T, the tenor's open F#4 in D tuning read 5.5 cents flat in `Aculele.Tuning` (4.0 with EJ65C), past the test's 5-cent tuner tolerance. The test then read 0.3-1.3 s after the pluck. Since 0.4.20 it reads 1-2 s, the pitch a tuner settles on, where every open string lies within 3.1 cents. In Standard tuning EJ65T moves no note's pitch by more than 1.4 cents, open to the twelfth fret. That note's first half second plays 5-7 cents flat and beats against its in-tune tail about once a second. A player would tune such an open string to the tuner's reading instead; whether the open strings should follow that reading is undecided.
- **The nylon strings are stiffer than five sampled ukuleles' strings.** Measured alike on five Pianobook sample packs (`Tools/compare_sample_packs.py`), those instruments' inharmonicity sits with Aculele's fluorocarbon, register by register, and at a third to a half of its nylon: about 2e-4 on the C string against the nylon concert's 8e-4. The nylon figure is right for D'Addario's EJ65 sets by Woodhouse and Lynch-Aird's measurements; the recorded instruments' strings are not stated. Their partials above 1 kHz still die as fast as the nylon's, where fluorocarbon's ring 1.2-1.4 times longer (1.4-1.6 before fretted notes lost at their stop in 0.4.33), so neither set matches them on both counts. By the owner's choice, five of the seven presets keep nylon, and fluorocarbon is the player's choice on Strings ([benchmark](Docs/validation/sample-pack-benchmark-2026-10-29.md)).
- **Real takes of one note differ more in brightness and level than Aculele's.** Between the round robins of two sampled ukuleles, a note's power above 2.5 kHz varies by 3.6-4.0 dB (SD) and its level by 1.6-2.5 dB; Aculele's by 1.6-2.7 and 0.7-0.8 dB. The recorded takes' brightness does not follow their level, so it is not a harder or softer stroke. Since 0.4.32 each nail pluck draws how much of its release the nail carries, which closed part of the brightness gap; varying the nail's edge two octaves either way, the contact's width by 1.5 octaves, or the release angle by 8°, each moved it less. What makes up the rest is not identified. A force residual of ±3 dB brings the level spread to the DI's 1.6 dB, but no Output trim then keeps Tenor Picked Melody and Low G Tenor within the level check, so the residual stays ±1 dB ([benchmark](Docs/validation/sample-pack-benchmark-2026-10-29.md)).
- **The bridge level rests on one measured instrument.** The paper puts its accelerometer "at the base of the lowest string". That is the C string by pitch, and the G string's place by the guitar convention, which would halve the level. The C reading was chosen by ear (`Docs/decisions.md`); no measurement has decided between them.
- Pluck release, contact noise, strum speeds and finger-chord timing are Acustra's fits to guitar recordings (GuitarSet, the flat-top bank), reused unchanged. The nail's contact is GuitarSet's plectrum, and a chuck is GuitarSet's muted guitar string: no ukulele nail or chuck has been measured. There is no flesh-only fingerpicking: the fingertip's fitted release left the first 10 ms of a note 5-13 dB duller at 2-8 kHz than the nail, which the one real recording favours, so Finger plucks with the nails. The nylon fretting spread scales a steel-string estimate by stiffness; it is not a nylon measurement.
- **A fretted note's loss at its stop is fitted, not measured.** Its size, 0.022 dB per round trip, brings the open strings' ring against fretted notes' to the five sampled ukuleles' (1.15-1.32 times against their 1.15-1.46); no fingertip's impedance has been measured, and it does not change with how hard the finger presses. Fretted notes' partials from 400 Hz to 1.5 kHz now die up to 12% sooner than the shortest of those instruments'.
- The piezo's saddle and element are a guitar's: a 3.8 g bone saddle, its element resonance chosen at 6 kHz within a guitar pickup's documented 5-7 kHz. No ukulele undersaddle pickup has been measured.
- **Wood changes only the top, by species averages.** Each wood is an equivalent plate of the species' mean density and elastic constants; a real top's thickness, bracing and grain, which builders vary by wood, are not modelled, and the back and sides stay the scaled bank's. Koa's elastic ratios have never been published, so black walnut's stand in for them. There is no laminate top.
- **The wheel's vibrato is measured only at the 1st, 6th and 11th frets.** Rodrigues measured 4-16 cents at the 1st fret against 20-60 at the 6th and 11th; Aculele takes the widest swing at each and draws a straight line between the 1st and the 6th. Its rate does not follow the wheel: his slower vibratos, 0.9-3.2 Hz, are not reachable.
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
