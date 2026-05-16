# Firestaff 0.3.8

Release build from current `main` after the DM1 Launch freeze fix, with the latest verified V2 parity gates included.

Highlights:

- Keeps the DM1 Launch handoff fix from v0.3.7: pressing Launch goes directly into the playable dungeon instead of hanging in TITLE/entrance playback.
- Adds verified DM1 V2 movement/camera source-lock gates.
- Adds verified DM1 V2 viewport material/asset/effect gates.
- Adds DM1 V2 HUD touch interaction bridge and settings persistence gates.
- Makes ReDMCSB/source-lock verification scripts portable between Mac and N2 by removing hardcoded worker paths.

Validation:

- `cmake --build build --target firestaff firestaff_m12_startup_menu_probe firestaff_m12_menu_mouse_probe -j4`
- `ctest --test-dir build -R 'm12_menu_mouse|m12_startup_menu|dm1_launcher_click_smoke|dm1_v2_movement_camera|dm1_v2_viewport_asset|dm1_v2_runtime_shell|dm1_v2_hud_touch|dm1_v2_settings' --output-on-failure`

Install note: copy `Firestaff.app` out of the DMG before launching it. Do not run the app directly from the mounted DMG volume.
