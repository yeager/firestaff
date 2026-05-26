# CSB V1 Phase 1 — Boot / Runtime Profile
**Source-lock doc · H2231 · audit date: 2026-05-26**

---

## What this phase delivers

| Deliverable | File(s) | Description |
|-------------|---------|-------------|
| Runtime profile header | `include/csb_v1_runtime_pc34_compat.h` | Types, enums, API declarations |
| Runtime profile impl | `src/csb/csb_v1_runtime_pc34_compat.c` | Full implementation |
| Source-lock doc | `docs/source-lock/csb_v1_phase1_boot_runtime_profile_H2231.md` | This file |

Phase 1 creates a **CSB-specific runtime profile** that is separate from the DM1 profile.  All subsequent CSB V1 phases (2–7) build on this profile.

---

## Scope: what IS and IS NOT in this phase

### In scope (this doc)

- **Asset discovery** differences from DM1
  - CSB hashes: dungeon `6695d2acebce49f95db1d8f3a5c733de`, graphics files `CSBGRAPH.DAT` / `CSB.DAT`
  - Hash-based discovery (replacing filename-based)
  - Game variant detection (PC/Amiga/ST by MD5)

- **Menu launch** differences from DM1
  - Same M12 launcher entry point (`gameId = "csb"`)
  - CSB-specific ENTRANCE sequence uses `C28_ENTRANCE_CSB` palette index
  - Title screen and startup flow from ReDMCSB ENTRANCE.C

- **Save namespace** differences from DM1
  - Save file: `csb_save_N.fsav` (slot 0-9) in `Firestaff/csb/saves/`
  - Save directory: `~/.local/share/firestaff/csb/saves/` (Linux) / `~/Library/Application Support/Firestaff/csb/saves/` (macOS) / `%APPDATA%\Firestaff\csb\saves\` (Windows)
  - Header: MAGIC `0x43534201` (`CSB\1`) vs DM1 `0x444D0001` (`DM\0\1`)
  - Deobfuscation key index: `C29` (CSB) vs `C10` (DM1) — ReDMCSB SAVEHEAD.C lines 44-47

- **Deterministic tick config**
  - 55ms nominal tick (same as DM1; GAMELOOP 55ms periodicity)
  - `uint64_t` game_ticks accumulator for headless probing
  - Separate difficulty scale: `100/125/150/200` (×100% = +0/+25/+50/+100% creature stats per champion)

- **Variant-specific diagnostics**
  - 8 known variants (PC3.4 EN, PC3.4 Multi, ST2.0, ST2.1, Amiga3.5 EN, Amiga3.5 Multi, ST-F20J, ST-F20E)
  - MEDIA reference tag for each variant
  - MD5 for each variant's gfx and dungeon assets

- **Chaos Magic init at boot**
  - `CSB_V1_ChaosMagicState`  — spell grid version, chaos_level, magic_initialized
  - `caster.c F0211_CASTER_ClearSpellEffects` fired at dungeon load

### Out of scope (deferred to later phases)

| Deferred to | Topic |
|-------------|-------|
| Phase 2 | Dungeon data model (hash-gated load, F0237 differences from DM1) |
| Phase 3 | Wall/door/floor rendering differentials |
| Phase 4 | Mechanics parity (sensors, actuators, teleporters, pits) |
| Phase 5 | Creature roster, AI, attack bytes |
| Phase 6 | Utility disk champion import flow |
| Phase 7 | Verification probes and fixtures |

---

## Architecture: CSB Runtime Profile vs DM1

```
firestaff_game_loop.c                     (FS_GameLoop)
    game == FS_GAME_CSB
         │
         ▼
  FS_GameState.csb_viewport  ← only viewport subset, not profile
  FS_GameConfig.game == CSB  ← enum, used to branch data_dir
         │
         ▼  ← Phase 1 adds a proper CSB_V1_RuntimeProfile here
  CSB_V1_RuntimeProfile  ← owns: dungeon paths, variant, chaos magic,
                             party pos, tick state, state machine
         │
         ├─ boots dungeon by CSB hash
         ├─ detects variant by gfx + dungeon MD5
         ├─ initializes Chaos Magic
         ├─ owns save namespace: csb_save_%d.fsav in csb/saves/
         └─ drives tick quantum at 55ms/V1-tick
```

### Key difference from DM1 V2 runtime profile

```
DM1 V2 profile:  owns V2 presentation state and V2 settings
                (dm1_v2_presentation_profile_pc34.h)

