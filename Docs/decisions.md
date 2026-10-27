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
