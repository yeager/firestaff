# Firestaff 0.3.6

Fixes the DM1 options-screen Launch button.

- Aligns the modern launcher mouse hitbox with the visible centered Launch button.
- Updates the DM1 launcher smoke to click the visible Launch button, not the stale right-aligned hitbox.
- Makes the launcher smoke fail if no launch handoff is reached, so this regression cannot pass silently again.
- Adds the M12 mouse probe to CTest and refreshes its modern five-card layout expectations.

Validation:

- `cmake --build build --target firestaff firestaff_m12_startup_menu_probe firestaff_m12_menu_mouse_probe -j4`
- `ctest --test-dir build -R 'm12_menu_mouse|m12_startup_menu|dm1_launcher_click_smoke' --output-on-failure`
- Manual negative check confirmed the old stale Launch coordinate no longer counts as the visible button path.

If macOS blocks the app because it is unsigned, right-click the app and choose Open.
