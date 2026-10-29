# Making Aculele sound like a real ukulele: research plan, 27 October 2026

The owner asked for research specific to the ukulele, and for the model to
match a real one. This plan sets out what is known, what this round changed,
and what is still open, in order of how much each item is likely to be heard.
Each step says what evidence would settle it. Where no measurement can settle
a choice, the step goes to an A–Z listening test (see `CLAUDE.md`).

## Where the model stands

The strings are the strong half. Their gauges, moduli and losses are
published measurements. The open strings tune within 3 cents, the
inharmonicity matches the model's own prediction, and a G4's fundamental
decays at the rate a real one does
([feasibility study](../research/ukulele-feasibility-2026-10-27.md)).

The body is the weak half. Its banks are the g21 flamenco guitar's (Mores'
archive), scaled to soprano, concert and tenor size by the equal-thickness
plate law. Against one real recording, the Duke Sonic Dictionary's
"Ukulele Arpeggio" (CC BY-NC 4.0, used only for comparison, never fitted or
distributed), the model played the same notes on the same strings. Its band
power relative to the total, for Concert Mahogany, finger, was:

| Band | Model minus recording |
| --- | --- |
| around 250 Hz | -8 to -11 dB |
| 1-1.6 kHz | +5.6 to +10.1 dB |
| 3-8 kHz | -5 to -15 dB |

The recording is one unidentified ukulele in a reverberant room. It shows
where to look. It is not a target to fit.

## Done in this round

