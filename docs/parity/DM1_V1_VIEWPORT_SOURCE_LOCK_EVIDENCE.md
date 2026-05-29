# DM1 V1 Viewport Source-Lock Evidence

**Lane:** DM1 V1 finish-quality — viewport rendering parity
**Status:** SOURCE_LOCKED ⚠️ (pixel-diff evidence pairing blocked)
**Last updated:** 2026-05-29

---

## Scope

This document provides ReDMCSB source-lock anchors for every significant decision in the
DM1 V1 viewport rendering pipeline.  "SOURCE_LOCKED" means the Firestaff C implementation
is cross-referenced against the original decompilation — pixel-diff evidence against
real DOSBox captures does NOT yet exist, which is why this lane remains ⚠️ BLOCKED.

**Standard for MATCHED:** `SOURCE_LOCKED` is sufficient for implementation correctness but
NOT for `MATCHED` parity.  Every viewport claim needs at minimum three paired
(original DOSBox → Firestaff V1 output) screenshot comparisons at canonical game states.

See `DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md` for the capture procedure that would close this gap.

---

## Architecture Overview

```
COMMAND.C G0447/G0448 — arrow/wall click hit zones
  ↓
DUNGEON.C G0306/G0307 — view-cone map-square sampling (party+fwd+side)
  ↓
DUNGEON.C:1371-1391 — pre-step camera interpolation for V2 only
  ↓
DUNVIEW.C viewport draw walk — back-to-front cell walk, clipped by wall square
  ↓
DRAWVIEW.C F0097 — viewport blit + VBlank wait
  ↓
VIEWPORT.C F0566 — bitplane blit to screen at (x=0, y=33, 224x136)
```

---

## 1. Viewport Dimensions and Blit

| Claim | ReDMCSB anchor |
|-------|---------------|
| Viewport source: 224×136 at y=33 | VIEWPORT.C:15-28 (F0564 F0566) |
| Screen destination: 320×200 | VIEWPORT.C:20-24 |
| 4-bitplane VGA blit (16 pixels/byte) | VIEWPORT.C:51-98 |
| VBlank wait after blit | DRAWVIEW.C:709-722 (F0097) |
| Palette-as-before for screenshots | VBLANK.C:93-315, probes/firestaff_dm1_v1_viewport_palette_as_before_probe.c |

**Firestaff source:** `src/engine/m11_game_view.c` → `m11_draw_viewport()`

---

## 2. View-Cones Sampling (G0306/G0307)

**ReDMCSB:** DUNGEON.C:1371-1391 (discrete G0306/G0307, DUNVIEW.C:8606-8612 renders from those)

| Connsquare | Map offset | Pixel position on screen |
|-----------|-----------|------------------------|
| D3L (left wall) | party.mapX + fwd*fX + side*rX - (-1)*fX | x≈17, y≈20 |
| D3R (right wall) | party.mapX + fwd*fX + side*rX - (+1)*fX | x≈206, y≈20 |
| D3C (front wall) | party.mapX + fwd*fX + fwd*fY | x≈112, y≈18 |
| D3L2 | party.mapX + fwd*fX - 2*fX | x≈17, y≈18 |
| D3R2 | party.mapX + fwd*fX + 2*fX | x≈206, y≈18 |
| D2L/D2R | party.mapX + fwd*fX + (-2..-1)*fX | x≈55, y≈45 |
| D2C | party.mapX + fwd*fX + 2*fY - fwd*fX | x≈112, y≈50 |
| D1L/D1R | party.mapX + fwd*fX + (-1..0)*fX | x≈80, y≈80 |
| D1C (front cell) | party.mapX + fwd*fX + fY | x≈112, y≈85 |
| D0 (center/party square) | party.mapX | x≈112, y≈116 |

**Source:** DUNGEON.C:1371-1391, DUNVIEW.C:8318-8542

**Firestaff source:** `src/memory/memory_dungeon_pc34_compat.c` → `m11_get_viewcone_square()`

---

## 3. Dungeon Viewport Back-to-Front Walk

**ReDMCSB:** DUNVIEW.C:8318-8542 (F0128_DUNGEONVIEW_Draw_CPSF)

