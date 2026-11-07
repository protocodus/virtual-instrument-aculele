# The note survey at 44.1, 48 and 96 kHz, 2026-11-07

The Rack plays at the host's rate, so the key survey of
[note-survey-2026-11-07.md](note-survey-2026-11-07.md) (every key of every
factory preset at velocities 60 and 110, keys mode) was repeated at 44.1 and
96 kHz with `AculeleNoteSurvey --rate`, added for this, and each note
compared with its 48 kHz reading. Engine 0.4.46, nothing changed in it.

## Two faults in the survey tool, fixed first

The first run at 96 kHz read every note 9 dB quieter at 1 s and decaying
10 dB/s faster than at 48 kHz. Single notes rendered through `AculeleProbe`
at both rates decay alike (open C: -10.3 and -10.0 dB/s; A string fret 5:
-27.4 and -29.6 dB/s, partial by partial within 3 dB/s), so the fault was
the tool's: its note slot and hold were sample counts fixed at 48 kHz
(120000 and 76800), so at 96 kHz the key was released at 0.8 s and the
"1 s" window read the release. They are now 2.5 s and 1.6 s at the rate.

With that fixed, level_db still read a mean 0.64 dB lower at 96 kHz, on
every preset, and 0.09 dB higher at 44.1 kHz. The K-weighting stages were
BS.1770's 48 kHz tables at every rate, which moves the +4 dB shelf's corner
with the rate. They are now designed at the rate from the standard's
analogue prototypes (the constants that reproduce the published 48 kHz
coefficients to 1e-15), so at 48 kHz nothing changes, byte for byte; the
spectra's FFT lengths grow with the rate so the windows stay in seconds.

## The engine across rates

Every note's reading at the other rate minus its 48 kHz reading, all
seven presets, both velocities (574 notes a rate).

| Column | 44.1 kHz mean | sd | largest | 96 kHz mean | sd | largest |
| --- | --- | --- | --- | --- | --- | --- |
| level_db (first second) | -0.02 dB | 0.31 | 1.63 | -0.10 dB | 0.55 | 2.68 |
| level_200ms_db | -0.01 dB | 0.29 | 1.59 | -0.08 dB | 0.51 | 2.54 |
| peak_dbfs | -0.09 dB | 0.48 | 3.54 | -0.15 dB | 0.54 | 2.69 |
| bright_db | +0.15 dB | 1.32 | 5.00 | -0.36 dB | 1.59 | 6.18 |
| f0_cents_error | +0.00 c | 0.39 | 3.57 | +0.02 c | 0.25 | 2.41 |
| pitch_cents_error | +0.01 c | 0.21 | 1.58 | +0.05 c | 0.16 | 0.82 |
| spike_db | +0.21 dB | 0.87 | 3.80 | +0.08 dB | 1.21 | 5.50 |
| decay_db_per_s | -0.06 | 0.98 | 4.82 | +0.27 | 1.12 | 8.47 |

No preset's mean level moves more than 0.4 dB (Concert Strum -0.38 dB at
44.1 kHz, Tenor Picked Melody -0.22 dB at 96 kHz), and the pitch the
partials carry moves under 0.1 cent on average and under 1.6 cents on any
note. The largest single differences are high soprano A-string notes
(Soprano D Tuning and Soprano Thumb, note 78 at velocity 110: 2.3 dB louder
over the first 200 ms at 96 kHz) and their 1 s readings, which are taken
at -60 to -70 dB where a fast-decaying note's tail is; the 8.5 dB/s decay
difference is one such note. The scatter about zero (sd 0.3-0.55 dB in
level) is the per-note variation the rate change reseeds, not a direction.

The engine is consistent across the host rates the Rack meets; the two
faults were the survey's, and are fixed in the same commit as this report.
