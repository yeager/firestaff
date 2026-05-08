# Pass378 — DM1 V1 original route semantic-clean blocker

Status: `BLOCKED_PASS378_ORIGINAL_ROUTE_NOT_SEMANTICALLY_CLEAN`

## Verdict

The current pass376 original raw route and the bounded source-portrait retry do not yield six semantically clean, distinct party/HUD/viewport states. The blocker is original-side route semantics, not Firestaff pairing mechanics.

## Current pass376 classifier

- pass: `False`
- class counts: `{'dungeon_gameplay': 4, 'wall_closeup': 2}`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 4, 'fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde': 2}`

## Source-portrait retry

- artifact dir: `verification-screens/pass378-source-portrait-sixshot-retry`
- pass80 pass: `False`
- class counts: `{'dungeon_gameplay': 3, 'entrance_menu': 2, 'title_or_menu': 1}`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 3}`

Problems:
- image0001-raw.png: classified title_or_menu, expected dungeon_gameplay
- image0002-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0003-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0004-raw.png: classified dungeon_gameplay, expected spell_panel
- image0006-raw.png: classified dungeon_gameplay, expected inventory
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

## Failing commands preserved

```bash
OUT=$PWD/verification-screens/pass378-source-portrait-sixshot-retry; rm -rf "$OUT"; OUT_DIR="$OUT" DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:2500 shot:party_hud click:111,82 wait:1200 shot:portrait_candidate click:130,115 wait:1200 shot:resurrect_choice enter wait:1500 shot:after_confirm f1 wait:1200 shot:spell_panel f4 wait:1200 shot:inventory_panel" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
python3 tools/pass80_original_frame_classifier.py verification-screens/pass378-source-portrait-sixshot-retry --expected pass77 --fail-on-duplicates
python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass378-source-portrait-sixshot-retry --out-dir verification-screens/pass378-source-portrait-sixshot-viewports --dry-run
```

## Next concrete command

```bash
python3 tools/pass162_original_party_route_unblock.py && python3 tools/pass166_source_portrait_click_route_probe.py && python3 tools/verify_pass378_dm1_v1_original_route_semantic_clean_blocker.py
```

Manifest: `parity-evidence/verification/pass378_dm1_v1_original_route_semantic_clean_blocker/manifest.json`
