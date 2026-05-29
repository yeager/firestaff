# DM1 V1 Wall Source-Lock Evidence

**Lane:** DM1 V1 finish-quality — wall rendering parity
**Status:** SOURCE_LOCKED ⚠️ (pixel-diff evidence pairing blocked)
**Last updated:** 2026-05-29

---

## Scope

This document provides ReDMCSB source-lock anchors for the DM1 V1 wall rendering
pipeline: wall-set index selection, flip orientation, occlusion semantics,
and alcove combinations.  "SOURCE_LOCKED" means Firestaff's C code is cross-referenced
against ReDMCSB source — it does NOT yet have paired DOSBox → Firestaff screenshot evidence.

**Standard for MATCHED:** Three paired original screenshots at canonical wall states
(front-wall D3C, side-wall D3L/D3R, alcove D3L+D3CR), matched against Firestaff V1 output.

See `DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md` for the capture procedure.

---

## Architecture Overview

```
Party position + direction → DUNGEON.C G0306/G0307 → view-cone squares
  ↓
Square.element ∈ {WALL, DOOR, ALCOVE, FLOOR} → DUNVIEW.C wall-set selection
  ↓
Flip logic → DUNVIEW.C bitmap selection → F0132 blit to screen
  ↓
Occlusion check → D3C blocks D3L/D3R when party south of wall
```

---

## 1. Wall-Set Index Table

**ReDMCSB:** DUNVIEW.C:6226-6331, 6837-6893, 6406-6437, 6545-6573, 7244-7312, 7727-7843, 7960-8162
(also `GRAPHICS558.H` — bitmap index constants)

### D3 Wall Sets (front row — most visually prominent)

| Wall | Native bitmap | Parity bitmap | Index | Source anchor | Notes |
|------|-------------|-------------|-------|--------------|-------|
| D3L2 | G3004_i_WallSet_Wall_D3L (-8) | G3005_i_WallSet_Wall_D3R (-9) | -8 | DUNVIEW.C:6321-6327 | Back row left 2nd |
| D3R2 | G3004_i_WallSet_Wall_D3R (-9) | G3005_i_WallSet_Wall_D3L (-8) | -9 | DUNVIEW.C:6330-6331 | Back row right 2nd |
| D3L | G3004_i_WallSet_Wall_D3L (-8) | G3005_i_WallSet_Wall_D3R (-9) | -8 | DUNVIEW.C:6421-6427 | Front row left |
| D3R | G3004_i_WallSet_Wall_D3R (-9) | G3005_i_WallSet_Wall_D3L (-8) | -9 | DUNVIEW.C:6568-6573 | Front row right |
| D3C | G2120_DoorFrameLeftD3L = -29 (≈CENTER_WALL) | same | -29 | DUNVIEW.C:6716-6720 | Center — no flip |

### D2 Wall Sets (second row)

| Wall | Native bitmap | Parity bitmap | Index | Source anchor | Notes |
|------|-------------|-------------|-------|--------------|-------|
| D2L2 | G3072_i_WallSet_Wall_D3R2 (-6) | G3004_i_WallSet_Wall_D3L2 (-5) | -6 | DUNVIEW.C:6880-6889 | Parallax back left |
| D2R2 | G3004_i_WallSet_Wall_D3L2 (-5) | G3072_i_WallSet_Wall_D3R2 (-6) | -5 | DUNVIEW.C:6882-6893 | Parallax back right |
| D2L | G3004_i_WallSet_Wall_D3L (-8) | G3005_i_WallSet_Wall_D3R (-9) | -8 | DUNVIEW.C:6968-6973 | Mid left |
| D2R | G3004_i_WallSet_Wall_D3R (-9) | G3005_i_WallSet_Wall_D3L (-8) | -9 | DUNVIEW.C:7119-7123 | Mid right |
| D2C | G2120_DoorFrameLeftD3L = -29 | same | -29 | DUNVIEW.C:7308-7312 | Mid center |

