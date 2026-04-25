# DM1 all-graphics parity — phase 40: creature source aspects + focused visible gate

Date: 2026-04-25 16:45 Europe/Stockholm

## Goal

Close the focused creature visibility gap left after the M618 family correction. The previous M618 pass proved the base family was `M618_GRAPHIC_FIRST_CREATURE = 584`, but the focused Trolin viewport scene was still visually identical to the empty corridor because normal V1 no longer drew the contents layer for open center cells.

This pass adds a narrow normal-path center-lane contents draw so source-backed items/creatures/projectiles can be visually gated, and corrects the creature aspect table against ReDMCSB `G0219_as_Graphic558_CreatureAspects`.

## Source anchors

- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `M618_GRAPHIC_FIRST_CREATURE = 584`
  - `M072_TRANSPARENT_COLOR`
- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0219_as_Graphic558_CreatureAspects`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - source object/creature/projectile contents pass uses viewport zones from the C2500/C3200 families

## Implemented

- `m11_game_view.c`
  - Added a center-lane normal V1 contents pass over open center cells after source wall/door/floor layer draws.
  - Scope is deliberately narrow: center cells only, existing asset-backed contents layer only, no procedural wall geometry re-enabled.
  - Replaced stale synthetic creature aspect entries with source-derived `G0219_as_Graphic558_CreatureAspects` values for all 27 DM1 creature types.
  - This fixed both native graphic offsets and transparent color keys. Example: Trolin now uses `firstNative=43`, transparent color `4`, and replacement colors `0x30`; D1 side = `584 + 43 + 1 = 628`, D1 attack = `584 + 43 + 3 = 630`.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_38L` focused D1C Trolin scene gate.
  - Updated creature sprite selection invariants to the source aspect offsets:
    - `INV_GV_264`: Trolin D1 side bitmap `628` with mirror for relFacing=1.
    - `INV_GV_266`: Trolin D1 attack bitmap `630`.
    - `INV_GV_268`: PainRat D1 front fallback `594` with mirror.
    - `INV_GV_269`: PainRat D1 back bitmap `596`.
  - Updated transparent-color/coord-set spot checks from source `G0219`:
    - `INV_GV_250`: GiantScorpion coordSet=1 transparent=13.
    - `INV_GV_251`: Rockpile coordSet=0 transparent=13.
    - `INV_GV_252`: RedDragon coordSet=1 transparent=4.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_38L focused viewport: D1C Trolin creature sprite changes the corridor frame`
- `PASS INV_GV_264 side-facing D1 Trolin selects native side bitmap (584+43+1) with mirror for relFacing=1`
- `PASS INV_GV_250 creature type 0 (GiantScorpion) coordSet=1 transparent=13`
- Probe summary: `374/374 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase40-focused-creature-source-aspects-20260425-1645/normal/35_focused_d1c_trolin_creature_vga.png`
- `verification-m11/dm1-all-graphics/phase40-focused-creature-source-aspects-20260425-1645/normal/35_focused_d1c_trolin_creature_vga_viewport_only.png`

Visual inspection result:

- Creature visible and centered.
- No obvious sprite corruption.
- Palette looks coherent.
- No cyan/black mask rectangle after correcting Trolin transparent color from stale `10` to source `4`.
- No UI bleed into the creature area.

## Remaining work

This is still an intermediate gate, not the final object/creature/projectile renderer:

- Bind exact object/creature/projectile placement to source `C2500_ZONE_` / `C3200_ZONE_` tables, including side cells and shift masks.
- Verify `G0223` creature/object coordinate scaling and multi-creature layouts against source, beyond the existing focused front-slot checks.
- Add focused visual gates for side-cell creatures and projectile sub-cell placement.
- Keep the center-lane contents pass until the full source-zone pass replaces it.
