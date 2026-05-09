# pass429 viewport projectile occlusion source/runtime lock

Source audit:
- `DUNVIEW.C:370-377` (`G2027_ac_ViewSquareIndexToViewDepth`, `G2028_ac_ViewSquareIndexTo`) maps MEDIA720 view-square ids to depth and projectile C2900 rows.
- `DUNVIEW.C:4547-4582` (`F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`) documents per-cell order: objects, creature, projectiles per ordered cell, then explosions/fluxcage after all cells.
- `DUNVIEW.C:5645-5885` (`F0115` projectile pass) source-locks projectile occlusion: unsupported G2028 rows skip; D3 front cells skip; D0 back cells skip; surviving projectiles use `C2900_ZONE_ + row*4 + viewCell`, scale index `(viewDepth << 1) - (viewCell >> 1)`, and `F0791...(... G0296_puc_Bitmap_Viewport, L2474_i_ZoneIndex, ...)`.
- `DEFS.H:2596-2611` and `DEFS.H:2641-2645` source-lock the MEDIA720 view-square ids and zero-based view-cell constants used by the runtime metadata.

Gates:
- `build/test_dm1_v1_viewport_3d_pc34_compat` -> `runtime_test.out`
- `scripts/verify_pass429_dm1_v1_projectile_occlusion_source_runtime_lock.py --run-runtime` -> `source_runtime_gate.out` / `.json`
- regression: `scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py --run-runtime` -> `pass395_regression_gate.out` / `.json`