### D1 Wall Sets (third row)

| Wall | Native bitmap | Parity bitmap | Index | Source anchor | Notes |
|------|-------------|-------------|-------|--------------|-------|
| D1L | D2L index | D2R index | -8 | DUNVIEW.C:7459-7460 | Near left |
| D1R | D2R index | D2L index | -9 | DUNVIEW.C:7627-7628 | Near right |
| D1C | D3C index | same | -29 | DUNVIEW.C:7842-7843 | Near center—flips |

### D0 Wall Sets (closest row — rarely visible)

| Wall | Native bitmap | Parity bitmap | Index | Source anchor | Notes |
|------|-------------|-------------|-------|--------------|-------|
| D0L | D2L index | D2R index | -8 | DUNVIEW.C:8036-8038 | Closest left |
| D0R | D2R index | D2L index | -9 | DUNVIEW.C:8142-8144 | Closest right |

### Special Walls

| Wall | Bitmap | Index | Source anchor |
|------|--------|-------|--------------|
| ALCOVE (D3LCR) | G0161_auc_Graphic558_Box_WallBitmap_D3LCR[4] + G3051_i_WallSetFlipped_Wall_D3L = -20 | -20 | DUNVIEW.C:7960-8038 |
| DOOR FRAME LEFT D3L | G2120_DoorFrameLeftD3L = -29 | -29 | DUNVIEW.C:7880-7960 |
| DOOR FRAME RIGHT D3R | derived | -29 | DUNVIEW.C:7880-7960 |

---

## 2. Flip Logic

**Rule:** A wall is flipped horizontally if it appears on the **opposite side** of the party
relative to its native position.

**ReDMCSB:** DUNVIEW.C:6421-6427 (D3L), DUNVIEW.C:6568-6573 (D3R),
DUNVIEW.C:6954-6964 (D2L), DUNVIEW.C:7105-7115 (D2R)

```
Left-positioned wall (D3L/D2L/D1L/D0L):
  → FLIPPED when party is on the RIGHT side of the wall
  → NOT FLIPPED when party is on the LEFT side of the wall

Right-positioned wall (D3R/D2R/D1R/D0R):
  → FLIPPED when party is on the LEFT side of the wall
  → NOT FLIPPED when party is on the RIGHT side of the wall

Center wall (D3C/D2C/D1C):
  → Always NOT FLIPPED (symmetric)
  → Always FLIPS with party facing (D3C front-only means only one orientation visible)
```

**Source:** DUNVIEW.C bitmap selection formula: `flip = (party_position_relative_to_wall != native_side)`

**Firestaff source:** `src/dm1/dm1_v1_viewport_3d_pc34_compat.c` → `dm1_viewport_3d_select_wall_bitmap()`

---

## 3. Wall Occlusion (D3C Blocks D3L/D3R)

**ReDMCSB:** DUNVIEW.C:6263-6264, 6330-6331, 6432-6437, 6568-6573, 6882-6893, 6968-6973, 7308-7312

When the party is **south of a wall** (facing north, toward a D3C wall), the front-center wall
(D3C) occludes the side walls (D3L and D3R) because it is drawn in front of them.

| Party position | D3C | D3L | D3R |
|---------------|-----|-----|-----|
| Directly south of wall | VISIBLE | OCCLUDED | OCCLUDED |
| Southwest of wall | VISIBLE | VISIBLE (left) | OCCLUDED |
| Southeast of wall | VISIBLE | OCCLUDED | VISIBLE (right) |
| West of wall (facing east) | OCCLUDED | VISIBLE | VISIBLE |
| East of wall (facing west) | OCCLUDED | VISIBLE | VISIBLE |

**Occlusion test in probe:** `firestaff_dm1_v1_walls_occlusion_blockers_probe.c`

**Firestaff source:** `src/memory/memory_dungeon_pc34_compat.c` → occlusion check in square visibility

