# DM2 V1 — Test Coverage: Well-Tested Areas vs Gaps

## Overview

DM2 V1 is a stub-only codebase. All 11 source files in `src/dm2/` are
compile-safe skeleton implementations — no functional implementation, no tests.
The test coverage picture is almost entirely gaps.

**Current state**: 0 DM2-specific tests exist. 11 stub `.c` files compile into
`firestaff_dm2` static library but are never exercised by CTest.

---

## 1. What IS Covered (DM1/CSB pattern, reference for DM2)

Firestaff's DM1 V1 test suite (`test_dm1_v1_*.c`, 39 integration tests) provides
a structural template for what DM2 V1 coverage should eventually look like.
These DM1 areas map to equivalent DM2 systems:

| DM1 V1 Tested Area | DM2 V1 Equivalent (not yet tested) | Gap Severity |
|-------------------|-------------------------------------|-------------|
| Dungeon loader bounds | DM2 dungeon loader (SKULL.ASM) | CRITICAL — no fixture |
| GRAPHICS.DAT loading | DM2 GRAPHICS.DAT (hash known) | CRITICAL — no fixture |
| Viewport 3D render | DM2 outdoor renderer | HIGH |
| Save/load roundtrip | DM2 SKSAVE* format | HIGH |
| Combat resolver | DM2 combat (extended weapons) | HIGH |
| Movement pipeline | DM2 movement/click routing | MEDIUM |
| Creature AI behavior | DM2 creature AI | MEDIUM |
| Spell casting | DM2 tech/magic hybrid | MEDIUM |
| Sensor/trigger | DM2 actuators, doors, pits | MEDIUM |
| Champion stats/panel | DM2 champion classes (4 new types) | MEDIUM |

---

## 2. Source File Inventory and Coverage Status

### 2.1 dm2_v1_game.c (INIT/SHIM ONLY)

**Coverage**: NONE

```
Lines: ~60 (stub)
Function: dm2_v1_init(), dm2_v1_load_dungeon(), dm2_v1_enter_shop(), dm2_v1_is_outdoor()
Status: Returns -1 for dungeon load — extraction pipeline missing
Test: NONE
```

This is the top-level entry point. Nothing is tested.

### 2.2 dm2_v1_dungeon_loader.c

**Coverage**: NONE (but closest to testable)

```
Lines: ~50
Function: dm2_v1_dungeon_load(), dm2_v1_dungeon_get_square_type(),
         dm2_v1_dungeon_is_outdoor(), dm2_v1_dungeon_free()
Format: Level header (16-bit LE count + type/width/height/offset per level),
        tile data (16-bit LE square types)
Source: SKULL.ASM
Status: Parser is written; no fixture to test against
Test: NONE
```

This is the most testable file — purely data transformation, no I/O dependencies
except the raw bytes. Once a fixture exists, unit tests are straightforward.

### 2.3 dm2_v1_outdoor_renderer.c

**Coverage**: NONE

```
Lines: ~35 (stub)
Function: outdoor viewport rendering, sky gradient
Source: SKULL.ASM (outdoor rendering routines)
Status: Stub — no actual rendering code
Test: NONE
```

Requires a real dungeon + outdoor level to test visually.

### 2.4 dm2_v1_save_load.c

**Coverage**: NONE

```
Lines: ~20 (stub)
Function: SKSAVE* format load/save
Source: SKULL.ASM (savegame format)
Status: Stub — save/load not implemented
Test: NONE
```

### 2.5 dm2_v1_combat.c

**Coverage**: NONE

```
Lines: ~35 (stub)
Function: combat resolver with DM2 weapon types
Source: SKWIN c_ai.cpp (reference), SKULL.ASM (combat logic)
Status: Stub
Test: NONE
```

### 2.6 dm2_v1_tech_magic.c

**Coverage**: NONE

```
Lines: ~25 (stub)
Function: tech/magic hybrid item system (DM2's "magic items" vs "items")
Source: SKWIN c_ai.cpp
Status: Stub
Test: NONE
```

### 2.7 dm2_v1_companion.c

**Coverage**: NONE

```
Lines: ~30 (stub)
Function: companion/party AI
Source: SKWIN c_ai.cpp
Status: Stub
Test: NONE
```

DM2 companions are different from DM1 champions — they level up differently,
have different class progression. No test fixtures.

