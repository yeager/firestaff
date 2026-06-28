# DM2 V2 Phase 3 - HUD Widget Real-Size Slot Receipt

**Status:** PARTIAL - one scratch-only real-size chrome slot receipt landed 2026-06-29
**CTest name:** `firestaff_dm2_v2_hud_widget_real_slot_receipt_probe`

## Scope

This receipt advances the DM2 V2 HUD widget art gate beyond the checked-in
1x1 synthetic fixtures without shipping finished art or user-supplied game
data.

The probe creates a scratch asset root under `/tmp/scratch/`, writes one
valid 32x32 RGBA PNG for the `compass_rose` `hud_chrome` slot, and installs
a manifest with:

- `compass_rose` using `generator: "pbr_hero_receipt"`
- all six other widget/chrome slots using `generator: "placeholder"`

## Gate Coverage

The probe verifies:

- the generated PNG has a PNG signature and IHDR dimensions `32x32`
- the generated slot is larger than the synthetic 1x1 fixtures
- the manifest validates structurally
- the aggregate gate reports `PARTIAL`
- `real_count == 1` and `total == 7`
- `compass_rose` classifies as `REAL`
- the slot resolves through `hud_chrome/compass_rose_receipt.png`
- placeholder slots remain procedural fallbacks
- the manifest dimensions match the PNG IHDR dimensions
- source evidence still keeps the no-finished-art boundary visible

## Source-Lock Anchors

| Anchor | Why |
|--------|-----|
| `SKULL.ASM T560` | DM2 HUD rendering pipeline |
| `skproject/SKULLWIN/c_gui_vp.cpp` | DM2 UI chrome layout |
| ReDMCSB `PANEL.C F0354` | Champion status-box drawing |
| `include/dm2_v2_hud_widget_assets.h` | Placeholder-vs-real gate API |
| `src/dm2/dm2_v2_hud_widget_assets.c` | Slot category/path classifier |

## Honest Boundary

This is a receipt for the manifest/classification path only. It does not
ship finished PBR widget art, does not decode/blit the PNG into the runtime
HUD, does not use original DM2 assets, and does not create a README/public
visual claim.