---

## 4. Alcove Wall (D3LCR)

**ReDMCSB:** DUNVIEW.C:7960-8038

An **alcove** is a WALL square that has both a D3L side, a D3R side, and a front D3C.
The game renders the D3L and D3R as a combined alcove bitmap (G0161_D3LCR).

| Alcove component | Bitmap used |
|-----------------|-------------|
| Left alcove wall | G3050_i_WallSetFlipped_Wall_D3L = -19 (FLIPPED D3L) |
| Right alcove wall | G3051_i_WallSetFlipped_Wall_D3R = -20 (FLIPPED D3R) |
| Combined alcove box | G0161_auc_Graphic558_Box_WallBitmap_D3LCR[4] = {0, 115, 0, 50} |

**Alcove click routing:** CLIKVIEW.C:367 + DUNVIEW.C tests alcove in combination with
C05 wall ornament sensor (G2210_aai_XYZ clickable area at D3LCR position).

**What a paired capture would show:** The combined alcove bitmap fills the D3LCR cell area
with both left and right walls visible simultaneously — unique to the alcove configuration.

---

## 5. Champion Portrait on Wall Sensors

**ReDMCSB:** DUNVIEW.C:5096-5109, DUNGEON.C:2710-2783 (BUG0_05)

*See also: `DM1_V1_VIEWPORT_SOURCE_LOCK_EVIDENCE.md` Section 5.*

A champion portrait sensor (C127) on a wall square can be visible on all sides of that wall.
When a sensor with wall ornament (i.e., a portrait sensor that IS a wall ornament)
is on the wall, the portrait is drawn but the wall ornament data may conflict.

**BUG0_05:** Portrait drawn over wall ornament on the square adjacent to the wall sensor.

| Critical bug | Effect | ReDMCSB anchor |
|-------------|--------|---------------|
| BUG0_05 | Portrait re-renders on wall square adjacent to wall sensor | DUNVIEW.C:5096-5109 |
| BUG0_75 | Portrait ordinal not reset when dungeon view has no WALL squares → crash | DUNGEON.C:2710-2783 |

---

## 6. Wall Square Data Fields

**ReDMCSB:** DUNGEON.C square byte layout and wall-specific fields

| Square type | Element byte | Wall set index field | Door byte | Description |
|-------------|-------------|---------------------|-----------|-------------|
| WALL | `0x20` | `square >> 5` value | N/A | Solid wall square |
| DOOR | `0x28` (element DOOR) | door style | `square & 0x07` (0-7 door state) | Door cell |
| ALCOVE | `0x20` + sensor | N/A (uses D3LCR) | N/A | Alcove type |
| FLOOR | `0x00` | N/A | N/A | Floor with no walls |

**Wall set index derivation:** `(wall_bitmap_index + 8) & 0xFF` — maps the native bitmap
indices to logical wall-set categories used by the renderer.

---

## 7. Probes Locking This Lane

| Probe | What it locks | ReDMCSB anchor |
|-------|-------------|---------------|
| `firestaff_dm1_v1_wall_composition_contract_probe.c` | Wall-set bitmap selection, all 15 wall squares, flip logic | DUNVIEW.C:6226-6331, 6406-6573, 7244-7312 |
| `firestaff_dm1_v1_walls_occlusion_blockers_probe.c` | D3C occlusion of D3L/D3R when party south of wall | DUNVIEW.C:6263-6264, 6330-6331 |
| `firestaff_dm1_v1_side_contents_center_blocker_probe.c` | Side-panel center-blocker behavior | CLIKVIEW.C:290-510 |
| `firestaff_dm1_v1_door_occlusion_pixel_gate.c` | Door pixel-level occlusion: door over cell contents | DUNGEON.C:2601-2615 |
| `firestaff_dm1_v1_original_fakewall_view_collision_probe.c` | Fakewall collision + visibility semantics | DUNGEON.C:2721-2783 |

