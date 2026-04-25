# DM1 all-graphics phase 81 — update parity matrix viewport status

## Problem

`PARITY_MATRIX_DM1_V1.md` still described the viewport row as the old pass-40 `KNOWN_DIFF` state:

- Firestaff runtime viewport `(12,24,196,118)`
- DM1 source anchor encoded but not active
- next action: bind renderer to `M11_DM1_VIEWPORT_*`

That was stale after phases 76–79. Runtime now binds `M11_VIEWPORT_*` to the source DM1 viewport `(0,33,224,136)`, probe gates were migrated, and legacy prototype viewport constants were removed.

## Change

Updated the parity matrix viewport row:

- Current state now says `M11_VIEWPORT_*` binds to `M11_DM1_VIEWPORT_* = (0,33,224,136)`.
- Status is now `MATCHED` for **rectangle bounds**.
- Clarifies that pixel/content parity remains separate work.
- Next action now points at viewport content parity: source draw order, right-column/action UI polish, and original screenshot overlay.

## Gate

```text
grep m11_game_view.c confirms:
M11_VIEWPORT_X = M11_DM1_VIEWPORT_X
M11_VIEWPORT_Y = M11_DM1_VIEWPORT_Y
M11_VIEWPORT_W = M11_DM1_VIEWPORT_W
M11_VIEWPORT_H = M11_DM1_VIEWPORT_H
```
