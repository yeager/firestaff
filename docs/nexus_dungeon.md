# Dungeon Master Nexus V1 — Dungeon Design: Levels and Mechanics

## Sources
- `src/nexus/nexus_v1_dungeon.c`
- `src/nexus/nexus_v1_game.c`
- `docs/NEXUS_FILE_CLASSIFICATION.md`
- `docs/NEXUS_PLAN.md`
- `docs/NEXUS_FILE_CLASSIFICATION.md` (LEV*.DGN / SLEV*.BIN / SMAP*.BIN)
- `docs/NEXUS_PLAN.md` (level count confirmation)

---

## Overview

Nexus reuses the **DM1 dungeon layout** — same 16-level structure, same square
grid (32x32), same map geometry — but extends the data format to embed full 3D
polygon geometry per level. This is fundamentally different from DM1/DM2/CSB
where the dungeon file contained only square-type data and the renderer drew
walls programmatically per square.

---

## Level Count: 16 Levels (vs DM1's 10)

| Game | Levels | Notes |
|------|--------|-------|
| DM1 | 10 | Original (Levels 1-10) |
| CSB | 10 | Same as DM1 |
| DM2 | 10 indoor + outdoor zones | Skullkeep + overworld |
| **Nexus** | **16** | **Same as DM1 + DM1 prototype/beta levels** |

Nexus has **16 levels** (LEV00.DGN – LEV15.DGN). DM1 has 10. The extra levels
are likely the original DM1 design drafts or beta levels that were cut from the
final DM1 release but kept in Nexus's DM1-derived layout. This aligns with
Chaos Strikes Back's 10-level scope (same as DM1) while Nexus extends to 16.

---

## Dungeon File Format: LEV*.DGN (148-321 KB each)

Each level file is 10x larger than DM1's DUNGEON.DAT because it embeds 3D
geometry. Structure as inferred from `nexus_v1_level_load`:

```
LEV00.DGN (148-321 KB)
  [0] uint16: map width (typically 32)
  [2] uint16: map height (typically 32)
  [4] uint16[32][32]: square types (2048 bytes) — same as DM1 format
  [2052+] 3D geometry section (bulk of file, polygon vertex/index data)
            geometry_offset, geometry_size tracked in Nexus_V1_Level struct
  [end] Level script (SLEV*.BIN, 2-12 KB) — separate file
  [end] Level minimap (SMAP*.BIN, 17-30 KB) — separate file
```

The square type grid uses the **same DM1 format**: 2 bytes per square, lower
5 bits = square type (wall, open, door, pit, teleporter, etc.).

---

## Per-Level Supplementary Files

| File | Count | Size each | Purpose |
|------|-------|----------|---------|
| SNDLEV00-15.SAL | 16 | 290-460 KB | Sound effect bank for level |
| SNDLEV00-15.MAP | 16 | 66-90 bytes | Sound mapping table |
| SLEV00-15.BIN | 16 | 2-12 KB | Level script/trigger data |
| SMAP00-15.BIN | 16 | 17-30 KB | Minimap/automap binary data |

Total per-level overhead: ~350-550 KB per level (vs DM1: ~0 additional data per level).

---

## Dungeon Square Types (from DM1 inference)

Nexus inherits DM1's square type system. The lower 5 bits of each 16-bit square
entry encode:

| Value | Type | Description |
|-------|------|-------------|
| 0 | WALL | Solid wall, impassable |
| 1 | OPEN | Empty floor square |
| 2 | OPEN (alt) | Empty floor (variant) |
| 3 | DOOR | Door square (open/closed/destroyed states) |
| 4 | PIT | Open pit (creature fall damage) |
| 5 | PIT (imaginary) | Fake pit (illusory) |
| 6 | TELEPORTER | Teleport square |
| 7 | WATER | Water (no swimming) |
| 8+ | Special | Trap/trigger variants |

DM1's full square type system includes:
- Pit squares deal 50% HP damage to non-levitating creatures
- Teleporter squares require wariness >= 10 to enter voluntarily
- Door height vs creature height determines passability

Nexus inherits this system. `nexus_v1_level_get_square` returns square type
(0-31) for movement checks.

---

## Level Script System (SLEV*.BIN)

Each level has an associated script file (SLEV00.BIN – SLEV15.BIN, 2-12 KB).
These contain:

- **Trigger events** — what happens when party steps on certain squares
- **Conditional spawning** — creatures that appear based on game state
- **Puzzle state** — lever positions, switch states, door triggers
- **Inscriptions** — text displayed when squares are examined

The script format is binary and not yet reverse-engineered. Compare with
DM1 triggers (`docs/dm1_v1_triggers_*.md`) for structure inference.

---

## Minimap System (SMAP*.BIN)

Per-level binary minimap data (17-30 KB per level). This is NOT the ASCII
"MAP" command in DM1 (which generated a text map from square data). Instead,
Nexus pre-computes a binary minimap image for fast display. The 17-30 KB size
suggests an encoded raster image (not raw pixel data).

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## Per-Level Sound Design

- Each level has its own sound effect bank (SNDLEV*.SAL, 290-460 KB)
- Sound mapping (SNDLEV*.MAP) maps sound IDs to actual audio samples
- Levels 1-8 have associated Red Book Audio CD tracks (tracks 2-9)
- Audio driver: SDDRVS.TSK (26 KB)

The per-level audio approach is similar to DM2 (which also had CD audio per
zone), but Nexus implements it per dungeon level rather than per overworld zone.

---

## 3D Geometry Section (DGN Extension)

The main innovation in Nexus's dungeon format is the **embedded 3D geometry**
section. Each LEV*.DGN contains polygon data for:

- **Wall facets** — per-wall-face triangle pairs (left/right/center walls)
- **Floor polygons** — per-square floor mesh
- **Ceiling polygons** — per-square ceiling mesh
- **Door geometry** — open/closed/destroyed state vertex offsets

This data replaces DM1's programmatic wall rendering (which computed wall
sprites from square type at display time). In Nexus, the geometry is pre-
computed and stored, then transformed at render time by `nexus_v1_viewport.c`.

The geometry data is substantial: 147-321 KB per level vs DM1's ~20 KB total
for all 10 levels (the 3D geometry is ~95% of each DGN file by size).

---

## Level Transition

- `nexus_v1_load_level` (engine) loads LEV*.DGN by index (0-15)
- Level script (SLEV*.BIN) determines staircase behavior
- CD audio track switches on level transition
- Sound effects (SNDLEV*.SAL) change per level

---

## Comparison: DM1 vs Nexus Dungeon

| Aspect | DM1 | Nexus |
|--------|-----|-------|
| Levels | 10 | 16 |
| Dungeon format | DUNGEON.DAT (33 KB total) | LEV00-15.DGN (148-321 KB each) |
| Square grid | 32x32 x 2 bytes | 32x32 x 2 bytes (same) |
| 3D geometry | None (programmatic wall draw) | Embedded polygon mesh per level |
| Per-level scripts | None | SLEV*.BIN (2-12 KB each) |
| Minimap | ASCII MAP command | SMAP*.BIN binary (17-30 KB) |
| Per-level CD audio | None | Red Book Audio tracks 2-9 |
| Per-level SFX | Global | SNDLEV*.SAL per level (290-460 KB each) |
| Sound driver | None | SDDRVS.TSK (26 KB) |

---

## What's NEW vs DM1/DM2/CSB

1. **3D geometry pre-computation** — walls are stored as polygon meshes,
   not computed at render time from square types
2. **16 levels** — more than DM1's 10; includes prototype/beta levels
3. **Per-level CD audio tracks** — first DM game with substantial music
4. **Per-level sound banks** — each level has dedicated SFX
5. **Binary minimap** — pre-rendered minimap data vs ASCII MAP command
6. **Level scripts** — trigger/event data per level (not present in DM1)

---

## What's the Same as DM1

- Square type format (32x32 grid, 2 bytes/square, lower 5 bits = type)
- Map layout (same geometry as DM1)
- Square types: wall, open, door, pit, teleporter, etc.
- Champion advancement, combat, spell system (DM1-based logic)

---

## Status: PARTIALLY SOURCE-LOCKED

Level count and file structure are confirmed (ISO classification + engine code).
Square grid format matches DM1 (2 bytes/square). 3D geometry section is inferred
from file size analysis and `Nexus_V1_Level` struct fields (`geometry_offset`,
`geometry_size`). Level script and minimap formats are unreversed — binary-only
analysis with no byte verification against actual files.