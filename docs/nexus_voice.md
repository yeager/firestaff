# Nexus V1 — Voice / Dialogue

## Overview

Nexus V1 does **not** have a traditional voice/dialogue system. Unlike DM1 which has no voice either, Nexus might have had voice in FMV cutscenes, but the main game dungeon interactions are text-only.

## Key Findings

### No Voice Files in Classification

The `NEXUS_FILE_CLASSIFICATION.md` lists no voice-specific files. The audio infrastructure consists of:
- `SNDLEV00-15.SAL` (290-460 KB each) — per-level sound effect banks
- `SNDLEV00-15.MAP` (66-90 bytes each) — sound mapping tables
- `SDDRVS.TSK` (26 KB) — sound driver task
- CD-DA audio tracks 2-9 for music

No dedicated "voice" or "dialogue" files are listed.

### FMV Cutscenes (AVI)

Three AVI files exist:
- `DMV0.AVI` (34 MB) — intro
- `DMV1.AVI` (28 MB) — unknown (mid-game or ending)
- `DMV2.AVI` (39 MB) — ending

These are Saturn AVI format, not standard AVI. They may contain voice audio as part of the video stream.

### Text-Based Communication

Per `docs/nexus_text.md` and `docs/nexus_features.md`:
- Champion roster uses 8 Japanese names (ASCII + Shift-JIS)
- Spell names are Shift-JIS
- Monster names are Shift-JIS
- No speech synthesis or recorded dialogue for dungeon interaction
- UI text rendered via `FONT256.S2D` (Saturn SCR font)

### Sound Effect Banks vs Voice

`SNDLEV00-15.SAL` are described as "sound effect banks" per level, not voice. The `.MAP` files (66-90 bytes each) are "sound mapping tables" — likely index maps for triggering SFX by event ID.

### Comparison with DM1/CSB

| Aspect | DM1/CSB | Nexus V1 |
|--------|---------|----------|
| Voice/dialogue | None | None (except possibly in AVI FMV) |
| Text rendering | ASCII | ASCII + Shift-JIS |
| Champion speech | None | None |
| Creature sounds | SND3.DAT (PCM) | SNDLEV*.SAL (per-level SFX banks) |
| Menu interaction | Text only | Text only |
| FMV audio | None | AVI embedded audio |

## Status

- [ ] Voice/dialogue files — **None located** (text-only dungeon interaction)
- [ ] Champion speech — **Not implemented** (text only)
- [ ] FMV voice audio — **In AVI files** (format unknown, Saturn AVI codec)
- [ ] Speech synthesis — **None**
- [ ] Voice triggers — **None**

## Files

- `DMV0.AVI`, `DMV1.AVI`, `DMV2.AVI` — FMV files (may contain voice audio)
- `SNDLEV00-15.SAL` — sound effect banks (not voice)
- `SNDLEV00-15.MAP` — sound mapping tables
- `SDDRVS.TSK` — sound driver (SFX playback only, no voice)