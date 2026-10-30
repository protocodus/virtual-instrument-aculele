# Aculele — decision log

Directions chosen by ear, recorded per the A–Z listening-test convention in
the repository's `CLAUDE.md`, and the owner's product decisions. A choice made
by ear is recorded as made by ear, never written up as though a measurement
had settled it; an entry settled by a measurement says so and quotes it.

## 2026-10-30 — a re-plucked string's old wave retires once it is gone, 0.4.31 (standing rule, pending the owner's ear)

**What was decided.** A re-plucked string keeps its old wave as a second pair
of loops with a 10 ms T60, and it retired only after 80 ms below a quiet
threshold. Its reaction force includes the bridge's motion under its port, so
while anything sounded it never fell below that threshold: in continuous
strumming 3.8-4.0 such tails ran at once and none retired. The tail now also
retires eight T60s (80 ms, 480 dB of its own decay) after a contact, legato or
re-pluck arrival last fed it. On an SDK-like build (clang `-fno-builtin`, no
vectorisation, 64-frame blocks) steady strumming through every preset costs
10.8% less CPU (7.61 s to 6.79 s).

**The letters.** A: the shipping engine. B: the candidate. Two sets, the
`01-concert-strum` and `04-low-g-tenor` demos, whole-file RMS matched (B
trimmed by -0.094 and -0.066 dB). A-B nulls at -31.3 and -33.4 dB re A; the
demos' levels move under 0.1 dB, short windows within 0.6 dB. The set was not
committed.

**What it licensed.** B was adopted under the owner's standing rule that a
candidate wins unless the owner says otherwise; the owner has not listened
yet. It is not bit-identical, and is recorded as chosen by that rule, not by
ear or by a measurement of preference.

## 2026-10-29 — the wheel's vibrato swings both ways, 60 cents at 5.07 Hz, 0.4.30 (from a source, not by ear)

**What was decided.** The owner, playing the Rack device: "vibrato (mod max)
is almost inaudible". The full wheel swung the note 20 cents, only upward, at
1.4 Hz: Erkut et al.'s one player rose from the note, his slow rate was the
wheel's top, and the 20 cents were authored. Notes chosen on open strings,
which take no vibrato, made it rarer still. Measured on a held F4, the full
wheel raised the note's mean pitch by 11 cents.

**The source.** Rodrigues, "O vibrato no violão: aspectos qualitativos e
quantitativos" (MSc thesis, UFMG 2014), Experiment 2: three guitarists, axial
and transverse vibratos on the nylon 3rd and wound 4th strings at frets 1, 6
and 11, slow, moderate and fast. The axial vibrato, a classical player's and
a ukulele player's, swings both ways about the note in most takes. The
eighteen fast axial takes run at 3.1-6.1 Hz, median 5.07; at frets 6 and 11
they swing 20-60 cents from top to bottom, median 31, and the slow ones a
median 22, so the faster vibrato is the wider.

**What it licensed.** A two-sided sine at 5.07 Hz whose depth the wheel sets,
the full wheel the widest swing measured (60 cents) and half of it about the
median; Erkut's half-second onset stays. With the wheel up, `chooseString`
prefers a stopped string where it preferred an open one. The strings' loss
and dispersion designs hold at the swing's centre: re-solving them every 1.7
cents of a 60-cent swing took one stiff note from 5% to 16% of real time on
the development machine. Held at the centre, the fundamental at the swing's
peak and trough lies within 0.6 cents and partials 2-8 within 1.3 cents of a
design made there on ordinary notes; the stiffest (the nylon concert's C
string at the 14th fret) miss H5-H12 by up to 38 cents more at the trough,
where two sections already miss by 10-30. A full-wheel note now costs 4.2% of
real time, under 0.4.29's 4.3-5.3%. `Aculele.Vibrato` checks the swing, its
rate, that its mean stays on the note, that the wheel at zero changes no
sample, and the stopped strings.

**What it did not close.** At the 1st fret Rodrigues measured 4-16 cents, a
third of the swing higher up; Aculele's is the same at every fret. The slow
vibratos are not reachable from the wheel.

## 2026-10-29 — Wood offers spruce, cedar, mahogany and koa; maple is gone, 0.4.30 (owner's direction, from sources)

**What was decided.** The owner: "ensure wood is correct - there is maple,
spruce, etc, but ukuleles are from different woods usually". Wood's three
were Acustra's guitar woods. Ukulele tops are spruce or cedar over a hardwood
body, or mahogany or koa, often the whole body's; maple is a guitar's back
wood. Wood changes only the top, so it now offers those four.