CSB V1 profile: owns CSB-specific boot/dungeon/variant/chaos-magic state
                No V2 rendering (yet — V2 phases are separate)
                The profile is a CSB logical runtime descriptor,
                NOT a V2 presentation layer
```

---

## ReDMCSB Source References

### Boot / Entry

| Function | File | Topic |
|----------|------|-------|
| `F0806_ENTRANCE_INT` | ENTRANCE.C:80 | Game boot and loop |
| `F0807_ENTRANCE_DrawAnimationStep` | ENTRANCE.C:85 | Intro door animation |
| `F0579_ENTRANCE_InitializeBitPlanes` | ENTRANCE.C:1095 | Graphics initialization |
| `F0580_ENTRANCE_DrawDoorAnimationStep` | ENTRANCE.C:1123 | Door animation frame |
| `F0581_ENTRANCE_BlitDoors` | ENTRANCE.C:1133 | Door blit |
| `G0298_B_NewGame` | ENTRANCE.C:434 | State machine control |
| `C28_ENTRANCE_CSB` | ENTRANCE.C:437 | CSB-specific palette index |
| `G0309_i_PartyMapIndex` | ENTRANCE.C:68,410 | Party map index at start |
| `M567_COMMAND_ENTRANCE_DRAW_CREDITS` | ENTRANCE.C:1091 | Credits command |

### Save / Load

| Function | File | Topic |
|----------|------|-------|
| `F0433_STARTEND_ProcessCommand140_SaveGame` | LOADSAVE.C:550 | Save game |
| `F0435_STARTEND_LoadGame` | LOADSAVE.C:2192 | Load game |
| `F0429_IsReadSaveHeaderSuccessful` | SAVEHEAD.C:128 | Header verification |
| `F0430_IsWriteObfuscatedSaveHeaderSuccessful` | SAVEHEAD.C:67 | Header write |
| `F0417_SAVEUTIL_GetChecksumAndObfuscate` | LOADSAVE.C:240 (ref) | Obfuscation for MEDIA187 / MEDIA332 |
| `F0414_SAVEUTIL_ReplaceTildeByDriveLetterInString` | SAVEPATH.C | Save path substitution |

### Dungeon / World

| Function | File | Topic |
|----------|------|-------|
| `F0237_DUNGEON_DungeonLoad` | DUNGEON.C:1371-1391 | Hash-gated dungeon load |
| `G0525_GameID_AndDungeonID_AreValid_CPSD` | DECOMPDU.C | Game/dungeon ID validator |
| `G0534` | DECOMPDU.C | Additional save header data (134 bytes) |

### Chaos Magic

| Function | File | Topic |
|----------|------|-------|
| `F0211_CASTER_ClearSpellEffects` | CASTER.C | Spell grid reset at world load (boot) |
| `F0213_CASTER_` | CASTER.C | Per-square invocation slots |

### Utility Disk

| Constant / String | File | Topic |
|------------------|------|-------|
| `G3764_THAT_S_THE_CSB_UTILITY_DISK` | CEDTINC7.C | Utility disk prompt |
| `G3921 PLEASE_INSERT_UTIL_DISK` | CEDTDATA.C | Insert utility disk string |
| `F0428_RequireGameDiskInDrive` | REQDISK.C | Disk-in-drive check |
| `F0452_GetDiskTypeInDrive_CPSB` | REQDISK.C | Disk type: game vs save vs utility |

---

## Asset Hash Table

From `asset_status_m12.c` (`g_csbVersions[]`, confirmed all platforms):

| Variant | Platform | GRAPHICS.DAT MD5 |
|---------|----------|-----------------|
| PC 3.4 EN | DOS | `61fbfd56887c94adc26888a9491c6611` |
| ST 2.0/2.1 EN | Atari ST | `ebf6a57af3f27782e358c0490bfd2f2e` |
| Amiga 3.5 EN | Amiga | `291e1bc6803e3dc4b974c60117ca5d68` |
| Amiga 3.5 Multi | Amiga | `cefaddfdf5651df2c91f61b5611a8362` |

All CSB platforms share same dungeon hash: `6695d2acebce49f95db1d8f3a5c733de`

---

## Save Namespace

### File naming

| Game | File pattern | Example |
|------|-------------|---------|
| CSB (this phase) | `csb_save_%d.fsav` | `csb_save_0.fsav` |
| DM1 | `save_%02u.dat` | `save_00.dat` |
| DM2 | `dm2_save_%d.fsav` | TBD |
| Nexus | `nexus_save_%d.fsav` | TBD |

### Save directory layout

```
$XDG_DATA_HOME / %APPDATA%
└── Firestaff/
    ├── dm1/
    │   └── saves/           ← DM1 saves
    ├── csb/
    │   └── saves/           ← CSB saves (NEW — this phase)
    ├── dm2/
    │   └── saves/           ← DM2 saves
    └── nexus/
        └── saves/           ← Nexus saves
