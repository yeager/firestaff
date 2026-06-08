# DM1 V1 Viewport F0098 Floor/Ceiling Fallback Source Lock

Contract-only PC34 gate for the function-level `F0098_DUNGEONVIEW_DrawFloorAndCeiling` fallback path.

## Source Anchors

PASS `DUNVIEW.C:F0098:2962-3002` owns the floor/ceiling refresh body, including entry, black-area clear, ceiling/floor copies, viewport size restore, and dirty-flag clear.
PASS `DUNVIEW.C:F0128:8337-8338` calls F0098 only when `G0297_B_DrawFloorAndCeilingRequested` is set.
PASS `DUNVIEW.C:F0128:8564-8571` keeps the F0128 viewport enumeration interaction after the floor/ceiling fallback and reaches D0L/D0R at the function level.
PASS `DUNVIEW.C:F0125:8005/8033` binds D0L to `M610_VIEW_SQUARE_D0L` and the PC34 `C716_ZONE_WALL_D0L` fallback overlay.
PASS `DUNVIEW.C:F0126:8115/8139` binds D0R to `M611_VIEW_SQUARE_D0R` and the PC34 `C717_ZONE_WALL_D0R` fallback overlay.
PASS `DUNVIEW.C:F0104:3113-3151` preserves `C10_COLOR_FLESH` transparency for fallback overlay blits.
PASS `DUNVIEW.C:F0792:3288-3301` draws refreshed floor/ceiling bitmaps with `CM1_COLOR_NO_TRANSPARENCY`.
PASS `DUNVIEW.C:F0108:3940-4008` keeps zero floor-ornament ordinal as no-draw.
PASS `DUNVIEW.C:F0128:8606-8615` presents the viewport and pre-fills floor/ceiling for the next draw.
PASS `DEFS.H:2076/2088/2588-2589/4056-4057` locks `CM1_COLOR_NO_TRANSPARENCY`, `C10_COLOR_FLESH`, `M610/M611`, and `C716/C717`.

## Verification

PASS `test_dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat` asserts the function-level contract without real-asset dependencies.
PASS The gate is intentionally not direction-specific and does not duplicate `dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat`.
FAIL None.
