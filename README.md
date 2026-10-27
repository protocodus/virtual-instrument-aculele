# Aculele

Aculele is a physically modelled ukulele: four re-entrant nylon or fluorocarbon strings on a soprano, concert or tenor body, played by finger, thumb, plectrum or a finger strum. Its engine is Acustra's, forked from Acustra 1.9.0 and re-strung, re-sized and re-voiced as a ukulele. It is DSP only for now, a JUCE-free C++17 engine with a demo renderer and analysis tools; there is no VST3, AU, CLAP or standalone build yet.

## Audio demos

The main build renders these demos once Aculele has a plug-in to build. Until then, `RenderAculelePresetDemos` renders them locally from the phrases in `Docs/audio/midi/` (see Build).

<!-- audio-demos-begin -->
<!-- audio-demos-end -->

## Version history

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
- **Geometry.** The saddle spacing is 13.3 mm, which sets the bridge lever arms and the strum's travel. Pluck distances and the finger's contact width keep their fraction of the shorter string. The hand's band reaches 0.46 of the string, where a ukulele is strummed over the neck joint.
- **Strum technique.** `PickingTechnique::Strum` is the index finger swept down and up over the neck joint. It uses the plectrum's sweep timing and alternation with the finger's soft contact.
- **Tab input.** In string-per-channel mode (Mono Mode On, CC126 = 4, on channel 1), channels 1-4 are the G, C, E and A strings. A group of notes on those channels is one strummed or pinched stroke, so a chord can double a pitch on two strings, as G (0232) and Am (2000) do. One MIDI channel cannot express that. The demos are written this way.
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

Each preset's Output level brings its demo to -18 LUFS. The demo phrases in `Docs/audio/midi/` come from `Tools/write_preset_midi.py`.

## Known gaps

- **No ukulele body has been measured.** The radiation and bridge banks are a nylon flamenca guitar scaled down by the equal-thickness plate law. With the guitar contour off, the soprano's third-octave spectrum lies within about 4 dB of a real ukulele recording (an unidentified instrument) from 250 to 800 Hz, but it is still 7-12 dB too bright at 1-2 kHz. A real ukulele top is thinner and more lightly braced than the law assumes.
- **Doubled pitches need tab input.** On one MIDI channel, Acustra's hand planner places C (0003) and G7 (0212) as a player does, and D7 at a playable but unusual 5222. But one channel cannot ask for a unison, so G (0232), Am (2000), F (2010) and Em (0432) lose a string. The string left out is crossed as a muted string inside the stroke. Tab input avoids this; every demo but the Low G picked melody uses it.
- Pluck release, contact noise, strum speeds and finger-chord timing are Acustra's fits to guitar recordings (GuitarSet, the flat-top bank), reused unchanged.
- There is no koa wood; mahogany stands in for it.
- The strum technique has no nail-versus-flesh distinction, and there are no rasgueado or triplet-roll strokes.
- There is no plug-in, editor, screenshot, main build or release yet.

## Build

```sh
cmake -S . -B build-dsp -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-dsp
ctest --test-dir build-dsp -L '^(unit|audio|perf)$' --output-on-failure
./build-dsp/RenderAculelePresetDemos --jobs 4 Docs/audio/midi <out-dir>
./build-dsp/AculeleProbe <preset> <in.mid> <out.wav> [seconds] [--float] [--shape N] [--wood N] [--strings N] [--picking N] [--position X]
python3 Tools/analyse_probe.py <out.wav> <midi>:<start-seconds> ...
./build-dsp/AculeleBodyModes
```

CMake 3.22 and a C++20 compiler build the engine; `AculeleDSPCxx17` compiles it as C++17 with `-Werror`, as the Rack toolchain does. `Tools/analyse_probe.py` needs NumPy and SciPy. A demo renders at about 10x real time on one 2.8 GHz Xeon core, about 10% of that core; these are development-machine figures, not a DAW's.

## Licensing

Aculele is source-available under the PolyForm Noncommercial License 1.0.0 with an additional permission for official builds; see [LICENSE](LICENSE). The measurement-derived tables it carries from Acustra are adapted from CC BY 4.0 datasets, attributed with their changes in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