```

### Save header format (512 bytes, CHANGE7_29)

```
Offset  0- 31:  Game identification (plain)
  Magic           [0]   0x43534201 (CSB_MAGIC)
  HeaderVersion   [4]
  GameID          [6]   from DUNGEON.DAT header
  DungeonSeed     [8]
  PartyMapX/Y/Z  [12-16]
  PartyDirection [18]
  ChampionCount  [20]
  GameTime       [22-27]
  PlayTimeMs     [28-31]

Offset 32-127: Champion summaries (4 × 28 bytes = 112 bytes)

Offset 128-255: Reserved padding

Offset 256-511: Obfuscated block (256 bytes)
  XOR-obfuscated with DecryptionKey per F0417
  Deobfuscation key index: C29 (CSB) vs C10 (DM1)
  Last uint16 = checksum XOR key
```

---

## Variant Diagnostics Table

| `CSB_V1_VariantId` | Name | `media_ref` | Notes |
|-------------------|------|-------------|-------|
| 0 | Unknown | — | Not yet identified |
| 1 | PC DOS 3.4 EN | `MEDIA278:P20JA_P20JB` | |
| 2 | PC DOS 3.4 Multi | `MEDIA278:I34E_I34M` | |
| 3 | Atari ST 2.0 EN | `MEDIA332:S20E` | Same as S21E |
| 4 | Atari ST 2.1 EN | `MEDIA332:S21E` | Same F21E key index |
| 5 | Amiga 3.5 EN | `MEDIA529:A35E` | NONBOOT media tag |
| 6 | Amiga 3.5 Multi | `MEDIA529:A35M` | |
| 7 | Atari ST TT | `MEDIA529:F20J` | 68060 accelerator |
| 8 | Atari ST (F20E) | `MEDIA529:F20E` | Fallback ST save path |

---

## Difficulty Scale

| Champions | `difficulty_x100` | Label |
|-----------|-----------------|-------|
| 1 | 100 | Easy |
| 2 | 125 | Normal |
| 3 | 150 | Hard (default) |
| 4 | 200 | Extreme |

Formula: `100 + (champ_count - 1) × 25`

Reference: PROJEXPL.C (CHANGE7_20_IMPROVEMENT; CSBWin Character.cpp difficulty scale)

---

## Chaos Magic State at Boot

`CSB_V1_ChaosMagicState` initialized when `csb_v1_runtime_boot()` succeeds:

| Field | Initial value | Source |
|-------|-------------|--------|
| `magic_initialized` | `1` | F0211 fires at world load |
| `spell_grid_version` | `0` | Increments each V1 tick |
| `chaos_level` | `0` | Ambient oscillation tied to tick cycle |

Spell grid version increments each V1 tick, providing a monotonic counter for probe state verification.

---

## CMake Integration

`CSB_V1_SOURCES` in `CMakeLists.txt`:

```cmake
set(CSB_V1_SOURCES
    src/csb/csb_v1_runtime_pc34_compat.c   # NEW
    src/csb/csb_v1_game.c
    src/csb/csb_v1_game_state_pc34_compat.c
    src/csb/csb_v1_save_load_pc34_compat.c
    src/csb/csb_v1_character_pc34_compat.c
    src/csb/csb_v1_monster_pc34_compat.c
    src/csb/csb_v1_dungeon_loader_pc34_compat.c
    src/csb/csb_v1_dungeon_world_pc34_compat.c
    src/csb/csb_v1_viewport_pc34_compat.c
    src/csb/csb_v1_chaos_magic_pc34_compat.c
)
```

The new `csb_v1_runtime_pc34_compat.c` is added to the existing `CSB_V1_SOURCES` list in `CMakeLists.txt` (line ~76).

---

## Out-of-Scope Reminders

- **CSB V2 profile** is a separate architecture concern.  V2 presentation scaffold is Phase 1 of the CSB V2 TODO section.
- **DM2 / Nexus runtime profiles** are their own separate TODO items.
- **Custom dungeon** support for CSB is deferred (requires Phase 2 dungeon model work).
- **Champion import from DM1 save** at boot is Phase 6 (`csb_v1_character_import_dm1_save()`).

---

*End of Phase 1 source-lock doc.  Phase 2: Dungeon data model.*