**The sources.** Spruce and mahogany are unchanged. Cedar is western red
cedar's row of the USDA Wood Handbook (FPL-GTR-282, Tables 5-1, 5-2, 5-3a):
12% specific gravity 0.32 (358.4 kg/m3), bending E 7.7 GPa, ER/EL 0.081, GLR/EL
0.087, nuLR 0.378. Koa is not in the handbook. Skolmen, "Some woods of
Hawaii" (USDA Forest Service Gen. Tech. Rep. PSW-8, 1974), gives it as
identical in weight and strength with black walnut, and his appendix agrees:
three koa trees at 12% weigh specific gravity 0.55 (Table 1, Gerhards 1963),
walnut's 0.55, and koa bending specimens at 12% read 10.8 GPa (Table 2, Gerry
1955), 7% under walnut's 11.6. No orthotropic ratio of koa is published, so
koa takes walnut's handbook row whole (616 kg/m3, 11.6 GPa, ER/EL 0.106,
GLR/EL 0.085, nuLR 0.495).

**What it licensed.** `BodyMaterial::Koa` takes Maple's value 2, the closest
in density, and `BodyMaterial::Cedar` is 3. Against spruce, koa's top is 1.38
times as heavy and lowers T1 3%, and cedar's 0.80 times and lowers T1 1-2%;
cedar's bridge reads 1.29 times spruce's conductance and koa's 0.74.
`LevelMatchData.h` grew to four woods; the spruce and mahogany cells did not
move. Tenor Picked Melody, maple until now, is koa at -2.5 dB Output: its
typical playing reads -8.6 VU, as at the -2.9 dB that reads -9.0 its Strum
would read 1.7 dB under the target, past the 1.5 dB bound. Nothing here was
chosen by ear.

## 2026-10-29 — each pluck lands within ±0.09 of the string, 0.4.29 (measured; adopted under the owner's standing rule, not yet heard)

**What was decided.** How far the plucking hand lands from its place from
one pluck to the next. Acustra's ±0.02 of the string came from an archtop
guitar's takes. The round robins of two ukulele packs
(`Docs/validation/sample-pack-benchmark-2026-10-29.md`,
`Tools/compare_sample_packs.py --takes`) give a ukulele player's: once each
take's level and tilt are taken out, a note's partials scatter 2.4 dB from
take to take on Guffland's microphone and 3.0 dB on Leolani's DI. A force or
a release angle moves every partial together and a sharper release tilts
them; only a pluck point that moves scatters them one by one. Aculele
scattered 1.2-1.4 dB. Rendering the same notes at ±0.06 gives 2.1-2.4 dB and
at ±0.09 2.6-2.8 dB, between the two packs.

**The letters.** C4, E4, G4, A4 and C5 four times each at velocity 90,
0.55 s apart, through Low G Tenor (Finger, nylon, stereo microphones),
rendered with `AculeleProbe` at 48 kHz in 256-sample blocks, 13 s:

- A: the shipping engine (0.4.28), ±0.02.
- B: ±0.06.
- C: ±0.09.

The takes are level-matched on whole-file RMS. B moved by +0.03 dB and C by
+0.06 dB.

**What was chosen.** C, under the owner's standing rule to accept
candidates over the baseline, and because it is the spread whose scatter
lies between the two packs'. B, the gentler, would sit under both. The owner
has not heard the set; a choice of B by ear would replace C.

**What it licensed.** `takeSpread` = 0.09 in `AculeleEngine.cpp`, the scale
of both the hand's slow drift and each pluck's own draw. `LevelMatchData.h`
was re-pinned. Tenor Fluorocarbon Fingerstyle's Output moved from -0.6 to
-0.8 dB, Low G Tenor's from -1.1 to -1.0 dB and Tenor Picked Melody's from
-2.2 to -2.3 dB, so every preset reads -9 VU within 0.42 dB: Tenor Picked
Melody's typical playing reads -8.58, as at the -2.7 dB that would bring it
to -9.0 its Strum would read 1.8 dB under the target, past the 1.5 dB bound.

**What it did not close.** The packs' takes also differ by 3.6-4.0 dB (SD) in
their power above 2.5 kHz and by 1.1-1.2 dB per octave in tilt; Aculele's,
now 1.3-1.8 dB and 0.6-0.7 dB per octave. That brightness does not follow
the takes' level. Varying the nail's edge two octaves either way, or the
release angle by 8°, raised the brightness spread from 0.9 to 1.2 dB at
most; nothing was changed for it (README, Known gaps).

