# Pass280 — DM1 V2 entry viewport pixel-capture fixture path

## Scope

This pass creates a source-locked fixture path for comparing the canonical DM1 PC34 entry viewport pixels. It does **not** claim pixel parity.

## Locked state and region

- Canonical `DUNGEON.DAT` SHA256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- Initial party word: `0x0861` => map `0`, x `1`, y `3`, direction `2`
- Compare region: full PC34 viewport `x=0,y=0,w=224,h=136` (`30464` pixels)

## ReDMCSB audit anchors

- `DEFS.H:2478` and `DEFS.H:2484` lock `C112_BYTE_WIDTH_VIEWPORT` and `C136_HEIGHT_VIEWPORT`.
- `DUNVIEW.C:2962-3000` locks the floor/ceiling framebuffer setup and viewport dimensions.
- `DUNVIEW.C:8337-8542` locks the dungeon-view draw path and D3..D0 traversal already used by the V2 draw-list gates.
- `DUNVIEW.C:8606-8615` and `BASE.C:1309` identify the `G0296_puc_Bitmap_Viewport` blit path to screen.

## Landed artifact

- Fixture: `parity-evidence/fixtures/pass280_dm1_v2_entry_viewport_pixel_capture_fixture.json`
- Gate: `tools/verify_dm1_v2_viewport_pixel_capture_fixture_gate.py`
- CTest: `dm1_v2_viewport_pixel_capture_fixture_gate`

## Exact blocker

No matched original/ReDMCSB and Firestaff entry-state viewport PNG pair exists in this pass. A future pass may promote this only after both artifacts exist for the same state and `dm1_v2_vp_compare_viewport_region` reports zero mismatches over the full 224x136 region.
