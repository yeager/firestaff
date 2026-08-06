# Nexus V1 — Audio Format

> Statusen är källbunden metadata, inte färdig playback. Se
> [`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md).

## Overview

Dungeon Master Nexus audio consists of two distinct systems:
1. **CD-DA music** — Red Book Audio tracks 2-9 (disc-layout receipt only)
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
| Level selection | Not source-bound |
| Data source | Tracks 2-9 on Saturn CD |

### Track-to-level binding

The CUE/ISO receipt proves the existence and order of tracks 2–9. The
available DM.BIN/disassembly and format references do not prove which track a
dungeon level selects. Firestaff therefore keeps the level selector opaque and
returns `-1` until an original Saturn consumer or authenticated runtime trace
binds it.

## Per-Level SFX Banks

Located in Track 1 of the ISO (MODE1/2352, game data track).

### SNDLEV*.SAL Files (retail per-level banks)

- 16 files: `SNDLEV00.SAL` through `SNDLEV15.SAL`
- One per dungeon level
- Format: bounded DMWeb DataID 0 tone-bank directory plus additional data
- Contains: per-level tone-bank data and other SAL regions
- Firestaff decodes directory offsets and bounded 8/16-bit sample metadata;
  it does not assume that every remaining region is PCM

### SNDLEV*.MAP Files (66-90 bytes each)

- 16 files: `SNDLEV00.MAP` through `SNDLEV15.MAP`
- Small index/mapping files
- Records are parsed as raw selector/attribute/SAL offset/size fields
- Purpose and event meaning are not assigned until Saturn evidence proves it

### Comparison with DM1

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| SFX file | SND3.DAT (global) | SNDLEV00-15.SAL (per-level) |
| SFX format | PCM 8-bit | DataID 0 directory and bounded 8/16-bit metadata decoded |
| SFX size | ~28 KB | 290-460 KB per level |
| Mapping file | None | SNDLEV00-15.MAP (66-90 B) |
| Music | SONG.DAT (sequenced) | CD-DA tracks 2–9; level selector unbound |

Per-level SFX allows different sound environments per dungeon depth — deeper levels could have more ominous SFX.

## Sound Driver

### SDDRVS.TSK (26 KB)

- "Sound DRiVerS TaSK" — Saturn SH-2 sound driver
- 26 KB — compact driver binary
- Loads SNDLEV*.SAL and retains bounded MAP windows; event playback is gated
  until the Saturn event-to-selector and driver handoff are authenticated
- Retailens `SDDRVS.TSK` är en verifierad task-identitet. Dess event- och
  selector-ABI är inte fastställd; “likely” är inte ett runtime-bevis.

## Audio Engine (Firestaff Implementation)

In `src/nexus/nexus_v1_engine.c`:

```c
engine->audio_enabled = 1;  // audio subsystem init

int nexus_v1_load_level(...) {
    int new_track = nexus_v1_cd_track_for_level(level);
    /* new_track is currently -1: the Saturn level-to-CDDA selector is not
       source-bound, so no track is selected from the level number. */
}
```

Current state: the CDDA track layout is retained as a receipt, but no level
track number is computed or stored. The sound engine now
receives the active Nexus data root from `nexus_v1_engine.c`; any future
verified host CD-DA materialization is therefore resolved against the selected
source root, not a hardcoded `HOME/.firestaff/data/nexus` path. The retail
CUE/ISO AUDIO tracks are still not converted to host files here, so playback
remains gated and no substitute WAV/OGG/MP3 is fabricated.

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
| SFX banks | SAL DataID 0 directory + bounded tone metadata | ISO Track 1 (SNDLEV*.SAL) | Real files loaded/provenance-bound; playback blocked |
| SFX mapping | MAP format (66-90 B) | ISO Track 1 (SNDLEV*.MAP) | Bounded record parsing; event semantics unproven |
| Sound driver | SH-2 binary | SDDRVS.TSK | Not reverse-engineered |
| FMV audio | Saturn AVI codec | DMV*.AVI | Not implemented |

## Related Files

- `src/nexus/nexus_v1_engine.c` — audio enable, track switching
- `src/nexus/nexus_v1_game.c` — `nexus_v1_cd_track_for_level()`
- `docs/NEXUS_FILE_CLASSIFICATION.md` — file inventory
- `docs/NEXUS_PLAN.md` — audio implementation plan
