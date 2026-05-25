# Nexus V1 — Audio System

## Overview

Dungeon Master Nexus (Saturn, 1996) uses a fundamentally different audio architecture from DM1/CSB.

**CD-DA (Red Book) audio only** — no SONG.DAT, no sequenced music.

## Audio Architecture

### Source Detection

Nexus V1 supports two data source modes:

| Mode | Detection | Engine Source |
|------|-----------|---------------|
| ISO | Looks for `.cue` file in data directory | `NEXUS_SRC_ISO` |
| Extracted | Checks for `DM.BIN` or `LEV00.DGN` | `NEXUS_SRC_EXTRACTED` |

- **ISO mode**: Reads files directly from a CUE/BIN image via `nexus_iso_open_cue()`
- **Extracted mode**: Reads flat files from data directory (e.g., `DM.BIN`, `LEV*.DGN`)

### Audio Enable Flag

```c
engine->audio_enabled = 1;  // set at init time
```

The flag gates CD track changes — no track switch occurs if `audio_enabled` is false.

### CD Track Switching

```c
/* Map dungeon levels to CD audio tracks (Track 2-9).
 * 8 audio tracks for 16 levels — each track covers 2 levels. */
int nexus_v1_cd_track_for_level(int level) {
    if (level < 0 || level > 15) return 2; /* default: track 2 */
    return 2 + (level / 2);  /* Track 2-9 */
}
```

Track → Level mapping:

| Track | Levels |
|-------|--------|
| 2 | 0, 1 |
| 3 | 2, 3 |
| 4 | 4, 5 |
| 5 | 6, 7 |
| 6 | 8, 9 |
| 7 | 10, 11 |
| 8 | 12, 13 |
| 9 | 14, 15 |

### Track Change Flow

1. `nexus_v1_load_level()` called with dungeon level 0-15
2. `nexus_v1_cd_track_for_level()` computes target track (2-9)
3. If `new_track != engine->current_cd_track && engine->audio_enabled`:
   - Sets `engine->current_cd_track = new_track`
   - Prints: `"Nexus: CD track %d for level %d\n"`
4. **TODO**: Call SDL_mixer CD playback function

### SDL_mixer TODO

```c
/* FUTURE: CD audio playback via SDL_mixer.
 * DM Nexus (Saturn) uses CD-DA tracks for music. */
```

The CD audio playback is **not yet implemented**. The engine only prints the track number.

## Data Files

No audio-specific data files in the V1 engine source. Audio is driven entirely by the CD image tracks.

## Comparison with DM1

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Music format | SONG.DAT ( sequenced) | CD-DA (Red Book) |
| Music tracks | 1 file, multiple sequences | 8 CD tracks (track 2-9) |
| Level mapping | Song indices in SONG.DAT | Track numbers 2-9 per 2 levels |
| SFX | SND3.DAT (PCM) | **Not implemented** |
| Voice | In dungeon data | **Not implemented** |

## Status

- [x] CD track mapping per level — implemented
- [ ] CD audio playback — **TODO** (SDL_mixer stub)
- [ ] SFX system — **Not implemented**
- [ ] Voice/dialogue — **Not implemented**
- [ ] Footstep audio — configured (`cfg->footstep_audio = 1`) but no playback

## Files

- `src/nexus/nexus_v1_engine.c` — audio init, track switching, enable flag
- `src/nexus/nexus_v1_game.c` — `nexus_v1_cd_track_for_level()`
- `src/nexus/nexus_v2_config.c` — `cfg->enhanced_audio`, `cfg->footstep_audio`