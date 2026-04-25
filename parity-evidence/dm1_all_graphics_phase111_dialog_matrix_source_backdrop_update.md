# DM1 all-graphics phase 111 — dialog matrix after source backdrop/version work

## Problem

Passes 109–110 changed runtime dialog rendering:

- pass 109: source `C000_GRAPHIC_DIALOG_BOX` backdrop
- pass 110: source `C450_ZONE_DIALOG_VERSION` `V3.4` text

`PARITY_MATRIX_DM1_V1.md` still described dialog/endgame overlays only as debug-cleaned placeholders and did not record these new source-backed dialog steps.

## Change

Updated the dialog/endgame row:

- source evidence now explicitly mentions `C450_ZONE_DIALOG_VERSION`
- Firestaff state now records:
  - source `C000_GRAPHIC_DIALOG_BOX` backdrop is drawn from GRAPHICS.DAT graphic `17`
  - backdrop size is `224×136`
  - it is blitted at viewport origin
  - `V3.4` is printed at reconstructed C450 screen coordinate `(192,40)`
- status remains `KNOWN_DIFF (narrowed)`, because full dialog/endgame parity is still not done
- next action narrowed to source patch/choice zones and source message centering/wrapping

Updated the bottom line to mention source dialog backdrop + version-zone text.

## Gate

Documentation-only pass, backed by pass 110:

```text
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
# summary: 423/423 invariants passed
ctest: 5/5 PASS
```
