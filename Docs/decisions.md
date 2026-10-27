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

## 2026-10-27 — the bridge level comes from the one measured ukulele (measured; the reading is open)

Aculele inherited Acustra's `steelTopMobilityRatio`, 0.274, which brings the
flamenco guitar g21's bridge to a steel-string guitar's. That corrects
Acustra's proxy and says nothing about a ukulele. Elie, Gautier, David and
Curtit (Forum Acusticum 2011, Fig. 5) measured one ukulele at 0.009 s/kg
characteristic mobility, 1-6 kHz, with its usual rosewood bridge. With the
inherited ratio, Concert Mahogany's G string read 0.0051 s/kg
(`Tools/AculeleBridgeMobility.cpp`). The ratio is now 0.485, which puts that
string at 0.009 s/kg; `Aculele.BridgeMobility` holds it within 3%.

The level is the measurement's. Where it applies is not: the accelerometer
sat "at the base of the lowest string". On the paper's guitars that is the
bass-side edge, the G string's place. By pitch it is the C string, which
would make the ratio 0.982. Both readings are defensible, so the shipping
level, the G reading and the C reading went to the owner as a lettered set
(`bridge-listening-2026-10-27`, key unread by design). The G reading ships
until the owner chooses. The letters and the choice will be recorded here,
as chosen by ear.

Against the one comparison recording (Duke Sonic Dictionary, CC BY-NC 4.0,
compared only), 1-1.6 kHz stays +5.6 to +10.1 dB too strong at every level
tried (0.274, 0.50 and 1.0). So that excess is not the bridge's, and no
bridge level was chosen to reduce it.
