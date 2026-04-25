# DM1 all-graphics phase 39 — object M612 graphic family

Date: 2026-04-25 16:12 Europe/Stockholm
Scope: Firestaff V1 / DM1 floor/inventory object viewport sprite family.

## Change

Corrected object/item viewport sprite selection from old placeholder category bases (`267`, `313`, `344`, etc.) to the ReDMCSB/DM1 PC object graphic family:

- `M612_GRAPHIC_FIRST_OBJECT = 498`
- object info comes from `G0237_as_Graphic559_ObjectInfo`
- object aspect comes from `G0209_as_Graphic558_ObjectAspects`
- final native graphic is `498 + FirstNativeBitmapRelativeIndex`

Implemented source-backed mapping for the V1 item categories:

- scroll → object info index `0`
- container → `1 + subtype`
- potion → `2 + subtype`
- weapon → `23 + subtype`
- armour → `69 + subtype`
- junk → `127 + subtype`

Also corrected object sprite transparency to source behavior:

- object blit transparent color: `C10_COLOR_FLESH` (`10`)
- previous `0` transparency produced/encouraged bad mask behavior with the M612 family

## Source anchors

- `DEFS.H M612_GRAPHIC_FIRST_OBJECT = 498`
- `DUNGEON.C G0237_as_Graphic559_ObjectInfo`
- `DUNGEON.C F0141_DUNGEON_GetObjectInfoIndex`
- `DUNVIEW.C G0209_as_Graphic558_ObjectAspects`
- `DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`

## Updated invariants

- `INV_GV_109` — M612 potion aspect graphic `566` loads
- `INV_GV_112` — end of M612 family graphic `583` loads
- `INV_GV_113` — scroll aspect graphic `500` loads
- `INV_GV_114` — wall ornament check now points at corrected `M615=259`

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
PASS INV_GV_109 object sprite graphic 566 (M612 potion aspect) loads from GRAPHICS.DAT
PASS INV_GV_112 object sprite graphic 583 (end of M612 family) loads from GRAPHICS.DAT
PASS INV_GV_113 object sprite graphic 500 (scroll aspect) loads from GRAPHICS.DAT
PASS INV_GV_114 wall ornament graphic 259/M615 is loadable from GRAPHICS.DAT
```

## Artifacts

Final corrected-VGA screenshot set after transparency fix:

- `verification-m11/dm1-all-graphics/phase39-object-m612-family-20260425-1612b/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase39-object-m612-family-20260425-1612b/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase39-object-m612-family-20260425-1612b/normal/party_hud_top_190_crop_vga.png`

Visual inspection after transparency fix: no obvious object sprite corruption, no palette corruption, no UI bleed, and no object-attached black mask rectangles. The remaining large dark viewport regions appear to be dungeon darkness/void geometry rather than object sprite masks.

## Remaining work

- Replace approximate floor-object placement/scaling with exact source zones:
  - `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`
  - object pile shifts from `G0217` and `G0223`
  - exact object scales from `G2030_auc_ObjectScales`
- Support alcove object image (`MASK0x0010_ALCOVE`) and flip-on-right (`MASK0x0001_FLIP_ON_RIGHT`) exactly.
