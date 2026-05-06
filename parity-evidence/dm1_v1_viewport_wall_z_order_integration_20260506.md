# DM1 V1 viewport wall/z-order integration source audit (2026-05-06)

Source root audited before this pass:
`/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

## ReDMCSB anchors

- `DUNVIEW.C:4567-4581`, `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: the thing draw loop is per ordered cell for objects, creatures, and projectiles; explosion/fluxcage drawing restarts once after all packed cells are processed.
- `DUNVIEW.C:4853-4860`, `F0115`: visible objects are drawn from the current processed cell before creature/projectile passes.
- `DUNVIEW.C:5195-5202`, `F0115`: creatures are drawn after object scanning for the processed cell, with D4 depth excluded on PC34/I34E via `L2475_i_ViewDepth > 3`.
- `DUNVIEW.C:5681-5883`, `F0115`: projectiles restart from the first thing for the processed cell, compute PC34 zone `C2900_ZONE_ + depth*4 + cell`, then draw with `F0791_DUNGEONVIEW_DrawBitmapXX` on I34E.
- `DUNVIEW.C:5915-5933`, `F0115`: explosions are drawn only after all ordered cells, by restarting the first thing list again.
- `DUNVIEW.C:6254-6264`, `F0676_DrawD3L2`: PC34/I34E parity uses the opposite side-wall bitmap plus horizontal flip when `G0076_B_UseFlippedWallAndFootprintsBitmaps` is set; wall case draws ornament then returns.
- `DUNVIEW.C:6421-6437`, `F0116_DUNGEONVIEW_DrawSquareD3L`: same side-wall parity pattern; front alcove is the only wall case that continues into `F0115`, otherwise wall blocks contents.
- `DUNVIEW.C:6707-6720`, `F0118_DUNGEONVIEW_DrawSquareD3C_CPSF`: center wall uses `F0792_DUNGEONVIEW_DrawBitmapYYY(..., G0076...)`; front alcove can reveal contents, otherwise wall returns.
- `DUNVIEW.C:8466-8542`, `F0128_DUNGEONVIEW_Draw_CPSF`: viewport traversal is D4L/D4R/D4C, then PC34 side lanes D3L2/D3R2, then D3/D2/D1/D0 back-to-front.

## Firestaff lock updated

`dm1_v1_viewport_3d_pc34_compat` now exposes whether each F0115 layer repeats per ordered cell or runs once after all cells.  This prevents the integration layer from treating explosions as just another per-cell draw layer and locks the existing wall parity/occlusion metadata to the same source audit.
