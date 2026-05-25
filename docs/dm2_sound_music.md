# DM2 V1 — Music System

## Overview

DM2 uses MIDI/HMP music playback, a significant step up from DM1 CSB's
AdLib FM synthesis approach. The original DOS version uses Allegro4's
MIDI subsystem; the SKWin Windows port uses the native Windows MIDI API
via midiStreamOpen/midiStreamOut. The Firestaff SDL port adds WAV/OGG
playback as a replacement.

## HMP/MIDI Music Format

### What is HMP?
HMP (HMIDI Playlist) is a tracker-style MIDI format that stores sequences
of MIDI events with precise timing. Unlike standard MIDI files, HMP uses
a custom header and tick-based scheduling.

### File Locations
- SKWIN/data/00.hmp.mid through SKWIN/data/1c.hmp.mid (28 tracks)
- Files named as 2 hex digits + .hmp.mid extension
- Each track corresponds to a dungeon area/map

### HMP Parsing (SkWinMIDI.cpp)
The MIDI loop reads the HMP format manually:
- Header: MThd identifier, format, tracks, ticks per quarter note
- Track chunks: MTrk header + events packed in sequence
- Events parsed: Note On/Off, Control Change, Program Change, Tempo meta-events
- Tempo events (0x51) are extracted and used to set playback speed
- Uses Windows midiStream API for output

### HMP Processing Pipeline
1. load_file() — reads entire .hmp.mid into memory
2. Parse _mid_header to get track count and time division
3. Build trk[] array with per-track read pointers
4. get_buffer() merges events from all tracks in time order
5. midiStreamOut() streams events to MIDI hardware

## Music Track Table

### tMusicMaps[64] — Map-to-Track Mapping
Maps 0x00–0x3F (64 entries) map dungeon map numbers to HMP track indices.
Unused maps use 0xFF (no music).

Map indices: 0x00=0x02, 0x01=0x11, 0x02=0x0e, 0x03=0x1b
            0x04=0x04, 0x05=0x0c, 0x06=0x0c, 0x07=0x12
            0x08=0x0f, 0x09=0x0d, 0x0a=0x0c, 0x0b=0x0c
            0x0c=0x10, 0x0d=0x06, 0x0e=0x15, 0x0f=0x0e

Upper maps (0x30–0x3F): all 0xFF (no music).

### Track Count
28 distinct tracks (0x00–0x1b). No track 0x1d–0x1f in the data folder.

## Variant Folders

The MIDI loop branches on dungeon variant to support variant-specific music:
- DATA_DM1_KID, DATA_DM1_DM, DATA_DM1_CSB, DATA_DM1_TQ
- DATA_DM2_DM, DATA_DM2_CSB, DATA_DM2_TQ, DATA_DM2_BETA
- DATA_DM2_DEMO, DATA_DM2_SK, DATA (default)

This allows different music sets per variant. The folder is determined
by iDungeonGame which is set when creating the SkWinMIDI instance.

Custom folder override via sCustomFolder in SkWinMIDI.

## Music Playback Control

### REQUEST_PLAY_MUSIC(iMapNumber)
Called from the game when entering a new map area. Looks up the track
in tMusicMaps and either starts a new thread or queues the track change.

### Music Change Logic
If music has changed mid-playback (iCurrentRequestedMusic != iNextRequestedMusic),
the buffer loop breaks and a new thread starts with the new track.
Controlled by bMIDIMusicEnabled (SkCodeParam).

### Volume
midiOutSetVolume((HMIDIOUT)out, 0x0000) — currently hardcoded to 0 in the loop.
The master volume (v1dd1d1) is set separately in c_sound.

## Firestaff SDL Port — WAV/OGG Alternative

### c_music_wav Class
- do_music_wav(i16 nr) — plays ./DATA/sk%02d.ogg looped
- do_music_stop() — stops all samples via al_stop_samples()
- Uses Allegro5 audio (AL5) for playback in the SDL context

### File Naming
sk00.ogg, sk01.ogg, ... sk1c.ogg — same numbering as HMP tracks.

## DM2 vs DM1 CSB — Music Comparison

| Aspect | DM1 CSB | DM2 |
|--------|----------|-----|
| Format | AdLib FM synthesis | HMP/MIDI |
| Synthesis | Yamaha YM3812 (OPL2) | General MIDI |
| Track count | ~10 | 28 |
| Channel count | Limited to OPL2 channels | Standard GM 16-channel |
| Integration | Simple trigger | Per-map track selection |
| Volume control | Single global | Per-track velocity |
| Custom variants | None | Per-variant music folders |
| Dynamic change | Fixed | Thread-based mid-track change |

## Key Implementation Notes

1. HMP is NOT standard MIDI — requires custom parser (SkWinMIDI.cpp)
2. The midiStream callback (example9_callback) signals via SetEvent on MOM_DONE
3. MAX_BUFFER_SIZE = 32*12 — short buffer allows responsive track changes
4. Running status encoding is handled in get_next_event()
5. The DOS Allegro version uses different MIDI playback path (Allegro4 midi.c)
