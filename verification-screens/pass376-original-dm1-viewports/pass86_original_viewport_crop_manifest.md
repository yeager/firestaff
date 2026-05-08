# Pass 86 — original DM1 viewport crop manifest

This pass converts a semantically checked original-runtime screenshot route into pass-70-compatible viewport crops.

- raw dir: `verification-screens/pass376-original-route`
- output dir: `verification-screens/pass376-original-dm1-viewports`
- manifest: `verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv`
- viewport crop: `[0, 33, 224, 136]` (x, y, width, height)
- pass: `False`
- honesty: original-reference input preparation only; no pixel parity is claimed.

## Problems

- image0002-raw.png: classified wall_closeup, expected dungeon_gameplay
- image0004-raw.png: classified dungeon_gameplay, expected spell_panel
- image0005-raw.png: classified wall_closeup, expected dungeon_gameplay
- image0006-raw.png: classified dungeon_gameplay, expected inventory

## Frames

| scene | raw capture | classified | expected | output PNG | output PPM | sha256 |
|-------|-------------|------------|----------|------------|------------|--------|
| `ingame_start` | `verification-screens/pass376-original-route/image0001-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.ppm` | `701689e73fc0` |
| `ingame_turn_right` | `verification-screens/pass376-original-route/image0002-raw.png` | `wall_closeup` | `dungeon_gameplay` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/02_ingame_turn_right_original_viewport_224x136.png` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/02_ingame_turn_right_original_viewport_224x136.ppm` | `1e71ed879980` |
| `ingame_move_forward` | `verification-screens/pass376-original-route/image0003-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/03_ingame_move_forward_original_viewport_224x136.png` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/03_ingame_move_forward_original_viewport_224x136.ppm` | `701689e73fc0` |
| `ingame_spell_panel` | `verification-screens/pass376-original-route/image0004-raw.png` | `dungeon_gameplay` | `spell_panel` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/04_ingame_spell_panel_original_viewport_224x136.png` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/04_ingame_spell_panel_original_viewport_224x136.ppm` | `701689e73fc0` |
| `ingame_after_cast` | `verification-screens/pass376-original-route/image0005-raw.png` | `wall_closeup` | `dungeon_gameplay` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/05_ingame_after_cast_original_viewport_224x136.png` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/05_ingame_after_cast_original_viewport_224x136.ppm` | `1e71ed879980` |
| `ingame_inventory_panel` | `verification-screens/pass376-original-route/image0006-raw.png` | `dungeon_gameplay` | `inventory` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/06_ingame_inventory_panel_original_viewport_224x136.png` | `verification-screens/pass376-original-dm1-viewports/viewport_224x136/06_ingame_inventory_panel_original_viewport_224x136.ppm` | `701689e73fc0` |
