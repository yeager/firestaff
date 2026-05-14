# Pass505 DM1 V2 asset manifest validator gate

Date: 2026-05-14 22:45+02:00
Scope: DM1 V2 scaffolding/assets completion evidence. This pass does not increase the DM1 V2 completion percentage.

## Selected missing gate

`docs/parity/COMPLETION_MATRIX.md` scored DM1 V2 `viewport_ui_render` at `4/20` because only viewport/wall/upscale scaffolds were counted and full render/material/UI proof remains absent. The missing low-risk scaffolding/assets gate was that the repository-local V2 manifest validator existed but was not required by CTest or by `tools/verify_dm1_v2_completion_matrix.py`.

## ReDMCSB source audit

Primary source root audited: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

Relevant source anchors for keeping V2 asset manifests source-bound rather than free-form art claims:

- `DEFS.H:2348-2350`: floor and ceiling set constants (`M650_GRAPHIC_FLOOR_SET_0_FLOOR`, `M651_GRAPHIC_FLOOR_SET_0_CEILING`).
- `DEFS.H:2351-2377`: PC34/I34E wall and door-set graphic constants, including D0-D3 wall planes and door-set base.
- `DEFS.H:2405-2408`: derived viewport bitmap constant `C000_DERIVED_BITMAP_VIEWPORT`, documented as the 224x136 field/explosion viewport.
- `DEFS.H:2469-2484`: viewport byte-width and height constants (`C112_BYTE_WIDTH_VIEWPORT`, `C136_HEIGHT_VIEWPORT`).
- `DUNVIEW.C:2037-2054`: `F0094_DUNGEONVIEW_LoadFloorSet` loads floor/ceiling bitmaps as a paired source set.
- `DUNVIEW.C:2962-3002`: `F0098_DUNGEONVIEW_DrawFloorAndCeiling` draws the 224-wide ceiling/floor bands into the viewport.
- `DUNVIEW.C:3048-3130`: wall, door, floor-pit and stairs bitmap draw helper routes.
- `DUNVIEW.C:8418-8515`: flipped wall/floor/ceiling setup and D3 square draw-order composition route.

## Changes

- Added CTest `dm1_v2_asset_manifest_validator` for `tools/validate_v2_manifests.py`.
- Added CTest `dm1_v2_asset_manifest_validator_self_test` for the validator's synthetic regression fixtures.
- Required both gates in `tools/verify_dm1_v2_completion_matrix.py` so future matrix runs cannot omit all-assets manifest validation.
- Updated the Firestaff completion matrix wording for DM1 V2 `viewport_ui_render` without changing `27%`.

## Verification

- `python3 tools/validate_v2_manifests.py`: passed, 17 manifests / 107 asset entries / 0 errors / 0 warnings.
- `python3 tools/validate_v2_manifests.py --self-test`: passed.
- Targeted CTest: `dm1_v2_asset_manifest_validator`, `dm1_v2_asset_manifest_validator_self_test`, and `dm1_v2_completion_matrix_gate` passed.
- `python3 tools/verify_firestaff_completion_matrix.py`: passed, still `DM1 V2 | completionPercent=27%`.
- `git diff --check`: passed.
