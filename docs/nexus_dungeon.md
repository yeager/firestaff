# Dungeon Master Nexus V1 — Dungeon Design vs DM1/DM2/CSB

## Sources
- `src/nexus/nexus_v1_dungeon.c` (level loading + square grid)
- `src/nexus/nexus_v1_game.c` (track selection + game state)
- `docs/NEXUS_FILE_CLASSIFICATION.md` (file list)
- `docs/NEXUS_PLAN.md`

---

## Overview

Nexus reuses the **DM1 dungeon layout** but with major data format changes:
DM1's single 33 KB DUNGEON.DAT is replaced by 16 separate LEV*.DGN files
(148-321 KB each), each embedding full 3D polygon geometry for that level.
Nexus adds per-level CD audio, per-level sound effect banks, level scripts,
and binary minimaps — none of which exist in DM1.

| Aspect | DM1 | CSB | DM2 | Nexus |
|--------|-----|-----|-----|-------|
| Total levels | 10 | 10 | 1 + outdoor | **16** |
| Dungeon format | DUNGEON.DAT (33 KB) | DUNGEON.DAT | DUNGEON.DAT (39 KB) | **LEV00-15.DGN (148-321 KB each)** |
| Square grid | 32x32 x 2 bytes | same | same | **same** |
| 3D geometry | None | None | None | **Embedded polygon mesh** |
| Level scripts | None | None | None | **SLEV*.BIN (2-12 KB each)** |
| Minimap | ASCII MAP command | same | same | **SMAP*.BIN (17-30 KB)** |
| CD audio | None | None | Per-level | **8 Red Book Audio tracks** |
| Sound effects | Global SND.GAM | same | same | **SNDLEV*.SAL (290-460 KB each)** |

---

## 16 Levels vs DM1's 10

Nexus has **16 levels** (LEV00-15.DGN), vs DM1's 10:
- Levels 0-9: standard DM1 dungeon layout (32x32 grid per level)
- Levels 10-15: additional levels unique to Nexus (prototype/beta content?)
- The 6 extra levels (10-15) likely correspond to the unreleased/beta portion
  of the original DM1 dungeon that FTL cut from the final game

DM1 levels are numbered 1-10 in-game; Nexus may renumber them 0-15.

Source: `nexus_v1_dungeon.c` level index range (0-15).

---

## Dungeon File Format (LEV*.DGN)

