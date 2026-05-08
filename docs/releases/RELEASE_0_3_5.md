# Firestaff 0.3.5

Launcher smoke/release hardening for the DM1 start-menu path.

- Adds a headless DM1 launcher click smoke that opens DM1, presses Launch, and fails on the immediate Launch-click hang/regression.
- Keeps game-options presentation mode, persisted settings, and launch handoff state synchronized so Launch uses the selected mode consistently.
- Refreshes the M12 startup-menu probe for the presentation-mode row and keeps the 52-invariant launcher gate green.

Validation:

- `cmake --build build --parallel 4`
- `ctest --test-dir build -R 'dm1_launcher_click_smoke|m12_startup_menu|m11_phase_a|m11_audio|dm1_v2_upscale_dry_run_validator|dm1_v2_movement_viewport_pc34|dm1_v2_runtime_shell_pc34|dm1_v2_camera_controller_pc34|dm1_v2_movement_command_adapter_pc34' --output-on-failure`
- Local full `ctest` still has environment-bound ReDMCSB/original-data gates that require the N2 `<firestaff-data>/...` paths.

If macOS blocks the app because it is unsigned, right-click the app and choose Open.
