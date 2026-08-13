# Nexus V1 — Audio System

## Overview

Dungeon Master Nexus (Saturn, 1996) uses a fundamentally different audio architecture from DM1/CSB.

The retail disc has CD-DA tracks plus per-level `SNDLEV*.SAL`/`.MAP` sound
assets. The presence of either source does not prove the Saturn selector,
SCSP voice setup, or host playback path. Firestaff performs a bounded
diagnostic PCM materialization for authenticated DataID 0 tone-bank entries;
that cache is not proof of the Saturn codec/voice ABI and is not connected to
production playback.

## Audio Architecture

### Source Detection

Nexus V1 supports two data source modes:

| Mode | Detection | Engine Source |
|------|-----------|---------------|
| ISO | Looks for `.cue` file in data directory | `NEXUS_SRC_ISO` |
| Extracted | Requires authenticated `DM.BIN` and first playable `LEV01.DGN` | `NEXUS_SRC_EXTRACTED` |

- **ISO mode**: Reads files directly from a CUE/BIN image via `nexus_iso_open_cue()`
- **Extracted mode**: Reads flat files from data directory (e.g., `DM.BIN`, `LEV*.DGN`)

### Audio Enable Flag

```c
engine->audio_enabled = 1;  // set at init time
```

The flag gates CD track changes — no track switch occurs if `audio_enabled` is false.

### CD track selection

The CUE/ISO receipt proves tracks 2–9, but the level-to-track selector is not
source-bound. `nexus_v1_cd_track_for_level()` therefore returns `-1`; the old
`2 + (level / 2)` table was removed as a host assumption.

### Track Change Flow

1. `nexus_v1_load_level()` loads the authenticated level and its SAL/MAP pair.
2. No CDDA track is selected from the level number.
3. Playback remains blocked until the original selector and consumer are
   captured.

### SDL_mixer boundary

```c
/* FUTURE: CD audio playback via SDL_mixer.
 * DM Nexus (Saturn) uses CD-DA tracks for music. */
```

No source-faithful host playback is admitted. The runtime remains fail-closed
and does not print or synthesize a guessed track selection.

## Data Files

No audio-specific data files in the V1 engine source. Audio is driven entirely by the CD image tracks.

## Comparison with DM1

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Music format | SONG.DAT ( sequenced) | CD-DA (Red Book) |
| Music tracks | 1 file, multiple sequences | 8 CD tracks (track 2-9) |
| Level mapping | Song indices in SONG.DAT | Selector owner not captured |
| SFX | SND3.DAT (PCM) | SAL/MAP directory receipts; codec/dispatch unbound |
| Voice | In dungeon data | No authenticated voice consumer |

## Status

- [ ] CD track mapping per level — source selector not recovered
- [ ] CD audio playback — Saturn CDDA owner/track handoff not captured
- [x] Bounded SAL DataID 0 PCM diagnostic decode — authenticated banks and
  memory/noise source descriptors are regression-tested against the external
  corpus; this does not authorize event dispatch or playback.
- [ ] SFX system — original selector, SDDRVS/SCSP voice ownership and playback
  are not captured
- [ ] Voice/dialogue — **Not implemented**
- [ ] Footstep audio — configured (`cfg->footstep_audio = 1`) but no playback

## Files

- `src/nexus/nexus_v1_engine.c` — audio init, track switching, enable flag
- `src/nexus/nexus_v1_game.c` — `nexus_v1_cd_track_for_level()`
- `src/nexus/nexus_v2_config.c` — `cfg->enhanced_audio`, `cfg->footstep_audio`
