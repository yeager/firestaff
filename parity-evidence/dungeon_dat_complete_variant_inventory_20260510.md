# Complete Variant-Aware DUNGEON.DAT Inventory

**Timestamp:** 2026-05-10T11:12:08Z
**Model:** qwen3.6:35b, not Qwen 3.6 Plus; 65k context
**Source root:** `/Users/bosse/.openclaw/data/firestaff-original-games/DM/`

## Counts

- **candidate_files:** 5
- **distinct_sha256:** 3
- **symlinks:** 1
- **variant_families:** 3
- **greatstone_dungeon_resources:** 0
- **greatstone_matched_resources:** 0
- **redmcsb_source_files_with_dungeon_refs:** 99

## SHA256 Groups (Identical Files)

- **d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85** (3 identical copies):
  - `_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT`
  - `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT`
  - `_canonical/dm1/DUNGEON.DAT`

## Variant Families

### DM1 PC34 EN (3 files)
- `_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT`
- `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT`
- `_canonical/dm1/DUNGEON.DAT`
### DM1 PC34 FR (1 files)
- `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEONF.DAT`
### DM1 PC34 GE (1 files)
- `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEONG.DAT`

## File Details

### `DUNGEON.DAT`
- **Path:** `_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT`
- **Size:** 33357 bytes
- **SHA256:** `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- **Game:** DM1
- **Platform:** PC DOS
- **Language:** English
- **Variant family:** DM1 PC34 EN
- **Confidence:** high
- **Symlink:** False

### `DUNGEON.DAT`
- **Path:** `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT`
- **Size:** 33357 bytes
- **SHA256:** `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- **Game:** DM1
- **Platform:** PC DOS
- **Language:** English
- **Variant family:** DM1 PC34 EN
- **Confidence:** high
- **Symlink:** False

### `DUNGEONG.DAT`
- **Path:** `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEONG.DAT`
- **Size:** 33705 bytes
- **SHA256:** `b2478d5cc3213725329cb42684f0309e5e95fd6789dc2cb0c3377b178ad75817`
- **Game:** DM1
- **Platform:** PC DOS
- **Language:** German
- **Variant family:** DM1 PC34 GE
- **Confidence:** high
- **Symlink:** False

### `DUNGEONF.DAT`
- **Path:** `_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEONF.DAT`
- **Size:** 33687 bytes
- **SHA256:** `290543621ae7c465fee9651c4d3c44f5dc268f5e16fffc75da82a440274c0571`
- **Game:** DM1
- **Platform:** PC DOS
- **Language:** French
- **Variant family:** DM1 PC34 FR
- **Confidence:** high
- **Symlink:** False

