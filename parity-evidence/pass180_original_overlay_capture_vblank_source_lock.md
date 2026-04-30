# Pass 180 — original overlay/capture vblank source lock

Goal: strengthen the DM1 V1 original overlay/capture evidence without touching renderer or active viewport draw-order files.

## ReDMCSB source citations

- `STARTUP2.C:566-569` allocates `G0296_puc_Bitmap_Viewport` as `M075_BITMAP_BYTE_COUNT(224, 136)` and anchors black/floor subareas from that buffer.
- `DRAWVIEW.C:709-722` sets `G0324_B_DrawViewportRequested = C1_TRUE` after palette request handling, explicitly saying the viewport bitmap in `G0296_puc_Bitmap_Viewport` is drawn by the vertical blank handler.
- `BASE.C:961-987` is the vertical blank copy path: when `G0324_B_DrawViewportRequested` is set, it clears the flag, loads `G0296_puc_Bitmap_Viewport`, targets `G0348_Bitmap_Screen + #5280`, loops `#135` through 136 lines, and copies `40 + 40 + 32 = 112 bytes = 224 pixels` per line with `C160_BYTE_WIDTH_SCREEN` stride.
- `BASE.C:1246-1288` keeps viewport overlays routed through `F0020_MAIN_BlitToViewport(...)` into `G0296_puc_Bitmap_Viewport`.

## Focused verifier

`tools/verify_original_overlay_capture_source_lock.py` now includes `pc34-vblank-viewport-screen-copy`, source-locking the request-to-screen-copy leg that the overlay/capture pipeline depends on.

```text
probe=original_overlay_capture_source_lock
section=redmcsb_source_lock
redmcsbSource=/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source
sourceRange=COORD.C:1693-1698 id=viewport-screen-origin status=ok
sourceClaim=DM1 viewport screen anchor is x=0, y=33 for the PC34-era route.
sourceRange=DUNVIEW.C:2969-3000 id=viewport-buffer-compose status=ok
sourceClaim=Dungeon view is composed into G0296_puc_Bitmap_Viewport with the 224x136 viewport dimensions.
sourceRange=DRAWVIEW.C:840-858 id=viewport-flush-to-screen status=ok
sourceClaim=The composed viewport buffer is flushed to the screen viewport zone; platform paths are explicit.
sourceRange=BASE.C:961-987 id=pc34-vblank-viewport-screen-copy status=ok
sourceClaim=On the Atari/PC34-era vertical blank path, a viewport draw request copies 136 lines from G0296_puc_Bitmap_Viewport into G0348_Bitmap_Screen at offset 5280 (y=33).
sourceRange=BASE.C:1246-1288 id=generic-viewport-blit-helper status=ok
sourceClaim=Viewport overlays/panels use the shared helper that writes to G0296_puc_Bitmap_Viewport.
sourceRange=BASE.C:1394-1424 id=generic-screen-blit-helper status=ok
sourceClaim=HUD/screen regions use the shared helper that writes to G0348_Bitmap_Screen.
sourceRange=FILLBOX.C:814-822 id=screen-blit-fallback-helper status=ok
sourceClaim=The FILLBOX variant confirms the same screen blit helper route on SU1E/AU media.
sourceRange=CHAMDRAW.C:771-822 id=champion-status-hud status=ok
sourceClaim=Champion status HUD redraws are source-owned by CHAMDRAW status-box paths.
sourceRange=CHAMDRAW.C:1019-1052 id=champion-icon-hud status=ok
sourceClaim=Top champion icons/names are screen-HUD draws, not viewport-crop content.
sourceRange=PANEL.C:2375-2389 id=inventory-overlay-viewport status=ok
sourceClaim=Inventory panel is an overlay composed into the viewport buffer before capture comparison.
sourceRange=TIMELINE.C:1817-1829 id=timeline-hud-refresh-events status=ok
sourceClaim=Timed HUD refreshes route back into champion redraw state rather than a separate capture-only path.
redmcsbSourceLockOk=1
section=route_tool_lock
routeTool=scripts/dosbox_dm1_original_viewport_reference_capture.sh status=ok
routeTool=tools/pass80_original_frame_classifier.py status=ok
routeTool=tools/pass112_original_semantic_route_audit.py status=ok
routeToolLockOk=1
section=attempt_semantic_gate
attemptDir=verification-screens/pass112-n2-stable-hud-route
labelManifest=verification-screens/pass112-n2-stable-hud-route/original_viewport_shot_labels.tsv status=present
classifierJson=verification-screens/pass112-n2-stable-hud-route/pass80_original_frame_classifier.json status=present pass=0
captureCount=6
capture=image0001-raw.png class=dungeon_gameplay expected=dungeon_gameplay match=True
capture=image0002-raw.png class=dungeon_gameplay expected=dungeon_gameplay match=True
capture=image0003-raw.png class=wall_closeup expected=dungeon_gameplay match=False
capture=image0004-raw.png class=wall_closeup expected=spell_panel match=False
capture=image0005-raw.png class=wall_closeup expected=dungeon_gameplay match=False
capture=image0006-raw.png class=wall_closeup expected=inventory match=False
attemptProblem=image0003-raw.png: classified wall_closeup, expected dungeon_gameplay
attemptProblem=image0004-raw.png: classified wall_closeup, expected spell_panel
attemptProblem=image0005-raw.png: classified wall_closeup, expected dungeon_gameplay
attemptProblem=image0006-raw.png: classified wall_closeup, expected inventory
attemptProblem=duplicate raw frames detected: 1 unique sha256 value(s) repeat
duplicateSha256Counts={"ee7741746ea9b30739238e9f0780f57982bd0abe07bf60cea24e9cf92018e89c": 4}
semanticReadyForOverlay=0
honesty=source/tool lock only; semanticReadyForOverlay must be 1 before original-vs-Firestaff pixel parity claims
overallReadyForOverlayParity=0
```

Honesty: this pass strengthens source/capture-route evidence only. The stable HUD route attempt is still semantically blocked for later frames (`semanticReadyForOverlay=0`), so this does not claim original-vs-Firestaff pixel parity.
