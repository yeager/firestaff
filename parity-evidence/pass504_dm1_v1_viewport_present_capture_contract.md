# Pass504 - DM1 V1 viewport present/capture contract

Status: PASS504_DM1_V1_VIEWPORT_PRESENT_CAPTURE_CONTRACT_LOCKED

## Decision
The next viewport/walls parity promotion must be a same-frame capture contract tied to ReDMCSB F0128 composition and F0097 present, with canonical PC34 data hashes recorded.

## ReDMCSB source locks
- DUNVIEW.C:8318-8610 / F0128_DUNGEONVIEW_Draw_CPSF - ok=True; Composition reaches the F0097 present boundary after far-to-near wall/content replay.
- DRAWVIEW.C:709-858 / F0097_DUNGEONVIEW_DrawViewport - ok=True; PC34 present boundary blits G0296_puc_Bitmap_Viewport through C007_ZONE_VIEWPORT.

## Canonical DM1 PC34 data
- DUNGEON.DAT - ok=True; sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- GRAPHICS.DAT - ok=True; sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- TITLE - ok=True; sha256 adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745

## Capture contract
- hash-lock canonical PC34 DUNGEON.DAT, GRAPHICS.DAT, and TITLE before choosing the frame
- record map, x, y, facing, wall/door state, light/palette state, and viewport crop for the original frame
- record the matching Firestaff state and prove it reaches F0128_DUNGEONVIEW_Draw_CPSF
- prove the same frame reaches F0097_DUNGEONVIEW_DrawViewport before comparing pixels
- reject static duplicate screenshots and source-only evidence as parity promotion inputs

## Non-claims
- no new original capture was performed
- no original-vs-Firestaff pixel parity is promoted
- no movement-core files are touched
