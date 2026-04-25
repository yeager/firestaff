# DM1 all-graphics phase 1 asset boundary

Date: 2026-04-25 09:50 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 in-game runtime graphics.

## Rule

V1 runtime truth is original DM1 PC 3.4 `GRAPHICS.DAT` plus ReDMCSB/source-bound geometry. V2/upscaled/generated assets are not valid V1 truth and must not silently enter V1 rendering.

## Original asset bank

```text
2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e  /Users/bosse/.firestaff/data/GRAPHICS.DAT
```

## Runtime grep

Saved report:

- `verification-m11/dm1-all-graphics/phase1-asset-boundary-20260425-0950/v1-runtime-v2-grep.txt`

The checked V1 runtime files (`m11_game_view.c`, `asset_loader_m11.c`, `main_loop_m11.c`, `firestaff_main_m11.c`, and the M11 game-view probe) contain no direct `assets-v2` / `v2-assets` file reads. V2 references inside `m11_game_view.c` are embedded generated slice assets gated by `FIRESTAFF_V2_VERTICAL_SLICE` and are explicitly out of V1 parity scope.

## Tooling warning

The scripts `tools/build_original_vs_4k_asset_pdf.py`, `tools/build_trusted_original_vs_v2_asset_pdf.py`, and V2 generator/render scripts are comparison or V2 prep tools only. They are not V1 truth unless the original-side mapping is separately source-locked.

## Verification

Phase 1 evidence was generated alongside the current CTest/probe gates. The V1 `GRAPHICS.DAT` SHA-256 is recorded in:

- `verification-m11/dm1-all-graphics/phase1-asset-boundary-20260425-0950/GRAPHICS.DAT.sha256`
