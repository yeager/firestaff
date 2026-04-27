# Pass 94 follow-up — original route diagnostic operator checklist

Date: 2026-04-27

Scope: manual/original DOSBox route verification only. This checklist turns the pass94 Hall-map diagnostic into a reproducible operator check for Daniel. It is **not** overlay readiness evidence, not promoted reference evidence, and not a parity claim.

## Exact command

From the repo root, first print the built-in command and audit expectations:

```sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh --print-pass94-diagnostic
```

Then run the diagnostic exactly as printed:

```sh
OUT_DIR=$PWD/verification-screens/pass94-hall-map-enter-diagnostic \
DM1_ORIGINAL_STAGE_DIR=$PWD/verification-screens/dm1-dosbox-capture/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:1200 shot:pre_enter_menu click:260,50 wait:1200 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Classify the captured raw screenshots:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass94-hall-map-enter-diagnostic \
  --expected pass94-diagnostic \
  --fail-on-duplicates
```

## Screenshot labels that must appear

After the run, `verification-screens/pass94-hall-map-enter-diagnostic/original_viewport_shot_labels.tsv` must contain these six route-token labels, in order:

| # | route-token label | operator meaning |
|---|---|---|
| 01 | `title` | startup/title/menu frame before selecting ENTER |
| 02 | `pre_enter_menu` | entrance menu visible before the mouse click |
| 03 | `after_enter_click` | frame immediately after `click:260,50` |
| 04 | `forward_1` | first movement-pad forward click result |
| 05 | `forward_2` | second movement-pad forward click result |
| 06 | `left_turn_probe` | left-turn click result |

These labels are only the names attached to `shot:<label>` capture tokens. They do **not** prove that the visual state matches the label.

## Classifier labels that mean PASS

The classifier must report exactly this sequence for `--expected pass94-diagnostic`:

```text
title_or_menu, entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay
```

PASS means only this narrow diagnostic succeeded:

- `click:260,50` leaves the entrance menu by shot 03.
- shots 03-06 are raw `320x200` dungeon gameplay frames.
- the movement probe is plausible enough to use as the next route-debugging starting point toward a mana-bearing champion such as `ELIJA [10,7]` or `ZED [8,6]` from `parity-evidence/pass94_dm1_hall_map_route_assumptions.md`.

PASS does **not** mean the original overlay route is ready.

## Outcomes that mean BLOCKED

Preserve the run as blocker evidence and do not promote captures if any of these happen:

- `after_enter_click` classifies as `entrance_menu`: `click:260,50` did not activate ENTER.
- any shot classifies as `non_graphics_blocker`: raw capture is not valid `320x200` DM1 graphics.
- shots 04-06 classify as `wall_closeup`, `title_or_menu`, or `graphics_320x200_unclassified`: the movement probe is not usable Hall gameplay evidence.
- `--fail-on-duplicates` reports duplicate frames: input timing/state is not trustworthy enough for route evidence.
- `original_viewport_shot_labels.tsv` is missing, malformed, or not in the six-label order above.

## Critical warning: diagnostic labels are not overlay readiness labels

Do not confuse pass94 diagnostic labels with the six semantic overlay-readiness labels.

Diagnostic labels such as `title`, `pre_enter_menu`, `after_enter_click`, `forward_1`, `forward_2`, and `left_turn_probe` are route-debug breadcrumbs. They are allowed to include title/menu/entrance states and cannot be used for original-vs-Firestaff overlay comparison.

The overlay route remains blocked until a separate champion-party capture records and verifies the real semantic checkpoints:

```text
shot:party_hud
shot
shot
shot:spell_panel
shot
shot:inventory_panel
```

Only after those frames classify as the expected gameplay/spell/inventory states should any original screenshots be considered for promotion or parity comparison.
