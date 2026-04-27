# Pass 94 — DM1 Hall map route assumptions for original capture

Date: 2026-04-27

## Scope

This note uses the OldGames Hall of Champions map only as a route-planning reference. It is **not** runtime evidence, not overlay evidence, and not a parity claim. The next useful work is still an original DOSBox capture/classifier run that proves the raw frames are `320x200` gameplay and that the requested semantic checkpoints are real.

Map references:

- `references/firestaff/dm1/hall-of-champions-map.md`
- `references/firestaff/dm1/hall-of-champions-map-email.png`
- OldGames map URL: `https://www.oldgames.sk/sk/dungeon-mapper/map/1685`

## Map-derived facts relevant to champion routing

From the map/page, the Hall of Champions is a `20 x 21` level. The entrance marker is at the south-east side, near coordinate `[17,2]`, with the arrow pointing north into the level. The map lists champion alcoves by coordinate, including:

- `[11,8]` — `HALK THE BARBARIAN`, Journeyman Fighter, `Mana: 0`.
- `[10,7]` — `ELIJA LION OF YAITOPYA`, Novice Fighter / Apprentice Priest, `Mana: 22`.
- `[8,6]` — `ZED DUKE BANVILLE`, all four classes novice, `Mana: 10`.
- `[13,13]` — `DAROOU`, Apprentice Fighter / Neophyte Wizard, `Mana: 6`.
- `[4,2]` — `CHANI SAYYADINA SIHAYA`, Novice Fighter / Apprentice Wizard, `Mana: 20`.

For the current six-shot overlay route, a champion with mana is more useful than Halk because the route includes the spell panel and a post-cast checkpoint. Map-derived preference order for the next original-route test:

1. `ELIJA` at `[10,7]` — close to the south/central hall and has enough mana for a visible spell-panel/cast test.
2. `ZED` at `[8,6]` — also close and has mana, but slightly farther west.
3. `DAROOU` at `[13,13]` — has mana, but is farther north-east and less useful as a first unblock target.

## Important blocker narrowed by the map

Pass 93 shows the automation can reach an `entrance_menu` frame but does not yet start in a champion-party state. The map suggests the next route attempt should stop treating `enter`, `space`, or `f1` as champion-party selectors at the entrance menu. Instead, first click the visible `ENTER` menu item in the original frame, then navigate from the map entrance toward a mana-bearing champion alcove.

The map cannot prove the input sequence. It only supplies expected waypoints and champion targets.

## Exact next capture sequence to try

Use this as a diagnostic original capture, not as parity evidence. The capture script can print this command and its exact audit expectations with:

```sh
scripts/dosbox_dm1_original_viewport_reference_capture.sh --print-pass94-diagnostic
```

It is designed to answer two questions in one lightweight run:

1. Does `click:260,50` activate the `ENTER` menu item and leave the entrance-menu screen?
2. Do the first movement clicks produce gameplay frames consistent with leaving `[17,2]` northbound toward the Hall route?

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

Token intent:

- `click:260,50` targets the visible `ENTER` control in the original entrance-menu frame seen in prior pass87/pass93 captures.
- `click:276,140` targets the forward arrow in the original movement pad.
- `click:246,140` targets the left-turn arrow in the original movement pad.

Expected audit result for this diagnostic is **not** a valid overlay route yet. The useful result is whether shot 3 leaves `entrance_menu` and whether shots 4-6 show navigable Hall-of-Champions gameplay rather than title/menu/wall-closeup false positives. If the entrance click works, `tools/pass80_original_frame_classifier.py --expected pass94-diagnostic` should expect:

```text
title_or_menu, entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay
```

## If the diagnostic confirms entrance click works

Then run the actual champion-route attempt with the same `click:260,50` entrance activation and use the map to steer west/north from `[17,2]` toward `ELIJA [10,7]` or `ZED [8,6]`. Keep the six required semantic labels only after a champion is selected:

```text
shot:party_hud
shot
shot
shot:spell_panel
shot
shot:inventory_panel
```

Do not promote any resulting images until `tools/pass80_original_frame_classifier.py` accepts the raw frames as real gameplay states and `original_viewport_shot_labels.tsv` records the expected semantic checkpoints.
