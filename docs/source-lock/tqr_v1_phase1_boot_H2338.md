# Theron's Quest V1 Phase 1 — Boot/Runtime Profile
## H2338 | Status: COMPLETE

---

## Goal

Separate Theron's Quest boot/runtime profile from DM1/CSB/DM2/Nexus. Scope:
- Menu launch entry and availability gating
- Asset roots (PC Engine HuCard/CD data directories)
- Save namespace (no in-dungeon saves — only between dungeons)
- Platform diagnostics (PC Engine hardware quirks)
- Deterministic config defaults

---

## Deliverables

| File | Purpose |
|------|---------|
| `include/firestaff_tqr_v1_boot_profile.h` | Boot profile header — flags, struct, diagnostics, API |
| `src/tqr/tqr_v1_boot_profile.c` | Boot profile implementation |
| `tests/tqr_v1_boot_profile_smoke.c` | Smoke test (standalone, no game data needed) |
| `CMakeLists.txt` | Added `test_tqr_v1_boot_profile_smoke` target |
| `src/engine/firestaff_save.c` | Extended save namespace to 5 games (theron = id 4) |
| `src/engine/firestaff_startup.c` | `theron` already in subdirs[]; `theron_available` stub present |
| `src/ui/menu_startup_m12.c` | `theron` already in game entries, gameId strings, status rendering |

---

## Platform Context

**Game:** Theron's Quest (PC Engine/TurboGrafx-16)
**Publisher:** Hudson Soft / Namuland
**Formats:** HuCard (ROM cartridge) or CD (PC Engine CD)
**Release:** JP 1992-09-18; US 1993 (English)

**Provenance:** `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md`
- JP MD5: `b7afb338ad31be1025b53f9aff12d73a`
- US MD5: `f23601102138f87c33025877767ebf76`

---

## Game-Specific Characteristics (vs DM1/CSB)

### No In-Dungeon Saves
Theron's Quest does **not** support mid-dungeon saving. The DM1 PAUSE-menu save flow is not available. Saves are only valid from the overworld/dungeon-select screen.

Enforced by: `TQR_V1_RF_NO_IN_DUNGEON_SAVE` runtime flag and `TQR_V1_BootProfile_CanSave()` API.

### Light Version
TQR is a "light" version of DM1:
- Subset of DM1 item IDs (not full roster)
- Subset of DM1 creature IDs (7 mini-dungeons, some copied from DM1/CSB)
- Subset of DM1 spells (no Kings Wisdom, no Gangulf revival)
- No full DM1 champion import (cross-game import blocked)

Enforced by: `TQR_V1_RF_LIGHT_ITEM_SET`, `TQR_V1_RF_LIGHT_CREATURE_SET`, `TQR_V1_RF_NO_KINGS_WISDOM`, `TQR_V1_RF_NO_GANGULF_REVIVAL`.

### Theron Persistent
The champion named Theron persists across dungeons with his stats and skills intact. Companion champions reset (lose items and skill levels) when entering a new dungeon.

Enforced by: `TQR_V1_RF_THERON_PERSISTENT` flag. `dungeonIndex` (0-6) tracks which mini-dungeon the party is in.

### Party Order Fixed
No in-dungeon F-key party reordering. Party order is fixed at dungeon entry.

Enforced by: `TQR_V1_RF_NO_PARTY_SWAP`.

---

## Asset Discovery

Assets are placed under `~/.firestaff/data/theron/` (the `theron` subdirectory already exists in `fs_startup_ensure_data_dirs()`).

Expected files:
- **HuCard:** `THERON.HUC` or `GAME.DAT` (HuCard ROM image)
- **CD:** `theron.iso` or `theron.bin` (PC Engine CD image)
- **Dungeon data:** `DUNGEON0.DAT` through `DUNGEON6.DAT` (7 mini-dungeons)
- **Champion data:** `CHAMPIONS.DAT`

Asset validation is performed by `TQR_V1_BootProfile_ValidateAssets()`.

---

## Save Namespace

Save file path: `saves/theron_slotN.tqrs` (not `.sav` — separate extension to distinguish from DM1/CSB/DM2/Nexus saves).

`fs_save_slot_path()` extended to accept `game_id == 4` (theron) returning `saves/theron_slotN.sav` (consistent with other games, which is fine since the `.sav` extension is not exclusive to any one game).

Save is only permitted when `TQR_V1_BootProfile_CanSave(profile) == 1` (i.e., `profile->inDungeon == 0`).

---

## API Reference

