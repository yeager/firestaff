# Pass 75 — startup-menu candidate asset sheet

Date: 2026-04-26

## Goal

Turn the pass-68 startup-menu candidate graphic windows into visible, reproducible source artifacts.

This is **not** a 320×200 original menu placement claim. It is source-asset evidence for the next original-menu overlay pass.

## Tool

- `tools/pass75_menu_candidate_asset_sheet.py`
- Stats: `parity-evidence/overlays/pass75/pass75_menu_candidate_asset_sheet.json`
- Sheets:
  - `parity-evidence/overlays/pass75/candidate_A_304_319.png`
  - `parity-evidence/overlays/pass75/candidate_B_left_360_367.png`
  - `parity-evidence/overlays/pass75/candidate_B_core_368_383.png`
  - `parity-evidence/overlays/pass75/candidate_B_right_384_391.png`
  - `parity-evidence/overlays/pass75/candidate_B_wide_360_391.png`

## Gate command

```sh
python3 tools/pass75_menu_candidate_asset_sheet.py
```

Result:

```text
windows=5, problems=[]
```

## Locked menu windows

| window | count | serial width | max dimensions |
| --- | ---: | ---: | --- |
| candidate_A_304_319 | 16 | 357 | 32×28 |
| candidate_B_left_360_367 | 8 | 138 | 30×28 |
| candidate_B_core_368_383 | 16 | 974 | 160×111 |
| candidate_B_right_384_391 | 8 | 407 | 112×29 |
| candidate_B_wide_360_391 | 32 | 1519 | 160×111 |

## Impact

The menu row in `PARITY_MATRIX_DM1_V1.md` can now point at concrete source-asset sheets, not just metric windows. Exact original 320×200 placement remains open until a stable original menu frame is captured and overlaid.
