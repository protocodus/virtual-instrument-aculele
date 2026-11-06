#!/usr/bin/env python3
"""Render the songs in Docs/audio/midi/songs/ and mix their parts.

Each part plays on the ukulele Tools/write_preset_midi.py's SONGS gives it,
through AculeleProbe (the Performer as the demo renderer drives it), at the
same length; the parts are panned and trimmed as SONGS says, summed, and the
mix is scaled so its peak sits at -1 dBFS. Output is 48 kHz 24-bit stereo,
<out-dir>/<song>.wav, never committed. With --parts, each part's panned,
trimmed and scaled float stem and its strings' activity (AculeleProbe
--activity) are kept as <out-dir>/<song>/<part>.wav and .csv, for the films.

Finger Lookahead is on: a rendered song has no latency to save, and with it
a fretting finger lets go and slides to its next fret before the pluck.

    python3 Tools/render_songs.py build-dsp <out-dir> [--parts]
"""

import pathlib
import subprocess
import sys
import tempfile

import numpy as np

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from write_preset_midi import SONG_DIR, SONGS, preset_slugs  # noqa: E402

TAIL = 4.0  # seconds after a song's last event


def read_float_wav(path):
    data = pathlib.Path(path).read_bytes()
    offset = 12
    while data[offset:offset + 4] != b"data":
        offset += 8 + int.from_bytes(data[offset + 4:offset + 8], "little")
    return np.frombuffer(data[offset + 8:], dtype="<f4").reshape(-1, 2).astype(np.float64)


def write_wav24(path, audio, rate=48000):
    pcm = np.clip(np.round(audio * 8388607), -8388608, 8388607).astype("<i4")
    frames = pcm.reshape(-1).view(np.uint8).reshape(-1, 4)[:, :3].tobytes()
    header = (b"RIFF" + (36 + len(frames)).to_bytes(4, "little") + b"WAVEfmt "
              + (16).to_bytes(4, "little") + (1).to_bytes(2, "little") + (2).to_bytes(2, "little")
              + rate.to_bytes(4, "little") + (rate * 6).to_bytes(4, "little") + (6).to_bytes(2, "little")
              + (24).to_bytes(2, "little") + b"data" + len(frames).to_bytes(4, "little"))
    pathlib.Path(path).write_bytes(header + frames)


def write_float_wav(path, audio, rate=48000):
    data = audio.astype("<f4").tobytes()
    header = (b"RIFF" + (36 + len(data)).to_bytes(4, "little") + b"WAVEfmt "
              + (16).to_bytes(4, "little") + (3).to_bytes(2, "little") + (2).to_bytes(2, "little")
              + rate.to_bytes(4, "little") + (rate * 8).to_bytes(4, "little") + (8).to_bytes(2, "little")
              + (32).to_bytes(2, "little") + b"data" + len(data).to_bytes(4, "little"))
    pathlib.Path(path).write_bytes(header + data)


def last_event_seconds(path):
    """The time of a format 0 file's last event, with its tempo map."""
    data = pathlib.Path(path).read_bytes()
    division = int.from_bytes(data[12:14], "big")
    position, end = 22, 22 + int.from_bytes(data[18:22], "big")
    seconds, tempo, status = 0.0, 500000, 0
    while position < end:
        delta = 0
        while True:
            byte = data[position]
            position += 1
            delta = delta << 7 | byte & 0x7F
            if byte < 0x80:
                break
        seconds += delta * tempo / 1e6 / division
        if data[position] == 0xFF:
            kind, size = data[position + 1], data[position + 2]
            if kind == 0x51:
                tempo = int.from_bytes(data[position + 3:position + 6], "big")
            position += 3 + size
            continue
        if data[position] & 0x80:
            status = data[position]
            position += 1
        position += 1 if status & 0xF0 in (0xC0, 0xD0) else 2
    return seconds


def main():
    keep = sys.argv[3:] == ["--parts"]
    if len(sys.argv) != 3 and not keep:
        sys.exit(__doc__)
    probe = pathlib.Path(sys.argv[1]) / "AculeleProbe"
    out = pathlib.Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    slugs = preset_slugs()
    for song, (_, parts) in SONGS.items():
        seconds = max(last_event_seconds(SONG_DIR / f"{song}-{part}.mid") for part in parts) + TAIL
        stems = {}
        with tempfile.TemporaryDirectory() as scratch:
            folder = out / song if keep else pathlib.Path(scratch)
            folder.mkdir(exist_ok=True)
            for part, (preset, options, pan, trim) in parts.items():
                wav = pathlib.Path(scratch) / f"{part}.wav"
                activity = ["--activity", str(folder / f"{part}.csv")] if keep else []
                subprocess.run([str(probe), str(slugs.index(preset)), str(SONG_DIR / f"{song}-{part}.mid"),
                                str(wav), f"{seconds:.3f}", "--float", "--lookahead", "1", *options,
                                *activity], check=True)
                audio = read_float_wav(wav) * 10 ** (trim / 20)
                audio *= [min(1.0, 1.0 - pan), min(1.0, 1.0 + pan)]
                stems[part] = audio
        mix = sum(stems.values())
        scale = 10 ** (-1 / 20) / np.abs(mix).max()
        write_wav24(out / f"{song}.wav", mix * scale)
        if keep:
            for part, audio in stems.items():
                write_float_wav(out / song / f"{part}.wav", audio * scale)
        print(out / f"{song}.wav")


if __name__ == "__main__":
    main()
