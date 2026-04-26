# Pass 125 — DM1 PC 3.4 input/mouse command routing probe

Scope: bounded input parity evidence only. No HUD/UI/inventory/viewport rendering changes.

Source anchors:

- ReDMCSB `COMMAND.C:F0358_COMMAND_GetCommandFromMouseInput_CPSC` scans mouse input rows in order, checks `(ButtonsStatus & Button)`, and matches inclusive source zones.
- `COMMAND.C:F0359_COMMAND_ProcessClick_CPSC` tries primary mouse input first and falls back to secondary when the primary returns `C000_COMMAND_NONE`.
- `G0447_as_Graphic561_PrimaryMouseInput_Interface` maps champion status boxes/name-hands/bar graphs/icon zones to command ids.
- `G0448_as_Graphic561_SecondaryMouseInput_Movement` maps `C007_ZONE_VIEWPORT` left-click to `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` and screen right-click to `C083_COMMAND_TOGGLE_INVENTORY_LEADER`.
- `G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory` maps viewport-relative inventory slot zones `C507..C536` to commands `C028..C057`.
- `F0673_SetMouseInputBoxFromZone` / `F0358` preserve the `CM2_VIEWPORT_RELATIVE` semantics by adding/subtracting `G2067/G2068` viewport origin.

Implementation:

- Added `M11_GameView_GetV1MouseCommandForPoint()` as a bounded source-backed resolver for probe/future routing use.
- Added mouse masks (`0x0001` right, `0x0002` left), coordinate-space enum (`SCREEN`, `VIEWPORT`) and input-list enum (`INTERFACE`, `MOVEMENT`, `INVENTORY`).
- Added `INV_GV_430..437` invariants to `firestaff_m11_game_view_probe` covering:
  - champion status box right-click -> C007 toggle inventory;
  - champion status box left-click -> C012 status-box click;
  - C187 bar graph left-click scans before C151 and returns C007;
  - inclusive source zone edge matching;
  - C007 viewport left-click -> C080;
  - right-click screen zone over viewport -> C083;
  - C507..C536 inventory slot zones -> C028..C057 using viewport-relative coordinate semantics;
  - inventory right-click screen-zone close command C011 precedence over slot hits.

Verification:

```sh
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
cmake --build build --target firestaff_m11_phase_a_probe firestaff_m11_audio_probe capture_ingame_series -- -j2
ctest --test-dir build --output-on-failure
```

Results:

- `firestaff_m11_game_view_probe`: `543/543 invariants passed`.
- `ctest`: `100% tests passed, 0 tests failed out of 5` after building the auxiliary ctest executables.

Remaining blockers / honesty notes:

- The resolver is intentionally bounded to the requested parity rows; it does not yet claim every GRAPHIC561 mouse table row (movement arrows, action/spell panels, panel chest/rename, etc.).
- Runtime `M11_GameView_HandlePointer` still has Firestaff-local routing paths for some clicks; this pass adds source-backed evidence/helper only and avoids broad UI routing changes.
