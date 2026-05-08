# Firestaff v0.3.11

DM1 V1 movement, viewport, touch, and runtime-route verification release.

## Highlights

- Closes the live route-to-game-view bridge for DM1 V1 movement input.
- Adds source-locked DM1 V1 touch/live dispatch coverage.
- Adds viewport redraw, wall/occlusion, and draw-order verification gates.
- Narrows and documents remaining original-capture/debugger blockers with evidence.
- Improves launcher/keypad route handling and regression probes.
- Fixes m11 game-view probe invariants for isolated strafe and post-move teleporter/pit transition coverage.

## Verification

Local release gates passed on macOS:

- `cmake --build build-release-push -j2`
- `ctest --test-dir build-release-push --output-on-failure -R 'm12_startup_menu|m11_game_view|m11_viewport_state|m11_turn_viewport_orientation|dm1_v1_hall_walkaround_runtime|dm1_v1_title_launcher|dm1_v1_movement_core|dm1_v1_viewport_wall|dm1_v1_viewport_redraw|touch|pass3(4|5|6|7|8|9|60|61|62)'`
- `git diff --check origin/main..HEAD`
- high-confidence secret scan against `origin/main..HEAD`

## Known blockers

- Original debugger/FIRES true-stop and direct-PTY strict stop evidence remains narrowed but not fully closed.
- GRAPHICS.DAT wall-decode/original-capture parity work remains evidence-gated.
