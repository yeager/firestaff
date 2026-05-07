# DM1 V1 viewport/walls ReDMCSB source-lock evidence — 2026-05-06

Scope: normal DM1 V1 viewport/world visuals, with emphasis on wall zones, front/side walls, draw order, ornaments/items/creatures, and occlusion.  This is an audit note only; no runtime renderer behavior is changed here.

## Primary source inspected

ReDMCSB source root:

- `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

Primary file:

- `DUNVIEW.C`

## Wall zone / graphic anchors

ReDMCSB binds the DM1 wall-set order in `G2107_WallSet[15]` (`DUNVIEW.C:183-200`).  The media-722/DM1 PC ordering is:

- D0R, D0L, D1R, D1L, D1C
- D2R2, D2L2, D2R, D2L, D2C
- D3R2, D3L2, D3R, D3L, D3C

The corresponding Firestaff wall graphics and layout-696 zone placements are already source-bound in `m11_game_view.c`:

- front wall pass: `m11_draw_dm1_front_walls` at `m11_game_view.c:9265`, using D1C/D2C/D3C placements documented inline at `m11_game_view.c:9270-9277`.
- side wall pass: `m11_draw_dm1_side_walls` at `m11_game_view.c:9810`, using D3L2/D3R2 through D0L/D0R source zones in `kSideBlits` at `m11_game_view.c:9823-9840`.

## ReDMCSB draw-stack order

`F0128_DUNGEONVIEW_Draw_CPSF` is the top-level dungeon-view draw routine (`DUNVIEW.C:8318`).  It establishes the floor/ceiling/wall-set flip state before drawing squares (`DUNVIEW.C:8336-8443`).  Its world draw sequence is far-to-near:

1. D4 object/creature/projectile cells: `DUNVIEW.C:8466-8477`
2. D3L2/D3R2: `DUNVIEW.C:8478-8486`
3. D3L, D3R, D3C: `DUNVIEW.C:8488-8499`
4. D2L2/D2R2, D2L, D2R, D2C: `DUNVIEW.C:8500-8521`
5. D1L, D1R, D1C: `DUNVIEW.C:8522-8533`
6. D0L, D0R, D0C: `DUNVIEW.C:8534-8542`

Firestaff keeps the split renderer bounded by equivalent source-shape gates:

- `m11_draw_viewport` samples cells once and derives `maxVisibleForward` at `m11_game_view.c:17872-17896`.
- side walls and wall ornaments are drawn before side contents in the normal path at `m11_game_view.c:17918-17923`; side contents follow at `m11_game_view.c:17963`.
- center-blocker replay narrows near-side wall/ornament redraws at `m11_game_view.c:17950-17954`.

## Wall blockers and same-lane occlusion

ReDMCSB wall square handlers draw the opaque wall panel and then return, so later branches for contents/items/creatures/projectiles are not reached for that blocked square:

- D3L wall branch: draw wall at `DUNVIEW.C:6406-6428`, check wall ornaments/alcove at `DUNVIEW.C:6432-6435`, then return at `DUNVIEW.C:6437`.
- D3R wall branch: draw wall at `DUNVIEW.C:6544-6564`, check wall ornaments/alcove at `DUNVIEW.C:6568-6571`, then return at `DUNVIEW.C:6573`.
- D1L wall branch: draw wall at `DUNVIEW.C:7436-7455`, wall ornament/alcove side check at `DUNVIEW.C:7459`, then return at `DUNVIEW.C:7460`.
- D1R wall branch: draw wall at `DUNVIEW.C:7604-7623`, wall ornament/alcove side check at `DUNVIEW.C:7627`, then return at `DUNVIEW.C:7628`.

Firestaff guards the equivalent regression class with:

- center-lane blocker limit: `m11_dm1_max_visible_forward_from_center` at `m11_game_view.c:9221`.
- side-lane blocker helpers: `m11_dm1_side_lane_clear_before_depth` and `m11_dm1_side_lane_clear_for_rel` at `m11_game_view.c:9495-9526`.
- side-wall guard before each blit: `m11_game_view.c:9853`.
- side contents guard before item/creature/projectile drawing: `m11_game_view.c:12040`.

## Ornaments, items, creatures, projectiles

ReDMCSB uses `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` (`DUNVIEW.C:4547`) as the per-square contents renderer.  Relevant ordering evidence:

- object/item blits and clickable-box expansion happen before the creature pass (`DUNVIEW.C:5128-5191`), and creature drawing starts at `DUNVIEW.C:5201`.
- creature bitmap selection uses side/back/attack/front flags in the creature graphic info, e.g. side-image selection at `DUNVIEW.C:5321-5328`, back-image selection at `DUNVIEW.C:5330-5368`, and non-attack flip handling at `DUNVIEW.C:5369-5378`.
- per-square callers pass explicit cell-order masks such as `C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT` for open D3L (`DUNVIEW.C:6476-6480`) and related D2/D1 variants throughout `DUNVIEW.C:6970-7704`.

Firestaff source-backed equivalents are currently split across wall ornaments, side contents, object placement, and creature viewport paths.  The narrow source-shape gates verified here protect against contents leaking through nearer wall blockers; they do not claim full pixel parity for every creature frame or ornament coordinate.

## Verification

Run on N2 from `<firestaff-repo>`:

```sh
python3 tools/verify_v1_viewport_occlusion_gate.py
python3 tools/verify_v1_viewport_side_wall_occlusion_gate.py
ctest --test-dir build --output-on-failure -R "^(v1_viewport_occlusion_gate|v1_viewport_side_wall_occlusion_gate|firestaff_dm1_v1_walls_occlusion_blockers_probe)$"
git diff --check
```

These gates are source-shape checks, not a screenshot/pixel-parity proof.  They are useful because they bind Firestaff’s split renderer to the ReDMCSB occlusion invariants above.

## Remaining blocker / next work

The biggest remaining risk is not wall-zone identity; it is full `DUNVIEW.C` stack fidelity: exact per-square element handling, door/pass ordering, wall-ornament coordinate replay, and creature frame/half-square ordering need continued source-locked narrowing before claiming complete DM1 V1 viewport parity.
