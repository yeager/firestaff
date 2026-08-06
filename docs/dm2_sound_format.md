# DM2 V1 — Audio Format

## Overview

DM2's audio assets span two distinct layers: music (HMP/MIDI) and sound effects
(stored in GDAT2 V5 dungeon data files). The game does not ship with standalone
VOC/WAV/MP2/OGG files — audio samples are packed inside the GDAT container.

## Music Format — HMP/MIDI

### HMP (HMI MIDI Playlist)

- Format: HMI's `HMIMIDIP013195` MIDI-like stream, not a Standard MIDI file.
- Storage: PC DM2 keeps 29 streams (`00`--`1c`) in verified
  `GRAPHICS.DAT` GDAT `MUSICS/<track>/dtHMP/0` records.  The first stream is
  raw GDAT entry 5595.
- Routing: the first 44 bytes of the original, 63-byte `SONGLIST.DAT` select
  one of those streams for each dungeon map.  The canonical PC file's
  SHA-256 is
  `401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72`.
- Channels: MIDI-style channel events. Firestaff's bounded source inspector
  reads the real `013195` header (subtrack count at byte 48, BPM at byte 56),
  the observed first chunk at byte 904, and every 12-byte chunk header. HMP
  delta values use least-significant seven-bit groups with a high-bit
  terminator; Standard MIDI variable-length fields remain most-significant
  first.

`SKWIN/data/*.hmp.mid` and `SKULLWIN/Data/*.hmp.mid` are pre-converted
Standard MIDI developer/port assets. They are useful as behavioural reference,
but are not original PC runtime inputs and Firestaff must not open them when
playing DM2.

### HMP vs Standard MIDI
HMP uses MIDI-like events but has its own header, track directory and timing
encoding. `SkWinMIDI.cpp` consumes only the converted Standard MIDI sidecars;
it is not a decoder for the original GDAT HMP streams. Firestaff directly
validates the original track partitions and event bounds, but intentionally
rejects playback until a source-faithful scheduler/backend handoff is proven.

### Converted SKWIN sidecars

`00.hmp.mid` through `1c.hmp.mid` are 29 converted Standard MIDI files with
two-digit hexadecimal names. They are not an installation requirement.

## Sound Effect Format — GDAT2 V5

### GDAT2 Format
Sound effects are stored in the GDAT2 V5 section of the dungeon data file.
GDAT2 V5 is a packed binary format containing:

- Sound entries (64 total across categories)
- Audio sample data (8-bit PCM, signed)
- Sample rate metadata
- Category/index/sound ID mapping

### Sample Format
From the c_sfx system:
- Sample rate: PLAYBACK_FREQUENCY = 5500 Hz (DOS), 6000 Hz (SKWin SDL)
- Bit depth: 8-bit unsigned (converted: 0x80 + raw_byte)
- Channels: Mono
- Encoding: 8-bit signed samples packed in GDAT2

### GDAT2 V5 Structure (Sound Section)
Sound entries are resolved via:
- dm2sound.v1dfda4[64] — index table
- glbSoundList[64] — entry lookup from GDAT
- xsndptr2 + 7*n — s_ssound struct per entry

Each entry contains:
- w_00: sample index
- b_02: category
- b_03: index
- b_04: sound ID
- w_05: ???

## SKWin SDL Audio Format

### sdlAudMix Callback
The SDL audio callback operates at ~6000 Hz sample rate:
- Stream: 8-bit unsigned audio
- Conversion: 0x80 + raw_sample (SndBuf conversion)
- Buffer: 16-slot ring buffer (MAX_SB = 16)
- Mixing: Simple add-into-stream per active buffer

### Sound Buffer (SndBuf)
struct SndBuf {
  void *pMem;   // allocated sample memory
  int pos;      // current play position
  int len;      // total sample length
  int dist;     // distance for attenuation
};

Sample memory is converted at alloc time: each byte = 0x80 + raw_value.

### Distance Attenuation
SndPlayLo takes dX, dY parameters representing stereo position/distance.
The dist field = abs(dX) + abs(dY) for simple attenuation.

## Audio Initialization

### DOS (Allegro4)
1. init_sfx() — initializes Allegro4 sample system
2. init_midi() — initializes Allegro4 MIDI subsystem
3. glbSoundCardType detection (SoundBlaster=6, etc.)
4. PLAYBACK_FREQUENCY = 5500 Hz

### SKWin (SDL)
1. OpenAudio() — SDL_OpenAudio at 6000 Hz
2. as.callback = sdlAudMix
3. SDL_OpenAudio(&as, &asavail) — get actual format
4. sbs array zeroed, ready for SndPlayHi/SndPlayLo

## Port-side WAV/OGG alternatives

skproject's `c_music_wav` can select operator-provided `skNN.ogg` alternatives
in its port. They are not original DM2 game data and are not a Firestaff
fallback: a verified original stream that cannot yet be decoded remains silent.

## Original DM2 DOS Audio

### DOSBox Compatibility
The DOS release includes HMI driver components. skproject's Allegro calls are
port implementation details, not evidence that the PC game shipped the
converted `.hmp.mid` files.

### No Standalone Audio Files
The original DM2 DOS release does not contain standalone audio files
like VOC, WAV, MP2, or OGG. All SFX are embedded in GDAT2 V5.
This differs from DM1 which had some standalone sample files.

## Format Comparison with DM1 CSB

| Aspect | DM1 CSB | DM2 |
|--------|---------|-----|
| Music | AdLib FM OPL2 | HMP/MIDI (GM) |
| Music container | Embedded in EXE | GRAPHICS.DAT GDAT `MUSICS/dtHMP` |
| SFX format | Packed DAT | GDAT2 V5 |
| SFX sample rate | 11025 Hz | 5500 Hz (DOS), 6000 Hz (SKWin) |
| SFX bit depth | 8-bit | 8-bit |
| SFX channels | Mono | Mono |
| Compression | None apparent | None apparent |
| Standalone files | Some WAV/VOC | None (all in GDAT2) |
| OGG support | None | Firestaff SDL port only |
