# Pass349 — DM1 V1 full launcher keypad runtime route

Status: `FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED`

## Scope

Proves the full product launcher path, not only the direct M11 game-view harness: `firestaff --script` launches DM1 V1 from the outer M12 launcher, then `key:kp*` keypad script tokens are delivered through SDL event polling into the active DM1 V1 live viewport route and the source-locked compat movement pipeline.

No DOSBox/original FIRES debugger hit or pixel parity is claimed here.

## ReDMCSB source audit anchors

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

- `COMMAND.C:636-685` — `G0459_as_Graphic561_SecondaryKeyboardInput_Movement` binds I34E movement commands to `0x004B`, `0x004C`, `0x004D`, `0x004F`, `0x0050`, `0x0051`.
- `IO2.C:5-61` — `F0540_INPUT_Crawcin` reads `IODRV_00_GetKeyboardInput`; I34E shifted/extended arrow scancodes normalize to the same movement-table codes.
- `COMMAND.C:1709-1813` — `F0361_COMMAND_ProcessKeyPress` checks primary keyboard input, then `G0444_ps_SecondaryKeyboardInput`, and queues matching commands.
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` processes one queued command and dispatches turns to `F0365` and steps to `F0366`.
- `GAMELOOP.C:150-155` — movement cooldown counters decrement per loop.
- `GAMELOOP.C:164-168,215` — the game loop drains keyboard input through `F0361` and processes the queue through `F0380`.
- `STARTUP2.C:1179-1183` — after dungeon load, active game input tables install interface primary and movement secondary keyboard/mouse tables.
- `CLIKMENU.C:142-174` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` applies turn commands via `F0284_CHAMPION_SetPartyDirection`.
- `CLIKMENU.C:180-347` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` resolves relative movement and calls `F0267_MOVE_GetMoveResult_CPSCE` for accepted movement.
- `MOVESENS.C:316-326` — `F0267_MOVE_GetMoveResult_CPSCE` is the move/sensor application entry.

## Product route proved

Script used by the verifier:

`enter,down,down,down,down,down,down,enter,key:kp4,key:kp4,key:kp4,key:kp5,key:kp6`

Route meaning:

1. `enter` opens DM1 options in the outer launcher.
2. Six `down` tokens select the V1 launch row.
3. Second `enter` requests runtime launch through M12.
4. `key:kp4,key:kp4,key:kp4` are SDL keypad events that turn from south → east → north → west.
5. `key:kp5` moves forward west in the Hall of Champions, changing party position from `(1,3)` to `(0,3)`.
6. `key:kp6` turns right to north and leaves the last pipeline result as a viewport-dirty turn command.

The verifier records `full_launcher_keypad_runtime_probe.json` from the running `firestaff` binary via `FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON`. Observed final state:

- `launchedEver=1`, `active=1`, title `DUNGEON MASTER`, source `dm1`
- party `mapIndex=0`, `mapX=0`, `mapY=3`, `direction=0`, `championCount=0`
- last pipeline result `dequeued=1`, `command=2`, `turnApplied=1`, `viewportDirty=1`

This proves the route reached the active live viewport and compat movement path after full launcher handoff.

## Verification artifacts

- Verifier: `tools/verify_pass349_dm1_v1_full_launcher_keypad_runtime_route.py`
- Manifest: `parity-evidence/verification/pass349_dm1_v1_full_launcher_keypad_runtime_route/manifest.json`
- Runtime probe: `parity-evidence/verification/pass349_dm1_v1_full_launcher_keypad_runtime_route/full_launcher_keypad_runtime_probe.json`

## Gates

- `python3 -m py_compile tools/verify_pass349_dm1_v1_full_launcher_keypad_runtime_route.py`
- `tools/verify_pass349_dm1_v1_full_launcher_keypad_runtime_route.py` → `FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED`
- Verifier-internal CMake configure/build in `~/.openclaw/data/firestaff-builds/pass349-verify`
- Verifier-internal `test_dm1_v1_movement_pipeline_pc34_compat` → `138 passed, 0 failed`
- `cmake --build ~/.openclaw/data/firestaff-builds/pass349-verify --target firestaff_m11_hall_walkaround_runtime_probe -j2`
- `ctest --test-dir ~/.openclaw/data/firestaff-builds/pass349-verify -R "dm1_v1_movement_pipeline|dm1_v1_hall_walkaround_runtime" --output-on-failure` → 2/2 passed
- `git diff --check`
