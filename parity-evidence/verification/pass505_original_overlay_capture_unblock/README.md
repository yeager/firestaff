# Pass 505 - original overlay/capture unblock

This lane keeps the original overlay blocker honest and narrows it with one fresh N2 recapture attempt.

## ReDMCSB source anchors

- COORD.C:1693-1698 fixes the PC34 viewport screen anchor at x=0, y=33.
- DUNVIEW.C:2962-3003 composes floor/ceiling into G0296_puc_Bitmap_Viewport and stamps the 224x136 viewport dimensions.
- DRAWVIEW.C:840-858 flushes G0296_puc_Bitmap_Viewport through F0021_MAIN_BlitToScreen(...) or VIDRV_09_BlitViewPort.
- BASE.C:961-987 copies 136 viewport lines from G0296_puc_Bitmap_Viewport to G0348_Bitmap_Screen at offset 5280 (y=33).
- BASE.C:1246-1288 is the shared viewport blit helper for overlay/panel writes into G0296_puc_Bitmap_Viewport.
- BASE.C:1394-1424 and FILLBOX.C:814-822 are screen blit helper paths into G0348_Bitmap_Screen.
- CHAMDRAW.C:771-822 and CHAMDRAW.C:1019-1052 own status-box and champion-icon HUD redraws.
- COMMAND.C:375-405 owns inventory/status/movement mouse routes; COMMAND.C:473-482 owns spell-area subroutes.
- PANEL.C:2375-2389 expands the inventory graphic into G0296_puc_Bitmap_Viewport.
- TIMELINE.C:1817-1829 refreshes all champion status boxes through champion redraw state.

## Fresh N2 attempt

Command shape:

    OUT_DIR=$PWD/verification-screens/pass505-original-overlay-mouse-route-recapture
    DM1_ORIGINAL_STAGE_DIR=/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DungeonMasterPC34
    DM1_ORIGINAL_PROGRAM='DM VGA'
    DM1_ROUTE_SKIP_STARTUP_SELECTOR=1
    DOSBOX=/usr/bin/dosbox
    WAIT_BEFORE_INPUT_MS=5000
    NEW_FILE_TIMEOUT_MS=5000
    DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:party_hud kp5 wait:900 shot kp5 wait:900 shot click:250,80 wait:900 shot:spell_panel click:250,120 wait:900 shot rclick:300,190 wait:900 shot:inventory_panel'
    xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
    python3 tools/pass80_original_frame_classifier.py verification-screens/pass505-original-overlay-mouse-route-recapture --expected pass84-overlay --fail-on-duplicates
    python3 tools/pass112_original_semantic_route_audit.py verification-screens/pass505-original-overlay-mouse-route-recapture --out-json parity-evidence/pass505_original_overlay_mouse_route_recapture_audit.json --out-md parity-evidence/pass505_original_overlay_mouse_route_recapture_audit.md

## Result

The tool blocker for N2 geometry is fixed: /usr/bin/dosbox under Xvfb emits exact 640x400 2x screenshots, and the capture normalizer now converts only that exact size to original 320x200 before crop/classifier gates.

The semantic blocker remains: the fresh attempt produced six 320x200 frames, but all classified as graphics_320x200_unclassified, with two repeated hashes (a17790109e74... and b1cefb2478a8...). This does not reach dungeon_gameplay, spell_panel, or inventory, so it is still not eligible for original-vs-Firestaff overlay parity.

Next blocker: determine why DM VGA on N2/DOSBox 0.74 captures low-content alternating frames under Xvfb before route events reach the expected runtime states. The exact tools are present (/usr/bin/dosbox, /usr/bin/xvfb-run, /usr/bin/xdotool, Python Pillow); the missing evidence is a semantic original runtime route with six non-duplicate frames classified as dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory.