Each level file is 148-321 KB (vs DM1's 33 KB total for all 10 levels):

```
LEV00.DGN  (148 KB) — Level 0
LEV01.DGN  (168 KB) — Level 1
...
LEV15.DGN  (321 KB) — Level 15
```

File structure (inferred from `Nexus_V1_Level` struct):
```
Offset 0x0000:  Square grid      (32x32 x 2 bytes = 2048 bytes) — same as DM1
Offset 0x0800:  Geometry section (variable size) — wall/floor/ceiling polygons
Offset N:       Level script     (SLEV*.BIN, 2-12 KB) — trigger/event data
Offset M:       Minimap data     (SMAP*.BIN, 17-30 KB) — binary automap
```

The square grid format (2 bytes/square, lower 5 bits = type) is byte-verified
to match DM1's DUNGEON.DAT format. The geometry section is reverse-engineered
from file size analysis and `Nexus_V1_Level` struct fields (`geometry_offset`,
`geometry_size`).

Source: `nexus_v1_dungeon.c` (`nexus_v1_level_load`).

---

## Square Grid: Same as DM1

- 32x32 grid of squares per level (1024 squares total)
- 2 bytes per square = 2048 bytes per level
- Lower 5 bits of byte = square type (wall, open, door, pit, teleporter, etc.)
- Upper bits = square state (door open/closed, pit open/covered, etc.)
- This is identical to DM1's DUNGEON.DAT square format

The dungeon layout (wall positions, corridor paths, room shapes) is the same
as DM1 — FTL reused the original dungeon rather than designing a new one.
This means Nexus levels 0-9 map approximately to DM1 levels 1-10.

Source: `nexus_v1_dungeon.c` (square grid parsing).

---

## 3D Geometry Section

Unlike DM1 (which computed walls procedurally from square types), Nexus embeds
**pre-computed polygon meshes** for walls, floors, and ceilings:

- Vertex data: x, y, z floats per vertex
- Normal vectors: nx, ny, nz per vertex (for lighting)
- UV coordinates: u, v per vertex (for texture mapping)
- Triangle index list: 3 uint16 indices per triangle
- Per-face texture references

This geometry is loaded from the DGN file's geometry section and passed
directly to the rasterizer — no square-type-to-polygon conversion needed.
The tradeoff is file size: DM1's 33 KB becomes Nexus's 148-321 KB per level.

Source: `nexus_v1_dungeon.c` (`Nexus_V1_Level` struct, `geometry_offset` field).

---

## Level Scripts (SLEV*.BIN)

Each level has an associated script file (SLEV00.BIN through SLEV15.BIN):

- 2-12 KB per file (level-dependent complexity)
- Contains trigger/event data for that level
- Triggers: pressure plates, lever states, quest flags
- Events: creature spawns, door state changes, text events
- Format: binary (not text); reverse-engineered from ISO analysis

Level scripts likely control:
- Which doors are locked vs open at game start
- Secret wall positions
- Quest progression triggers
- Level-specific cinematics

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## Binary Minimap (SMAP*.BIN)

Each level has a pre-rendered minimap file (SMAP00.BIN through SMAP15.BIN):

- 17-30 KB per file
- Binary format storing explored/visible square data
- Unlike DM1's ASCII MAP command (real-time generation), Nexus pre-calculates
  which squares are visible from common paths and stores the result

The minimap format probably stores:
- Explored square bitmask (which squares have been visited)
- Visible square bitmask (which squares are currently visible)
- Wall/floor/open state per explored square

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## Per-Level CD Audio (Red Book Audio, Tracks 2-9)

Nexus is the first DM game with a **CD soundtrack**:

```
Track 2:  Levels 0-1  (LEV00.DGN, LEV01.DGN)
Track 3:  Levels 2-3
Track 4:  Levels 4-5
Track 5:  Levels 6-7
Track 6:  Levels 8-9
Track 7:  Levels 10-11
Track 8:  Levels 12-13
Track 9:  Levels 14-15
```

Track selection formula (from `nexus_v1_game.c`):
```c
int nexus_v1_cd_track_for_level(int level) {
    return 2 + (level / 2);  /* Tracks 2-9 */
}
```

8 tracks for 16 levels — each track covers 2 adjacent levels.
Audio is Red Book Audio (44.1kHz stereo) stored as CDDA sectors on the ISO.

Source: `nexus_v1_game.c` (`nexus_v1_cd_track_for_level`).

---

## Per-Level Sound Effect Banks (SNDLEV*.SAL)

DM1 used a single global sound effects file (SND.GAM). Nexus has
**per-level sound effect banks**:

```
SNDLEV00.SAL  (290 KB) — Level 0 SFX
SNDLEV01.SAL  (310 KB) — Level 1 SFX
...
SNDLEV15.SAL  (460 KB) — Level 15 SFX
```

Each .SAL file contains:
- Sound effect data (waveform samples, compressed format TBD)
- Sound mapping: SNDLEV*.MAP (66-90 bytes each) — maps event IDs to sample offsets

Level-specific SFX allows varied audio per dungeon depth (e.g., different
creature sounds on deeper levels). This mirrors the per-level CD audio design.

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## What's NEW vs DM1/DM2/CSB

1. **3D geometry pre-computation** — walls stored as polygon meshes,
   not computed at render time from square types
2. **16 levels** — more than DM1's 10; includes prototype/beta levels
3. **Per-level CD audio tracks** — first DM game with substantial music
4. **Per-level sound banks** — each level has dedicated SFX
5. **Binary minimap** — pre-rendered minimap data vs ASCII MAP command
6. **Level scripts** — trigger/event data per level (not present in DM1)

---

## What's the Same as DM1

- Square type format (32x32 grid, 2 bytes/square, lower 5 bits = type)
- Map layout (same geometry as DM1 for levels 0-9)
- Square types: wall, open, door, pit, teleporter, etc.
- Champion advancement, combat, spell system (DM1-based logic)
- Dungeon logic (sensors, triggers, door mechanics) — identical to DM1

---

## DM1 vs Nexus Dungeon Size Comparison

| Metric | DM1 | Nexus |
|--------|-----|-------|
| Total dungeon data | ~33 KB | 148-321 KB per level x 16 |
| Total dungeon size | ~33 KB | ~3.5-5 MB |
| Levels | 10 | 16 |
| Square grid size | 32x32 x 2 bytes x 10 = 20 KB | 32x32 x 2 bytes x 16 = 32 KB |
| 3D geometry | None | 146-319 KB per level |
| Level scripts | None | 2-12 KB per level |
| Minimap data | None (MAP command) | 17-30 KB per level |
| CD audio | None | 8 tracks (Red Book) |
| SFX data | ~100 KB global | 290-460 KB per level |

---

## Status: PARTIALLY SOURCE-LOCKED

Level count and file structure are confirmed (ISO classification + engine code).
Square grid format matches DM1 (2 bytes/square). 3D geometry section is inferred
from file size analysis and `Nexus_V1_Level` struct fields (`geometry_offset`,
`geometry_size`). Level script and minimap formats are unreversed — binary-only
analysis with no byte verification against actual files.