### `DUNGEON.DAT`
- **Path:** `_canonical/dm1/DUNGEON.DAT`
- **Size:** 33357 bytes
- **SHA256:** `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- **Game:** DM1
- **Platform:** PC DOS
- **Language:** English
- **Variant family:** DM1 PC34 EN
- **Confidence:** high
- **Symlink:** True

## Greatstone Cross-Reference

Greatstone inventory JSON not available. No cross-reference possible.

## ReDMCSB Source References

Source files with DUNGEON.DAT references: 99
Total reference points: 738

- **asset_status_m12.c** (lines 70-76, 5 refs)
  - L70: `"DUNGEON.DAT",`
  - L71: `"DUNGEONF.DAT",`
  - L72: `"DUNGEONG.DAT",`
- **base_frontend_pc34.c** (lines 13-13, 1 refs)
  - L13: `BOOLEAN G0298_B_NewGame = C001_MODE_LOAD_DUNGEON;`
- **custom_dungeon_m12.c** (lines 4-124, 10 refs)
  - L4: `* Scans dataDir/custom/ for subdirectories containing a DUNGEON.DAT,`
  - L8: `* Depends on: memory_dungeon_dat_pc34_compat.h (DungeonHeader_Compat).`
  - L25: `/* Minimum plausible DUNGEON.DAT: header (44) + at least 1 map desc (16). */`
- **custom_dungeon_m12.h** (lines 7-62, 5 refs)
  - L7: `* Scans a "custom/" subdirectory under the data dir for DUNGEON.DAT`
  - L8: `* files.  Each immediate subdirectory containing a DUNGEON.DAT is`
  - L40: `char path[CUSTOM_DUNGEON_PATH_MAX];           /* full path to DUNGEON.DAT */`
- **data_validator_m12.c** (lines 158-166, 4 refs)
  - L158: `{"csb", "HCSB.DAT", "708e113c869ab922633e885aa72a3c77", "CSB Amiga Utility Disk `
  - L159: `{"csb", "HCSB.DAT", "7496b3b8b9ff6e2368eac9a16be8230b", "CSB Amiga Utility Disk `
  - L164: `{"csb", "HCSB.DAT", "bbf3ada2da9722577feea4fa213b32f1", "CSB Amiga Utility Disk `
- **diag_offsets.c** (lines 7-13, 3 refs)
  - L7: `struct DungeonDatState_Compat state;`
  - L12: `if (argc < 2) { fprintf(stderr, "Usage: %s <DUNGEON.DAT>\n", argv[0]); return 1;`
  - L13: `if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) { fprintf(stderr, "FAI`
- **diag_things.c** (lines 7-15, 5 refs)
  - L7: `struct DungeonDatState_Compat state;`
  - L11: `if (argc < 2) { fprintf(stderr, "Usage: %s <DUNGEON.DAT>\n", argv[0]); return 1;`
  - L13: `if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) { fprintf(stderr, "FAI`
- **dm1_v1_collision_door_pc34_compat.c** (lines 55-448, 9 refs)
  - L55: `const struct DungeonDatState_Compat* dungeon,`
  - L60: `const struct DungeonMapDesc_Compat* map;`
  - L114: `const struct DungeonDatState_Compat* dungeon,`
- **dm1_v1_collision_door_pc34_compat.h** (lines 151-231, 6 refs)
  - L151: `const struct DungeonDatState_Compat* dungeon,`
  - L165: `const struct DungeonDatState_Compat* dungeon,`
  - L184: `const struct DungeonDatState_Compat* dungeon,`
- **dm1_v1_dungeon_data_pc34_compat.c** (lines 5-31, 4 refs)
  - L5: `* dm1_v1_dungeon_loader, dm1_v1_group_management, dm1_v1_event_timer,`
  - L28: `bool m11_dd_load_dungeon(M11_DD_DungeonData *dd, const char *dungeon_dat_path)`
  - L30: `if (!dd || !dungeon_dat_path) return false;`
- **dm1_v1_dungeon_data_pc34_compat.h** (lines 45-125, 5 refs)
  - L45: `#include "dm1_v1_dungeon_loader_pc34_compat.h"`
  - L76: `/* ─ Dungeon structure (from dungeon_loader) ─ */`
  - L121: `* Load dungeon from DUNGEON.DAT and populate the store.`
- **dm1_v1_dungeon_loader_pc34_compat.c** (lines 1-72, 4 refs)
  - L1: `#include "dm1_v1_dungeon_loader_pc34_compat.h"`
  - L19: `bool m11_dl_load_from_file(M11_DL_DungeonState *state, const char *path) {`
  - L24: `if (fread(&state->header, sizeof(M11_DL_DungeonHeader), 1, f) != 1) {`
- **dm1_v1_dungeon_loader_pc34_compat.h** (lines 1-77, 6 refs)
  - L1: `#ifndef FIRESTAFF_DM1_V1_DUNGEON_LOADER_PC34_COMPAT_H`
  - L2: `#define FIRESTAFF_DM1_V1_DUNGEON_LOADER_PC34_COMPAT_H`
  - L50: `} M11_DL_DungeonHeader;`
- **dm1_v1_engine_pc34_compat.c** (lines 30-110, 4 refs)
  - L30: `"dm1_v1_dungeon_loader_pc34_compat",`
  - L108: `if (config->dungeon_dat_path) {`
  - L109: `if (!m11_dd_load_dungeon(&engine->dungeonData,`
- **dm1_v1_engine_pc34_compat.h** (lines 68-126, 2 refs)
  - L68: `const char *dungeon_dat_path;    /* Path to DUNGEON.DAT */`
  - L126: `*   5. Dungeon data load (DUNGEON.DAT + GRAPHICS.DAT)`
- **dm1_v1_movement_command_core_pc34_compat.c** (lines 95-122, 2 refs)
  - L95: `const struct DungeonDatState_Compat* dungeon,`
  - L122: `const struct DungeonDatState_Compat* dungeon,`
- **dm1_v1_movement_command_core_pc34_compat.h** (lines 63-63, 1 refs)
  - L63: `const struct DungeonDatState_Compat* dungeon,`
- **dm1_v1_movement_pipeline_pc34_compat.c** (lines 42-246, 4 refs)
  - L42: `const struct DungeonDatState_Compat* dungeon,`
  - L47: `const struct DungeonMapDesc_Compat* map;`
  - L180: `const struct DungeonDatState_Compat* dungeon,`
- **dm1_v1_movement_pipeline_pc34_compat.h** (lines 150-150, 1 refs)
  - L150: `const struct DungeonDatState_Compat* dungeon,`
- **dm1_v1_room_transition_pc34_compat.c** (lines 88-88, 1 refs)
  - L88: `/* DUNGEON.DAT / current-map data consequences.`
- **dm1_v1_save_load.c** (lines 262-262, 1 refs)
  - L262: `*  10. Read dungeon via F0434_IsLoadDungeonSuccessful_CPSC`
- **dm1_v1_save_load.h** (lines 151-151, 1 refs)
  - L151: `*   - Reads dungeon via F0434_IsLoadDungeonSuccessful_CPSC`
- **dm1_v2_viewport_renderer_pc34.c** (lines 309-441, 8 refs)
  - L309: `int dm1_v2_vp_dungeon_dat_init(DM1_V2_DungeonDatState* outState,`
  - L335: `MAP.RawMapDataByteOffset. The canonical PC34 DUNGEON.DAT stores a trailing`
  - L364: `int dm1_v2_vp_dungeon_dat_get_square_raw(const DM1_V2_DungeonDatState* state,`
- **dm1_v2_viewport_renderer_pc34.h** (lines 96-198, 4 refs)
  - L96: `} DM1_V2_DungeonDatState;`
  - L195: `int dm1_v2_vp_dungeon_dat_init(DM1_V2_DungeonDatState* outState, const uint8_t* `
  - L196: `int dm1_v2_vp_dungeon_dat_get_square_raw(const DM1_V2_DungeonDatState* state, in`
- **firestaff_base_frontend_probe.c** (lines 20-20, 1 refs)
  - L20: `BOOLEAN G0298_B_NewGame = C001_MODE_LOAD_DUNGEON;`
- **firestaff_combined_frontends_probe.c** (lines 747-14736, 13 refs)
  - L747: `DUNGEON_HEADER* G0278_ps_DungeonHeader;`
  - L1106: `for (L0255_i_TargetMapIndex = 0; L0255_i_TargetMapIndex < G0278_ps_DungeonHeader`
  - L1243: `if (G0283_pT_SquareFirstThings[G0278_ps_DungeonHeader->SquareFirstThingCount - 1`
- **firestaff_combined_with_real_byteops_probe.c** (lines 745-14791, 13 refs)
  - L745: `DUNGEON_HEADER* G0278_ps_DungeonHeader;`
  - L1104: `for (L0255_i_TargetMapIndex = 0; L0255_i_TargetMapIndex < G0278_ps_DungeonHeader`
  - L1241: `if (G0283_pT_SquareFirstThings[G0278_ps_DungeonHeader->SquareFirstThingCount - 1`
- **firestaff_extracted_frontends_probe.c** (lines 745-14872, 13 refs)
  - L745: `DUNGEON_HEADER* G0278_ps_DungeonHeader;`
  - L1104: `for (L0255_i_TargetMapIndex = 0; L0255_i_TargetMapIndex < G0278_ps_DungeonHeader`
  - L1241: `if (G0283_pT_SquareFirstThings[G0278_ps_DungeonHeader->SquareFirstThingCount - 1`
- **firestaff_m10_champion_lifecycle_probe.c** (lines 68-779, 8 refs)
  - L68: `"Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);`
  - L698: `* Block K — Loop guard + DUNGEON.DAT spot-check (3 invariants)`
  - L740: `struct DungeonDatState_Compat dungeon;`
- **firestaff_m10_combat_probe.c** (lines 174-696, 7 refs)
  - L174: `fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);`
  - L665: `struct DungeonDatState_Compat dungeon;`
  - L672: `if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon)) {`

... and 69 more files (see JSON for complete list)

## Hard Warnings

- **PC 3.4 multilingual uses separate dungeon files:** `dungeon.dat` (EN), `dungeonf.dat` (FR), `dungeong.dat` (GE). These are NOT interchangeable.
- **Never compare dungeon maps/start positions/dimensions/invariants without SHA256 + variant name.** Different files may encode different dungeon layouts.
- **Greatstone catalogue labels are not proof of binary identity.** A label like 'DM PC 3.4 DUNGEON.DAT' does not guarantee the file matches any specific variant.
- **Different platforms may encode dungeon data differently.** Amiga, Atari ST, FM Towns, PC-98, X68000, SNES, and PC DOS may use different binary formats for dungeon data.
- **CSB uses `CSB.DAT` (not `DUNGEON.DAT`).** DM2 uses `DM2DUNGEON.DAT`. Do not confuse these with DM1's `DUNGEON.DAT`.
- **Save games use compressed dungeon format** (signature 0x8104). These are NOT the same as release DUNGEON.DAT files.

## Blockers / Gaps

- Greatstone inventory JSON not found at any candidate path. Cross-reference unavailable.
- Missing: CSB.DAT (CSB (Chaos Strikes Back)) — no local copy found.
- Missing: DM2DUNGEON.DAT (DM2 (Skullkeep)) — no local copy found.
- Missing: HCSB.DAT (CSB utility disk dungeon data) — no local copy found.

## Recommended Next Firestaff Actions

- Acquire actual DUNGEON.DAT files (PC 3.4 EN/FR/GE) and place in canonical anchors directory.
- Acquire CSB.DAT (Amiga) and DM2DUNGEON.DAT for cross-game parity.
- Obtain Greatstone dungeon_map resource inventory JSON for cross-reference.
- Verify ReDMCSB source archive (ReDMCSB_WIP20210206) is available for source audit.
- Run inventory again after files are acquired to populate SHA256 groups and variant families.