### 2.8 V2 files (dm2_v2_*.c) — not in scope for V1 testing

---

## 3. Test Coverage Gaps by Category

### Category: Dungeon Data Loading

| File | What needs testing | Status |
|------|-------------------|--------|
| dm2_v1_dungeon_loader.c | Valid dungeon file load | MISSING FIXTURE |
| dm2_v1_dungeon_loader.c | Truncated dungeon handling | MISSING FIXTURE |
| dm2_v1_dungeon_loader.c | Outdoor level detection | MISSING FIXTURE |
| dm2_v1_dungeon_loader.c | Square type lookup (rd16 LE) | MISSING FIXTURE |
| dm2_v1_dungeon_loader.c | Memory cleanup (valgrind) | MISSING FIXTURE |

**FIXTURE NEEDED**: `tests/fixtures/dm2/dungeon_pc_en.DAT`
**Source**: `Dungeon-Master-II-Skullkeep_DOS_EN.zip` → extract → SHA256 verify
**Hash**: `6caccd7875009e82fe2e28e7f6d6adc0` (known, from dm2_v1_game.c)

### Category: Graphics Data

| File | What needs testing | Status |
|------|-------------------|--------|
| dm2_v1_game.c | GRAPHICS.DAT hash lookup | MISSING FIXTURE |
| dm2_v1_outdoor_renderer.c | Outdoor bitmap rendering | MISSING FIXTURE + STUB |

**FIXTURE NEEDED**: `tests/fixtures/dm2/graphics_pc_en.DAT`
**Hash**: `25247ede4dabb6a71e5dabdfbcd5907d` (known)

### Category: Save/Load

| File | What needs testing | Status |
|------|-------------------|--------|
| dm2_v1_save_load.c | Save format write | MISSING IMPLEMENTATION |
| dm2_v1_save_load.c | Save format read | MISSING IMPLEMENTATION |
| dm2_v1_save_load.c | Roundtrip integrity | MISSING FIXTURE |

**FIXTURE NEEDED**: `tests/fixtures/dm2/save_slot_0.bin`
**Source**: SKULL.ASM savegame format section (not yet analyzed in docs)

### Category: Combat

| File | What needs testing | Status |
|------|-------------------|--------|
| dm2_v1_combat.c | Damage formula (DM2 vs DM1) | MISSING IMPLEMENTATION |
| dm2_v1_combat.c | Weapon type modifiers | MISSING IMPLEMENTATION |
| dm2_v1_combat.c | Magic weapon bonus | MISSING IMPLEMENTATION |
| dm2_v1_combat.c | Armor absorption | MISSING IMPLEMENTATION |

**NOTE**: `SKWIN/c_ai.cpp` has combat resolver — needs disassembly cross-reference
with SKULL.ASM before test values can be determined.

### Category: Integration (Dungeons + Rendering + Input)

| What needs testing | Status |
|--------------------|--------|
| Outdoor level rendering | MISSING FIXTURE + STUB |
| Indoor-to-outdoor transition | MISSING IMPLEMENTATION |
| Party movement in outdoor | MISSING IMPLEMENTATION |
| Companion AI in outdoor | MISSING IMPLEMENTATION |
| Click routing for outdoor | MISSING IMPLEMENTATION |
| Ladder/pit/teleport transitions | MISSING IMPLEMENTATION |
| Shop/trader interaction | MISSING IMPLEMENTATION |

---

## 4. What DOESN'T Need Testing (Not Yet in Scope)

- V2 rendering modes (dm2_v2_*.c) — separate from V1 parity
- WebAssembly/Emscripten target — platform not prioritized for DM2 yet
- Nexus support — separate game, Phase 0 not started
- Theron's Quest — separate game, Phase 1 not started

---

## 5. Coverage Summary

| Category | Files | Tests | Coverage |
|----------|-------|-------|---------|
| Dungeon loader | 1 | 0 | 0% |
| Game entry point | 1 | 0 | 0% |
| Outdoor renderer | 1 | 0 | 0% |
| Save/load | 1 | 0 | 0% |
| Combat | 1 | 0 | 0% |
| Tech/magic | 1 | 0 | 0% |
| Companion | 1 | 0 | 0% |
| **TOTAL (V1)** | **7** | **0** | **0%** |

The DM2 V1 codebase is a blank slate for testing. The first meaningful
coverage milestone is fixture creation (dungeon + graphics extraction from zip).
