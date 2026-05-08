# Pass401 — DM1 V1 viewport door occlusion source lock

Status: `PASS401_DOOR_OCCLUSION_METADATA_NARROWED`

## ReDMCSB audit anchors
- `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF` — visible-square order `D4L/D4R/D4C → D3L2/D3R2 → D3L/D3R/D3C → D2L2/D2R2 → D2L/D2R/D2C → D1L/D1R/D1C → D0L/D0R/D0C`, lines `8446-8542`.
- `DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` — packed cell-order contract and thing overlay order, lines `4561-4581`; projectile pass `5679-5904`; explosion pass after ordered cells `5915-5933`.
- `DUNVIEW.C:F0116_DUNGEONVIEW_DrawSquareD3L` — door-front split: rear cells `C0x0218` before frame/door, frame+door, front cells `C0x0349` after door, lines `6443-6459`.
- `DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF` — same split for D3C, lines `6722-6746`.
- `DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C` — same split for D2C, lines `7314-7341`.
- `DUNVIEW.C:F0111_DUNGEONVIEW_DrawDoor` — door bitmap/ornament draw helper, lines `4218-4334`.

## Result
Closed/narrowed the next viewport occlusion blocker by making the front-door occlusion split explicit in `dm1_v1_viewport_3d_pc34_compat`: rear cell contents are source-locked to draw before the door frame/bitmap and front cell contents after it. This complements the existing wall-return/front-alcove metadata and gives the renderer a verified contract for door occlusion ordering without claiming pixel parity.

## Verifier/test
- C test: `test_dm1_v1_viewport_3d_pc34_compat` now checks `dm1_viewport_3d_door_front_occlusion_spec_*`, packed order decode (`0x0218` rear, `0x0349` front), and exact line anchors.
- Python gate: `tools/verify_dm1_v1_viewport_3d_occlusion_metadata_gate.py` now checks ReDMCSB door-front source slices and local metadata needles.
