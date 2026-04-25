# DM1 all-graphics phase 5 — action/PASS area cleanup

Date: 2026-04-25 10:35 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 action/PASS and spell-strip area.

## Change

Normal V1 idle action area no longer draws invented Firestaff utility readouts over the original DM1 action/spell strip.

Removed from normal V1 when the authentic `GRAPHICS.DAT` action/spell frames are active:

- active champion name in the idle action-area header
- `HP... ST...` utility text
- invented light-level bar

Those remain available only in debug/procedural fallback paths. In source-faithful V1:

- idle mode is owned by F0386-style action-hand icon cells
- action-menu mode is owned by F0387-style `graphic 10` menu rendering and still draws the acting champion name/action rows after a fresh action-area blit

## Source/evidence anchors

- `GRAPHICS.DAT` graphic `0010` / `C010_GRAPHIC_MENU_ACTION_AREA`: `87x45`
- `GRAPHICS.DAT` graphic `0009` / `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND`: `87x25`
- zone constants from `DEFS.H`:
  - `C011_ZONE_ACTION_AREA`
  - `C013_ZONE_SPELL_AREA`
  - `C075_ZONE_ACTION_RESULT`
  - `C077_ZONE_ACTION_AREA_TWO_ACTIONS_MENU`
  - `C079_ZONE_ACTION_AREA_ONE_ACTION_MENU`
  - `C082..C084_ZONE_ACTION_AREA_ACTION_*`
  - `C089..C092_ZONE_ACTION_AREA_CHAMPION_*_ACTION`
  - `C093_ZONE_ACTION_AREA_CHAMPION_0_ACTION_ICON`
  - `C098_ZONE_ACTION_AREA_PASS`
- recovered layout table provenance: `zones_h_reconstruction.json`, from `GRAPHICS.DAT` entry `696`, SHA-256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`

## Artifacts

Generated normal V1 screenshot set:

- `verification-m11/dm1-all-graphics/phase5-action-area-20260425-1035/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase5-action-area-20260425-1035/normal/*.png`

Action/spell crop PNGs for quick review:

- `verification-m11/dm1-all-graphics/phase5-action-area-20260425-1035/normal/party_hud_statusbox_gfx_action_spell_crop.png`
- `verification-m11/dm1-all-graphics/phase5-action-area-20260425-1035/normal/19_dm_action_menu_mode_action_spell_crop.png`
- `verification-m11/dm1-all-graphics/phase5-action-area-20260425-1035/normal/20_dm_action_menu_post_click_action_spell_crop.png`

Visual inspection of the idle crop: no obvious prototype `HP/ST`, light bar, or champion-name overlay remains; PASS/action-area rows do not appear sheared/distorted.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Remaining work

This phase does not complete full action-area parity. Still needed:

- exact crop/pixel comparison of `C010`/`C009` against original capture/Greatstone/SCK reference
- exact zone-driven action result `C075` and menu-row hit-test evidence
- source-bound spell/rune and champion/status passes
