# pass361 DM1 V1 viewport occlusion/redraw order gate

Date: 2026-05-08
Branch: worker/fix-blockers-pass304-original-capture-20260507
Scope: source-bound verification gate. No runtime behavior changes.

## ReDMCSB source audit anchors

Primary source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `DUNVIEW.C:8318-8618` locks `F0128_DUNGEONVIEW_Draw_CPSF`: floor/ceiling composition, far-to-near square drawing, then viewport presentation request.
- `DUNVIEW.C:8445-8542` locks the visible-square order inside `F0128`: D3 extra panels, D3 side/center, D2 side/center, D1 side/center, then D0 side/center.
- `DUNVIEW.C:6400-6835` locks D3 wall/door behavior: solid wall branches draw their wall zone and return, while door-front branches split rear contents, door/frame, then front contents.
- `DUNVIEW.C:7244-7937` locks D2C/D1C center-wall and door-front behavior, including center wall early-return occlusion and two-pass door-front object ordering.
- `DRAWVIEW.C:709-722` locks the post-redraw handoff: `F0097_DUNGEONVIEW_DrawViewport` requests the viewport blit and waits for a vertical blank after `G0296_puc_Bitmap_Viewport` has been composed.

## Firestaff gate

- `m11_game_view.c:m11_draw_viewport` is checked for the same broad redraw ordering: clear/background first, source-backed floor/wall/door layers next, a nearer-side replay after a blocking center wall/door, then side and center content, with the procedural corridor renderer restricted to debug HUD mode.
- The gate chains the pass359 viewport wall draw-order/occlusion sweep and the two narrow CTest probes that cover draw order and wall/blocker occlusion.

## Result

This pass adds a landable regression gate for the specific seam left after pass359: the normal DM1 V1 viewport redraw must preserve original-source occlusion order when a center blocker forces nearer side layers to be replayed before contents and before any final viewport present.
