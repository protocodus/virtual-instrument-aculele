# Aculele against open ukulele recordings and a measured body, 2026-10-28

Two open references were found and compared against 0.4.18 and 0.4.19. Neither
is in this repository. Nothing was fitted from the recording. The radiation
measurement sets one slope, which `Aculele.Radiation` checks.

## The FreePats tenor: single notes (CC0)

FreePats' "Ukulele" set of 2026-08-11, by Mateusz Dąbrowski, is released under
CC0 1.0 (<https://freepats.zenvoid.org/GuitarFamily/ukulele.html>). Archive
SHA-256: `cb857464cc847c261d954a4bfe3610380f96ff4d2603d94573ab3529aa574775`.

The instrument:
- **Body and strings.** A Flight Fireball tenor, by its README. The maker's
  listings give a solid mango body and Worth Brown fluorocarbon strings. It is
  in Standard tuning.
- **Notes.** Thirteen single notes from C4 to C6, all at one dynamic. The
  technique, string and fret are not stated.
- **Recording.** A RØDE Podcaster at an unstated distance, high-passed at 190 Hz.
- **Processing.** Each note is level-matched over its first 300 ms and trimmed
  at twice the noise floor.

`Tools/compare_recorded_notes.py` renders the same pitches with Tenor
Fluorocarbon Fingerstyle on Mahogany and the mono microphone, and compares the
two. Its header gives the commands.

- **Tone below 1 kHz matches.** Each partial's level against its note's power,
  over 30–330 ms, is within 1 dB of the recording's from 250 Hz to 1 kHz. The
  one exception is 630 Hz, which is +7 dB, from 4 partials near the tenor's T1.
- **Decay below 3.2 kHz is close.** Partial T60s there run from 16% shorter
  than the recording's to 31% longer, band by band.
- **Decay above 3.2 kHz is faster in the recording.** Its partials die 1.7–1.8
  times faster: a median T60 of 0.72 s against 1.26 s at 3.2–4.7 kHz, and
  0.50 s against 0.91 s at 4.7–7 kHz. The model's partials already decay
  faster than Woodhouse and Lynch-Aird's string-only loss allows for Worth
  Clear, the fluorocarbon it models. Worth sells its Brown strings as the
  mellower set, so the recording's faster decay points at its strings. Brown's
  loss modulus is not published, so nothing changed.
- **Partial levels around 1–8 kHz differ.**
  - At 1.3–1.6 kHz the render is 5–7 dB under the recording on the mono
    microphone and 2–5 dB under on the stereo pair, so the gap follows the
    microphone.
  - At 6.3–8 kHz the render is 12–18 dB over the recording with nails. The
    fingertip measured before 0.4.17 was 5–9 dB over. The recording's
    technique and its microphone's response above 6 kHz are unknown.
  - 0.4.19 brought 4–8 kHz 2 dB closer.

## The Labraña–Espinoza soprano: radiation per newton (CC BY 4.0)

Labraña, Gajardo, Correa and Espinoza deposited the measurements behind
"Vibroacoustic reproducibility of 3D-printed PLA+ and wood soundboards for
ukuleles" (Zenodo, doi:10.5281/zenodo.22692775, 2026, CC BY 4.0). The deposit
holds:
- **Angular radiativity**, the sound pressure per newton at the bridge. It is
  measured at 1 Hz steps to 20 kHz, at nine angles in each of two planes.
- **Instruments.** Three complete instruments with printed PLA+ tops, and a
  "Hybrid" with an Engelmann spruce top on a printed PLA+ body.
- **Size.** The deposit's scan mesh gives the body as 162 by 235 mm, a soprano.

The data shows how the bodies fall above 1 kHz:
- **The spruce top.** Power-averaged over its 18 positions, the Hybrid's
  third-octave levels fall 3.204 dB per octave from 1 to 6.3 kHz, as a
  least-squares slope.
- **The PLA+ tops** fall 3.5–3.8 dB per octave. Per angle, the slopes run
  -1.5 to -4.9 dB per octave, and most lie between -3 and -4.
- **Above 8 kHz** every instrument's level rises again, so that range is not
  used.

The same measure on Aculele's body banks, from bridge force to microphone
pressure, is `Tools/AculeleRadiation.cpp`:

| Spruce body | 0.4.18 | 0.4.19 |
| --- | --- | --- |
| Soprano | -1.15 | -3.21 |
| Concert | -2.06 | -3.21 |
| Tenor | -2.38 | -3.21 |

All values are in dB per octave. Mahogany and maple fell 0.1–0.4 dB per octave
less than spruce, and still do (-2.8 to -3.1 in 0.4.19).

0.4.19's third octaves for the soprano are within 2 dB of the Hybrid's from 1.6
to 4 kHz, relative to each one's 1 kHz band. The bands at 1.26 kHz and 5 kHz
remain 5 and 3 dB under. The concert and tenor take the soprano's measured
slope. Neither is measured.

The deposit's modal analysis puts the Hybrid's plate–cavity modes at 202, 324
and 374 Hz, with an operating shape at 281 Hz (`scripts/radiation.m`). The
scaled bank's spruce soprano has its A0 and T1 at 266 and 653 Hz. The printed
body is not a commercial soprano, so the model's low modes were left alone.

## Against the Duke arpeggio (CC BY-NC 4.0, comparison only)

| Soprano (Finger) | 0.4.18 | 0.4.19 |
| --- | --- | --- |
| 1–2.5 kHz | +2.6 to +14.5 | +2.3 to +12.1 |
| 3–6.3 kHz | +2.6 to +5.1 | -3.1 to +1.4 |

All values are dB of model over recording, by third octave. Below 1 kHz
nothing changed.

### Where the 1–2.5 kHz excess comes from (29 October)

The same arpeggio, rendered on 0.4.19 with the soprano's Finger, Mahogany and
the stereo pair (Concert Strum's other settings, Pluck Position 0.50). Each
note's harmonics are measured over 20–170 ms after its onset, in dB against
its first harmonic, then averaged over the 16 notes:

| | H2 | H3 | H4 | H5 | H6 | H7 | H8 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Recording | -2.5 | -28.0 | -28.9 | -32.0 | -38.7 | -36.7 | -39.2 |
| Render, nail (shipping) | -10.4 | -20.8 | -17.6 | -14.8 | -28.1 | -23.4 | -27.1 |
| Render, fingertip | -11.2 | -22.1 | -22.9 | -18.4 | -30.8 | -32.3 | -35.1 |
| Measured soprano's radiation less the model's | -8.2 | -1.4 | -3.5 | -4.8 | -4.7 | -6.6 | -4.8 |

The last row is the Labraña–Espinoza soprano's power-mean radiation minus
the model body's, both smoothed over a sixth of an octave. It is read at
each note's harmonics and taken against its first.

- **The excess is in the excitation and the body together.** The render's
  H3–H8 stand 7–17 dB over the recording's.
  - The measured soprano's radiation in place of the model's would take
    1.4–6.6 dB of that, because it radiates far more at 300–400 Hz, where
    these notes' fundamentals lie.
  - The fingertip in place of the nail would take 1.3–8.9 dB.
- **Neither excitation fits the whole band.** In the long-term spectrum,
  relative to its loudest band, the nail lies within 3 dB of the recording
  from 3.2 to 5 kHz. The fingertip lies 5–12 dB under there.
- **H2 points elsewhere.** The recording's H2 stands 8 dB higher than the
  render's with either excitation, and the measured body would lower it
  further. A pluck nearer the bridge raises H2: the default Pluck Position,
  0.28, puts the Finger at 0.23 of the string against this preset's 0.31,
  which raises H2 by about 2.6 dB by the ideal pluck comb. The recording's
  pluck point is unknown.

So no single change closes the gap. The nail is the owner's open choice (the
0.4.17 page), and the body awaits a measured ukulele.
