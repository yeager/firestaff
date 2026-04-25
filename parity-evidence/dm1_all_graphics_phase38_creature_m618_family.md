# DM1 all-graphics phase 38 — creature M618 native graphic family

Date: 2026-04-25 15:58 Europe/Stockholm
Scope: Firestaff V1 / DM1 creature native sprite graphic family.

## Change

Corrected creature native sprite base from old placeholder graphics to the ReDMCSB/DM1 PC creature family:

- `M618_GRAPHIC_FIRST_CREATURE = 584`
- `G0219_as_Graphic558_CreatureAspects[].FirstNativeBitmapRelativeIndex` is now added to `584`

Previous V1 code used `446` for native creature pose selection and had stale enum/comments around `246`/`439`. That pointed creature native-pose selection at non-creature graphic families.

Updated:

- `m11_creature_sprite_for_pose(...)` native base: `446 -> 584`
- stale creature graphic constants/comments
- asset invariants `INV_GV_102..103`
- native-pose selection invariants `INV_GV_156..159`, `INV_GV_264`, `INV_GV_266`, `INV_GV_268..269`

Examples now verified:

- GiantScorpion front/fallback native: `584 + 0 = 584`
- Trolin side native: `584 + 84 + 1 = 669`
- Trolin attack native: `584 + 84 + 3 = 671`
- PainRat front fallback: `584 + 18 = 602`
- PainRat back native: `584 + 18 + 2 = 604`

## Source anchors

- `DEFS.H M618_GRAPHIC_FIRST_CREATURE = 584`
- `DUNVIEW.C G0219_as_Graphic558_CreatureAspects`
- `DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `373/373 invariants passed`
- CTest: `4/4 PASS`

Relevant output:

```text
PASS INV_GV_102 creature sprite base 584/M618 loads as 112x84
PASS INV_GV_103 creature type 1 sprite 588 loads as 64x66
PASS INV_GV_156 front-facing D1 GiantScorpion view selects native front bitmap (M618+0)
PASS INV_GV_157 side-facing D1 GiantScorpion falls back to front (no SIDE bit, no FLIP_NON_ATTACK)
PASS INV_GV_159 front-facing attack GiantScorpion falls back to front (no ATTACK bit, no FLIP_ATTACK)
PASS INV_GV_264 side-facing D1 Trolin selects native side bitmap (584+84+1) with mirror for relFacing=1
PASS INV_GV_266 front-facing attack D1 Trolin selects native attack bitmap (584+84+3=671)
PASS INV_GV_268 side-facing D1 PainRat falls back to front (584+18) mirrored (FLIP_NON_ATTACK set)
PASS INV_GV_269 back-facing D1 PainRat selects native back bitmap (584+18+2=604)
```

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase38-creature-m618-family-20260425-1558/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase38-creature-m618-family-20260425-1558/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase38-creature-m618-family-20260425-1558/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious palette corruption, UI bleed, or black mask rectangles in the checked frame. This frame does not contain a visible creature, so creature visual confirmation still needs a focused creature scene.

## Remaining work

- Add focused creature scene captures with actual visible creatures.
- Replace derived creature bitmap IDs/scale handling with exact source-derived bitmap cache behavior.
- Pixel-lock creature placement via `C3200_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` zones and `G0223` shift sets.
