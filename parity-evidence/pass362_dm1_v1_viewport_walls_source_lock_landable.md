# pass362 DM1 V1 viewport/walls draw-order source-lock landable metadata

Date: 2026-05-08
Branch: worker/fix-blockers-pass304-original-capture-20260507
Scope: metadata/probe gate only. No renderer behavior changes.

## Primary ReDMCSB evidence audited first

Primary source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `DUNVIEW.C:8318-8618` — `F0128_DUNGEONVIEW_Draw_CPSF` locks full viewport redraw order, parity setup, square traversal, viewport handoff, and anticipatory floor/ceiling redraw.
- `DUNVIEW.C:8445-8542` — `F0128` PC34/I34E visible-square call order: D3 extras, D3L/D3R/D3C, D2 extras, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C.
- `DUNVIEW.C:6213-6356` — D3L2/D3R2 far-extra field/wall/door/content order and lane-specific cell-order constants.
- `DUNVIEW.C:6361-6835` — `F0116`/`F0117`/`F0118` D3 side/center wall, door, wall-ornament, and blocker returns.
- `DUNVIEW.C:7244-7937` — `F0121`/`F0124` D2C/D1C center wall and door-front branches, including front-alcove exceptions and door two-pass contents.
- `DUNVIEW.C:7960-8308` — `F0125`/`F0126`/`F0127` near D0 wall/door/content order.
- `DRAWVIEW.C:709-722` — `F0097_DUNGEONVIEW_DrawViewport` requests viewport blit and waits one VBlank after composition.

Secondary local data anchor: canonical DM1 PC34 `GRAPHICS.DAT` and `DUNGEON.DAT` symlinks under `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/`. These are asset anchors only; this pass does not claim pixel parity.

## Landable update

This pass adds a single chained verifier that keeps the draw-order and wall/occlusion probes landable together:

- runs `tools/verify_pass361_dm1_v1_viewport_occlusion_redraw_order_gate.py`;
- runs `tools/verify_dm1_v1_viewport_3d_occlusion_metadata_gate.py`;
- runs CTest probes `dm1_v1_viewport_draw_order_probe` and `firestaff_dm1_v1_walls_occlusion_blockers_probe` when a build tree is available;
- verifies the evidence cites exact ReDMCSB file/function/line anchors and includes canonical original-data hashes.

## Result

Landable as source-lock metadata/probe coverage. Remaining blocker is still original runtime pixel/semantic promotion, not this source-order gate.
