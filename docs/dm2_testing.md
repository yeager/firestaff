# DM2 V1 — Testing: How to Test DM2 Parity with Original

## Overview

DM2 V1 testing is a planned Phase 8 deliverable (per TODO.md). Currently no
DM2-specific test suite exists — the codebase has 11 stub .c files in
`src/dm2/` that form the skeleton of DM2 V1 but none are exercised by CTest.

Source reference: SKULL.ASM (disassembly from `gbsphenx/skproject`), SKWIN/SKULLWIN
Allegro C++ sources, and community docs in `docs/dm2_*.md`.

---

## 1. What "Parity Testing" Means for DM2

Unlike DM1 where source-lock evidence (ReDMCSB) provides a definitive reference,
DM2 has a more fragmented reference landscape:

| Reference | Reliability | Coverage |
|-----------|-------------|---------|
| SKULL.ASM (disassembly) | HIGH — disassembly of actual DOS binary | Core dungeon/combat/logic |
| SKWIN C++ (Allegro) | MEDIUM — modernized rewrite, not 1:1 | Rendering, platform I/O |
| SKULLWIN C++ | MEDIUM — older port, closer to DOS | Rendering, menus |
| Community docs (dm2_*.md) | MEDIUM-LOW — reconstructed behavior | Design intent, bug lists |
| Actual DM2 game | HIGH — ground truth | All systems |

**Testing strategy**: Use SKULL.ASM as the primary source-lock reference for
logic, SKWIN/SKULLWIN as secondary rendering evidence, and the original DOS/Windows
binary as the final arbiter for disputed behavior.

---

## 2. Required Test Fixtures (Not Yet Present)

Before any parity test can run, these fixtures must exist under `tests/fixtures/dm2/`:

```
tests/fixtures/dm2/
├── dungeon_pc_en.DAT          # canonical DM2 PC English DUNGEON.DAT
├── dungeon_pc_fr.DAT          # French variant
├── dungeon_pc_de.DAT          # German/JewelCase variant
├── minimal.DAT                # minimal valid dungeon for headless tests
└── corrupted.DAT              # truncated/malformed dungeon for error tests
```

**Why missing**: DM2 dungeon files are still inside zip archives on disk.
The `dm2_v1_load_dungeon()` function in `src/dm2/dm2_v1_game.c` explicitly
returns -1 with the message "dungeon files need to be extracted from zip
archives first". No extraction pipeline exists yet.

**Action needed**: Write `scripts/extract_dm2_archives.sh` (or Python equivalent)
that locates `Dungeon-Master-II-Skullkeep_DOS_EN.zip`, extracts
`DUNGEON.DAT`, `GRAPHICS.DAT`, `SKULL.EXE`, `SONGLIST.DAT`, validates SHA256
hashes, and places files in the canonical path.

Required hashes:
- `6caccd7875009e82fe2e28e7f6d6adc0` (DUNGEON.DAT PC English)
- `25247ede4dabb6a71e5dabdfbcd5907d` (GRAPHICS.DAT PC English)

### 2.2 Graphics Data Fixture

`tests/fixtures/dm2/graphics_pc_en.DAT` — hash `25247ede4dabb6a71e5dabdfbcd5907d`
Already listed in `dm2_v1_game.c` `dm2_graphics_hashes[]` but no fixture exists.

### 2.3 Save Game Fixture

```
tests/fixtures/dm2/save_slot_0.bin  # known-good save, mid-game
tests/fixtures/dm2/save_slot_empty.bin
```

Required for save/load roundtrip tests.

---

## 3. Testing Hierarchy

### Level 0 — Build Integrity (exists today, bare minimum)

- CMake compiles `firestaff_dm2` static library without error
- All 11 `dm2_v1_*.c` files compile with `-Wall -Wextra`
- No undefined symbols at link time

**Current status**: Likely passes (stubs are compile-safe)

### Level 1 — Dungeon Loader Tests (planned)

```
test_dm2_v1_dungeon_loader.c
├── dm2_v1_dungeon_load__valid_file__returns_0
├── dm2_v1_dungeon_load__truncated_file__returns_neg1
├── dm2_v1_dungeon_get_square_type__outdoor_level__returns_correct_type
├── dm2_v1_dungeon_get_square_type__out_of_bounds__returns_neg1
├── dm2_v1_dungeon_is_outdoor__level_0_outdoor__returns_true
├── dm2_v1_dungeon_is_outdoor__interior_level__returns_false
└── dm2_v1_dungeon_free__after_load__no_leak (valgrind)
```

**Source-lock basis**: `SKULL.ASM` dungeon loading routines.

### Level 2 — Game State Init Tests (planned)