All probe tests passed 29/29 on `f7f3291f` (see `ctest` run 2026-05-29).

---

## 8. What a Paired Capture Would Show

### Capture A: Front Wall (D3C)
- **Game state:** Party facing north, D3C is a solid wall at front-center
- **Expected:** No occlusion — D3C rendered at bitmap -29, no D3L/D3R side walls visible
- **Pixel-diff:** Firestaff viewport_crop at y=33, x=0..223 matches DOSBox capture

### Capture B: Side Wall (D3L/D3R with Alcoves)
- **Game state:** Party facing east, D3L is alcove wall on left
- **Expected:** D3L rendered at bitmap -8 (or -19 if alcove), flipped correctly
- **Pixel-diff:** Same MAE < 2.0 threshold

### Capture C: Door State Cycle
- **Game state:** Same square three captures in sequence: opened / half-open / closed door
- **Expected:** Door frame constant; door panel changes per state byte
- **Occlusion check:** Contents behind door correctly blocked when closed

---

## Known Gaps (⚠️ BLOCKED)

| Gap | Blocking | How to close |
|-----|----------|--------------|
| No original DOSBox wall screenshots | Cannot pixel-compare | DOSBox Staging capture session |
| BUG0_05 portrait rendering | Not formally probe-tested against source | Add portrait ordinal render test |
| BUG0_75 (portrait crash) | Hall-of-Champions-specific edge case | Extended probe with map=0, no WALL squares |
| Fakewall rendering | Covered by fakewall probe only | DOSBox capture at F3/F0 fakewall squares |

---

## ReDMCSB Source Anchors Summary

| Anchor | Lines | What it does |
|--------|-------|-------------|
| DUNVIEW.C:3940-4008 | F0108 | Floor ornament before F0115 objects |
| DUNVIEW.C:6226-6331 | M587_VIEW_WALL_D3L2/D3R2 | Back row wall-set selection |
| DUNVIEW.C:6406-6437 | M587_VIEW_WALL_D3L | D3L flip condition |
| DUNVIEW.C:6545-6573 | M587_VIEW_WALL_D3R | D3R flip condition |
| DUNVIEW.C:6707-6720 | M587_VIEW_WALL_D3C | D3C center wall (no flip) |
| DUNVIEW.C:6880-6893 | D2L2/D2R2 | Second row wall-set selection |
| DUNVIEW.C:6968-6973 | D2L | D2L flip condition |
| DUNVIEW.C:7105-7115 | D2R | D2R flip condition |
| DUNVIEW.C:7308-7312 | D2C | D2C center (no flip) |
| DUNVIEW.C:7459-7460 | D1L | D1L flip condition |
| DUNVIEW.C:7627-7628 | D1R | D1R flip condition |
| DUNVIEW.C:7842-7843 | D1C | D1C no flip |
| DUNVIEW.C:7960-8038 | alcove D3LCR | Alcove combined bitmap |
| DUNVIEW.C:8318-8542 | F0128 | Back-to-front cell walk |
| DUNGEON.C:2601-2615 | door state | Door state byte decoding |
| DUNGEON.C:2710-2783 | G0289 portrait ordinal | Portrait sensor data management |
| COMMAND.C:367 | G2210_aai_XYZ | Wall ornament clickable zone |
| CLIKVIEW.C:367,376,379 | F0376/F0798 | Click routing for wall/door/ornaments |
| GRAPHICS558.H:160-172 | G2120, G3050, G3051 | Bitmap index constants |
| GRAPHICS558.H:416-426 | G0161_D3LCR | Alcove box dimensions |
| GRAPHICS558.H:577-580 | G0711/G0712 | D3L2/D3R2 wall frame data |

---

*Document generated from probe source-lock audit and ReDMCSB WIP20210206 cross-reference.*
*Maintainer: Firestaff parity lane DM1 V1 wall rendering*