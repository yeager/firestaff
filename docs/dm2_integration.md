# DM2 V1 — Integration Testing: How DM2 Modules Connect

## Overview

DM2 V1 is a stub-only codebase — 11 `.c` files in `src/dm2/` are skeleton
implementations with no functional code and no integration points exercised.
This document describes how the modules are *supposed* to connect (per the
source references and architecture docs), so that when implementation begins,
the integration points are clear and testable.

---

## 1. Module Dependency Graph

```
firestaff_m10 (shared M10 engine)
       |
       +-- firestaff_dm2 (V1 dungeon/combat/party logic)
       |      |
       |      +-- dm2_v1_game.c          (top-level state + entry)
       |      +-- dm2_v1_dungeon_loader.c (dungeon format, outdoor levels)
       |      +-- dm2_v1_outdoor_renderer.c (viewport rendering)
       |      +-- dm2_v1_save_load.c     (SKSAVE* format)
       |      +-- dm2_v1_combat.c        (combat resolver)
       |      +-- dm2_v1_tech_magic.c    (tech/magic items)
       |      +-- dm2_v1_companion.c     (companion AI)
       |
       +-- firestaff_dm2_v2 (V2 enhanced modes)
              +-- dm2_v2_viewport_renderer.c
              +-- dm2_v2_outdoor_enhanced.c
              +-- dm2_v2_companion_ui.c
              +-- dm2_v2_tech_crafting.c

firestaff_shared (asset discovery, memory, frontend)
       |
       +-- asset_find_by_hash (hash-based file discovery)
       +-- memory/ (allocator, cache)
       +-- frontend/ (menu system, input)
```

### Key Integration Points

1. `dm2_v1_game.c` -> `dm2_v1_dungeon_loader.c` (dungeon loading)
2. `dm2_v1_game.c` -> `asset_find_by_hash` (hash-based asset discovery)
3. `dm2_v1_dungeon_loader.c` -> `firestaff_shared` (memory allocator)
4. `dm2_v1_outdoor_renderer.c` -> `firestaff_m10` (rendering primitives)
5. `dm2_v1_save_load.c` -> `firestaff_shared` (file I/O)
6. `dm2_v1_combat.c` -> `firestaff_m10` (game state structures)
7. V1 combat <-> V2 viewport (V1 logic, V2 presentation split)

---

## 2. Entry Point: dm2_v1_game.c

This is the top-level DM2 state manager. It wires together:

```
dm2_v1_init()
  -> initializes DM2_V1_GameState struct
  -> sets data_dir (from caller)
  -> sets party defaults (x=15, y=15, dir=0, gold=100, time=720)

dm2_v1_load_dungeon()
  -> asset_find_by_md5_list() to locate dungeon
  -> dm2_v1_dungeon_load() to parse header + tiles
  -> stores result in state

dm2_v1_enter_shop()
  -> checks outdoor flag
  -> routes to shop UI

dm2_v1_is_outdoor()
  -> returns outdoor flag from state
```

**Integration test for entry point**:
```
test_dm2_v1_game__init__creates_valid_state
test_dm2_v1_game__load_dungeon__hash_lookup_called
test_dm2_v1_game__load_dungeon__returns_neg1_when_not_found
test_dm2_v1_game__is_outdoor__default_false
test_dm2_v1_game__enter_shop__fails_when_null_state
```

---

## 3. Dungeon Loader: dm2_v1_dungeon_loader.c

This module parses the DM2 dungeon.dat format and exposes square-type
lookups. It is the most self-contained module and the easiest to test
in isolation once a fixture exists.

**Header format** (from `SKULL.ASM`):
```
Offset 0-1:  level_count (uint16 LE)
Offset 2:    level_type[0] (0=interior, 1=outdoor)  <- DM2 extension
Offset 3:    level_width[0]
Offset 4:    level_height[0]
Offset 5-8:  level_offset[0] (uint32 LE)
... (8 bytes per level)
```

**Tile data** (column-major, 16-bit LE square types, 5 significant bits):
```
square_type = rd16(data + offset) & 0x1F
```

**Integration with game state**:
```
dm2_v1_game.c: dm2_v1_load_dungeon()
  -> dm2_v1_dungeon_loader.c: dm2_v1_dungeon_load()
      -> allocates DM2_V1_DungeonData
      -> reads level header
      -> stores raw_data

dm2_v1_game.c: movement/click events
  -> dm2_v1_dungeon_loader.c: dm2_v1_dungeon_get_square_type(level,x,y)
      -> used for collision detection, interaction routing
      -> returns square_type (wall/floor/door/pit/etc.)
```

