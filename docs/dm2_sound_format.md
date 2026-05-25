# DM2 V1 — Audio Format

## Overview

DM2's audio assets span two distinct layers: music (HMP/MIDI) and sound effects
(stored in GDAT2 V5 dungeon data files). The game does not ship with standalone
VOC/WAV/MP2/OGG files — audio samples are packed inside the GDAT container.

## Music Format — HMP/MIDI

### HMP (HMIDI Playlist)
- Format: Custom tracker MIDI, .hmp.mid extension
- Container: Standard MIDI file structure with MThd/MTrk chunks
- Storage: SKWIN/data/00.hmp.mid — SKWIN/data/1c.hmp.mid (28 files)
- Channels: Standard General MIDI 16-channel
- Resolution: Variable ticks per quarter note (from file header)

### HMP vs Standard MIDI
HMP uses the same underlying MIDI events but with a custom timing encoding.
The parser in SkWinMIDI.cpp handles:
- Variable-length delta times (standard MIDI)
- Running status mode
- Tempo meta-events (0x51) for BPM setting
- End-of-track meta-events (0x2F)

### Files in SKWIN/data/
00.hmp.mid through 1c.hmp.mid (28 tracks total)
Hexadecimal numbering, 2 digits, .hmp.mid extension.

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

## Firestaff SDL Port — WAV/OGG Music

### c_music_wav
Alternative to HMP music for the Firestaff SDL builds:
- File: ./DATA/sk%02d.ogg (e.g., sk00.ogg, sk01.ogg)
- Format: OGG Vorbis (lossy compressed)
- Playback: Allegro5 audio playback (al_play_sample equivalent)
- Loop: ALLEGRO_PLAYMODE_LOOP

### File Naming Convention
skNN.ogg where NN = same hex track number as HMP files (00–1c).

## Original DM2 DOS Audio

### DOSBox Compatibility
The DOS release (DOS_EN zip) includes hmidet.386 and hmidrv.386 —
these are HIMEM.SYS-style extended MIDI drivers for DOS.
The game uses the Allegro4 MIDI API which wraps these drivers.

### No Standalone Audio Files
The original DM2 DOS release does not contain standalone audio files
like VOC, WAV, MP2, or OGG. All SFX are embedded in GDAT2 V5.
This differs from DM1 which had some standalone sample files.

## Format Comparison with DM1 CSB

| Aspect | DM1 CSB | DM2 |
|--------|---------|-----|
| Music | AdLib FM OPL2 | HMP/MIDI (GM) |
| Music container | Embedded in EXE | Standalone .hmp.mid files |
| SFX format | Packed DAT | GDAT2 V5 |
| SFX sample rate | 11025 Hz | 5500 Hz (DOS), 6000 Hz (SKWin) |
| SFX bit depth | 8-bit | 8-bit |
| SFX channels | Mono | Mono |
| Compression | None apparent | None apparent |
| Standalone files | Some WAV/VOC | None (all in GDAT2) |
| OGG support | None | Firestaff SDL port only |
