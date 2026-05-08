# Pass402 — DM1 V1 side content far-to-near order

Status: `PASS402_SIDE_CONTENT_ORDER_NARROWED`

## ReDMCSB anchors

- `DUNVIEW.C:8318-8542` — `F0128_DUNGEONVIEW_Draw_CPSF` replays visible squares far-to-near: D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, then D0L/D0R/D0C.
- `DUNVIEW.C:4547-4910` — `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` draws objects, creatures, projectiles and explosions during each square replay from encoded cell-order nibbles.

## Firestaff change

`m11_draw_dm1_side_contents()` now iterates `depth = 2 .. 0` instead of `0 .. 2`, keeping the side content pass in ReDMCSB far-to-near order while preserving the existing center-line and side-lane blocker guards.

This closes the narrow blocker where far side-lane items/creatures/projectiles could be drawn after, and therefore over, nearer side-lane contents because the pass was near-to-far.

## Verification

- `python3 scripts/verify_pass402_dm1_v1_side_content_far_to_near.py`

Scope guard: this is an ordering/occlusion-source lock, not a pixel-parity claim and not the full door two-pass F0115 split for every side/front door branch.
