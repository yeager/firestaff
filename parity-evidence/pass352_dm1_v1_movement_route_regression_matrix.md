# Pass352 DM1 V1 movement route regression matrix

Status: `PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED`

This pass is a consolidation gate after pass346/pass347 integration. It does not duplicate the pass348 NumLock blocker classification or the pass349 full-launcher route proof; it pins the combined movement-route surface so stale blockers cannot regress silently.

## ReDMCSB source audit anchors

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

- `IO2.C:32` calls `IODRV_00_GetKeyboardInput` in the P20JA path before PC key normalization.
- `IO2.C:37` calls `IODRV_00_GetKeyboardInput` in the P20JB/I34E/I34M/P31J path.
- `IO2.C:47` starts the I34E/I34M shifted extended-arrow normalization switch; lines `47-59` rewrite shifted arrow scancodes to `L`, `P`, `K`, `M` (`0x004C`, `0x0050`, `0x004B`, `0x004D`).
- `COMMAND.C:636-685` defines `G0459_as_Graphic561_SecondaryKeyboardInput_Movement`; I34E/I34M lines `677-683` map `0x004B/0x004C/0x004D/0x004F/0x0050/0x0051` to `C001/C003/C002/C006/C005/C004`.
- `STARTUP2.C:1179-1182` wires primary/secondary mouse and keyboard input tables, including `G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement`.
- `COMMAND.C:1379` starts `F0358_COMMAND_GetCommandFromMouseInput_CPSC`; `COMMAND.C:1452` starts `F0359_COMMAND_ProcessClick_CPSC`; `COMMAND.C:1641` looks up primary then secondary mouse input tables; `COMMAND.C:1709` starts `F0361_COMMAND_ProcessKeyPress`; `COMMAND.C:1765` and `COMMAND.C:1792` enqueue primary/secondary keyboard commands into `G0432_as_CommandQueue`.
- `COMMAND.C:2045-2125` starts `F0380_COMMAND_ProcessQueue_CPSC`, checks movement-disabled gate, dequeues `G0432_as_CommandQueue`, and preserves pending-click replay.
- `MENUDRAW.C:5-19` draws movement arrows from `C013_GRAPHIC_MOVEMENT_ARROWS` / `C009_ZONE_MOVEMENT_ARROWS`; this is display evidence only, not an input dispatch proof.
- `CLIKMENU.C:142-174` dispatches turn commands; `CLIKMENU.C:180-347` dispatches move commands.

## Regression matrix

| Lane | Route surface | Product seam | Gate expectation |
| --- | --- | --- | --- |
| Script tokens | bare `left/right/up/down/strafe-left/strafe-right` script tokens | `main_loop_m11.c:m11_map_script_token` → `M11_GameView_HandleInput` → `m11_dm1_v1_pipeline_command_for_input` | Same live party state as the proven keypad route for equivalent commands |
| Arrow key symbols | `key:left`, `key:right`, `key:up`, `key:down` | `main_loop_m11.c:m11_script_keycode_from_name` + SDL key switch | Same command ids as ReDMCSB movement table after normalization |
| PC34 numpad aliases | `key:kp1`..`key:kp6` and `key:kp-1`..`key:kp-6` | explicit SDL keypad cases added by pass346 | No OS NumLock synthesis needed; keypad aliases map to PC34 movement commands |
| Direct command queue | already resolved `C001..C006` command ids | `DM1_V1_MovementPipeline_EnqueueCommandPc34Compat` | Bypasses OS delivery but still exercises F0380 → F0365/F0366 compat movement pipeline |
| Touch dispatch guard | primary-before-secondary mouse table route | pass347/pass350 touch live dispatch gate | No stale blocker remains in the movement route matrix; touch evidence stays separate from keyboard/keypad proof |

## Retired stale blockers in this consolidated scope

- `BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF` is retired for DM1 V1 M11 launcher script movement routes by pass344/pass349/pass352 gates.
- `BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E` is retired for M11 SDL `key:kp*` routing only; pass348 still correctly does **not** claim original DOS/I34E keyboard-buffer proof.
- Touch primary/secondary dispatch ordering is guarded by pass347/pass350 and included here only as a route-regression dependency, not re-proved.

## Non-claims

- No DOSBox/original debugger hit is claimed.
- No pixel parity is claimed.
- No raw original runtime keyboard-buffer proof is claimed.
