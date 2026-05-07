# Pass294 — DM1 V2 entry viewport blocker manifest

Base: `a621562`

## Result

No source correction was safe to land in this pass. The pass286 comparator now records a deterministic blocker manifest with mismatch-region metrics and the exact next implementation seam.

## Metrics

- Original crop: `parity-evidence/verification/pass282_dm1_v2_original_pixel_capture/original_entry_viewport_224x136.png`
- Firestaff export: `parity-evidence/verification/pass285_dm1_v2_firestaff_entry_viewport_224x136.png`
- Diff PNG: `parity-evidence/verification/pass286_dm1_v2_entry_viewport_original_vs_firestaff_diff.png`
- Mismatch: `16405/30464` pixels (`0.5385044642857143`)
- Original unique RGBA colors: `6`
- Firestaff unique RGBA colors: `2`
- Connected mismatch components: `1223`
- Largest component: `13618` pixels, bbox `[0,0,223,135]`
- Top dense 16x16 tiles are fully mismatched (`256/256`) around x `64..159`, y `16..79`, i.e. the central/top wall-set/ceiling-visible region.
- Top color mismatch: original `#929292ff` -> Firestaff `#b6b6b6ff`, `6591` pixels.

## Source-locked blocker

This is not another palette blocker. Pass288 already normalized symbolic materials to the PC34 grayscale palette. The remaining seam is original bitmap-backed viewport materialization:

- `DUNVIEW.C:8337-8367` draws/rebuilds original floor/ceiling bitmaps into `G0296_puc_Bitmap_Viewport`.
- `DUNVIEW.C:8490-8542` walks visible squares in D3/D2/D1/D0 order and dispatches each square draw function.
- For the entry front blocking wall, `DUNVIEW.C:6699-6702` draws wall-set bitmap frames through `F0100_DUNGEONVIEW_DrawWallSetBitmap` / `F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency`, then `DUNVIEW.C:6720` returns.

Firestaff still exports `dm1_v2_vp_render_composition_flat()`: a symbolic 224x136 rectangle materialization with two colors. Pixel parity needs the original `GRAPHICS.DAT` wall-set/floor/ceiling bitmap blit + clipping path for the same route state (`map=0 x=1 y=3 dir=2`).

## Reproduction

```sh
cmake -S . -B build-pass294
cmake --build build-pass294 --target dm1_v2_entry_viewport_png_export
ctest --test-dir build-pass294 --output-on-failure -R 'dm1_v2_(entry_viewport_png_export_gate|entry_viewport_png_comparator_gate|completion_matrix)'
python3 tools/verify_dm1_v2_entry_viewport_png_comparator_gate.py
```

Evidence JSON: `parity-evidence/verification/pass286_dm1_v2_entry_viewport_png_comparator_gate.json` (`comparison.mismatchRegions`).
