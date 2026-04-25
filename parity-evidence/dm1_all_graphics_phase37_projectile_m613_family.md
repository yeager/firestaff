# DM1 all-graphics phase 37 — projectile M613 graphic family

Date: 2026-04-25 15:42 Europe/Stockholm
Scope: Firestaff V1 / DM1 projectile viewport graphic family.

## Change

Corrected projectile viewport graphics from the wrong `416..438` family to the ReDMCSB/DM1 PC projectile family:

- `M613_GRAPHIC_FIRST_PROJECTILE = 454`
- projectile viewport family is `454..485`
- source projectile aspects come from `G0210_as_Graphic558_ProjectileAspects`

The old code incorrectly used `416 + (subtype & 0x1f)`, which points into a different graphic family. Runtime projectile subtype mapping now uses source aspect-style mappings:

- fireball → `454 + 28 = 482`
- slime → `454 + 30 = 484`
- lightning bolt → `454 + 9 = 463`
- poison bolt/cloud → `454 + 31 = 485`
- harm non-material/open door/default spell → `454 + 29 = 483`
- kinetic fallback → `454`

Updated:

- projectile range constants/comments
- `m11_draw_projectile_sprite` accepted source range
- projectile sampling from dungeon things/runtime projectiles
- projectile asset invariants `INV_GV_225..228`

## Source anchors

- `DEFS.H M613_GRAPHIC_FIRST_PROJECTILE = 454`
- `DUNVIEW.C G0210_as_Graphic558_ProjectileAspects`
- `DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
- `DUNGEON.C F0142_DUNGEON_GetProjectileAspect`

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
PASS INV_GV_225 projectile sprite (graphic 454/M613) loads as 14x11 from GRAPHICS.DAT
PASS INV_GV_226 all 32 projectile sprites (454-485/M613 family) load from GRAPHICS.DAT
PASS INV_GV_227 projectile sprite (graphic 479) loads as 84x18 from GRAPHICS.DAT
PASS INV_GV_228 projectile sprite (graphic 480) loads as 8x14 from GRAPHICS.DAT
```

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase37-projectile-m613-family-20260425-1542/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase37-projectile-m613-family-20260425-1542/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase37-projectile-m613-family-20260425-1542/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression, no projectile-sized black rectangle/mask corruption, and no UI bleed in the checked frame.

## Remaining work

- Replace approximate projectile placement with exact `C2900_ZONE_ + viewSquareTo*4 + viewCell` zone placement.
- Model projectile aspect rotation/back-graphic/vertical-flip behavior exactly.
- Add focused projectile scene gates for fireball, lightning, poison, default spell, and kinetic throw.