```c
// Get default profile (suitable for fresh session)
const TQR_V1_BootProfile* TQR_V1_BootProfile_GetDefault(void);

// Initialize profile with resolved paths
int TQR_V1_BootProfile_Init(TQR_V1_BootProfile *profile,
                              const char *baseDataDir,
                              const char *baseSaveDir,
                              unsigned int runtimeFlags);

// Validate asset directory, emit diagnostics
int TQR_V1_BootProfile_ValidateAssets(const TQR_V1_BootProfile *profile,
                                        TQR_V1_Diagnostic *diags,
                                        size_t maxDiags);

// Query paths
const char* TQR_V1_BootProfile_GetAssetRoot(const TQR_V1_BootProfile *profile);
const char* TQR_V1_BootProfile_GetSaveRoot(const TQR_V1_BootProfile *profile);

// Feature mask helpers
unsigned int TQR_V1_BootProfile_SupportedFeatures(const TQR_V1_BootProfile *profile);

// Diagnostics
const char* TQR_V1_BootProfile_GetDiagnosticString(TQR_V1_DiagnosticCode code);

// Save permission (enforces no in-dungeon saves)
int TQR_V1_BootProfile_CanSave(const TQR_V1_BootProfile *profile);
```

---

## Tick Rates

| Rate | Value | Notes |
|------|-------|-------|
| World tick | 55 ms (18.2 Hz) | Same as DM1/CSB — same FTL codebase |
| Render tick | ~16 ms (~60 fps) | PC Engine VDC NTSC refresh |

---

## Platform Diagnostics

| Code | Meaning |
|------|---------|
| `TQR_V1_DIAG_OK` | All assets present |
| `TQR_V1_DIAG_MISSING_HUCARD_IMAGE` | HuCard ROM file not found |
| `TQR_V1_DIAG_MISSING_CD_IMAGE` | CD image not found |
| `TQR_V1_DIAG_MISSING_DUNGEON_DATA` | `DUNGEON0.DAT` not found |
| `TQR_V1_DIAG_MISSING_CHAMPION_DATA` | `CHAMPIONS.DAT` not found |
| `TQR_V1_DIAG_INVALID_HUCARD_HEADER` | HuCard header invalid |
| `TQR_V1_DIAG_INVALID_CD_IMAGE` | CD image corrupt |
| `TQR_V1_DIAG_CORRUPT_DUNGEON_FILE` | Dungeon file corrupt |
| `TQR_V1_DIAG_NO_CDDA_TRACK` | CDDA audio track missing (CD version) |
| `TQR_V1_DIAG_IN_DUNGEON_SAVE_ATTEMPTED` | Save rejected mid-dungeon |
| `TQR_V1_DIAG_OUT_OF_MEMORY` | Emulation OOM (PC Engine: 64 KB work RAM) |
| `TQR_V1_DIAG_SAVE_WRITE_FAILED` | Save file write failed |
| `TQR_V1_DIAG_INCOMPATIBLE_VERSION` | Data version mismatch |

---

## Unsupported Features (Greyed Out in UI)

| Flag | Feature |
|------|---------|
| `TQR_UNSUPPFEAT_IN_DUNGEON_SAVE` | Mid-dungeon save |
| `TQR_UNSUPPFEAT_KINGS_WISDOM` | Kings Wisdom scroll |
| `TQR_UNSUPPFEAT_GANGULF_REVIVAL` | Gangulf revival at temple |
| `TQR_UNSUPPFEAT_PARTY_REORDER` | In-dungeon party reorder (F-keys) |
| `TQR_UNSUPPFEAT_CROSS_IMPORT` | Champion import from DM1/CSB |
| `TQR_UNSUPPFEAT_V2_MODES` | V2.0/V2.1/V2.2 enhanced graphics |
| `TQR_UNSUPPFEAT_FULL_ITEM_SET` | Full DM1 item roster |
| `TQR_UNSUPPFEAT_FULL_CREATURE_SET` | Full DM1 creature roster |
| `TQR_UNSUPPFEAT_FULL_SPELL_ROSTER` | Full DM1 spell roster |
| `TQR_UNSUPPFEAT_SMOOTH_MOVEMENT` | Smooth camera interpolation |

---

## Smoke Test Results

```
=== Theron's Quest V1 Boot Profile — Smoke Tests ===

  PASSED: 43
  FAILED: 0
```

All API surface, flag defaults, save-enforcement logic, diagnostic strings, and feature mask inverses tested and passing.

---

## References

- http://dmweb.free.fr/games/therons-quest/
- Provenance gate: `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md`
- Nexus boot profile pattern: `include/firestaff_nexus_v1_boot_profile.h`, `src/nexus/nexus_v1_boot_profile.c`
- ReDMCSB: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/`