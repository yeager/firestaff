# Pass212 — DM1 V1 original state-aware movement probe

Status: `BLOCKED_MISSING_POST_COMMAND_REDRAW_OBSERVABLE`

Scope: N2-only follow-up to the pass210/pass211 fixed-sleep blocker. This pass tested whether a classifier-gated route can replace fixed sleeps by waiting until the original DOSBox run first reaches `dungeon_gameplay` before movement shots.

## Source seam audited

The relevant ReDMCSB seam is already source-locked and was rechecked on N2 during this pass:

- `tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py` → PASS; key citations include `COMMAND.C:2075`, `COMMAND.C:2096`, `COMMAND.C:2127`, `COMMAND.C:2151`, `COMMAND.C:2155`, `CLIKMENU.C:269`, `CLIKMENU.C:318`, `MOVESENS.C:763-784`, `MOVESENS.C:799-818`.
- `tools/verify_dm1_v1_entry_movement_viewport_source_lock.py` → PASS.
- `tools/verify_dm1_v1_movement_command_gate_source_lock.py` → PASS; cites `COMMAND.C:2045-2156`, `CLIKMENU.C:142-174`, `CLIKMENU.C:180-347`, `MOVESENS.C:442-818`, and `GAMELOOP.C:35-97`.
- `tools/verify_dm1_v1_movement_source_lock.py` → PASS.
- `tools/verify_dm1_v1_movement_timing_source_lock.py` → PASS; output explicitly source-locks `COMMAND.C:2095-2100`, `COMMAND.C:2118-2127`, `CLIKMENU.C:256-269`, `CLIKMENU.C:330-346`, `CHAMPION.C:1180-1215`, and `MOVESENS.C:752-775`.
- `tools/verify_dm1_v1_party_movement_sensor_order_source_lock.py` → PASS; source-locks `MOVESENS.C:442-818`, `MOVESENS.C:1553-1793`, and `COMMAND.C:2045-2155`.
- `tools/verify_dm1_v1_projectile_movement_interlock_source_lock.py` → PASS; confirms movement/projectile gate behavior at `COMMAND.C:2095-2100`, `COMMAND.C:2104-2110`, `CLIKMENU.C:330-346`, and `GAMELOOP.C:150-155`.

Interpretation: a promotable movement capture must observe the source sequence `F0380_COMMAND_ProcessQueue_CPSC` → `F0365_COMMAND_ProcessTypes1To2_TurnParty` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` → `F0267_MOVE_GetMoveResult_CPSCE` where applicable → `F0002_MAIN_GameLoop_CPSDF` redraw → `F0128_DUNGEONVIEW_Draw_CPSF` / `F0097_DUNGEONVIEW_DrawViewport` present. A raw screenshot class of `dungeon_gameplay` is necessary but not sufficient to prove that sequence happened.

## Probe run

Command:

```sh
rm -rf verification-screens/pass212-n2-state-aware-movement-probe
xvfb-run -a python3 tools/pass118_state_aware_original_route_driver.py \
  --out "$PWD/verification-screens/pass212-n2-state-aware-movement-probe" \
  --gate-timeout 24
```

This uses the existing state-aware driver instead of a fixed-sleep route. It repeatedly captures and classifies gate frames until `pass80` sees `dungeon_gameplay`, then emits six final movement/control shots.

Gate evidence from `verification-screens/pass212-n2-state-aware-movement-probe/pass118_gate.tsv`:

| gate | class | sha12 | file |
|---|---|---|---|
| 01 | `entrance_menu` | `ceb0c2eec633` | `gate01_entrance_menu.png` |
| 02 | `entrance_menu` | `ceb0c2eec633` | `gate02_entrance_menu.png` |
| 03 | `entrance_menu` | `7f34445c6d04` | `gate03_entrance_menu.png` |
| 04 | `graphics_320x200_unclassified` | `2908c8c701b8` | `gate04_graphics_320x200_unclassified.png` |
| 05 | `graphics_320x200_unclassified` | `0c198ddafde3` | `gate05_graphics_320x200_unclassified.png` |
| 06 | `entrance_menu` | `d3ee1a4f1a36` | `gate06_entrance_menu.png` |
| 07 | `entrance_menu` | `17bd7e878157` | `gate07_entrance_menu.png` |
| 08 | `entrance_menu` | `17bd7e878157` | `gate08_entrance_menu.png` |
| 09 | `dungeon_gameplay` | `48ed3743ab6a` | `gate09_dungeon_gameplay.png` |

So the state-aware gate does fix the pass210 first-shot problem: final shots are not taken while the frame is still `entrance_menu`.

## Movement-only classifier gate

Command:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass212-n2-state-aware-movement-probe \
  --expected dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay \
  --fail-on-duplicates \
  --out-json verification-screens/pass212-n2-state-aware-movement-probe/pass80_movement_six_class_gate.json \
  --out-md verification-screens/pass212-n2-state-aware-movement-probe/pass80_movement_six_class_gate.md
```

Exit: `1`.

Result:

- `capture_count`: `6`
- `class_counts`: `{"dungeon_gameplay": 6}`
- `duplicate_sha256_counts`: `{"48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397": 6}`
- `problems`: `duplicate raw frames detected: 1 unique sha256 value(s) repeat`

Shot binding from `pass118_driver.log` and `pass80_movement_six_class_gate.json`:

| shot | route label | key/action before shot | class | sha12 |
|---|---|---|---|---|
| 1 | `gate_confirmed_gameplay` | none | `dungeon_gameplay` | `48ed3743ab6a` |
| 2 | `move_up_after_gate` | `Up` | `dungeon_gameplay` | `48ed3743ab6a` |
| 3 | `turn_right_after_gate` | `Right` | `dungeon_gameplay` | `48ed3743ab6a` |
| 4 | `move_left_after_gate` | `Left` | `dungeon_gameplay` | `48ed3743ab6a` |
| 5 | `spell_or_party_key_after_gate` | `F1` | `dungeon_gameplay` | `48ed3743ab6a` |
| 6 | `inventory_or_party_key_after_gate` | `F4` | `dungeon_gameplay` | `48ed3743ab6a` |

## Decision

The fixed-sleep blocker has been narrowed: waiting for `dungeon_gameplay` prevents entrance-menu final shots, but it still does **not** prove party-control or post-command movement/redraw. Every post-gate key collapses to the identical raw frame SHA `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`.

Exact blocker: the current observable is only a screen-layout classifier. It can say “not entrance menu”, but it cannot prove that the queued input reached `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC`, mutated `G0308_i_PartyDirection` / `G0306_i_PartyMapX` / `G0307_i_PartyMapY`, released `G0321_B_StopWaitingForPlayerInput`, and then presented a new viewport through `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF` / `DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport`.

## Landable next action

Do **not** promote another fixed-sleep route. The next route driver needs a post-command readiness oracle, not a longer delay. Acceptable implementations:

1. emulator/memory-backed oracle for `G0308_i_PartyDirection`, `G0306_i_PartyMapX`, `G0307_i_PartyMapY`, `G0321_B_StopWaitingForPlayerInput`, and ideally `G0305_ui_PartyChampionCount`; or
2. a stricter visual oracle that requires a non-duplicate raw SHA after each movement/turn command and blocks immediately when `pass80 --fail-on-duplicates` would fail.

Until one of those observables exists, the missing signal is: **post-command redraw/state mutation after party-control input**.

Non-claims: no <private-host> use, no push, no OpenAI API key, no Sonnet/q3.6 use, no original-vs-Firestaff pixel parity claim.
