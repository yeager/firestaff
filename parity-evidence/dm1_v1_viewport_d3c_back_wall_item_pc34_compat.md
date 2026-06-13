# dm1_v1_viewport_d3c_back_wall_item_pc34_compat

- status: `DM1_V1_D3C_F0115_BACK_WALL_ITEM_THING_PASS_LOCKED_NON_DUPLICATIVE_WITH_D3C_F0107_D3C_F0108_D3C_F0111_AND_F0115_SIBLINGS`
- generatedUtc: `2026-06-13T10:50:00.000000+00:00`
- redmcsb: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- parity claim: **not made**; this is a contract-only, no-asset
  thing-pass source-lock gate for the back-wall item (M550_FIRST_THING
  on cells back-left=3 and back-right=2) at the D3C view square
  (M600_VIEW_SQUARE_D3C=11, depth 3, lateral 0) of the DM1 V1 dungeon
  view.

## Lane

The DM1 V1 back-wall item is the *thing* (M550_FIRST_THING, item
type C05_THING_TYPE_WEAPON .. C10_THING_TYPE_JUNK) drawn on the back
cells (view_cell 3 = back-left and view_cell 2 = back-right, per
DEFS.H:2642-2645) of the D3C view square, the back-most cell in the
3×3 view grid. The lane pins the F0115_DUNGEONVIEW_DrawObjects
CreaturesProjectilesExplosions_CPSEF dispatch contract for D3C across
the four D3C square elements (wall, door front, corridor, pit /
teleporter) and the C10_COLOR_FLESH transparent blit used to draw
the item bitmap over the destination pixel.

Concretely, the contract is:

- The D3C wall branch (DUNVIEW.C:6697-6720) draws C704_WALL_D3C,
  calls F0107 alcove helper, and returns BEFORE F0115. A back-wall
  item on a plain D3C wall is NOT visible.
- The D3C door front (C17_ELEMENT_DOOR_FRONT) F0115 call at
  DUNVIEW.C:6723 with `C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT`
  decodes (after the F0115:4794-4800 door-pass nibble strip
  `MASK 0x0008_DOOR_FRONT`) to cells front-left (0) and front-right
  (1). At D3C depth 3 the F0115:4920-4923 view_cell > 1 predicate
  clips the front cells, so door pass 1 does NOT write a visible
  back-wall item.
- The D3C door front F0115 call at DUNVIEW.C:6816 (after
  `L0204_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT`
  at lines 6727-6729 and the goto T0118028 at line 6742) decodes to
  cells back-left (3) and back-right (2). At D3C depth 3 the back
  cells are visible, so door pass 2 IS where the back-wall item is
  drawn at zone `C2500_ZONE_ + M550_FIRST_THING*4 + 3 = 2511`
  (DUNVIEW.C:5075).
- The D3C corridor / pit / teleporter F0115 call at DUNVIEW.C:6816
  with `L0204_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT`
  (set at DUNVIEW.C:6812-6813 for the corridor/pit/teleporter
  T0118027 label) decodes to cells front-left, front-right,
  back-left, back-right in nibble order. At D3C depth 3 the front
  cells are clipped; the back cells are visible.
- The C10_COLOR_FLESH (palette index 10) transparent blit at
  F0115:5180-5188 leaves the destination pixel unchanged.
- The F0128 dispatch at DUNVIEW.C:8499 routes the D3C thing-pass
  after D3L (C13_VIEW_SQUARE_D3L) and D3R (C14_VIEW_SQUARE_D3R) at
  relative depth 3 / lateral 0.
- The M550_FIRST_THING ordinal (DEFS.H:2549) is the row index for
  the C2500-zone item formula at F0115:5075 (`2500 +
  M550_FIRST_THING * 4 + view_cell`).

The lane is intentionally non-duplicative with the integrated:

- `test_dm1_v1_viewport_d3c_f0107_wall_ornament_pc34_compat` (C704
  + F0107 alcove helper; this gate is the F0115 thing-pass branch
  not the F0107 alcove branch).
- `test_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_pc34_compat`
  (F0108 / M589_VIEW_FLOOR_D3C / C1503 floor-ornament zone; this
  gate is the M550_FIRST_THING item zone, not the M558 floor-ornament
  zone).
- `test_dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat`
  (F0111 door-draw helper; this gate is the F0115 thing-pass
  bracketed around F0111).
- `test_dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat`
  (C14/C15 view squares, M702/M703 wall zones, depth 3 lateral ±2;
  this gate is the C11/M600 view square, M704 wall zone, depth 3
  lateral 0).
- `test_dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_lock`
  (depth 1 lateral ±2; this gate is depth 3 lateral 0).
- `test_dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_lock`
  (depth 0 lateral ±2; this gate is depth 3 lateral 0).
