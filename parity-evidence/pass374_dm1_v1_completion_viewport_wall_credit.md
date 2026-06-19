# Pass374 — DM1 V1 viewport/wall completion credit

Status: `BLOCKED_PASS374_DM1_V1_VIEWPORT_WALL_COMPLETION_CREDIT`

## ReDMCSB source audit first

- `DUNVIEW.C:2962-3003` — `F0098_DUNGEONVIEW_DrawFloorAndCeiling`: floor/ceiling base is copied into viewport buffers before wall replay.
- `DUNVIEW.C:3048-3082` — `F0100/F0101/F0102 viewport blitters`: transparent wall, opaque wall, and door bitmap routines target the viewport bitmap.
- `DUNVIEW.C:4547-4910` — `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: contents are replayed by ordered cell nibbles rather than by a flat depth sort.
- `DUNVIEW.C:6400-6835` — `F0116/F0117/F0118 D3 side and center square draws`: D3 wall/door branches draw wall layers, door passes, and return/occlude as source order dictates.
- `DUNVIEW.C:7244-7937` — `F0121/F0124 D2C and D1C center square draws`: center wall/door branches layer opaque wall/door graphics before ordered contents.
- `DUNVIEW.C:8318-8618` — `F0128_DUNGEONVIEW_Draw_CPSF`: main viewport pass redraws floor/ceiling, replays visible squares far-to-near, then requests viewport presentation.
- `DRAWVIEW.C:709-722` — `F0097_DUNGEONVIEW_DrawViewport`: viewport redraw request is presented and synchronized after composition.

## Landable update

This pass credits pass373 in the conservative completion matrix: DM1 V1 moves from `55/100` to `56/100`; `viewport_ui_render` moves from `10/20` to `11/20`.

The credit is narrow: live Firestaff launcher movement now reaches the source-locked wall/door/occlusion redraw stack. It does not claim pixel parity or original overlay regression.

## Gates

- `redmcsb_source_lock` ok=True
- `redmcsb_source_lock` ok=True
- `redmcsb_source_lock` ok=True
- `redmcsb_source_lock` ok=True
- `redmcsb_source_lock` ok=True
- `redmcsb_source_lock` ok=True
- `redmcsb_source_lock` ok=True
- `pass373_prerequisite` ok=False
- `completion_matrix_credit` ok=True
- `completion_doc_credit` ok=True
- `firestaff_completion_matrix_verifier` ok=True
- `firestaff_completion_status_cli` ok=True

Manifest: `parity-evidence/verification/pass374_dm1_v1_completion_viewport_wall_credit/manifest.json`
