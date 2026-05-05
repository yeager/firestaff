# Firestaff 0.3.7

Fixes a remaining DM1 Launch freeze/regression.

- The modern launcher now hands off directly to the playable dungeon after pressing Launch.
- The original DM1 TITLE and entrance transitions remain available for explicit probe/debug runs through `FIRESTAFF_PLAY_ORIGINAL_TITLE` and `FIRESTAFF_PLAY_ORIGINAL_ENTRANCE`, but they no longer block the normal Launch path.
- Keeps the v0.3.6 centered Launch hitbox fix and hard launch smoke gate.

Validation:

- `cmake --build build --target firestaff firestaff_m12_startup_menu_probe firestaff_m12_menu_mouse_probe -j4`
- `ctest --test-dir build -R 'm12_menu_mouse|m12_startup_menu|dm1_launcher_click_smoke' --output-on-failure`
- Headless post-launch run: `SDL_VIDEODRIVER=dummy FIRESTAFF_AUTOTEST=1 ./build/firestaff --duration 3000 --width 1920 --height 1080 --data-dir ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1 --script 'click:590:500,click:960:609'`

Install note: copy `Firestaff.app` out of the DMG before launching it. Do not run the app directly from the mounted DMG volume.