Draw order (back-to-front, painter's algorithm):

```
for each relForward in [ -3, -2, -1, 0 ]:
    for each relSide in [LEFT, CENTER, RIGHT]:
        draw all visible things in cell(relForward, relSide)
```

Within each cell, draw order:
1. Floor ornament (F0108, BEFORE F0115 objects)
2. Object piles (F0115: objects by cell, D2/D3 shrink + palette)
3. **Creature** ← important: creature drawn AFTER objects in same cell
4. Projectiles (F0115 restarts thing list after creature)
5. Explosions (F0115 final pass, after all thing processing)

**Firestaff source:** `probes/dm1/firestaff_dm1_v1_viewport_draw_order_probe.c` — per-frame trace

---

## 4. Wall Composition (Wall-Set Bitmap Selection)

**ReDMCSB:** DUNVIEW.C:6226-6331, 6837-6893, 6406-6437, 6545-6573, 7244-7312, 7727-7843, 7960-8162

All wall indices documented in `probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c`:

| Square | Native wall bitmap | Parity wall bitmap | Zone | Flips |
|--------|------------------|-------------------|------|-------|
| D3L2 | DM1_WALL_D3L2 (-5) | DM1_WALL_D3R2 (-6) | WALL_D3L2 | yes |
| D3R2 | DM1_WALL_D3R2 (-6) | DM1_WALL_D3L2 (-5) | WALL_D3R2 | yes |
| D3L | DM1_WALL_D3L (-8) | DM1_WALL_D3R (-9) | WALL_D3L | yes |
| D3R | DM1_WALL_D3R (-9) | DM1_WALL_D3L (-8) | WALL_D3R | yes |
| D3C | DM1_WALL_D3C (-27) | DM1_WALL_D3C | WALL_D3C | no |
| D2L2 | DM1_WALL_D2L2 | DM1_WALL_D2R2 | WALL_D2L2 | yes |
| D2R2 | DM1_WALL_D2R2 | DM1_WALL_D2L2 | WALL_D2R2 | yes |
| D2L | DM1_WALL_D2L | DM1_WALL_D2R | WALL_D2L | yes |
| D2R | DM1_WALL_D2R | DM1_WALL_D2L | WALL_D2R | yes |
| D2C | DM1_WALL_D2C | DM1_WALL_D2C | WALL_D2C | no |
| D1L | DM1_WALL_D1L | DM1_WALL_D1R | WALL_D1L | yes |
| D1R | DM1_WALL_D1R | DM1_WALL_D1L | WALL_D1R | yes |
| D1C | DM1_WALL_D1C | DM1_WALL_D1C | WALL_D1C | no |

**Flip rule:** left wall (D3L/D2L/D1L/D0L) is **flipped horizontally** when party appears to the
right of the wall (i.e., when parity-side fills the D3R/D2R/D1R/D0R position).
Right wall (D3R/D2R/D1R/D0R) is **flipped** when party appears to the left.

**Firestaff source:** `src/dm1/dm1_v1_viewport_3d_pc34_compat.c` → `dm1_viewport_3d_select_wall_bitmap()`

---

## 5. Champion Portrait Sensor on Wall Squares

**Bug:** BUG0_05 — "A champion portrait sensor on a wall square is visible on all sides
of the wall. If there is another sensor with a wall ornament on one side of the wall then the
champion portrait is drawn over that wall ornament."

**Repercussion:** G0289 portrait ordinal not reset when dungeon view has no WALL squares,
causing portrait redraw crash (BUG0_75, DM1 V1 UI/mouse fix committed 2026-05-28
in `330e576f`).

**ReDMCSB:** DUNVIEW.C:5096-5109 (portrait draw in MEDIA009/MEDIA008 paths),
DUNGEON.C:2710-2783 (sensor data and portrait ordinal management)

**Source:** COMMAND.C:413-415 in ReDMCSB maps portrait sensor click routing for C05 wall
ornament interaction.

**What a paired capture would show:** A champion portrait (32×29 pixels, from
C026_GRAPHIC_CHAMPION_PORTRAITS) drawn on a D3C wall square with a C127 sensor, at the
correct portrait-ordinal position, not overpainting adjacent wall ornaments.

**Probe:** `probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c` does NOT yet have
a portrait sensor test; this is a known gap.

---

## 6. Door Rendering

| Door type | Rendered | Source-lock anchor |
|-----------|---------|-------------------|
| Front door cell | Door frame + open/closed state | DUNVIEW.C:4600-4690 |
| Wall door button | Button only (C05 click box) | CLIKVIEW.C:367, G2210_aai_XYZ |
| Door state 0 (closed) | Full frame + door subbitmap | DUNGEON.C:2601-2615 |
| Door state 1 (half-open) | Partial blit, passage visible | DUNGEON.C |
| Door state 2 (open) | Frame only, passage clear | DUNGEON.C |

**Door occlusion:** Door in cell blocks things behind it; rendering must stop at closed door.

**Firestaff source:** `probes/dm1/firestaff_dm1_v1_door_occlusion_pixel_gate.c`

---

## 7. Screen Zones and Click Routing

**ReDMCSB:** COMMAND.C G0447/G0448 (primary/secondary mouse input maps),
CLIKVIEW.C:290-510 (F0376/F0798 click routing)

| Zone | Pixel region | What it routes to |
|------|-------------|-----------------|
| VIEWPORT | x=0..223, y=33..168 | D3L..D1C cell routing (G0448 arrow zones) |
| CHAMPION PANEL | x=0..319, y=0..64 | Champion status panels (C151/C187) |
| ACTION ROW | x=0..319, y=169..199 | Command execution buttons |
| C05 WALL ORNAMENT | viewport clickable area | Door button / wall ornament (G2210 entry) |
| FOCUS CARD | x=218..303, y=106..139 | Debug/special overlay |

**Critical:** Focus-card shortcut at x=218..303,y=106..139 preempts G0448 arrow zones unless
`showDebugHUD || !m11_v1_chrome_mode_enabled()` gates it (FIX committed 2026-05-28, `330e576f`).

**Firestaff source:** `src/engine/m11_game_view.c` → `M11_GameView_HandlePointerButton()`

---

## 8. Floor and Ceiling

**ReDMCSB:** DUNVIEW.C:2962 (base floor/ceiling composite)

Floor and ceiling render before any walls, in a simple front-to-back (no painter's
algorithm needed — floor is always at the bottom of the viewport, ceiling at top).

**Firestaff source:** `m11_draw_dungeon_viewport()` → floor/ceiling pass

---

## Probes That Lock This Lane

| Probe | What it locks |
|-------|-------------|
| `firestaff_dm1_v1_viewport_draw_order_probe.c` | Draw order contract: wall→floor_orn→object→creature→proj→explosion |
| `firestaff_dm1_v1_viewport_palette_as_before_probe.c` | Palette register state preserved across frames |
| `firestaff_dm1_v1_wall_composition_contract_probe.c` | Wall-set bitmap selection and flip logic |
| `firestaff_dm1_v1_walls_occlusion_blockers_probe.c` | D3C blocks D3L/D3R when party south of wall |
| `firestaff_dm1_v1_door_occlusion_pixel_gate.c` | Door renders over cell contents when closed |
| `firestaff_dm1_v1_side_contents_center_blocker_probe.c` | Side-panel center-blocker |

---

## What a Paired Capture Would Show

A paired capture would consist of three original DOSBox viewport frames matched to
Firestaff V1 output from identical game state.  The following semantic checks would
confirm MATCHED:

1. **Same wall-set indices** for all 9 visible cells: D3L, D3C, D3R, D2L, D2C, D2R, D1L, D1C, D1R
2. **Same flip orientation** for D3L and D3R (left wall flipped in certain party positions)
3. **Same creature ordering** within each cell (creature after all objects)
4. **Portrait ordinal** matches C127 sensor ordinal in dungeon
5. **Door state** matches square's door state byte
6. **No pixel-diff artifacts** at object/wall boundaries

**MAE threshold:** < 2.0 (out of 255 per channel), max delta < 8.

---

## Known Gaps (⚠️ BLOCKED)

| Gap | Why it blocks | How to close |
|-----|--------------|-------------|
| No original DOSBox `dungeon_gameplay` frames | Cannot do pixel-diff verification | DOSBox Staging capture with screen-detect automation |
| Portrait sensor rendering | BUG0_05 not fully probe-tested | Add portrait ordinal render test to wall probe |
| BUG0_75 (portrait ordinal crash) | Race condition in dungeon with no WALL squares | May need extended probe with Hall of Champions |
| Focus-card overlap | Overlaps top arrow row at certain resolutions | Covered by existing fix, needs confirmation probe |

---

## ReDMCSB Source Anchors Summary

| Anchor | Lines | What it does |
|--------|-------|-------------|
| VIEWPORT.C:15-28 | F0564 | InitializeBitPlanes: 224×136 source |
| VIEWPORT.C:51-98 | F0566 | BlitToScreen: 4 bitplane blit |
| DRAWVIEW.C:709-722 | F0097 | DrawViewport: requests blit + wait VBlank |
| DUNGEON.C:1371-1391 | G0306/G0307 | View-cone square sampling |
| DUNGEON.C:2710-2783 | G0289 | Portrait ordinal management |
| DUNVIEW.C:2962 | F0098 | Floor/ceiling composite |
| DUNVIEW.C:3940-4008 | F0108 | Floor ornaments before objects |
| DUNVIEW.C:4547-4582 | F0115 | Object/creature/projectile/explosion order |
| DUNVIEW.C:4820-4918 | F0115 object path | Objects by cell, D2/D3 shrink |
| DUNVIEW.C:5201-5514 | F0115 creature path | Creature pose, scaling, clipping |
| DUNVIEW.C:6226-6331 | M587_VIEW_WALL_D3L2/D3R2 | Wall-set bitmap selection |
| DUNVIEW.C:6406-6437 | M587_VIEW_WALL_D3L | Flip logic for D3L wall |
| DUNVIEW.C:6545-6573 | M587_VIEW_WALL_D3R | Flip logic for D3R wall |
| DUNVIEW.C:6707-6720 | M587_VIEW_WALL_D3C | Front wall (D3C) no flip |
| DUNVIEW.C:8318-8542 | F0128 | Back-to-front cell walk |
| COMMAND.C:375-405 | G0447/G0448 | Mouse input → command mapping |
| CLIKVIEW.C:290-510 | F0376/F0798 | Click routing by zone |
| CLIKVIEW.C:367 | G2210_aai_XYZ | C05 door button / wall ornament click box |

---

*Document generated from probe source-lock audit (probes/dm1/*) and ReDMCSB WIP20210206 cross-reference.*
*Maintainer: Firestaff parity lane DM1 V1 viewport*