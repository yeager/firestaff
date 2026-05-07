# pass344 — DM1 V1 full launcher `--script` handoff

Status: `MOVEMENT_PROVED_FULL_LAUNCHER`

## ReDMCSB anchors cited before probing

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

- `ENTRANCE.C:739-747` — entrance installs `G0441_ps_PrimaryMouseInput = G0445_as_Graphic561_PrimaryMouseInput_Entrance`, no secondary mouse movement table, and no secondary keyboard movement table.
- `ENTRANCE.C:856-882` — entrance discards queued input, waits in entrance mode, and calls `F0380_COMMAND_ProcessQueue_CPSC()` until the entrance command changes `G0298_B_NewGame`.
- `STARTUP2.C:1179-1182` — after dungeon load, runtime installs the active game input tables: primary mouse/interface, secondary mouse/movement, primary keyboard/interface, secondary keyboard/movement.
- `GAMELOOP.C:164-168,215` — the game loop drains keyboard input and processes the command queue while waiting for player input.
- `COMMAND.C:243-260` — PC34 keyboard movement table maps turn/move command equivalents.
- `COMMAND.C:375-405` — active game interface/movement mouse tables include champion/interface commands and movement/viewport commands.
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC()` is the command-dispatch seam used after input has been queued.
- `COMMAND.C:2438-2451` — entrance enter/resume commands switch the new-game/load mode.

## Firestaff handoff finding

The outer launcher path is valid when driven through the same explicit launch seam that M12 uses internally:

1. `main_loop_m11.c` reads one `--script` token per loop through `m11_next_script_input()`.
2. Before launch, those tokens are applied to `M12_StartupMenu_HandleInput()`; the top-level launcher deliberately does not short-circuit `Enter`/`Right` into `M11_GameView_OpenSelectedMenuEntry()`.
3. Only the M12 game-options launch row sets `state->launchRequested = 1`.
4. `main_loop_m11.c:m11_open_requested_launch()` opens the selected runtime with `M11_GameView_OpenSelectedMenuEntry()`.
5. After `gameView.active` becomes true, remaining route tokens are sent to `M11_GameView_HandleInput(&gameView, input)`, which is the already-proved movement/input path from earlier passes.

The confusing blocker was the local persisted launcher config on N2: `/home/trv2/.config/firestaff/startup-menu.toml` had `presentation_mode = "v3-modern-3d"`. V3 intentionally blocks launch. The verifier uses a clean temporary `HOME` so default M12 config is V1 original and the exact outer launcher script reaches runtime.

## Exact narrow launcher command/probe

Recorded in `parity-evidence/verification/pass344_dm1_v1_full_launcher_script_handoff/manifest.json`.

Script used:

`enter,down,down,down,down,down,down,enter,left,up,right`

Meaning:

- `enter` opens DM1 game options from the main launcher.
- Six `down` tokens reach the V1 launch row because V1 hides/skips aspect and resolution rows.
- The second `enter` requests launch.
- `left,up,right` are post-launch route-token equivalents consumed by the active game view.

## Verification artifacts

- `tools/verify_pass344_dm1_v1_full_launcher_script_handoff.py`
- `parity-evidence/verification/pass344_dm1_v1_full_launcher_script_handoff/manifest.json`
- `parity-evidence/verification/pass344_dm1_v1_full_launcher_script_handoff/full_launcher_script_probe.redacted.log`

## Gates

Required gates run after this evidence was written:

- `python3 -m py_compile tools/verify_pass344_dm1_v1_full_launcher_script_handoff.py`
- `python3 tools/verify_pass344_dm1_v1_full_launcher_script_handoff.py`
- `git diff --check HEAD~1..HEAD`

## Status

`MOVEMENT_PROVED_FULL_LAUNCHER`
