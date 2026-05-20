# DM1 V1 teleporter field source lock

Status: source-backed implementation present; the TODO row should be retired after parent verification.

The original DM1 PC34 viewport does not draw a procedural sparkle, crosshair, or particle effect for visible teleporters. ReDMCSB draws the teleporter as a field bitmap overlay:

- `DUNVIEW.C:377` maps each view square through `G2035_ac_ViewSquareIndexToFieldAspectIndex`.
- `DUNVIEW.C:756-773` defines the PC34 `G0188_aauc_Graphic558_FieldAspects` rows: relative bitmap index, base start unit, transparent color, mask, dimensions, and X offset.
- `DUNVIEW.C:4382-4470` `F0113_DUNGEONVIEW_DrawField` loads `C076_GRAPHIC_FIRST_FIELD + NativeBitmapRelativeIndex`, applies the optional field mask, and calls `F0133_VIDEO_BlitBoxFilledWithMaskedBitmap` with `M005_RANDOM(2)+BaseStartUnit` and `M003_RANDOM(32)`.
- `DUNVIEW.C:6288-8308` draws teleporter fields into the matching wall zones after the square thing pass for the relevant view square.

Firestaff V1 now keeps the runtime path on that source shape:

- `src/engine/m11_game_view.c:m11_draw_dm1_teleporter_fields` samples only open and visible teleporter squares and draws the GRAPHICS.DAT field bitmap into the source wall-zone rectangles.
- `src/engine/m11_game_view.c:m11_draw_dm1_field_zone` applies the source field/mask bitmap and uses `animTick` as a deterministic stand-in for ReDMCSB's randomized fill offsets.
- The older procedural cyan frame/cross/dots cue was removed from the normal cell-effect cue path so asset-backed teleporter fields are the only V1 visual effect.

Verification gate: `tools/verify_dm1_v1_teleporter_visual_effect_source_lock.py`.
