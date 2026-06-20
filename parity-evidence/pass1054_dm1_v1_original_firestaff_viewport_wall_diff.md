# Pass1054 DM1 V1 original-to-Firestaff viewport/wall diff scout

Status: `PASS1054_ONE_WALL_CROP_EXACT_MATCH_REST_SCOUT_ONLY`

This pass compares the four pass1052 original PC 3.4 viewport crops against the
24-pose Firestaff Hall-of-Champions capture set produced by
`firestaff_dm1_v1_viewport_wall_capture_probe`.

The pairing method is nearest-neighbor image matching over the 224x136 viewport
crop.  Only exact pixel matches are promotable evidence.  Nonzero-diff rows are
scouting data and are not same-state parity claims.

## Inputs

- Original crops: `verification-screens/pass1052-dm1-original-route-24h-turncycle/viewport_224x136/`
- Firestaff captures: `/tmp/firestaff_dm1_viewport_wall_pass1052_verify/`
- Firestaff capture report: `/tmp/firestaff_dm1_viewport_wall_pass1052_verify/dm1_v1_viewport_wall_capture.md`

## Results

Artifacts live under
`verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/`.

| Original crop | Best Firestaff crop | MAE | Changed pixels | Result |
|---|---|---:|---:|---|
| `01_party_hud_original_viewport_224x136.png` | `hall_1_4_dirS_viewport_224x136.ppm` | 4.234178 | 1303 | Scout only |
| `02_left_1_wall_original_viewport_224x136.png` | `hall_1_4_dirE_viewport_224x136.ppm` | 0.000000 | 0 | **Exact pixel match** |
| `03_left_2_view_original_viewport_224x136.png` | `hall_1_2_dirS_viewport_224x136.ppm` | 12.887474 | 3967 | Scout only |
| `04_left_3_view_original_viewport_224x136.png` | `hall_1_4_dirE_viewport_224x136.ppm` | 4.937533 | 2858 | Scout only |

## Promoted evidence

The pass1052 wall crop `02_left_1_wall_original_viewport_224x136.png` is pixel-identical
to Firestaff's `hall_1_4_dirE_viewport_224x136.ppm` crop:

- original SHA256: `8d5d9bd870d9aab74907fcd2051ae71547dd27583b2b81758ebebf32cfa2161c`
- Firestaff PNG copy SHA256: `8d5d9bd870d9aab74907fcd2051ae71547dd27583b2b81758ebebf32cfa2161c`
- changed pixels: `0 / 30464`
- MAE: `0.0`

## Non-claims

- The three nonzero rows are not same-state parity evidence; they only show nearest
  visual candidates for the next route-pairing pass.
- This does not close the broader viewport route transcript gap.
- This does not close collision or creature-chain evidence.
