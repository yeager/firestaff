# Pass 118 — state-aware original route driver

- date: 2026-04-28T16:00+02:00
- host: N2 / firestaff-worker via `N2`
- branch: `sync/n2-dm1-v1-20260428`
- run base: `<N2_RUNS>/20260428-1600-pass118-state-aware-route-driver`
- attempt: `<N2_RUNS>/20260428-1600-pass118-state-aware-route-driver/state_aware_driver`
- scope: one consolidated sequential pass to replace fixed-sleep route spraying with a frame-classified entrance-menu disappearance gate before movement/control probes.

## Commands

```bash
RUN=<N2_RUNS>/20260428-1600-pass118-state-aware-route-driver
rm -rf "$RUN/state_aware_driver" && mkdir -p "$RUN/state_aware_driver"
xvfb-run -a python3 tools/pass118_state_aware_original_route_driver.py --out "$RUN/state_aware_driver" > "$RUN/state_aware_driver.run.log" 2>&1
python3 tools/pass80_original_frame_classifier.py "$RUN/state_aware_driver" --expected pass77 > "$RUN/pass80.log" 2>&1
python3 tools/pass113_original_party_state_probe.py "$RUN/state_aware_driver" --out-json "$RUN/state_aware_driver/pass113_original_party_state_probe.json" --out-md "$RUN/state_aware_driver/pass113_original_party_state_probe.md" > "$RUN/pass113.rerun.log" 2>&1
```

## State-aware gate

The pass118 driver starts original PC DM1 (`DM -vv -sn -pk`), sends initial `Return`, then captures/classifies gate frames with pass80 heuristics. It does not issue movement/control probes until it observes `dungeon_gameplay` after the entrance menu.

| # | class | sha | note |
|---|-------|-----|------|
| 01 | `entrance_menu` | `c6f457763e2e` | `dungeon entrance/menu controls still occupy the right column` |
| 02 | `entrance_menu` | `c6f457763e2e` | `dungeon entrance/menu controls still occupy the right column` |
| 03 | `entrance_menu` | `5e71f65e56d9` | `dungeon entrance/menu controls still occupy the right column` |
| 04 | `title_or_menu` | `307323fbc1f7` | `sparse viewport plus colorful/right-column title-menu art` |
| 05 | `entrance_menu` | `c7050df75088` | `dungeon entrance/menu controls still occupy the right column` |
| 06 | `entrance_menu` | `24712fca8db3` | `dungeon entrance/menu controls still occupy the right column` |
| 07 | `entrance_menu` | `17bd7e878157` | `dungeon entrance/menu controls still occupy the right column` |
| 08 | `entrance_menu` | `17bd7e878157` | `dungeon entrance/menu controls still occupy the right column` |
| 09 | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |

Gate result: `dungeon_gameplay` was confirmed on gate frame 09, so movement/control probes were issued only after entrance-menu disappearance.

## Final six-frame probe

Manifest:

```tsv
index	filename	route_label	route_token
01	image0001-raw.png	gate_confirmed_gameplay	shot:gate_confirmed_gameplay
02	image0002-raw.png	move_up_after_gate	shot:move_up_after_gate
03	image0003-raw.png	turn_right_after_gate	shot:turn_right_after_gate
04	image0004-raw.png	move_left_after_gate	shot:move_left_after_gate
05	image0005-raw.png	spell_or_party_key_after_gate	shot:spell_or_party_key_after_gate
06	image0006-raw.png	inventory_or_party_key_after_gate	shot:inventory_or_party_key_after_gate
```

Classifier outcome:

| file | class | sha | reason |
|------|-------|-----|--------|
| `image0001-raw.png` | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |
| `image0002-raw.png` | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |
| `image0003-raw.png` | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |
| `image0004-raw.png` | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |
| `image0005-raw.png` | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |
| `image0006-raw.png` | `dungeon_gameplay` | `48ed3743ab6a` | `viewport content with mostly dark in-game right column` |

Pass80 problems:

- image0004-raw.png: classified dungeon_gameplay, expected spell_panel
- image0006-raw.png: classified dungeon_gameplay, expected inventory

Pass113 outcome:

- party_control_ready: `false`
- direct_start_no_party_signature: `true`
- classes: `dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`

## Diagnosis

Pass118 solves the immediate route-driver gap from pass117: the automation now waits on frame class and confirms the transition from `entrance_menu` to `dungeon_gameplay` before issuing movement/control probes. It does **not** solve party control readiness. After the gate, all six final captures are the same blank/no-party dungeon hash (`48ed3743ab6a`), and F1/F4/control probes do not produce spell or inventory classes.

No new source dependency was needed beyond the pass117 ReDMCSB BUG0_73 input-queue rationale; this pass isolated the startup/entrance gate itself. The remaining blocker is therefore not just fixed sleeps firing too early: after a deterministic class gate, the original route still lands in a no-party/direct-start state with no proven champion recruitment or party-control HUD.

## Remaining blocker

Need a deterministic original PC DM1 champion/entrance selection gate that proves recruitment/party-control readiness after `dungeon_gameplay`, not merely entrance-menu disappearance. The tool route driver can now wait on state; the unsolved route semantics are which original input/menu/champion-selection action changes the blank right-column dungeon state into a party-control-ready HUD/spell/inventory state.