## 2026-10-29 — a string's two planes sound 2 cents apart, and the presets keep nylon, 0.4.28 (chosen by ear)

**What was decided.** Measured on five DecentSampler ukulele packs the owner
supplied (`Docs/validation/sample-pack-benchmark-2026-10-29.md`), partials
2-4 of real notes swell and fade within two seconds: after taking out each
partial's decay curve, a dip of more than 6 dB shows in 38-84% of notes at
H2, 40-86% at H3 and 22-100% at H4, through a DI as much as microphones.
Aculele's did in 4-24%, 0-8% and 4-12%. Its two planes shared one loop, and
the bridge alone parted them by a fraction of a cent. Doubling the saddle's
height and releasing half of every pluck normal to the top changed nothing;
splitting the planes' loops did. What parts a real string's planes is not
identified, so the split's size comes from matching the recordings, and two
sizes match them equally.

**The letters.** C4, E4, G4 and A4 alone, then C4 E4 G4 C5 picked and left
ringing, through Concert Strum's construction with Finger, rendered with
`AculeleProbe` at 48 kHz in 256-sample blocks:

- A: the shipping engine (0.4.26).
- B: the normal plane's loop 0.2% shorter, about 3.5 cents higher.
- C: 0.4% shorter, about 6.9 cents.

The takes are level-matched on whole-file RMS. B moved by +0.15 dB and C by
+0.35 dB.

**What was chosen.** B by ear, with the owner's own size: 2 cents, under
both B's 3.5 and C's 6.9. B was first adopted under the owner's standing
rule to accept candidates; the owner then picked B and set the split. 2
cents was not one of the letters and was not rendered for the owner before
the choice. Over the three bodies it beats in 36-44% of notes at H2, 40-64%
at H3 and 44-96% at H4, still within the recordings' range.

**What it licensed.** `normalPlaneDelayRatio` = 2^(-2/1200) in
`AculeleEngine.cpp`. `LevelMatchData.h` was re-pinned. Soprano D Tuning's
Output moved from +1.4 to +1.6 dB, Tenor Fluorocarbon Fingerstyle's from
-0.7 to -0.6 dB, Low G Tenor's from -1.2 to -1.1 dB and Tenor Picked
Melody's from -2.1 to -2.2 dB, so every preset reads -9 VU within 0.13 dB.
Tenor Picked Melody stays 0.1 dB above the -2.3 dB its typical playing asks
for, where its Strum would read 1.52 dB under the target, past the 1.5 dB
bound.

**The strings.** On the same packs the instruments' inharmonicity sits with
Aculele's fluorocarbon and at a third to a half of its nylon, while their
partials above 1 kHz die as fast as the nylon's. The same phrase was put to
the owner on nylon (A), fluorocarbon (B) and fluorocarbon at String Age 1.0
(C). The owner chose A by ear: the presets keep nylon, and fluorocarbon
stays the player's choice on the Strings control. Nothing changed.

## 2026-10-29 — a softer thumb lets go as a pad does, 0.4.27 (adopted under the owner's standing rule, not yet heard)

**What was decided.** The dynamics check of 2026-10-29
(`Docs/validation/dynamics-2026-10-29.md`) found Thumb uneven: soft notes
came back brighter. The thumb's pad took an edge radius scaled by 1/v alone,
so its release time r/u went as 1/v^2. On Concert Strum an E4 at velocity 16
let go over 232 samples, 1.6 of its periods. The slip that follows is
shifted so the line starts at rest, in a loop about 13% longer than the
line, and that shift put back the harmonics the slow release had taken. From
velocity 127 to 16 the E4's H2 rose 9 dB and its H5 10 dB against H1, and
the 2.5-6 kHz share of G4, C5 and E5 rose 5.8, 4.6 and 7.5 dB. The Finger
and Strum pads already take the force law `fingerContactForceExponent`, a
contact that grows with force as Hertz contact does (radius as F^(1/3)).
The thumb is the same flesh, so it takes the same law.

**The letters.** A thumb arpeggio, C4 E4 G4 C5, played five times at
velocity 112, 80, 56, 36 and 22, on Soprano Thumb, rendered with
`AculeleProbe` at 48 kHz in 256-sample blocks:

- A: the shipping engine (0.4.26).
- B: the thumb's pad under the Finger and Strum pads' force law.

Matched on whole-file RMS: A was moved by -0.06 dB and B by +0.06 dB. The
2.5-6 kHz share of each pass's first two seconds reads -22.4, -25.1, -23.1,
-21.3 and -21.2 dB in A, and -22.1, -24.3, -25.5, -24.9 and -24.8 dB in B.

