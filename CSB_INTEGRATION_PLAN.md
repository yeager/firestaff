# CSB Integration Plan — Firestaff
## Chaos Strikes Back Technical Integration Guide

**Document:** CSB_INTEGRATION_PLAN.md  
**Repo:** `/home/trv2/work/firestaff`  
**Status:** Draft — Needs Validation  
**Last Updated:** 2026-05-26

---

## Changelog

| Date | Change |
|------|--------|
| 2026-05-26 | Initial draft from CSB source analysis + Firestaff existing code |

---

## Section 1: CSB vs DM1 Technical Comparison

### 1.1 Champions System

**Finding:** CSB uses **4 champions**, not 6.

Source evidence:
- `include/csb_v1_character_pc34_compat.h` line 20: `CSB_V1_MAX_CHAMPIONS = 4`
- `include/csb_v1_character_pc34_compat.h`: "CSB uses imported DM1 champions. No champion selection hall."
- CSBCode.cpp lines 369-372: Fighter, Ninja, Priest, Wizard classes

CSB does **not** have a Hall of Champions. Champions are imported from an existing DM1 save game via `csb_v1_import_dm1_save()`. The startup flow:

```
CSB Title Screen
  -> Import DM1 Champions (select existing DM1 save file)
  -> Champions loaded into party
  -> Dungeon entrance (no HoC sequence)
```

**Consequence:** No champion creation/selection screen needed. The existing DM1 champion system handles this.

### 1.2 Dungeon Structure

**Finding:** CSB uses 12 levels (vs DM1 14) plus The Void endgame area.

Source evidence:
- `include/csb_v1_dungeon_loader_pc34_compat.h` line 19: `CSB_V1_MAX_LEVELS = 12`
- ReDMCSB DUNGEON.C F0148-F0170: shared format between DM1 and CSB

CSB dungeon format is **column-major** (same as DM1):
- `csb_v1_dungeon_get_square_type()`: index = x * height + y
- Per-square: low 5 bits = square type, bits 5-14 = first thing index

**DSA (Dungeon Scripting Architecture):** CSB introduces thing type 15 (`CSB_V1_THING_TYPE_DSA = 15`) — programmable dungeon events attached to rooms.

Dungeon header format:
```
bytes 0-1:  level_count (LE uint16)
bytes 2-3:  thing_type_count (LE uint16)
per level:  width (uint8), height (uint8), offset (uint32 LE)
```

### 1.3 Graphics Format

**Finding:** CSB uses `GRAPHICS.DAT` (not `.BDY`). DM1-format file.

Source evidence:
- CSBCode.cpp line 109: "Cannot open GRAPHICS.DAT" — CSB loads GRAPHICS.DAT
- Bitmaps.cpp line 186: 4-bit packed pixels (same as DM1)

The planar vs chunky pixel layout is **Atari ST platform specific**. PC version uses standard chunky 4-bit packed format. DM1 graphics loader handles this already.

CSB graphics differences from DM1:
- Custom wall sets per dungeon level (different theme tiles)
- Custom room backgrounds (per DSA script)
- Extended creature graphics
- Prison door intro sequence (pre-dungeon animation)

### 1.4 Load Sequence

DM1: `TITLE → ENTRANCE → (Hall of Champions) → Dungeon`

CSB: `STARTUP SCREEN → Import DM1 save → (no HoC) → Dungeon`
                      `→ Prison Door animation (level 0 intro)`

CSB has no Hall of Champions. Party import is mandatory.

### 1.5 Chaos Magic System (DSA)

CSB extends DM1 spell system with DSA:
- Bytecode-based scripting with 20+ opcodes
- Stack machine (64-deep stack, 256 global flags)
- Scripts attached to dungeon levels/rooms

DSA opcodes: SET, CLEAR, TOGGLE, TEST, JUMP, CALL, RETURN, DELAY, SOUND, SPAWN, MOVE, DAMAGE, TELEPORT, MESSAGE

---

## Section 2: Integration Architecture

### 2.1 Engine Sharing

**Recommendation:** Use the **same M11 engine** for both DM1 and CSB, with game-specific compatibility layers.

```
M12 Launcher
  → DM1: m11_dm1_compat_* (existing)
  → CSB: csb_v1_*_pc34_compat_* (new)
```

The existing `src/csb/` directory follows this pattern. The V1/V2 split mirrors DM1:
- **V1:** Low-level game logic (PC 3.4 compatibility layer)
- **V2:** Enhanced rendering features (smooth movement, minimap, etc.)

Key shared components:
- **Dungeon format:** ReDMCSB DUNGEON.C — already shared
- **Command queue:** ReDMCSB COMMAND.C — shared queue structure
- **Viewport rendering:** ReDMCSB DUNVIEW.C — shared core