1. **Bridge level from the one measured ukulele.** Aculele inherited
   Acustra's correction (0.274) that turns a flamenco guitar's bridge into a
   steel-string guitar's. That is a guitar-only proxy correction. Elie,
   Gautier, David and Curtit (Forum Acusticum 2011,
   <https://members.loria.fr/BElie/papers/Elie_et_al_FA_2011.pdf>) measured
   one ukulele by impact hammer and accelerometer, with the strings damped.
   Its characteristic mobility (their Fig. 5) was:
   - 0.034 s/kg with no bridge;
   - 0.012 s/kg with a 3 g mahogany bridge;
   - 0.009 s/kg with its usual 7 g rosewood bridge;
   - 0.0094 s/kg with a lightened 6 g rosewood bridge.

   The paper's "base of the lowest string" has two readings on a ukulele.
   By the guitars' convention it is the bass-side edge, the G string. By
   pitch it is the C string. Both were rendered as a lettered set with the
   old level, and the owner chose the C reading by ear. Concert Mahogany's C
   string, which read 0.0025 s/kg before, is now pinned to 0.009, and
   `Aculele.BridgeMobility` checks it. The bridge is about 3.6 times as
   mobile as it was.

   Changing the bridge did not move the 1-1.6 kHz excess. That excess is
   +5.6 to +10.1 dB at every level tried (0.274, 0.50 and 1.0), so it does
   not come from the bridge draining the strings.
2. **A one-channel strum sounds all four strings.** Reason, and any piano
   roll, sends a chord on one MIDI channel, and one channel cannot ask for
   a doubled pitch. A strum of three notes used to leave one string out, so
   G, Am, F and Em lost a string. Now the hand frets a chord tone on the free
   string, at the cheapest fret for the hand holding the shape:
   - C4 E4 G4 plays 0003;
   - C4 E4 A4 plays 2000;
   - D4 G4 B4 plays 0232;
   - D4 F#4 A4 plays 2220.

   That string is held, and let go, with the chord's keys
   (`Aculele.ChordFill`). Tab input is unchanged.

## Open, in order of audible impact

### 1. The 1-1.6 kHz excess: the morphed plate modes

`AculeleBodyModes` shows where the excess sits. On the concert, three
morphed plate modes at 1265, 1509 and 1591 Hz radiate within 2.3 dB of T1.
The morph moves every guitar plate mode up by the same factor as T1, so the
flamenca's strong upper plate modes land in exactly the band that is too
loud.

The modal density says the same thing. Elie et al. counted about 0.016
modes per Hz between 1 and 4.4 kHz on their ukulele (ESPRIT, 0.0155-0.0175).
The model's G-string conductance has 10, 14 and 15 resolved peaks there
(soprano, concert, tenor), about 0.003-0.004 per Hz. Peak counting
undercounts overlapping modes, but not fourfold. The scaled top has too few
modes, and each one radiates too much.

Hypothesis: a ukulele top is thinner, relative to its size, than the
equal-thickness law assumes. For a plate, modal density scales as A/h and
characteristic mobility as 1/h². A thinner top would put more, weaker modes
in this band.

Next steps:

- Collect published top thicknesses and bracing for soprano, concert and
  tenor ukuleles, from makers' and luthiers' specifications, and the flamenca
  guitar's that g21 stands for. A documented thickness ratio is a derivable
  input. A ratio chosen to remove the excess is not.
- Add thickness to the Shape morph only if that ratio exists. Then check it
  against Elie's modal density and against the bridge level, which it would
  change, before any listening.
- If two documented ratios are each defensible, render them as an A–Z set.

Checked on 28 October: no ratio the published figures allow closes the gap.

- No maker publishes a top thickness. Builders on luthiersforum.com give
  about 1.8 mm for a hardwood concert top and 0.075-0.085 in (1.9-2.2 mm)
  for tenors, and say a smaller body takes a thinner top. Classical and
  flamenco tops are usually put at 2.0-2.5 mm. That is a ratio of about 0.7
  to 1.0, from builders' practice, not from g21 or from Elie's ukulele.
- Modal density goes as A/h, so that ratio adds at most 1.4 times the
  modes. The measured ukulele has four to five times the model's.
- The plate modes' frequencies go as h/A, so the same ratio would lower the
  1265-1591 Hz cluster to 0.9-1.6 kHz. That moves the excess rather than
  removing it, and the bridge level is pinned to Elie's measurement either
  way.
- Elie's soundboard weighed 42 g (their Table I), but the paper gives
  neither its size nor its thickness, so it cannot pin a ratio either.

So thickness does not enter the morph, and the excess stays until a ukulele
body is measured (item 7).

Checked again on 29 October, for the low modes rather than the excess: the
thickness ratio is about 1, not 0.7-1.0.

- Flamenco tops are thinner than the 2.0-2.5 mm above. Sheldon Urlik's *A
  Collection of Fine Spanish Guitars* measures each top at 14 points. Over
  19 of its flamencas, made 1883-1988, the top averages 1.82 mm around the
  bridge (Joss Winn's tally, "Notes on a flamenco guitar (#5)", 2020).
- The ukulele builders above take a hardwood top to about 1.8 mm and a
  softwood top to about 2.1 mm before scraping, and tenors to 1.9-2.2 mm.
  Against 1.82 mm, that is a ratio of 1.0-1.2, which the equal-thickness law
  already assumes.
- Two measured ukuleles put their first top mode well under the law's T1:
  - a mid-priced concert at 395 Hz, against 466-485 Hz (Sunta, University of
    Ljubljana thesis, 2020);
  - the Labraña–Espinoza spruce soprano at 324 Hz, against 653 Hz.

  A thinner top would lower T1, but the documented thicknesses do not allow
  one. The difference lies in what thickness leaves out, such as bracing and
  the back, and only a measured body can place it.

### 2. The 3-8 kHz deficit

There are three candidates:

- the excitation: the finger's contact width, and nail against flesh;
- the strings' high-frequency loss: the loss law is Woodhouse and
  Lynch-Aird's, and its high-frequency end has not been checked against a
  ukulele string's own decay;
- radiation from the small body above its coincidence frequency.

The bridge-force spectrum already sits close to the recording's, which
points at radiation. The comparison should be made string by string on
close-microphone single notes (item 6). One reverberant arpeggio cannot
separate these candidates.

### 3. The low end around 250 Hz

The concert's A0 is at 215 Hz and its T1 at 466 Hz. Elie's ukulele showed
bridge peaks near 450 and 880 Hz. The -8 to -11 dB deficit may come from the
recording (its microphone distance, the room, a different body size) as
much as from the model. Leave this until a measured body or a close-mic
reference exists.

### 4. Strumming

Strum speed, the stroke's span law, scatter and edge gains are GuitarSet's
plectrum measurements, reused. No peer-reviewed study of ukulele finger
strumming, or of nail against flesh, was found. Options:

- onset and level statistics from open-licence strummed-ukulele recordings,
  if any can be found with a clear licence;
- a short recording session on a known instrument, with a contact pickup on
  each string;
- a nail/flesh excitation pair, which needs one of the above.

### 5. Koa

Most Hawaiian ukuleles are koa, and the model has none: mahogany stands in.
The US Forest Products Laboratory's Wood Handbook and similar references
publish koa's density and moduli. Adding it is a new Wood choice, which
changes the controls and the Rack device, so it needs the owner's decision.

### 6. Reference recordings

Two open sets were found. Their licences must be confirmed on the source
page before use (Freesound refuses scripted access from here):

- "Ukelele single notes, close-mic", Freesound pack 8545 by stomachache,
  reported as CC BY 3.0, with a room-mic companion, pack 8547. Single notes
  isolate one string and the body. Recorded in a shower on a Sony PCM-D50.
- Freesound pack 40584 by Sadiquecat, a soprano. Its licence is unknown.

A CC BY or CC0 set can be compared against and fitted from, with
attribution. A CC BY-NC one, like the Duke recording, stays comparison-only.

### 7. The decisive step: measure a ukulele

Every item above is bounded by having no measured ukulele body. Measure one
soprano, one concert and one tenor (solid and laminate, if possible) the way
g21 was measured:

- bridge admittance by impact hammer and accelerometer at the base of both
  the G and the C string. This would also check the C reading the owner
  chose by ear;
- radiated pressure at the microphone positions the capture model assumes,
  in a dead room;
- the strings damped, the instrument hung by its headstock.

Elie et al. used a PCB 086E80 hammer and a PCB 352C23 accelerometer. A
ukulele bank fitted from such a capture would replace the scaled guitar bank
outright, as Mores' archive gave Acustra its own.

## What needs the owner

- Decide whether koa is worth a new Wood choice.
- Decide whether to measure an instrument. It needs a real ukulele and
  hardware that this environment does not have.