**Key integration tests**:
```
test_dm2_v1_dungeon__load__level_count_matches_fixture
test_dm2_v1_dungeon__get_square_type__indoor_level_returns_type
test_dm2_v1_dungeon__is_outdoor__level_0_returns_true_or_false
test_dm2_v1_dungeon__load_free__no_leak (valgrind)
test_dm2_v1_dungeon__loader_to_game__square_type_used_for_collision
```

---

## 4. Outdoor Renderer: dm2_v1_outdoor_renderer.c

DM2 introduces outdoor areas — open sky, terrain, buildings — that do not
exist in DM1. This module renders the outdoor viewport.

**Integration with dungeon loader**:
```
dm2_v1_dungeon_loader.c: dm2_v1_dungeon_is_outdoor(level)
  -> called by outdoor renderer to determine which viewport to draw
  -> if level is outdoor -> draw sky gradient + terrain
  -> if level is interior -> draw dungeon walls (via M10 viewport)
```

**Integration with game state**:
```
dm2_v1_game.c: render loop
  -> checks dm2_v1_is_outdoor(state)
  -> if true: calls outdoor_renderer
  -> if false: delegates to M10 viewport system
```

**Key integration tests**:
```
test_dm2_v1_outdoor__render__sky_gradient_drawn
test_dm2_v1_outdoor__render__outdoor_level_renders_differently_than_interior
test_dm2_v1_outdoor__game_state__is_outdoor_drives_which_renderer
```

---

## 5. Save/Load: dm2_v1_save_load.c

DM2 save format (SKSAVE*) is different from DM1. This module handles
serialization of the DM2 game state.

**Integration with game state**:
```
dm2_v1_game.c: on save event
  -> dm2_v1_save_load.c: dm2_v1_save_game(state, path)
      -> serializes DM2_V1_GameState
      -> writes SKSAVE format

dm2_v1_game.c: on load event  
  -> dm2_v1_save_load.c: dm2_v1_load_game(state, path)
      -> reads SKSAVE format
      -> restores DM2_V1_GameState
```

**Key integration tests**:
```
test_dm2_v1_save__roundtrip__gold_position_preserved
test_dm2_v1_save__roundtrip__party_champions_preserved
test_dm2_v1_save__load__outdoor_state_restored
test_dm2_v1_save__corrupted__returns_neg1
```

---

## 6. Combat: dm2_v1_combat.c

DM2 combat extends DM1 with new weapon types (axe, polearm, bow),
different damage formulas, and magic weapon bonuses. This module
resolves combat interactions.

**Integration with game state**:
```
dm2_v1_game.c: on combat event
  -> dm2_v1_combat.c: dm2_v1_resolve_combat(party, creature, weapon_type)
      -> uses party position, creature stats, weapon modifiers
      -> returns damage dealt

dm2_v1_dungeon_loader.c: creature placement
  -> dm2_v1_combat.c: uses creature group data from dungeon
```

**Key integration tests**:
```
test_dm2_v1_combat__resolve__axe_vs_champion_correct_damage
test_dm2_v1_combat__resolve__polearm_reach_bonus
test_dm2_v1_combat__resolve__magic_weapon_applies_bonus
test_dm2_v1_combat__resolve__armor_reduces_damage
test_dm2_v1_combat__game_state__damage_updates_champion_hp
```

---

## 7. Tech/Magic: dm2_v1_tech_magic.c

DM2's item system is a hybrid of "tech" (scientific) and "magic" items.
This module handles item interactions for both categories.

**Integration with combat**:
```
dm2_v1_combat.c: weapon type resolution
  -> dm2_v1_tech_magic.c: dm2_v1_get_item_bonus(item)
      -> returns tech bonus or magic bonus depending on item type
```

---

## 8. Companion: dm2_v1_companion.c

DM2 companions (non-champion party members) have different leveling
and stat progression from DM1 champions. This module manages companion
state and AI.

**Integration with game state**:
```
dm2_v1_game.c: party management
  -> dm2_v1_companion.c: companion tick/update
      -> companion AI decision
      -> stat gain on level up
```

---

## 9. V1 <-> V2 Interface

The V2 modules (`dm2_v2_*.c`) render the V1 game state with enhanced
graphics. The key interface is:

