# DM1 all-graphics parity — phase 2977–2996: V1 action-hand / ready-hand separation

## Scope

Narrow the normal V1 HUD action-icon/menu path against DM1 PC 3.4 `F0386_MENUS_DrawActionIcon` and `F0389_MENUS_SetActingChampion`.

## Source evidence

- ReDMCSB / DM1 PC 3.4 `ACTIDRAW.C:F0386_MENUS_DrawActionIcon` reads exactly `Champion.Slots[C01_SLOT_ACTION_HAND]` for action-area icons.
- ReDMCSB / DM1 PC 3.4 `MENU.C:F0389_MENUS_SetActingChampion` uses the same action-hand slot when deciding whether to open the action menu.
- `DEFS.H` distinguishes `C00_SLOT_READY_HAND` from `C01_SLOT_ACTION_HAND`; the ready hand is not a fallback source for action-area icons/actions.

## Firestaff change

- `m11_get_action_hand_thing(...)` no longer falls back from the action hand to the ready/left hand.
- It still honors the optional compat `CHAMPION_SLOT_ACTION_HAND` mirror when populated, then falls back to the current `CHAMPION_SLOT_HAND_RIGHT` action-hand mirror.
- This prevents a ready-hand-only object from changing the action icon/menu to that object's ActionSet; it remains the source empty-hand action set.

## Probe invariant

- Added `INV_GV_316`: a champion with only the ready/left hand populated by a dagger still activates as empty hand (`PUNCH`, `KICK`, `WAR CRY`) rather than dagger actions.

## Gates

- `cmake -S . -B build && cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — PASS
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — PASS (`557/557 invariants`, including `INV_GV_316`)
- `cmake --build build -- -j2 && ctest --test-dir build --output-on-failure` — PASS (`5/5 tests`)
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./run_firestaff_m11_ingame_capture_smoke.sh` — PASS (`6 screenshots`)
- Secret scan of patch diff for credential markers — PASS
