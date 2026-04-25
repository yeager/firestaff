# DM1 all-graphics phase 109 — source dialog-box backdrop

## Problem

After pass 107, default V1 hid the most obvious placeholder dialog labels, but the dialog overlay still used a procedural black rectangle and borders rather than the source DM1 dialog-box graphic.

ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` starts by expanding `G0343_puc_Graphic_DialogBox`, loaded from `C000_GRAPHIC_DIALOG_BOX`, into the viewport bitmap.

Local GRAPHICS.DAT mapping:

- `C000_GRAPHIC_DIALOG_BOX` is GRAPHICS.DAT graphic index `17`
- size `224×136`, matching the viewport

## Change

Added `M11_GFX_DIALOG_BOX = 17` and a small source-backdrop helper:

```c
m11_draw_dm_dialog_backdrop(...)
```

When a dialog overlay is active in default V1 chrome mode and assets are available, Firestaff now blits the source `C000_GRAPHIC_DIALOG_BOX` at the DM1 viewport origin `(0,33)` before drawing the message text.

The old procedural rectangle remains as fallback when assets are missing or V2/non-V1 mode is active.

## Gates

Added invariant:

- `INV_GV_172C` — V1 dialog overlay blits source `C000` dialog-box backdrop

```text
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
# summary: 422/422 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

This does not yet complete full `F0427_DIALOG_Draw` parity:

- source `V3.4` version-zone print is not wired
- 1/2/4-choice patch zones are not wired
- source choice zones/keyboard/mouse choice flow are not wired
- message centering/wrapping still needs source-zone parity
