# Nexus V1 — Sound Effects

## Overview

Sound effects (SFX) in Nexus V1 are **not currently implemented** in the engine source. Unlike DM1 (which has a full SND3.DAT PCM sound system), Nexus V1 does not reference any SFX data files or playback routines in the current codebase.

## Current Implementation Status

### Confirmed Absent

The following DM1 SFX infrastructure has **no equivalent** in Nexus V1:

| DM1 Component | Nexus V1 Equivalent |
|--------------|---------------------|
| `SND3.DAT` (PCM SFX) | None |
| `channel` / sound playback structs | None |
| `fs_sound_play()` | None |
| `FS_SOUND_*` event triggers | None |

Searches across all `src/nexus/` source files for `sfx`, `sound`, `SND`, `channel` return no SFX implementation.

### Audio Enable Flag

The only audio infrastructure present:

```c
engine->audio_enabled = 1;  // in nexus_v1_init()
```

This flag is used for CD track switching only — it does not gate SFX because no SFX system exists.

### Footstep Audio

In `src/nexus/nexus_v2_config.c`:
```c
cfg->enhanced_audio = 1;
cfg->footstep_audio = 1;
```

These are V2 config flags and do not have V1 runtime implementation.

## Potential Implementation Path

The Saturn version of Dungeon Master Nexus would likely have SFX data on the CD, possibly as:

1. **CD-DA SFX tracks** — dedicated audio tracks for sound effects (like music)
2. ** interleaved with game data** — SFX samples embedded in data files
3. **Resource fork or auxiliary data track** — separate data track for PCM samples

Without the actual CD image or extracted data, the SFX format and playback mechanism are **unknown and not implemented**.

## Comparison with DM1

| Aspect | DM1 V1 | Nexus V1 |
|--------|--------|----------|
| SFX file | SND3.DAT | Not implemented |
| SFX format | PCM (8-bit, various rates) | Unknown |
| Trigger system | FS_SOUND_* events | None |
| Footstep SFX | Yes (party footstep) | No |
| Combat SFX | Yes (weapon, impact) | No |
| UI SFX | Yes (menu clicks) | No |

## Files Examined

- `src/nexus/nexus_v1_engine.c` — no SFX references
- `src/nexus/nexus_v1_game.c` — no SFX references
- `src/nexus/nexus_v1_combat.c` — no sound calls
- `src/nexus/nexus_v1_creatures.c` — no sound calls
- `src/nexus/nexus_v1_magic.c` — no sound calls

## Status

- [ ] SFX data files — **Not located**
- [ ] SFX playback — **Not implemented**
- [ ] Combat SFX — **Not implemented**
- [ ] Footstep SFX — **Not implemented**
- [ ] UI SFX — **Not implemented**
- [ ] Creature SFX — **Not implemented**