CSB-specific extensions:
- **DSA scripting:** `csb_v1_chaos_magic_pc34_compat.c`
- **Custom backgrounds:** `csb_v1_viewport_pc34_compat.c`
- **Difficulty scaling:** `csb_v1_game_state_pc34_compat.c` (1.5x creature stats)

### 2.2 Game Type

The launcher already uses string-based `gameId`: `"dm1"`, `"csb"`, `"dm2"`, `"nexus"`.

**No new enum needed.** The `gameId` string drives compatibility layer selection.

Existing `asset_find_by_md5_list()` handles CSB dungeon detection:
```c
static const char *const csb_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",  /* CSB Atari ST + Amiga (EN) */
    NULL
};
```

### 2.3 Asset Pipeline

```
~/.firestaff/data/csb/
  GRAPHICS.DAT        (hash-verified)
  DUNGEON.DAT         (hash-verified)
  saves/
    csbgame.dat       (CSB save format)
```

**GRAPHICS.DAT** loading: Extend `dm1_v1_graphics_pc34_compat` to handle CSB wall set indices. 4-bit packed pixels, 16-color palette.

**DUNGEON.DAT** loading: Use `csb_v1_dungeon_load()` from `csb_v1_dungeon_loader_pc34_compat.c`. Column-major squares, FTL compression, DSA thing type 15.

### 2.4 Memory / State

CSB state struct (from `include/csb_v1_game.h`):
- `party_x`, `party_y`, `party_dir`
- `current_level`
- `dm1_import_done`
- `difficulty` (1.5x creature stats)

**Party import flow:**
1. User selects "Import DM1 Save" on CSB title
2. `csb_v1_import_dm1_save()` reads DM1 save file
3. Champions extracted and placed in CSB party struct
4. `dm1_import_done = 1`

**Champion compatibility:** `CSB_V1_MAX_CHAMPIONS = 4` matches DM1. No mapping needed.

### 2.5 Input / Command System

CSB uses same command queue as DM1 (ReDMCSB COMMAND.C). Queue structure:
- `G0432_as_CommandQueue[M529_COMMAND_QUEUE_SIZE + 1]` — circular queue
- Max 4 actual commands in queue at once

Viewport click zones: Same as DM1 (ReDMCSB DUNVIEW.C). CSB adds custom background rendering.

---

## Section 3: Research Tasks (Open Items)

### Task 1: Complete CSB .C File Listing

All CSB source files identified:

| File | Lines | Purpose |
|------|-------|---------|
| CSBCode.cpp | 11,686 | Core: DBank init, LoadDungeon, command dispatch |
| Viewport.cpp | 7,297 | Viewport rendering, click zones |
| Graphics.cpp | 3,186 | Graphics loading, palette, drawing |
| SaveGame.cpp | ~2,953 est. | Save/load logic |
| Chaos.cpp | ~5,336 est. | Chaos magic effects (DSA wrapper) |
| Character.cpp | ~5,500 est. | Champion management |

**Remaining:** Need exact line counts for Character.cpp, Chaos.cpp, SaveGame.cpp, Attack.cpp, Monster.cpp, Magic.cpp, Menu.cpp, Sound.cpp.

### Task 2: Extract GRAPHICS.BDY Magic Bytes

**Status:** Not yet done.

CSB PC uses `GRAPHICS.DAT` (not `.BDY`). The `.BDY` format is likely the **Amiga** platform version. Need to:

1. Check `csb-amiga` extracted data for GRAPHICS.BDY format
2. Extract header bytes: magic signature, version, block count
3. Compare with DM1 GRAPHICS.DAT header

Commands to run on N2:
```bash
ls -la ~/.openclaw/data/firestaff-original-games/DM/_extracted/csb-amiga/HardDisk/
xxd ~/.openclaw/data/firestaff-original-games/DM/_extracted/csb-amiga/HardDisk/ | head -40
```

### Task 3: Compare Save/Load Format DM1 vs CSB

**Status:** Not yet done.

From `csb_v1_save_load_pc34_compat.c`, CSB save format is simple binary dump of game state struct. Need to:

1. Find DM1 save format struct in ReDMCSB LOADSAVE.C
2. Compare with `CSB_SaveData` in `csb_v1_game_state_pc34_compat.h`
3. Identify champion export fields, dungeon state, DSA flags

DM1 save source: ReDMCSB LOADSAVE.C F0419 (save) and F0420 (load).

### Task 4: Identify Unique CSB Content

**Status:** Partially documented.

**Unique monsters:**
- Dark Knight (stronger than DM1 Knight)
- Wraith (phase-through walls)
- Master Lich (boss-tier)
- Stone Golem (high resistance)

**Unique items:**
- Axiom (CSB-exclusive weapon)
- Armor of Darkness

**Unique events:**
- DSA room triggers (switches, spawns, teleports)
- Prison door sequence (level 0 intro)
- The Void (endgame level, no return)

### Task 5: Map CSB Commands → DM1 Commands

**Status:** Not yet done.