**What was chosen.** B, without a listening verdict, under the owner's
standing rule of 2026-10-28 to lean towards accepting candidates. The page
stays up for the owner to check:
<https://claude.ai/artifact/4eBCYAhDu4t9QeFJkAszHh>. With B the E4 lets go
over 18 samples at velocity 16; its H2 and H5 rise 5 dB, and the G4 and C5
end 1.7 and 2.3 dB brighter at velocity 16 than at 127. The E5 still ends
5.8 dB brighter, and most notes brighten by up to 2.3 dB from velocity 32
to 16: the shift is still there (README, Known gaps). Between soft and full
a note darkens more gradually than before, so a medium stroke is slightly
brighter: Soprano Thumb's demo, at velocity 72-96, is 0.2 dB brighter at
2.5-6 kHz. Every velocity's level
stays within 0.2 dB, and velocity 127 renders the same samples.

**What it licensed.** `AculeleEngine::initialisePluck` gives the thumb's pad
the pads' force law, and `Aculele.ThumbDynamics` checks that its release
time ratio from velocity 127 to 16 stays between 3 and 8 and that the E4's
2.5-6 kHz share falls at every step from 127 to 32. `LevelMatchData.h` was
re-pinned, the Thumb cells by 0.1-0.3 dB; no preset trim moved. A release
without the slip's lag, its corner only, was tried and made Thumb and Pick
less even, so it was not offered.

## 2026-10-29 — a strum's added string counts in the hand's stretch, 0.4.22 (the owner's 0.4.11 direction, checked against a chord book)

**What was decided.** In 0.4.11 the owner chose a fretting rule on the
understanding that it would be refined until no voicing got worse (below).
A three-note stroke's free string already counted for its open strings and
its frets when the shape was chosen, but the hand's stretch was costed
without it. The hand frets that string too, so its fret now counts in the
stretch. No choice was made by ear, and no listening test was run.

