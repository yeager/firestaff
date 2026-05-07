# DM1 V1 viewport/walls/occlusion source lock — N2 2026-05-07

Scope: narrow ReDMCSB source audit for the DM1 V1 viewport draw lane, wall/side-wall/front-wall blockers, and object/creature/projectile/explosion layering.  No renderer behavior was changed in this pass; this records the source contract already enforced by `scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py`.

## Source roots

- Primary source: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- DM1 canonical anchors: `<firestaff-original-games>/_canonical/dm1/`
- Source hashes observed by the gate:
  - `DUNVIEW.C` sha256 `a1eb2774b6e3962e51361aac67da50c2cfb40daf25211c449586dc3ffdfd8846`
  - `DUNGEON.C` sha256 `57b2aba28bab373ce1cd0e09b906def30b7754210a7634595029c0611d79259c`
  - `DRAWVIEW.C` sha256 `c351ac4ec146174f92aca504d90757657e6c79b47d20f9b2ba7df44b078fa9af`

## Findings

1. **Viewport draw order is back-to-front and source-owned by `F0128_DUNGEONVIEW_Draw_CPSF`.**  `DUNVIEW.C:8318-8618` sets wall flip/parity state, handles floor/ceiling presentation, then calls the visible square lane in order: D4 object-only cells (`DUNVIEW.C:8469`, `8473`, `8477`), D3 side lanes and center (`8482-8499`), D2 (`8513-8521`), D1 (`8525-8533`), D0 side squares (`8537-8541`), then D0C (`8542`).  PC34/I34E also restores the native wall set after parity flipping at `DUNVIEW.C:8578`.

2. **Side-wall blockers return before open/door/field content paths.**  D3L2 uses `case C00_ELEMENT_WALL`, draws `G2107_WallSet[C11_WALL_D3L2]`, checks the facing ornament, then returns (`DUNVIEW.C:6253-6264`).  D1L/D1R do the same for near side walls (`DUNVIEW.C:7436-7460`, `7604-7628`).  D0L/D0R nearest side walls draw their side-wall zones and return (`DUNVIEW.C:8007-8038`, `8117-8144`).

3. **Center front wall occlusion has a single alcove exception.**  `F0124_DUNGEONVIEW_DrawSquareD1C` draws the D1C wall panel from `G2107_WallSet[C04_WALL_D1C]` (`DUNVIEW.C:7784-7840`), then only calls `F0115...` when `F0107...IsDrawnWallOrnamentAnAlcove` succeeds (`DUNVIEW.C:7842-7843`).  The branch returns at `DUNVIEW.C:7872`, so a normal front wall blocks door/field/content drawing behind it.

4. **Door-front layering is two-pass.**  In the D1C front-door branch, ReDMCSB draws behind-door contents first (`DUNVIEW.C:7875` with door pass 1), draws frame/button/door (`DUNVIEW.C:7877-7908`), then draws front cells with door pass 2 (`DUNVIEW.C:7910-7937`).  This is the contract for center-door object/projectile occlusion ordering.

5. **Field/teleporter overlay is after square content for D1C.**  The D1C teleporter/open tail draws content through `F0115...` (`DUNVIEW.C:7937`) and overlays the field via `F0113_DUNGEONVIEW_DrawField` using the G2035 field-aspect mapping and D1C wall zone (`DUNVIEW.C:7946-7955`).

6. **Thing layering inside visible cells is source-defined by `F0115`.**  The function comment at `DUNVIEW.C:4561-4582` defines packed cell-order semantics, alcove mode, door passes, per-cell objects first, then creature, then projectiles, followed by a separate all-cells explosion/fluxcage pass.  Object filtering starts at `DUNVIEW.C:4820-4865`; creature pass begins at `DUNVIEW.C:5201-5208`; projectile pass restarts the list at `DUNVIEW.C:5679-5683` and blits through C2900 zones at `DUNVIEW.C:5876-5883`; explosion pass restarts after all cells at `DUNVIEW.C:5915-5933`.

7. **Projectile door occlusion is data-backed.**  `DUNGEON.C:560-565` defines `G0254_as_Graphic559_DoorInfo`; only door type 0 (portcullis) carries `MASK0x0002_PROJECTILES_CAN_PASS_THROUGH` (`DUNGEON.C:562`).  Wooden/iron/ra doors do not carry that projectile-through bit in this source table.

## Gate

`python3 scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py` passed on N2 and covers the source ranges above plus viewport presentation, floor/ceiling base copy, wall/door blit zones, field cache/mask path, square aspect setup, F0115 C2500/C2900 perspective rows, and DM1 canonical anchors (`GRAPHICS.DAT`, `DUNGEON.DAT`, `TITLE`, `README.md`).
