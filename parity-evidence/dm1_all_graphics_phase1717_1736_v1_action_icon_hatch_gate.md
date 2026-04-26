# DM1 all-graphics parity — phase 1717–1736: V1 action-icon hatch gate

## Scope

Make the global disabled-action hatch condition for V1 action-hand icon cells explicit and probe-visible.

## Source anchors

- `MENUS.C F0386_MENUS_DrawActionIcon` hatches the action icon cell after drawing when actions are globally disabled.
- ReDMCSB source conditions represented in M11 state:
  - champion candidate selection active (`G0299_ui_CandidateChampionOrdinal`)
  - party resting (`G0300_B_PartyIsResting`)
- Firestaff also has a visible candidate mirror panel state for the same selection lockout.

## Implemented

- Added `M11_GameView_ShouldHatchV1ActionIconCells(...)`.
- Routed `m11_draw_dm_action_icon_cells(...)` through the helper instead of keeping the gate inline.
- Added invariant coverage for idle false, resting true, candidate-panel true, and candidate-ordinal true.
- The hatch drawing pattern and cell geometry remain unchanged.

## New invariant

- `INV_GV_300M`: V1 action icon hatch gate follows resting/candidate global source disable states.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300M V1 action icon hatch gate follows resting/candidate global source disable states
```