**What it measured.** This uses 0.4.11's check against the MIT-licensed
chords-db, for evaluation only:
- **From a fresh hand.** Close triads on a book shape rose from 180 to 181
  of 288 on Standard and in D tuning. Low G stayed at 253 of 378, and the
  four-note voicings stayed at 391 of 401 and 623 of 623. The one that moved
  is F4 G#4 B4, Fdim, from 1-5-1-2, which spans four frets, to the books'
  4-5-4-2 (D tuning's Gdim likewise). None moved off a book shape.
- **After another chord.** Each of the 2,379 chord-book voicings and close
  triads, in all three tunings, was followed by four others drawn at random
  (seed 7), 9,516 changes in all. The second chord landed on a book shape
  6,781 times against 6,770: 11 moved onto one and none off one. Of the 30
  others that changed, 10 moved between two book shapes. Cm after a chord at
  the tenth fret now strums 5-3-3-6, nearer the hand, where it strummed
  0-3-3-3 with an E-string fill whose fret the hand had not paid for.

**Rejected: a fill that adds no bass.** In 32 of the 107 triads that still
miss, a book shape sounds the same three pitches. In 24 of those 32 the
model frets a fill below the lowest note given, as Abdim's 4-2-4-5 puts D4
under G#4, where the books go higher up the neck (7-8-7-5). A rule that kept
a fretted fill at or above that note, with open strings still free to sound
below it, took 128 Standard triads off a book shape and put none on one. The
books fret such a bass themselves, as Ab's 5-3-4-6 puts Eb4 on the C string
under Ab4 C5 Eb5. So the rule was not adopted. The 24 follow the 0.4.11
rules: open strings lead in first position, and above it the hand stays
lowest.

## 2026-10-29 — the nylon tenor takes its maker's tenor set, 0.4.21 (the owner's 0.4.14 choice, extended)

**What was decided.** The owner chose by ear in 0.4.14 that each Shape takes
the set its maker sells for it (letter C, below). The nylon tenor was left on
the concert set, EJ65C, because D'Addario's tenor set, EJ65T, failed
`Aculele.Tuning` in its 0.3-1.3 s window. Since 0.4.20 that test reads 1-2 s
after the pluck, and EJ65T passes it: on the nylon tenor every open string
lies within 1.3 cents in every tuning. No new choice was made by ear, and no
listening test was run.

**What changed.** The nylon tenor now takes EJ65T (.029, .041, .0327,
.0285 in). In Standard tuning, from the open strings to the twelfth fret,
each note was played alone at velocity 90 on both sets:
- **Pitch.** No note moved by more than 1.4 cents, read over 1-2 s, or by
  more than 0.5 cents over 0.1-0.5 s.
- **Level.** On Low G Tenor and on the nylon tenor in Standard tuning, no
  note's level over its first 0.5 s moved by more than 0.26 dB.

`LevelMatchData.h` was re-pinned on the new strings, which also moves the
fluorocarbon tenor. Following `AculeleLevels --report`, Soprano D Tuning moved
+1.3 to +1.4 dB, Tenor Fluorocarbon Fingerstyle -0.6 to -0.7, Low G Tenor -1.1
to -1.2, and Soprano Thumb -0.1 to 0.0.

Tenor Picked Melody stays at -2.1, where it reads -8.90 VU. At the suggested
-2.2 dB, its Strum would read 1.51 dB under the target, past the check's 1.5
dB bound.

**What it depends on.** This rests on the 1-2 s tuning window, which went to
the owner for a yes or no with 0.4.20. The owner kept it on 2026-10-29
(the 0.4.20 entry, below).

## 2026-10-29 — the tenor's air resonance rings where real tenors' do, 0.4.20 (adopted under the owner's standing rule, not yet heard)

**What was decided.** This was settled by measurements, not by ear. Kala
publishes no soundhole, and the tenor's 54 mm was an estimate. A builder's
published dimensions give a tenor 2 5/8 in (66.7 mm; Kawika, ukuleles.com,
"Sounds"), and players report 2 1/2 to 2 5/8 in. Both are defensible. Three
tenors' air resonances decide between them: the same builder's two, tapped
at the bridge, at 211 and 214 Hz, and the knock FreePats recorded on a
Flight Fireball tenor (CC0) at 212 Hz. The coupled morph puts A0 at 207 Hz
with 2 1/2 in and at 212 Hz with 2 5/8 in, against 190 Hz with 54 mm. All
three measurements are informal, and none is of a Kala.

**The letters.** A rising and falling line of single notes from G3 to C4,
and the demo phrases of Low G Tenor and Tenor Fluorocarbon Fingerstyle,
rendered with `AculeleProbe` at 48 kHz in 256-sample blocks:

- A: the shipping engine (0.4.19), with its level table and trims.
- B: the tenor's soundhole at 66.7 mm, with its re-pinned level table and
  trims.

The takes are level-matched on whole-file RMS. B moved by -2.54, +1.41 and
+0.03 dB.

**What was chosen.** B, without a listening verdict, under the owner's
standing rule of 2026-10-28 to lean towards accepting candidates. The page
stays up for the owner to check:
<https://claude.ai/artifact/Liwe4ESdC94j6tsM1X3YdK>. On Low G Tenor, held
notes' level over 1.5 s against the median of G3 to C4 was +5.8 dB at G3,
+2.5 at G#3 and +0.4 at A3. It is now -3.3, +3.3 and +4.1 dB. G3's
fundamental T60 rose from 6.9 to 9.7 s.

**What it licensed.** The tenor's entry in `ukuleleBodies`.
`LevelMatchData.h` was re-pinned. Tenor Fluorocarbon Fingerstyle's Output
moved from -0.7 to -0.6 dB, Low G Tenor's from -2.0 to -1.1 dB and Tenor
Picked Melody's from -2.6 to -2.1 dB, so every preset reads -9 VU within
0.14 dB.

`Aculele.Tuning` now reads 1-2 s after the pluck, not 0.3-1.3 s. The tenor's
open F#4 in D tuning on fluorocarbon has two members. One starts 7.6 cents
flat (7.2 now) and is gone by 1 s. The other lasts and sits at +0.8 cents,
before and after. The old window read whichever member was stronger inside
it. At 54 mm that was the in-tune member, +0.09 cents. From 63.5 mm up it is
the flat member, -6.6 cents at 66.7 mm, past the 5-cent tolerance, although
the note settles where it did. The test's comment gives the diagnosis.

**The owner's verdict (2026-10-29).** Asked whether the check should keep
reading the settled pitch, 1-2 s after the pluck, or go back to 0.3-1.3 s
with the 54 mm soundhole restored, the owner kept the 1-2 s window. That was
a product decision on what the check reads, not a listening verdict on the
soundhole, which has still not been heard.

## 2026-10-28 — the bodies fall above 1 kHz as a measured soprano does, 0.4.19 (adopted under the owner's standing rule, not yet heard)

**What was decided.** This was settled by a measurement, not by ear. The
Labraña–Espinoza deposit (Zenodo, doi:10.5281/zenodo.22692775, 2026, CC BY
4.0) gives a soprano ukulele's sound pressure per newton at the bridge
(Engelmann spruce top, printed PLA+ body, 162 by 235 mm). Its
18-position power mean falls 3.204 dB per octave from 1 to 6.3 kHz, as the
least-squares slope of its third octaves. Its three printed tops fall
3.5-3.8. The same measure on Aculele's spruce bodies, from bridge force to
microphone, gave -1.15 on the soprano, -2.06 on the concert and -2.38 on the
tenor. A real recording had already shown the model too bright in this band:
the Duke arpeggio, 3-6 kHz, by 2.6-5.1 dB. Whether the concert and tenor fall
as the soprano does is not measured. They take the soprano's slope, which
keeps the three bodies consistent with the one measurement. The other reading would cut every body by the
soprano's own correction, which would leave the concert and tenor at -4.1 and
-4.4 dB per octave, steeper than any measured top.

**The letters.** Each preset's own demo phrase on Soprano D Tuning, Concert
Strum and Tenor Fluorocarbon Fingerstyle, rendered with `AculeleProbe` at 48
kHz in 256-sample blocks:

- A: the shipping engine (0.4.18).
- B: three RBJ high shelves (Q 0.707) at 1.41, 2.83 and 5.66 kHz on the
  summed microphone pressure. Each one cuts by 2.30 dB on the soprano, 1.25
  dB on the concert and 0.91 dB on the tenor, which brings each spruce body
  to -3.21 dB per octave.

The takes are level-matched on whole-file RMS. B moved by +0.06, -0.02 and
+0.04 dB.

**What was chosen.** B, without a listening verdict, under the owner's
standing rule of 2026-10-28 to lean towards accepting candidates. The page
stays up for the owner to check:
<https://claude.ai/artifact/XBbTjFkbJZbTcZCoaiUrww>. Against the Duke
arpeggio, the soprano now lies at -3.1 to +1.4 dB of the recording from 3 to
6.3 kHz, and at +2.3 to +12.1 dB from 1 to 2.5 kHz. Against the measured
soprano, relative to the 1 kHz band, it lies within 2 dB from 1.6 to 4 kHz.

**What it licensed.** `radiationTiltShelfHz` and `radiationTiltShelfDb` in
`AculeleEngine.cpp`, in the capture filter's slots that the guitar contour
leaves unused. `Aculele.Radiation` holds each spruce body to -3.204 within
0.15 dB per octave. `LevelMatchData.h` was re-pinned. No preset trim moved:
every preset reads -9 VU within 0.3 dB.

## 2026-10-28 — a nail or pick keeps its own edge at every velocity, 0.4.18 (adopted under the owner's standing rule, not yet heard)

**What was decided.** Measured on single E4 notes (Tenor Fluorocarbon
Fingerstyle and Tenor Picked Melody, 30-330 ms after the pluck), the
fingertip's fundamental followed the linear force law the owner chose on
2026-10-13 (velocity 20: -15.9 dB against velocity 127, the law -16.1).
The nail's and the pick's fell away below velocity 50: -30.2 and -22.5 dB
at velocity 20, -47.5 and -38.9 dB at velocity 10. The pick's edge radius
was scaled by 1/v when the force went from v^2 to v, to keep the earlier
release shape, so the slip time r/u went as 1/v^2. At velocity 20 an E4
then let go over 107 samples, 0.7 of its 146-sample period. The edge was
fitted at full velocity on 2026-09-27/28, when the force went as v^2 and
the slip time as 1/force. A fixed edge keeps that relation.

**The letters.** A line of 24 eighth notes at 90 bpm, velocity 18 rising to
127 and falling back to 18, on Tenor Fluorocarbon Fingerstyle, Tenor Picked
Melody and Low G Tenor, rendered with `AculeleProbe` at 48 kHz in
256-sample blocks:

- A: the shipping engine (0.4.17).
- B: the pick's edge radius without the 1/v scale.

The takes were not level-matched, because the soft notes' level is what is
being judged. The velocity-127 notes are bit-identical in both letters.

**What was chosen.** B, without a listening verdict, under the owner's
standing rule of 2026-10-28 to lean towards accepting candidates. The page
stays up for the owner to check:
<https://claude.ai/artifact/D9AmYce8U7XUF2rwBXyjQp>. With B the same E4
reads -18.0 dB (nail) and -16.2 dB (pick) at velocity 20, and -26.5 and
-22.7 dB at velocity 10. Its upper partials still fall further: at velocity
20 the pick's H7 is 7.6 dB further down than its H1. Velocities above 60
change by under 1 dB.

**What it licensed.** `AculeleEngine::initialisePluck` passes the pick its
fitted edge, Touch and gesture compliance only. The pad's 1/v keyboard
response is unchanged. `LevelMatchData.h` was re-pinned. Soprano D Tuning's
Output moved from +1.2 to +1.3 dB, Low G Tenor's from -1.7 to -2.0 dB and
Concert Piezo Stage's from -0.1 to 0.0 dB. Tenor Picked Melody's Output
stays at -2.6 dB, where it reads -8.7 VU: at the -2.9 that reads -9.0, its
Thumb would read 1.7 dB under the target, past `Aculele.Levels`' 1.5.

## 2026-10-28 — Finger plucks with the nails, 0.4.17 (adopted under the owner's standing rule, not yet heard)

**What was decided.** The owner heard a wooden "knock" at the start of every
note. Of the Knock Finder's cuts the owner kept A, the shipping attack. The
next grounded question was the onset's top end. Over the first 10 ms of the
Duke Sonic Dictionary's "Ukulele Arpeggio" (CC BY-NC 4.0, an unidentified
ukulele, technique unknown; compared against, never fitted), G4's 2-4 kHz
reads -10.2 dB and its 4-8 kHz -10.3 dB against the fundamental, mostly
inharmonic. Aculele's fingertip, the release fitted to the Eastman finger
takes, read -28.0 and -33.8 dB on Low G Tenor and -22.9 and -30.8 dB on Tenor
Fluorocarbon Fingerstyle. Nothing measured gives a fingertip's slip more
noise. The nail is a grounded alternative: it is the Pick's whole contact
(GuitarSet's plectrum), which a strum's downstroke and a thumb's upstroke
already take, and fingerstyle players pluck with the nail as well as with
the flesh (Ukulele Tricks, "Fingerstyle Lesson 4"; Got A Ukulele,
"Fingernails for ukulele"). On the nail the same notes read -22.8 and -23.8
dB (Low G Tenor) and -17.9 and -17.9 dB (Tenor Fluorocarbon Fingerstyle):
5-13 dB nearer the recording, still under it.

**The letters.** The two Finger presets' demos, rendered with
`RenderAculelePresetDemos` at 48 kHz in 256-sample blocks:

- A: the shipping engine (0.4.15), fingertip release.
- B: every Finger stroke, thumb and fingers, takes the nail's contact.

Each B was matched to its A by whole-file RMS: -0.22 dB (Low G Tenor) and
-0.18 dB (Tenor Fluorocarbon Fingerstyle).

**What was chosen.** B, without a listening verdict. The owner's standing
rule of 2026-10-28 is to lean towards accepting candidates, and the
measurement above favours B. The page with A and B stays up for the owner
to check: <https://claude.ai/artifact/2tZ4LziG4ZK15b4WnCYbzV>.

A variant with the thumb on its pad and the fingers on their nails was tried
and not kept. Low G Tenor's low G, under the thumb's pad, then read 2.1 dB
quieter on the piezo than on the microphones, past `Aculele.Levels`' 1.5 dB
bound. With every stroke on the nail it reads 1.4 dB under.

**What it licensed.** `PerformanceRealism::fingerNails`, on by default,
makes `AculeleEngine::nailStroke` true for every Finger stroke.
`LevelMatchData.h` was re-pinned, and Tenor Fluorocarbon Fingerstyle's
Output moved from -0.5 to -0.7 dB and Low G Tenor's from -2.0 to -1.7 dB.

## 2026-10-28 — every pluck draws its own force and angle, and every string is let go at one force, 0.4.15 (chosen by ear)

**What was decided.** The owner heard the plucks of Low G Tenor (the E
string around C5, the A string), Tenor Picked Melody and Concert Piezo Stage
as "the same every time", Tenor Picked Melody's A string as too bright and
its lower notes as not concrete. Two questions came out of that. The first
was how much each pluck should differ. A melody of different notes at one
velocity varied by 0.2 dB, because the ±1 dB force residual was drawn only on
a repeated note, and the release angle varied by under 1°. The CC0
Shinyguitar archtop's four round robins per note and layer differ by a
median 0.65 dB over 30-250 ms. Chadefaux, Le Carrou, Fabre and Daudet (JASA
131 (2012) 844-855, Table VI) give each harpist's initial polarisation angle
over five chord and seven arpeggio plucks: about 2° read as ±2 SD of the
plucks, or 4° read as intervals of their mean. The second was whether the
finger lets every string go at one displacement or at one force. At one
displacement, the fluorocarbon tenor's 66 N A string is pulled 1.7 times as
hard as its 39.5 N C.

**The letters.** Part 1 used Low G Tenor, Tenor Picked Melody and Concert
Piezo Stage:

- A: the shipping engine (0.4.14).
- B: every pluck but a strum's draws the existing mean-one ±1 dB residual.
  Within the hand's two-second memory on the same string, it fades out as
  the commanded force changes, so an accent stays exact.
- C: B, with a release angle of 4° SD about the share's own angle.
- D: B, with 2° SD.

Part 2 used Tenor Picked Melody and Low G Tenor:

- A: the shipping engine.
- B: every string is released at one force. The released displacement takes
  T_ref / T_own in place of mu_concert / mu_own, where T_ref is the concert
  nylon set's geometric-mean tension, 37.7 N.

The phrase was 100 bpm eighth notes at velocity 88: up and down the E string
through C5, then the A, C and G strings, then C5 four times and a held C5 on
the A string. It was rendered through `AculeleProbe` at 48 kHz in 256-sample
blocks. Each letter was matched to its A by whole-file RMS:

| Set | Trims to match A (B, C, D) |
| --- | --- |
| Part 1, Low G Tenor | -0.15, -0.30, -0.21 dB |
| Part 1, Tenor Picked Melody | -0.13, -0.10, -0.12 dB |
| Part 1, Concert Piezo Stage | -0.12, -0.28, -0.18 dB |
| Part 2, Tenor Picked Melody | +1.32 dB (B) |
| Part 2, Low G Tenor | +1.78 dB (B) |

The median spread of the 30-250 ms level over 20 notes:

| Letter | Low G Tenor | Tenor Picked Melody | Concert Piezo Stage |
| --- | --- | --- | --- |
| A | 0.21 dB | 0.52 dB | 0.22 dB |
| B | 0.76 dB | 0.80 dB | 0.77 dB |
| C | 1.14 dB | 1.14 dB | 0.97 dB |
| D | 0.89 dB | 0.94 dB | 0.81 dB |

**The owner listened and chose D in Part 1 and B in Part 2.**

**What it licensed.** Both ship in 0.4.15 on every Shape, string set and
Picking. In the force residual (`pluckForceGain`), every pluck but a strum's
draws ±1 dB. The release angle turns by 2° SD with either source of the
draw: the hand's posture with the pluck's own draw, or the draw alone under
MPE pressure. Against the other strings, releasing at one force moves the
nylon sets' A string by -1.8 dB, Low G Tenor's G by +2.6 dB and its A by
-2.6 dB, and Tenor Picked Melody's G by +2.3 dB and its A by -2.0 dB.
`LevelMatchData.h` was re-pinned. Four preset trims moved to keep typical
playing at -9 VU:

| Preset | Old trim | New trim |
| --- | --- | --- |
| Soprano D Tuning | -0.9 | +1.2 |
| Tenor Fluorocarbon Fingerstyle | +0.1 | -0.5 |
| Low G Tenor | -0.9 | -2.0 |
| Tenor Picked Melody | -0.8 | -2.6 |

**What it did not license.** No ukulele player's force or release angle has
been measured. The ±1 dB residual stays an authored figure under the real
spread, and the 2° is the harpists' reading that the owner chose by ear.

**Not decided by either letter.** The open low G swells over 47 ms because it
sits 45 cents above the tenor's 191 Hz air resonance. The archtop's low E does
the same beside its own air mode, in 50 ms. The fluorocarbon A string's top
octave outlasts the E's, as Woodhouse and Lynch-Aird's measured loss gives.
Neither letter changes either of these.

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

*Corrected later on 2026-10-28:* the cause given above is wrong. Worked out at
each of 312 notes (the open strings to the twelfth fret, every body and string
set), the first-order shift of the damped loop's pole that the estimate leaves
out is under 0.5 cents beside the top's main resonance. There the body pulls
the string's polarisation normal to the top and hardly the parallel one, and
the shared delay tunes the lasting parallel one. F#4's first half second is
its normal member, and EJ65T's thicker E string is pulled further. So no
correction to the estimate would let EJ65T pass, and the tenor's nylon set
keeps EJ65C on that ground (README, Known gaps).

*Extended on 2026-10-29:* the nylon tenor took EJ65T in 0.4.21, once the test
read 1-2 s (above).

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
