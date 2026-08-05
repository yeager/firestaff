# Nexus V1 — Sound Effects

## Overview

Nexus V1 does not yet expose source-faithful SFX playback, but the engine now
loads and verifies the retail SAL/MAP corpus and decodes the bounded parts of
the SAL tone bank supported by the DMWeb format evidence. Playback remains
fail-closed until Saturn event-to-selector and SDDRVS handoff evidence exists.

## Current Implementation Status

### Confirmed source data

The supplied retail corpus contains `SNDLEV00.SAL` through `SNDLEV15.SAL`,
their matching `.MAP` files, and `SDDRVS.TSK`. Firestaff retains hash-bound
source provenance for all sixteen pairs. MAP records retain the observed raw
selector, attribute, big-endian SAL offset, and size, rejecting duplicate or
out-of-bounds windows. The SAL route decodes the DMWeb DataID 0 tone-bank
directory, including offsets, variable entries, PCM-width and source-control
bits, and bounded sample metadata. The real corpus profiles 45 decoded tones
from 49 directory entries per level; this is format evidence, not event proof.

The following DM1 SFX infrastructure has **no source-authenticated equivalent** in Nexus V1:

| DM1 Component | Nexus V1 Equivalent |
|--------------|---------------------|
| `SND3.DAT` (PCM SFX) | Per-level `SNDLEV*.SAL` + `SNDLEV*.MAP` |
| `channel` / sound playback structs | Saturn `SDDRVS.TSK` handoff not decoded |
| `fs_sound_play()` | Host dispatch scaffold, all events unmapped |
| `FS_SOUND_*` event triggers | Saturn event→MAP-selector mapping unproven |

The Nexus sound module parses real assets and emits runtime receipts, but does
not claim that a raw MAP selector is a gameplay event. The event table defaults
to unmapped and no production event calls playback.

### Audio Enable Flag

The only audio infrastructure present:

```c
engine->audio_enabled = 1;  // in nexus_v1_init()
```

This flag is used for CD track switching; SFX dispatch remains separately
blocked by the runtime receipt and unmapped event table.

### Footstep Audio

In `src/nexus/nexus_v2_config.c`:
```c
cfg->enhanced_audio = 1;
cfg->footstep_audio = 1;
```

These are V2 config flags and do not have V1 runtime implementation.

## Remaining implementation boundary

The Saturn version of Dungeon Master Nexus would likely have SFX data on the CD, possibly as:

1. **CD-DA SFX tracks** — dedicated audio tracks for sound effects (like music)
2. ** interleaved with game data** — SFX samples embedded in data files
3. **Resource fork or auxiliary data track** — separate data track for PCM samples

The actual CD image and extracted SAL/MAP data are present. The remaining
unknown is the complete SDDRVS task ABI/event dispatch and the runtime binding
from host events to the DataID 0 entry table. Until that handoff is decoded
from source or an authenticated Saturn trace, playback stays blocked.

## Comparison with DM1

| Aspect | DM1 V1 | Nexus V1 |
|--------|--------|----------|
| SFX file | SND3.DAT | SNDLEV*.SAL + SNDLEV*.MAP |
| SFX format | PCM (8-bit, various rates) | DataID 0 directory/metadata decoded; event semantics unproven |
| Trigger system | FS_SOUND_* events | Saturn event→MAP-selector mapping unproven |
| Footstep SFX | Yes (party footstep) | No |
| Combat SFX | Yes (weapon, impact) | No |
| UI SFX | Yes (menu clicks) | No |

## Files Examined

- `src/nexus/nexus_v1_sound.c` — bounded SAL/MAP decode and fail-closed dispatch
- `src/nexus/nexus_v1_audio_receipt.c` — hash-bound audio package receipts
- `tests/test_nexus_v1_sound_runtime_receipt.c` — real-corpus runtime proof
- `docs/wiki/Nexus-SAL-MAP-Internals.md` — field-level implementation boundary

## Status

- [x] SFX data files — **located and provenance-bound**
- [x] SAL DataID 0 directory and bounded tone metadata — **decoded from real corpus**
- [x] MAP records — **bounded and retained without assigning event meaning**
- [ ] SFX playback — **blocked pending Saturn event/SDDRVS evidence**
- [ ] Combat SFX — **Not implemented**
- [ ] Footstep SFX — **MAP event binding and SDDRVS handoff remain unproven**
- [ ] UI SFX — **Not implemented**
- [ ] Creature SFX — **Not implemented**
