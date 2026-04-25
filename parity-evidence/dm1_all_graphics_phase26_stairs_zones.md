# DM1 all-graphics phase 26 — source-bound stairs zones

Date: 2026-04-25 13:55 Europe/Stockholm
Scope: Firestaff V1 / DM1 stairs rendering.

## Change

Added first source-bound stairs rendering pass for normal V1.

The renderer now samples visible viewport cells with `DUNGEON_ELEMENT_STAIRS`, checks stairs orientation against party facing, and blits original wall-set 0 stairs graphics into the resolved ReDMCSB layout-696 zones.

## Source mapping

DM1 wall-set 0 stairs start at:

- `M645_GRAPHIC_FIRST_STAIRS = 108`
- `C018_STAIRS_GRAPHIC_COUNT = 18`

Mapped graphics:

- `108..114` — stairs up front D3/D2/D1/D0 variants
- `115..121` — stairs down front D3/D2/D1/D0 variants
- `122` — side D2
- `123` — up side D1
- `124` — down side D1
- `125` — side D0

Source anchors:

- `DUNVIEW.C F0120..F0127_DUNGEONVIEW_DrawSquare*`
- `G0079_ai_StairsNativeBitmapIndices`
- `C800..C833_ZONE_STAIRS_*`
- `DEFS.H MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION`
- `COORD.C F0635_` via `tools/resolve_dm1_zone.py`

## Covered zones

Front-facing stairs:

- Up front: `C800..C812`
- Down front: `C813..C825`

Side-facing stairs:

- Side D2: `C826..C827`
- Up side D1: `C828..C829`
- Down side D1: `C830..C831`
- Side D0: `C832..C833`

Right-hand positions currently use the original left-side bitmap positioned in the right-side resolved zone. That matches the existing pit-pass strategy, but deterministic comparison scenes are still needed to confirm whether explicit horizontal flip is required for some right-side stairs.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase26-stairs-zones-20260425-1355/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase26-stairs-zones-20260425-1355/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase26-stairs-zones-20260425-1355/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious stair graphic bleed or UI contamination in the sampled corrected-VGA frame.

## Remaining work

- Deterministic stair-specific capture scenes for front/side, up/down, left/right.
- Verify whether right-side stair/pit zone blits require horizontal flip (`F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally`).
- Next viewport domains: teleporters/fields and invisible pit variants.