- The F0107/F0108/F0115 viewport family, the chest-scroll-wheel
  family, the champion-panel family, the mirror-candidate family,
  the save/load family, the resurrect family, the occupied-slot
  family, the mechanics family, the scroll-wheel family, and the
  F0128 dispatch order family.
- The per-route F0115 corridor / pit / teleporter creature-row,
  projectile-row, and explosion-row angles (this gate pins the
  item row at M550_FIRST_THING ordinal 2 only).

## ReDMCSB anchors

- `DUNVIEW.C F0115:4547-4581` — F0115_DUNGEONVIEW_DrawObjects
  CreaturesProjectilesExplosions_CPSEF prototype and the 4-nibble
  structure comment (door-front pass bit 3 in low nibble + 3 cell
  nibbles).
- `DUNVIEW.C F0115:4794-4800` — strips the door-front pass nibble
  (MASK 0x0008_DOOR_FRONT) and computes the pass number from bit 0
  plus one.
- `DUNVIEW.C F0115:4853-4860` — view-square range M600..M609
  (D3C..D0C) gates the thing pass.
- `DUNVIEW.C F0115:4920-4923` — item visibility predicate
  (weapon..junk, view_cell > 1 at depth 3, i.e. back cells visible).
- `DUNVIEW.C F0115:5180-5188` — C10_COLOR_FLESH transparent blit.
- `DUNVIEW.C:6723` — D3C door-front F0115 call with
  `C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT`.
- `DUNVIEW.C:6816` — D3C corridor / pit / teleporter F0115 call
  with `L0204_i_Order = C0x3421`; door-front pass 2 with
  `L0204_i_Order = C0x0349`.
- `DUNVIEW.C F0128:8499` — D3C dispatch at relative depth 3 /
  lateral 0.
- `DUNGEON.C F0163:1769-1838` + `F0164:1840-1905` + `F0172:2466-2523`
  — thing-list and square-aspect sources.
- `DEFS.H:2549` — M550_FIRST_THING ordinal = 2.
- `DEFS.H:2607` — M600_VIEW_SQUARE_D3C = 11.
- `DEFS.H:2642-2645` — view cell numbering (0=front-left,
  1=front-right, 2=back-right, 3=back-left).
- `DEFS.H:2669 / 2672 / 2676` — 0x0218 / 0x0349 / 0x3421 cell
  orders.
- `DEFS.H:4042-4044` — C702 / C703 / C704 wall zones.
- `DEFS.H:4228-4236` — F0115 zone family definitions.
- `G2027_ac_ViewSquareIndexToViewDepth[11]` (DEFS.H:2554) — depth
  3 for M600/D3C.
- `G2026_ac_ViewSquareIndexToViewLane[11]` (DEFS.H:2546) — lateral
  0 for M600/D3C.

## Verification (this commit)

- `cmake --build builds/n2-build --target
  test_dm1_v1_viewport_d3c_back_wall_item_pc34_compat --parallel`
  builds clean (0 warnings).
- Strict `cc -Wall -Wextra -Werror -O2` build of both source and
  test files is also clean.
- The test runs 89/89 assertions with deterministic model hash
  `0x92F6385C` (stable across 3 consecutive runs).
- `ctest --test-dir builds/n2-build -R
  dm1_v1_viewport_d3c_back_wall_item_pc34_compat` passes 1/1.
- The related
  `dm1_v1_viewport_d3(c|l|l2|f0107|f0108|f0111|stairs_pit)|dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass|dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass|dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass`
  family (11 tests) still passes 11/11 (no regression).
- The broader `dm1_v1_viewport|csb_v1_viewport` family 182/184 with
  the 2 pre-existing failures
  (`pass512_dm1_v1_viewport_wall_clip_source_audit` missing the
  `dm1_viewport_3d_resolve_wall_blit_clip_gate` symbol,
  `dm1_v1_viewport_movement_completion_matrix` chained
  `FAIL_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS`) reproducing on
  the pre-change `171eea61a` and not introduced by this gate.
- `git diff --check` is clean.

## Files

- `include/firestaff/dm1/v1/viewport/d3c_back_wall_item_pc34_compat.h`
  (force-added, .gitignore `firestaff` line matches the
  `include/firestaff/` path, the same pre-existing pattern that bit
  pass768 / pass783 / pass786 / pass788 / pass789 / pass790plus /
  pass797 / C045 / champion_panel_name_box_clip /
  dead_member_hand_refresh / spell_area_clear_on_inventory /
  leader_swap_food_water).
- `src/dm1/dm1_v1_viewport_d3c_back_wall_item_pc34_compat.c`
- `tests/test_dm1_v1_viewport_d3c_back_wall_item_pc34_compat.c`
