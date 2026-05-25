# DM2 V1 — Sound System

## Overview

DM2 has a layered audio architecture with separate music and sound effect subsystems.
The game uses a master c_sound class that combines c_midi (music) and c_sfx (sound effects).
The SKWin (Windows/Firestaff SDL) port replaces the original DOS/Allegro4 audio with a
custom SDL ring-buffer mixer.

## Architecture

### Class Hierarchy
  c_sound (master audio, inherits c_sfx and c_midi)
    c_midi    — HMP/MIDI music playback
    c_sfx     — sound effect ring
    c_music_wav — WAV/OGG music (Firestaff SDL addition)
    c_lw      — light/wind helper class

### Key Source Files
SKULLWIN/c_sound.h/cpp  — Master audio class, dm2sound global
SKULLWIN/c_midi.h/cpp   — Allegro4 MIDI/HMP playback
SKULLWIN/c_music_wav.h/cpp — WAV/OGG music (Firestaff SDL port)
SKULLWIN/c_sfx.h/cpp    — Sound effect management
SKWIN/SkWinMIDI.h/cpp   — Windows MIDI stream renderer
SKWIN/SkwinSDL.h/cpp    — SDL audio backend for SKWin
SKWIN/defines.h         — SOUND_* constants
SKWIN/data/             — Music files 00.hmp.mid through 1c.hmp.mid

## Audio Backend — DOS vs SKWin

### DOS (Allegro4)
- init_sfx() / init_midi() called in c_sound::init()
- Sound card detection: glbSoundCardType, glbSoundBlasterBasePort
- PLAYBACK_FREQUENCY = 5500 Hz (DOS)
- Allegro4 sample playback for SFX

### SKWin (SDL)
- OpenAudio() initializes SDL audio at 6000 Hz
- sdlAudMix() callback fills the audio stream from a ring of SFX buffers
- SndPlayHi(buff, size, vol) — play centered sample at volume
- SndPlayLo(buff, size, dX, dY) — play sample with stereo panning (dX, dY)
- Max 16 simultaneous sound buffers (MAX_SB = 16)
- Samples are 8-bit unsigned, converted with 0x80 + raw_byte

## Sound Effect System

### Global Tables
- dm2sound.v1dfda4[64] — sound entry index table
- dm2sound.xsndptr2   — array of s_ssound structs (7 bytes each)
- glbSoundList[64]    — sound entry lookup table (from GDAT)

### Sound Queue System
Two queue functions enqueue world-positioned sounds:
- DM2_QUEUE_NOISE_GEN1(cat, idx, sfx_id, volume, x, y, flags) — 7 params
- DM2_QUEUE_NOISE_GEN2(cat, idx, sfx_id, extra, volume, x, y, flags) — 9 params (extra byte)

### Sound Playback Functions
DM2_PLAY_SOUND(wav, sfx*)  — play sound with SFX positioning struct
DM2_PROCESS_SOUND(wav)     — process one queued sound
DM2_SOUND7(n)              — query sound entry index
DM2_SOUND8(bool)           — sound toggle on/off
DM2_SOUND9(b, b, b)        — 3-byte sound query
DM2_QUERY_SND_ENTRY_INDEX(cat, idx, sfx) — resolve GDAT sound entry to index

## Music System

### HMP Format
- Files: DATA/00.hmp.mid through DATA/1c.hmp.mid (28 tracks)
- HMP = HMIDI playlist format, parsed by SkWinMIDI.cpp
- Windows MIDI stream API (midiStreamOpen, midiStreamOut)
- Track selection table: tMusicMaps[64] — maps dungeon map to track

### Music Folder per Variant
SkWinMIDI_MIDI_LOOP() branches on dungeon variant:
  DATA_DM1_KID, DATA_DM1_DM, DATA_DM1_CSB, DATA_DM1_TQ,
  DATA_DM2_DM, DATA_DM2_CSB, DATA_DM2_TQ, DATA_DM2_BETA,
  DATA_DM2_DEMO, DATA_DM2_SK, DATA (default)
Custom folder override supported via sCustomFolder.

### Music Track Map (tMusicMaps[64])
0x02, 0x11, 0x0e, 0x1b,  0x04, 0x0c, 0x0c, 0x12,
0x0f, 0x0d, 0x0c, 0x0c,  0x10, 0x06, 0x15, 0x0e,
0x11, 0x11, 0x11, 0x11,  0x03, 0x08, 0x11, 0x0e,
0x02, 0x17, 0x16, 0x14,  0x11, 0x00, 0x02, 0x02,
0x02, 0x09, 0x02, 0x03,  0x0e, 0x10, 0x1c, 0x16,
0x13, 0x09, 0x16, 0x03,  0x11, 0x02, 0xFF, 0xFF,
Maps 0x30–0x3F are 0xFF (no music)

## Key Globals
dm2sound        — c_sound — Master audio object
glbSoundList[64]— array   — Sound entry table
v1dff8a         — i8      — Current music track number
v1d1512         — i16     — Previous music track
v1dd1d1         — i8      — Master volume (default 90)
glbSoundCardType— i8     — Sound card type (SB=6)
glbSoundBlasterBasePort — ui16 — SB I/O base port
glbXAmbientSoundActivated — bool — Ambient sound state

## Comparison with DM1 CSB
Feature         | DM1 CSB           | DM2
Audio API       | AdLib FM + PC Spk | SoundBlaster-compatible
Music format    | AdLib instruments | HMP/MIDI (Windows)
Sound IDs       | Limited fixed set | 64-entry GDAT lookup
SFX channels    | 3-4 voices        | 16-slot ring buffer (SKWin)
Music tracks    | ~10               | 28 tracks (00-1c)
3D positioning  | None              | Distance-based attenuation
