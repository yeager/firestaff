# Nexus V1 — Audio Format

## Overview

Dungeon Master Nexus audio consists of two distinct systems:
1. **CD-DA music** — Red Book Audio tracks 2-9
2. **Per-level SFX banks** — SNDLEV*.SAL + SNDLEV*.MAP files

No voice format is implemented beyond potential FMV audio in AVI files.

## CD-DA Music Format

| Property | Value |
|----------|-------|
| Standard | Red Book Audio (CD-DA) |
| Sample rate | 44.1 kHz (standard) |
| Channels | Stereo |
| Bit depth | 16-bit (standard CD audio) |
| Track range | Tracks 2-9 (8 tracks total) |
| Coverage | 2 levels per track |
| Data source | Tracks 2-9 on Saturn CD |

### Track-to-Level Mapping

| Track | Levels | File (CUE) |
|-------|--------|------------|
| 2 | 0, 1 | Track 2 AUDIO |
| 3 | 2, 3 | Track 3 AUDIO |
| 4 | 4, 5 | Track 4 AUDIO |
| 5 | 6, 7 | Track 5 AUDIO |
| 6 | 8, 9 | Track 6 AUDIO |
| 7 | 10, 11 | Track 7 AUDIO |
| 8 | 12, 13 | Track 8 AUDIO |
| 9 | 14, 15 | Track 9 AUDIO |

## Per-Level SFX Banks

Located in Track 1 of the ISO (MODE1/2352, game data track).

### SNDLEV*.SAL Files (290-460 KB each)

- 16 files: `SNDLEV00.SAL` through `SNDLEV15.SAL`
- One per dungeon level
- "SAL" likely stands for "Sound Allocation" or similar
- Format: unknown binary (not SND3.DAT, not SONG.DAT)
- Contains: sound effect samples for that specific level
- 290-460 KB size range suggests compressed or structured data

### SNDLEV*.MAP Files (66-90 bytes each)

- 16 files: `SNDLEV00.MAP` through `SNDLEV15.MAP`
- Small index/mapping files
- 66-90 bytes suggests a simple table (event ID → sample offset/size)
- Purpose: maps game events to specific samples in the SAL bank

### Comparison with DM1

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| SFX file | SND3.DAT (global) | SNDLEV00-15.SAL (per-level) |
| SFX format | PCM 8-bit | Unknown (not PCM) |
| SFX size | ~28 KB | 290-460 KB per level |
| Mapping file | None | SNDLEV00-15.MAP (66-90 B) |
| Music | SONG.DAT (sequenced) | CD-DA tracks |

Per-level SFX allows different sound environments per dungeon depth — deeper levels could have more ominous SFX.

## Sound Driver

### SDDRVS.TSK (26 KB)

- "Sound DRiVerS TaSK" — Saturn SH-2 sound driver
- 26 KB — compact driver binary
- Loads and plays SNDLEV*.SAL samples based on SNDLEV*.MAP triggers
- Handles music track selection (CD audio)
- Likely a lightweight RTOS task on the Saturn

## Audio Engine (Firestaff Implementation)

In `src/nexus/nexus_v1_engine.c`:

```c
engine->audio_enabled = 1;  // audio subsystem init

int nexus_v1_load_level(...) {
    int new_track = nexus_v1_cd_track_for_level(level);
    if (new_track != engine->current_cd_track && engine->audio_enabled) {
        engine->current_cd_track = new_track;
        printf("Nexus: CD track %d for level %d\n", new_track, level);
        /* FUTURE: CD audio playback via SDL_mixer.
         * DM Nexus (Saturn) uses CD-DA tracks for music. */
    }
}
```

Current state: track number is computed and stored, but actual audio playback is a stub.

## FMV Audio

| File | Size | Description |
|------|------|-------------|
| DMV0.AVI | 34 MB | Intro cutscene |
| DMV1.AVI | 28 MB | Mid-game or ending |
| DMV2.AVI | 39 MB | Ending cutscene |

- Format: Saturn AVI (custom, not standard AVI)
- Audio embedded in video stream
- Codec: unknown proprietary format

## Summary Table

| Audio Type | Format | Location | Status |
|-----------|--------|----------|--------|
| Music tracks | CD-DA (Red Book) | CD tracks 2-9 | Track switching implemented; playback TODO |
| SFX banks | SAL format (unknown) | ISO Track 1 (SNDLEV*.SAL) | Parsing TODO |
| SFX mapping | MAP format (66-90 B) | ISO Track 1 (SNDLEV*.MAP) | Parsing TODO |
| Sound driver | SH-2 binary | SDDRVS.TSK | Not reverse-engineered |
| FMV audio | Saturn AVI codec | DMV*.AVI | Not implemented |

## Related Files

- `src/nexus/nexus_v1_engine.c` — audio enable, track switching
- `src/nexus/nexus_v1_game.c` — `nexus_v1_cd_track_for_level()`
- `docs/NEXUS_FILE_CLASSIFICATION.md` — file inventory
- `docs/NEXUS_PLAN.md` — audio implementation plan