Need to compare ReDMCSB COMMAND.C command IDs vs CSBWin CSBCode.cpp command handling. Source references:
- ReDMCSB COMMAND.C lines ~1-200 (command queue globals)
- CSBWin CSBCode.cpp command dispatch section

---

## Section 4: Known Blockers

### Blocker 1: DSA Bytecode Format Unknown

DSA bytecode reader is stubbed in `csb_v1_chaos_magic_pc34_compat.h`. Need to reverse from:
- CSBWin Chaos.cpp — DSA execution loop
- CSBWin DSA.cpp — script loading and parsing
- CSB original game data — DSA script blocks in DUNGEON.DAT

**Priority:** Medium. DSA can be stubbed for initial CSB gameplay.

### Blocker 2: CSB Graphics Wall Set Indices

The exact number of CSB wall sets and their indices in GRAPHICS.DAT are not documented. Need to:
1. Examine Graphics.cpp LoadGraphics function
2. Extract GRAPHICS.DAT from CSB game data
3. Count wall set blocks and compare with DM1

**Priority:** High. Without correct wall sets, CSB dungeon rooms render incorrectly.

### Blocker 3: CSB Champion Import Validation

The `csb_v1_import_dm1_save()` function is a stub. Need to:
1. Define exact DM1 save file format
2. Implement extraction of champions from DM1 save
3. Map DM1 champion fields to `CSB_V1_Champion` struct

**Priority:** High. Without import, CSB cannot start.

### Blocker 4: ReDMCSB Command Dispatcher

CSB command dispatch system needs source-lock validation against ReDMCSB COMMAND.C. Currently no Firestaff command dispatcher implementation exists for CSB.

**Priority:** High. Command dispatch is core game loop.

---

## Section 5: Recommended Implementation Order

**Phase 1: Foundation (Stub)**
- `csb_v1_import_dm1_save()` — stub with graceful failure
- `csb_v1_dungeon_load()` — already implemented
- CSB title screen with "Import DM1" option

**Phase 2: Dungeon Rendering**
- CSB wall set loader (extend DM1 graphics)
- Custom background renderer
- Basic room rendering (no DSA)

**Phase 3: Party & Combat**
- Champion import (full implementation)
- CSB combat resolver (extend DM1 combat)
- Monster stat scaling (1.5x)

**Phase 4: DSA System**
- Bytecode decoder (reverse from CSBWin/Chaos.cpp)
- Script trigger system
- DSA opcodes (SOUND, SPAWN, TELEPORT, etc.)

**Phase 5: Polish**
- Prison door intro animation
- The Void level
- V2 enhanced renderer

---

## Appendix A: Source File Map

| Firestaff Source | Reference Source | Lines | Status |
|-----------------|-----------------|-------|--------|
| `csb_v1_dungeon_loader_pc34_compat.c` | CSBCode.cpp:6800 + DUNGEON.C | ~150 | Implemented |
| `csb_v1_game_state_pc34_compat.c` | CSBCode.cpp:333 | ~150 | Implemented |
| `csb_v1_save_load_pc34_compat.c` | SaveGame.cpp | ~50 | Stub |
| `csb_v1_character_pc34_compat.c` | Character.cpp + SaveGame.cpp | ~250 | Stub |
| `csb_v1_viewport_pc34_compat.c` | Viewport.cpp + Graphics.cpp | ~100 | Stub |
| `csb_v1_chaos_magic_pc34_compat.c` | Chaos.cpp + DSA.cpp | ~400 | Stub |
| `csb_v1_monster_pc34_compat.c` | Monster.cpp + Attack.cpp | ~300 | Not started |
| `csb_v1_game.c` | CSBCode.cpp main | ~150 | Skeleton |

---

## Appendix B: Asset Hashes (Known)

| File | MD5 | Platform |
|------|-----|----------|
| `DUNGEON.DAT` | `6695d2acebce49f95db1d8f3a5c733de` | CSB Atari ST + Amiga (EN) |

GRAPHICS.DAT hash not yet recorded.

---

## Appendix C: Key Source References

**CSB Source:** `~/.openclaw/data/firestaff-csb-source/CSB/src/`
- CSBCode.cpp (11,686 lines) — DBank, LoadDungeon, command dispatch
- Viewport.cpp (7,297 lines) — viewport rendering
- Graphics.cpp (3,186 lines) — graphics loading
- SaveGame.cpp — save/load
- Chaos.cpp — DSA system
- Character.cpp — champion management

**ReDMCSB Shared:** `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- COMMAND.C (3,242 lines) — command queue, input dispatch
- DUNGEON.C — dungeon data format
- CHAMPION.C — champion core
- LOADSAVE.C — save/load format

**Firestaff Existing:**
- `src/csb/` — CSB V1/V2 implementation
- `src/dm1/` — DM1 implementation (reference for shared patterns)
- `include/csb_v1_*.h` — CSB compatibility layer headers