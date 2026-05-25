# Nexus V1 — Music Tracks

## Overview

Dungeon Master Nexus uses **CD-DA (Red Book audio)** on the Saturn CD — each track is a standalone audio file on the CD, separate from the game data tracks. This is fundamentally different from DM1's SONG.DAT sequenced format.

## CD Track Assignment

The Saturn version of Dungeon Master Nexus stores audio as CD-DA tracks. The engine references tracks 2-9 (track 1 is typically data or the game startup audio).

### Track-to-Level Mapping

```c
int nexus_v1_cd_track_for_level(int level) {
    if (level < 0 || level > 15) return 2;
    return 2 + (level / 2);  /* Track 2-9 */
}
```

| CD Track | Dungeon Levels | Description |
|----------|---------------|-------------|
| 2 | Level 0-1 | Entry / first dungeon depths |
| 3 | Level 2-3 | |
| 4 | Level 4-5 | |
| 5 | Level 6-7 | |
| 6 | Level 8-9 | |
| 7 | Level 10-11 | |
| 8 | Level 12-13 | |
| 9 | Level 14-15 | Deepest dungeon |

Each track covers exactly 2 dungeon levels.

## Track Switching Logic

Located in `src/nexus/nexus_v1_engine.c`:

```c
void nexus_v1_tick(Nexus_V1_Engine *engine) {
    // ...
}

int nexus_v1_load_level(Nexus_V1_Engine *engine, int level) {
    // ...
    /* Update CD audio track */
    int new_track = nexus_v1_cd_track_for_level(level);
    if (new_track != engine->current_cd_track && engine->audio_enabled) {
        engine->current_cd_track = new_track;
        printf("Nexus: CD track %d for level %d\n", new_track, level);
        /* FUTURE: CD audio playback via SDL_mixer.
         * DM Nexus (Saturn) uses CD-DA tracks for music. */
    }
    // ...
}
```

The track switch happens at level load time. The current track is stored in `engine->current_cd_track`.

## Implementation Status

| Feature | Status |
|---------|--------|
| Track number calculation | ✅ Implemented |
| Track switching on level change | ✅ Implemented |
| SDL_mixer CD playback | ❌ **TODO** |
| Loop/stop/pause control | ❌ **TODO** |
| Volume control | ❌ **TODO** |
| Crossfade between tracks | ❌ **TODO** |

The `engine->current_cd_track` value is maintained correctly. The actual CD audio playback via SDL_mixer is a planned feature stub.

## Audio Files on CD

In CUE/BIN format, CD-DA tracks are raw PCM audio files. The game data tracks (ISO9660) coexist with audio tracks on the same disc image.

No specific music filenames are referenced in the current source — the engine uses numeric track IDs that correspond to the CUE sheet track numbering.

## Comparison with DM1

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Source format | SONG.DAT binary | CD-DA (Red Book) |
| Track count | 13-14 song entries | 8 music tracks (2-9) |
| Level mapping | Song indices (0-13) | Tracks 2-9, 2 levels each |
| Transitions | Hard-coded crossfades | Level-load triggered |

## Related Source Files

- `src/nexus/nexus_v1_engine.c` — track switching in `nexus_v1_load_level()`
- `src/nexus/nexus_v1_game.c` — `nexus_v1_cd_track_for_level()`
- `src/nexus/nexus_v2_config.c` — `cfg->enhanced_audio` flag