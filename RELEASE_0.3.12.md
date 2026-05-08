# Firestaff v0.3.12

Release workflow follow-up for the DM1 V1 movement, viewport, touch, and runtime-route verification release.

## Highlights

- Carries the v0.3.11 DM1 V1 movement/viewport/touch verification changes.
- Fixes release workflow tag resolution so Windows and Linux package jobs use the tag-specific release notes file instead of the stale preview fallback.

## Verification

Local release gates passed on macOS before tagging:

- `cmake --build build-release-push -j2`
- `ctest --test-dir build-release-push --output-on-failure -R 'm12_startup_menu|m11_game_view|m11_viewport_state|m11_turn_viewport_orientation|dm1_v1_hall_walkaround_runtime|dm1_v1_title_launcher|dm1_v1_movement_core|dm1_v1_viewport_wall|dm1_v1_viewport_redraw|touch|pass3(4|5|6|7|8|9|60|61|62)'`
- `git diff --check origin/main..HEAD`
- high-confidence secret scan against `origin/main..HEAD`
- local macOS DMG creation and `hdiutil verify`