```
dm2_v2_viewport_renderer.c
  -> reads DM2_V1_GameState (read-only, V1 owns the state)
  -> applies V2 presentation (CRT filter, upscale, modern UI)
  -> does NOT modify game logic — purely presentational

dm2_v1_game.c: tick()
  -> updates DM2_V1_GameState (V1 logic)
  -> V2 renderer sees the updated state on next frame
```

**Integration test for V1/V2 split**:
```
test_dm2_v2__viewport__same_state_as_v1_renders_correctly
test_dm2_v2__viewport__v1_state_changes_reflect_in_v2_renderer_next_frame
```

---

## 10. Shared Infrastructure Integration

### Asset Discovery (asset_find_by_hash)

`dm2_v1_game.c` uses `asset_find_by_md5_list()` to locate dungeon and
graphics files. This is the same mechanism used by DM1/CSB.

**Interface contract**:
```c
// src/dm2/dm2_v1_game.c calls:
asset_find_by_md5_list(data_dir, dm2_dungeon_hashes, path, path_size, NULL, 4);
// Returns 1 on found, 0 on not found
// Fills path with full path to matched file
```

**Integration test**:
```
test_dm2_v1_game__load_dungeon__asset_find_by_hash_returns_path
test_dm2_v1_game__load_dungeon__wrong_hash_not_found
```

### Memory Allocator

`dm2_v1_dungeon_loader.c` allocates `raw_data` via standard `malloc`
(matching the pattern in `src/memory/`). Future work may wire this to
the `memory/` cache system for better memory management.

**Integration test**:
```
test_dm2_v1_dungeon__load__no_memory_leak (valgrind)
test_dm2_v1_dungeon__free__clean_unref (valgrind)
```

---

## 11. Module Connectivity Matrix

| Source | Depends On | Direction | Interface |
|--------|-----------|-----------|-----------|
| dm2_v1_game.c | dm2_v1_dungeon_loader.c | -> | DM2_V1_DungeonData* |
| dm2_v1_game.c | asset_find_by_hash | -> | asset_find_by_md5_list() |
| dm2_v1_game.c | dm2_v1_outdoor_renderer.c | -> | dm2_v1_is_outdoor() state flag |
| dm2_v1_game.c | firestaff_m10 | <- | M10_GameState (shared) |
| dm2_v1_dungeon_loader.c | firestaff_shared | -> | memory allocator (malloc) |
| dm2_v1_outdoor_renderer.c | firestaff_m10 | -> | M10 rendering primitives |
| dm2_v1_outdoor_renderer.c | dm2_v1_dungeon_loader.c | -> | dm2_v1_dungeon_is_outdoor() |
| dm2_v1_save_load.c | firestaff_shared | -> | file I/O |
| dm2_v1_combat.c | firestaff_m10 | -> | M10 game state structs |
| dm2_v1_tech_magic.c | dm2_v1_combat.c | -> | item bonus lookup |
| dm2_v1_companion.c | dm2_v1_game.c | <- | party state read |
| dm2_v2_viewport_renderer.c | dm2_v1_game.c | -> | DM2_V1_GameState (read-only) |

---

## 12. Integration Test Execution Order

When writing integration tests, use this dependency order:

1. **First**: `dm2_v1_dungeon_loader` — no dependencies on other DM2 modules
2. **Second**: `dm2_v1_game` — depends on dungeon_loader + asset_find_by_hash
3. **Third**: `dm2_v1_save_load` — depends on game state being init-able
4. **Fourth**: `dm2_v1_combat` — depends on game state + dungeon loader
5. **Fifth**: `dm2_v1_tech_magic` — depends on combat
6. **Sixth**: `dm2_v1_companion` — depends on game state
7. **Seventh**: `dm2_v1_outdoor_renderer` — depends on dungeon_loader + game state
8. **Last**: `dm2_v2_viewport_renderer` — V1 game state must be complete first

---

## 13. Fixture Requirements per Integration Test

| Integration Test | Fixture Needed | Source Archive |
|------------------|----------------|----------------|
| Dungeon loader unit tests | `dungeon_pc_en.DAT` | `Dungeon-Master-II-Skullkeep_DOS_EN.zip` |
| Game state init + load | `dungeon_pc_en.DAT` | same |
| Save/load roundtrip | `save_slot_0.bin` | extracted from in-game save |
| Combat resolver | `dungeon_pc_en.DAT` (creature placement) | same |
| Outdoor renderer | `dungeon_pc_en.DAT` + outdoor level | same |
| V2 renderer | V1 game state (from `dm2_v1_game.c`) | N/A — generated |