```
test_dm2_v1_game.c
├── dm2_v1_init__null_state__no_crash
├── dm2_v1_init__sets_defaults__gold_100
├── dm2_v1_init__sets_party_position__x15_y15
├── dm2_v1_is_outdoor__initially_false__returns_0
└── dm2_v1_enter_shop__null_state__returns_neg1
```

### Level 3 — Dungeon Integration Tests (planned)

Wires dungeon_loader to game state to outdoor_renderer.
Load real `dungeon_pc_en.DAT`, verify level count, outdoor level detection,
square-type lookup against SKULL.ASM reference values.

### Level 4 — Save/Load Roundtrip Tests (planned)

```
test_dm2_v1_save_load.c
├── dm2_v1_save__game_state__writes_valid_file
├── dm2_v1_load__valid_save__restores_state
├── dm2_v1_load__truncated_save__returns_neg1
└── dm2_v1_save_load__roundtrip__state_preserved (gold, position, party)
```

### Level 5 — Combat System Tests (planned)

```
test_dm2_v1_combat.c
├── dm2_v1_combat_resolve__fighter_vs_champion__correct_damage
├── dm2_v1_combat_resolve__magic_weapon__bonus_applied
└── dm2_v1_combat_resolve__armor_absorbs__damage_reduced
```

Reference: `SKULLWIN/c_ai.cpp` and DM2 extended weapon/spell tables.

### Level 6 — End-to-End Parity Against DOSBox Reference (future)

Run the original DM2 binary under DOSBox with a deterministic input script,
capture the game state hash at tick N, compare against Firestaff DM2
state hash at the same tick.

Reference: `docs/dm2_source_lock.md` — "No CSB/DM2 DOSBox/original capture
or rendering parity probe is enabled."

---

## 4. Testing Tools and Infrastructure

### 4.1 Headless Probe

A `firestaff_dm2_phase_a_probe` (analogous to `firestaff_m11_phase_a_probe`) would:
- Accept `--game=dm2` flag
- Load DM2 dungeon from `tests/fixtures/dm2/minimal.DAT`
- Execute a deterministic input script
- Emit a state hash at each tick
- Print `# summary: N/M passed`

**Not yet written** — Phase 8 deliverable.

### 4.2 Asset Extraction Script

`scripts/extract_dm2_assets.sh`: locates zip, extracts, SHA256-validates,
places in canonical path.

### 4.3 DOSBox Reference Runner

`scripts/run_dm2_reference_dosbox.sh`: Launch `SKULL.EXE` under DOSBox,
inject deterministic keystrokes, capture state dump at key ticks.

---

## 5. Parity Testing Checklist

| Feature | Test Status | Source Reference | Notes |
|---------|-------------|-----------------|-------|
| Dungeon header parsing | NOT TESTED | SKULL.ASM | 11 stubs, no tests yet |
| Level type detection | NOT TESTED | SKULL.ASM | dm2_v1_dungeon_is_outdoor() |
| Square type lookup | NOT TESTED | SKULL.ASM | rd16 LE format, 5-bit type |
| Party position init | NOT TESTED | SKULL.ASM | defaults to (15,15) |
| Outdoor renderer (sky) | NOT TESTED | SKULL.ASM | dm2_v1_outdoor_renderer.c |
| Save format (SKSAVE*) | NOT TESTED | SKULL.ASM | dm2_v1_save_load.c |
| Combat resolver | NOT TESTED | SKWIN c_ai.cpp | dm2_v1_combat.c |
| Tech/magic items | NOT TESTED | SKWIN c_ai.cpp | dm2_v1_tech_magic.c |
| Companion/party system | NOT TESTED | SKWIN c_ai.cpp | dm2_v1_companion.c |
| Asset loading (zip) | NOT TESTED | dm2_v1_game.c | extraction missing |

---

## 6. Known Testing Blockers

1. **No extracted dungeon fixtures** — dm2_v1_load_dungeon() fails without them
2. **No extraction pipeline** — zip to canonical fixture path not automated
3. **No DOSBox reference runner** — cannot validate parity against original
4. **No probe binary** — no headless test executable for CTest to run
5. **Phase 8 not started** — TODO.md shows all Phase 1-7 as for DM2 V1

---

## 7. Recommended Testing Order

1. **First**: Write extraction script, get `tests/fixtures/dm2/dungeon_pc_en.DAT`
2. **Then**: Write dungeon loader unit tests (Level 1)
3. **Then**: Write game state init + integration tests (Levels 2-3)
4. **Then**: Write save/load roundtrip tests (Level 4)
5. **Then**: Combat + companion system tests (Level 5)
6. **Finally**: DOSBox parity probe (Level 6)
