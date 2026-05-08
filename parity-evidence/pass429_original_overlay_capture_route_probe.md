# Pass 429 — original overlay capture route probe

Scope: source/capture-route proof only. This does **not** claim original-vs-Firestaff pixel parity and does not promote raw DOSBox screenshots.

## What is now locked

- Viewport capture geometry is source-locked to `COORD.C:1693-1698`: screen origin `x=0`, PC34 `y=33`.
- Viewport composition is source-locked to `DUNVIEW.C:2969-3000`: dungeon view is composed into `G0296_puc_Bitmap_Viewport` with `224x136` dimensions.
- Screen flush is source-locked to `DRAWVIEW.C:840-858` and `BASE.C:961-987`: the viewport buffer is copied to screen offset `5280`, i.e. the `y=33` 320x200 capture crop.
- HUD/status ownership is source-locked to `CHAMDRAW.C:771-822` and `CHAMDRAW.C:1019-1052`; top champion HUD is screen-owned, not viewport-crop content.
- Runtime movement/spell/inventory input ownership is source-locked to `COMMAND.C:375-405` and `COMMAND.C:473-482`:
  - movement routes use `C068`/`C069`/`C070` zones,
  - spell-panel entry uses `C100_COMMAND_CLICK_IN_SPELL_AREA` and the spell subroute zones,
  - inventory close/toggle routes include right-button commands (`C083`/`C011`) as well as champion status zones.
- Inventory overlay composition is source-locked to `PANEL.C:2375-2389`: the inventory panel is expanded into `G0296_puc_Bitmap_Viewport` before comparison.

## Capture-route unblock

`scripts/dosbox_dm1_original_viewport_reference_capture.sh` now supports serialized `rclick:<x>,<y>` tokens in both macOS Swift and Linux/N2 `xdotool` injectors. That matters because the ReDMCSB runtime route contains right-button inventory toggle/close paths; forcing all route probes through left-clicks could never faithfully exercise those source-owned commands.

## Current honest gate

`tools/verify_original_overlay_capture_source_lock.py` passes the ReDMCSB source/tool lock, including right-click token support, but the existing `pass112-n2-stable-hud-route` capture remains semantically blocked (`wall_closeup`/duplicate frames in later shots). So this pass unblocks the **route mechanism/proof** for representative original runtime movement/HUD/viewport overlay capture, but not pixel parity promotion.
