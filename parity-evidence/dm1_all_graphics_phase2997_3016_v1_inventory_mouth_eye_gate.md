# DM1 V1 inventory mouth/eye route gate

Source-locked the inventory mouth/eye route pair as a small V1 inventory/UI parity gate.

ReDMCSB citations:
- `COMMAND.C:426-427` maps inventory secondary mouse input `C070_COMMAND_CLICK_ON_MOUTH` to `C545_ZONE_MOUTH` and `C071_COMMAND_CLICK_ON_EYE` to `C546_ZONE_EYE`.
- `COMMAND.C:2314-2320` dispatches `C070` to `F0349_INVENTORY_ProcessCommand70_ClickOnMouth` and `C071` to `F0352_INVENTORY_ProcessCommand71_ClickOnEye`.
- `PANEL.C:1788-1817` gates empty-hand mouth press, sets `G0333_B_PressingMouth`, redraws food/water/poisoned panel, and redraws the viewport.
- `PANEL.C:2123-2159` sets `G0331_B_PressingEye`, draws the looking-eye icon in `C546_ZONE_EYE`, then routes to skills/statistics when leader hand is empty or leader-hand object description otherwise.
- `CHAMDRAW.C:1060-1078` prioritizes active mouth/eye panel redraws before normal `F0347_INVENTORY_DrawPanel` refresh.

Firestaff files:
- `inventory_mouth_eye_routes_pc34_compat.{c,h}` now expose concrete route rows plus mouth/eye panel result evaluation.
- `test_inventory_mouth_eye_routes_pc34_compat_integration.c` verifies command ids, zone ids, handler names, invalid ordinals, and panel result branches.
- `CMakeLists.txt` registers `inventory_mouth_eye_routes_pc34_compat` as a CTest gate.

Verification commands:
- `cmake --build build --target test_inventory_mouth_eye_routes_pc34_compat_integration firestaff_m11_game_view_probe`
- `./build/test_inventory_mouth_eye_routes_pc34_compat_integration`
- `ctest --test-dir build -R "inventory_mouth_eye_routes_pc34_compat|m11_game_view" --output-on-failure`
