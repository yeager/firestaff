# Pass 87 — original route click support and capture audit

Date: 2026-04-27

## Scope

This is original-overlay/capture tooling and evidence only. It does not change HUD, viewport/wall/item rendering, DM1 V1 gameplay code, or V2 assets, and it does **not** claim original-vs-Firestaff pixel parity.

## What changed

- `scripts/dosbox_dm1_original_viewport_reference_capture.sh` now accepts `click:<x>,<y>` route tokens in original DM1 `320x200` coordinates.
- The generated Swift helper maps those coordinates into the current DOSBox window using a centered 320:200 aspect-fit rectangle and posts serialized left-click events to the DOSBox PID.
- Click tokens are range-checked (`0 <= x < 320`, `0 <= y < 200`) and are logged with the mapped screen coordinate in `original-viewpoint-route-keys.log`.
- `tools/pass84_original_overlay_readiness_probe.py` now reports that serialized original-frame clicks are supported by the route script.

This unblocks testing source-backed mouse regions such as ReDMCSB/Item 561 movement/spell coordinates (`movement 234..318 x 125..167`, `spell 233..319 x 42..73`, runes/cast `235..318 x 51..73`) without manually editing the generated Swift helper.

## Capture attempt

Command:

```sh
OUT_DIR=$PWD/verification-screens/pass87-subagent-click-route \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=4000 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:5000 shot:party_hud kp7 wait:400 shot:turn_right kp8 wait:400 shot:move_forward click:228,94 wait:600 shot:spell_panel click:233,108 wait:300 click:247,108 wait:300 click:233,94 wait:600 shot:after_cast f1 wait:500 shot:inventory_panel' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Result: the route completed and normalized six raw screenshots/crops under `verification-screens/pass87-subagent-click-route/`.

The key log proves the click tokens were posted and mapped, for example:

```text
route-token click:228,94
click-mapped 228,94 -> 1187,507 window=1067x828
route-token click:233,108
click-mapped 233,108 -> 1204,554 window=1067x828
route-token click:247,108
click-mapped 247,108 -> 1251,554 window=1067x828
route-token click:233,94
click-mapped 233,94 -> 1204,507 window=1067x828
```

## Classifier audit

Command:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass87-subagent-click-route \
  --expected 'dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory' \
  --fail-on-duplicates \
  --out-json parity-evidence/overlays/pass87/pass87_click_route_classifier.json \
  --out-md parity-evidence/overlays/pass87/pass87_click_route_classifier.md
```

Result: **negative semantic evidence**, not parity evidence.

```text
class_counts={graphics_320x200_unclassified: 2, title_or_menu: 4}
problems=7
```

The route is still not semantically locked: later frames remain title/menu-like and four frames share the same SHA256. The useful outcome is that the original capture path can now express and post mouse-click route steps; the next worker should adjust the original startup/enter route rather than patch the helper by hand.

## Follow-up: Enter-key route reaches dungeon but still not semantic parity

A second route used the keyboard `enter` at the title/entrance screen before the same six-shot audit:

```sh
OUT_DIR=$PWD/verification-screens/pass89-subagent-enter-key-route \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=5000 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title_screen enter wait:1500 shot:after_enter kp7 wait:500 shot:turn_right kp8 wait:500 shot:move_forward click:228,94 wait:700 shot:spell_panel f1 wait:700 shot:inventory_panel' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Classifier command:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass89-subagent-enter-key-route \
  --expected 'title_or_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,inventory' \
  --fail-on-duplicates \
  --out-json parity-evidence/overlays/pass89/pass89_enter_key_route_classifier.json \
  --out-md parity-evidence/overlays/pass89/pass89_enter_key_route_classifier.md
```

Result: still **negative semantic evidence**. The route reaches an empty/new-game dungeon view, but it does not reach the intended champion/spell/inventory states. The final `f1` checkpoint is correctly guarded as `wall_closeup`, not inventory.

```text
class_counts={graphics_320x200_unclassified: 5, wall_closeup: 1}
problems=7
```

This is useful because it separates two blockers:

1. the capture helper can now send keyboard, screenshot, and mouse-click tokens and produce six normalized 320x200 captures;
2. the original-game route still needs a validated champion-party save/startup path before HUD/inventory/spell overlay comparison is honest.

## Readiness probe

Command:

```sh
python3 tools/pass84_original_overlay_readiness_probe.py > parity-evidence/pass87_original_overlay_click_readiness_probe.json
```

Result: `ready_for_overlay_comparison=false`, with `serialized_original_frame_clicks_supported_by_route_script=true` and the existing missing-input/route blockers still present.
