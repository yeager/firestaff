# Pass 126 HUD consolidated — V1 status shield border stack

## Scope

DM1/V1 champion status-box HUD parity: replace the previous single “highest active shield wins” overlay with the source-backed multi-overlay draw stack for the party-wide status shield borders.

## Source anchors

- `firestaff_pc34_core_amalgam.c` / ReDMCSB `CHAMDRAW.C F0292_CHAMPION_DrawState`:
  - Appends active borders in this order:
    - `C038_GRAPHIC_BORDER_PARTY_FIRESHIELD` when `G0407_s_Party.FireShieldDefense > 0`
    - `C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD` when `G0407_s_Party.SpellShieldDefense > 0`
    - `C037_GRAPHIC_BORDER_PARTY_SHIELD` when `G0407_s_Party.ShieldDefense > 0` or champion `ShieldDefense` is active
  - Then draws them with `while (BorderCount--)`, so draw order is reversed: `C037` party shield first, `C039` spell shield next, `C038` fire shield last/topmost.
- Existing asset IDs remain the original status-box border set `/C008/C033-C035/C037-C039`:
  - `C037` party shield border
  - `C038` fire shield border
  - `C039` spell shield border
- The required source-reference path remains preserved for broader ReDMCSB context: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/dmweb.free.fr_Stuff_ReDMCSB_Documentation_BugsAndChanges.htm.html`.

## Implemented

- Added `M11_GameView_GetV1StatusShieldBorderGraphicCountForChampion(...)` and `M11_GameView_GetV1StatusShieldBorderGraphicForChampionAt(...)` so the renderer can expose and draw the full source stack instead of one selected overlay.
- Kept `M11_GameView_GetV1StatusShieldBorderGraphic(...)` as a compatibility helper returning the topmost active border.
- Updated the V1 status-box draw path in `m11_game_view.c` to blit every active party-wide shield border in source reverse draw order.
- Added probe coverage for:
  - no shield / party shield / fire-over-party top border behavior;
  - all three party-wide shields drawing as `C037`, `C039`, `C038`.

## Deliberate remaining gap

`F0292` also includes per-champion `ShieldDefense` in the `C037` condition, but current `M11_GameViewState`/`ChampionState_Compat` used by M11 does not expose that field. Pass126 adopts the pass123 worker finding and implements the complete party-wide stack and documents the champion-local bridge as a state-carry gap rather than inventing a renderer-only field.

## New/updated invariants

- `INV_GV_15P`: V1 status shield top border follows F0292 reverse draw stack (fire topmost).
- `INV_GV_15P2`: V1 status shield border stack draws party, spell, then fire like F0292 while-count order.

## Verification

Commands run on N2 worktree `/home/trv2/.openclaw/data/firestaff-worktrees/pass126-hud-auto-1777392327`:

```text
cmake -S . -B build
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
cmake --build build -- -j2
ctest --test-dir build --output-on-failure
```

Results:

```text
# summary: 581/581 invariants passed
100% tests passed, 0 tests failed out of 7
```
