# DM1 all-graphics phase 28 — invisible pit graphic variants

Date: 2026-04-25 14:18 Europe/Stockholm
Scope: Firestaff V1 / DM1 invisible-pit rendering variants.

## Change

Added source-bound invisible-pit graphic selection for D0..D2 pit positions.

ReDMCSB does not always draw the normal open-pit graphics for pit squares. In `DUNGEON.C`, square aspect slot `M554_PIT_OR_TELEPORTER_VISIBLE` is populated from `MASK0x0004_PIT_INVISIBLE`; in `DUNVIEW.C`, D0..D2 pit drawing then selects the `M762..M767_GRAPHIC_FLOOR_PIT_INVISIBLE_*` graphics when that bit is set.

Implemented this selection in `m11_draw_dm1_floor_pits(...)`:

- normal pits keep graphics `49..57`
- invisible pits use graphics `58..63` where ReDMCSB has variants
- D3 invisible pits skip the normal D3 pit graphic, matching the source branches where D3 only draws pit graphics when the invisible bit is clear

## Source anchors

- `DEFS.H`
  - `MASK0x0004_PIT_INVISIBLE`
  - `M762_GRAPHIC_FLOOR_PIT_INVISIBLE_D2L` = `58`
  - `M763_GRAPHIC_FLOOR_PIT_INVISIBLE_D2C` = `59`
  - `M764_GRAPHIC_FLOOR_PIT_INVISIBLE_D1L` = `60`
  - `M765_GRAPHIC_FLOOR_PIT_INVISIBLE_D1C` = `61`
  - `M766_GRAPHIC_FLOOR_PIT_INVISIBLE_D0L` = `62`
  - `M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C` = `63`
- `DUNGEON.C F0173_DUNGEON_SetSquareAspect`
  - `P0317_pui_SquareAspect[M554_PIT_OR_TELEPORTER_VISIBLE] = M007_GET(square, MASK0x0004_PIT_INVISIBLE)`
- `DUNVIEW.C F012x_DUNGEONVIEW_DrawSquare*`
  - D2L/D2C/D2R select graphics `58/59`
  - D1L/D1C/D1R select graphics `60/61`
  - D0L/D0C/D0R select graphics `62/63`

## Resolved placements

Invisible variants were resolved through layout 696 / `COORD.C F0635_`:

- D2L: graphic `58`, zone `C855`, `srcX=1 x=0 y=76 w=71 h=12`
- D2C: graphic `59`, zone `C856`, `x=66 y=77 w=92 h=12`
- D2R: graphic `58`, zone `C857`, `x=153 y=76 w=71 h=12`
- D1L: graphic `60`, zone `C858`, `srcX=3 x=0 y=94 w=49 h=24`
- D1C: graphic `61`, zone `C859`, `x=41 y=94 w=144 h=24`
- D1R: graphic `60`, zone `C860`, `x=174 y=94 w=50 h=24`
- D0L: graphic `62`, zone `C861`, `srcX=4 x=0 y=126 w=14 h=10`
- D0C: graphic `63`, zone `C862`, `x=25 y=127 w=174 h=9`
- D0R: graphic `62`, zone `C863`, `x=206 y=126 w=18 h=10`

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

- `verification-m11/dm1-all-graphics/phase28-invisible-pit-variants-20260425-1418/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase28-invisible-pit-variants-20260425-1418/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase28-invisible-pit-variants-20260425-1418/normal/party_hud_top_190_crop_vga.png`

Visual note: the broad black central rectangle was already present in phase27 before this change, so it is not itself evidence of a new invisible-pit regression. This pass still needs a deterministic pit-focused scene to visually prove normal-vs-invisible pit selection.

## Remaining work

- Add focused pit scenes that explicitly cover `MASK0x0004_PIT_INVISIBLE` on/off at D0/D1/D2.
- Add source/original screenshot comparison for those focused scenes.
