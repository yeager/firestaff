# pass467 DM1 V1 viewport field/source-row clip gate

Date: 2026-05-09
Scope: DM1 V1 viewport/world visuals — teleporter fields, source-row clipping, and wall/door draw-order adjacency. Probe/evidence only; no renderer behavior changes.

## ReDMCSB source audit first

Primary source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `DUNVIEW.C:3394-3465` — `F0791_DUNGEONVIEW_DrawBitmapXX` rejects `CM1_UNKNOWN`, resolves `P2081_i_ZoneIndex` via `F0635_(..., G2032_ai_XYZ, ...)`, then blits to `G0296_puc_Bitmap_Viewport`. This is the shared C2500/C2900 source-zone clipping seam.
- `DUNVIEW.C:4382-4474` — `F0113_DUNGEONVIEW_DrawField` resolves the field zone through `F0635_(NULL, L2472_ai_XYZ, P2086_i_ZoneIndex, ...)`, selects `C076_GRAPHIC_FIRST_FIELD + M728_NATIVE_BITMAP_RELATIVE_INDEX(...)`, then masked-blits into `G0296_puc_Bitmap_Viewport` using `G2073_C224_ViewportPixelWidth`.
- `DUNVIEW.C:6219,6289,6831,7554,8159` — teleporter fields use `G2035_ac_ViewSquareIndexToFieldAspectIndex` and the C702..C717 wall-zone family for D3/D1/D0 rows.
- `DUNVIEW.C:4806-4812,5071-5078` — object rows select `G2028_ac_ViewSquareIndexTo[...]`, then `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` plus view-cell offset.
- `DUNVIEW.C:5668-5683,5882-5885` — projectile rows reject invalid/near/far cells, select `C2900_ZONE_ + G2028 row * 4 + view cell`, then draw through `F0791`.
- `COORD.C:2052-2174` — `F0635_` resolves layout records, object/creature shift masks, parent clipping, and final zone width/height.
- `DEFS.H:4042-4057,4228-4230` — wall field zones C702..C717 and content families C2500/C2900.

## Landable update

`tools/verify_v1_viewport_field_zone_aspect_clip_gate.py` now also locks:

- ReDMCSB `F0113` field-zone clipping through `F0635` and viewport masked blit;
- ReDMCSB `F0791` source-zone bitmap clipping through `F0635` and viewport blit;
- ReDMCSB `COORD.C:F0635_` parent-zone clipping mechanics;
- Firestaff `m11_draw_dm1_field_zone()` clipping to the real viewport edges, not synthetic pane bounds.

This pairs with the existing C2500/C2900 `tools/verify_v1_viewport_distance_row_clip_gate.py` and keeps the non-C2500/C2900 teleporter-field rows source-locked against the same clipping rule.

## Result

`python3 tools/verify_v1_viewport_field_zone_aspect_clip_gate.py` passes and reports exact ReDMCSB anchors for `F0113`, `F0791`, and `F0635_`.
