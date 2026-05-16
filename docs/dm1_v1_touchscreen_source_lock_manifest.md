# DM1 V1 touchscreen source-lock manifest

Scope: source-lock the active DM1 V1 in-game mouse/clickable UI zones and command routing before expanding touchscreen support. This is an evidence artifact only; it does not change input behavior.

## Primary ReDMCSB source

Base path on N2:

`/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

## Active in-game mouse tables

ReDMCSB `COMMAND.C:375-405` defines the active interface and movement mouse tables used during normal dungeon play.

| Table | Lines | Role | Firestaff abstraction |
| --- | ---: | --- | --- |
| `G0447_as_Graphic561_PrimaryMouseInput_Interface` | `COMMAND.C:375-395` | Champion status/toggles, champion corner icons, spell parent, action parent, freeze box. Searched before secondary movement. | `touch_click_zone_matrix_pc34_compat.c` primary-interface entries; `DM1_V1_InputCommandQueue_EnqueueEventPc34Compat` primary mouse routing. |
| `G0448_as_Graphic561_SecondaryMouseInput_Movement` | `COMMAND.C:396-405` | Six movement arrows, dungeon viewport click, right-button leader inventory toggle. | `touch_click_zone_matrix_pc34_compat.c` movement and viewport entries; `dm1_v1_input_command_queue_pc34_compat.c` secondary mouse routing. |
| `G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory` | `COMMAND.C:412-452` | Inventory close/save/rest/music, slot boxes, mouth/eye, panel. Uses both screen-relative and viewport-relative coordinates. | `touch_click_zone_matrix_pc34_compat.c` inventory-mode viewport-relative entries and dispatch mappers. |
| `G0452/G0453/G0454/G0455` | `COMMAND.C:461-497` | Action names/icons, spell area subroutes, champion names/hands subroutes. | `action_area_routes_pc34_compat.c`, `spell_area_routes_pc34_compat.c`, `spell_area_symbol_routes_pc34_compat.c`, `champion_name_hand_routes_pc34_compat.c`, and their inclusion in the touch matrix invariant test. |

## Command matching and queue routing

| Function | Lines | Source behavior | Firestaff abstraction |
| --- | ---: | --- | --- |
| `F0358_COMMAND_GetCommandFromMouseInput_CPSC` | `COMMAND.C:1379-1449` | Walks a `MOUSE_INPUT` table in source order, checks button mask, and tests absolute, screen-relative, or viewport-relative zones. | `TOUCHCLICK_Compat_HitTestPrimaryThenSecondary`, `TOUCHCLICK_Compat_HitTestInCoordMode`, and `command_for_mouse`. |
| `F0359_COMMAND_ProcessClick_CPSC` | `COMMAND.C:1452-1661` | Computes queue capacity, resolves release/stop events, searches primary mouse input first and secondary second, then queues command/x/y. | `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat` and `DM1_V1_InputCommandQueue_EnqueueEventPc34Compat`. |
| `F0360_COMMAND_ProcessPendingClick` | `COMMAND.C:1692-1707` | Replays one pending click after queue unlock. | `process_pending_click` and pending-click fields in `Dm1V1InputCommandQueuePc34Compat`. |
| `F0361_COMMAND_ProcessKeyPress` | `COMMAND.C:1709-1813` | Shares command queue with keyboard input and replays pending click after unlock. | Existing keyboard path in `dm1_v1_input_command_queue_pc34_compat.c`; touchscreen must enqueue through the same mouse command queue path, not bypass it. |
| `F0380_COMMAND_ProcessQueue_CPSC` | `COMMAND.C:2045-2156,2280-2335` | Locks queue, gates disabled movement, dequeues one command, replays pending click, dispatches turns/moves, spell/action/viewport/panel commands. | `DM1_V1_InputCommandQueue_ProcessOnePc34Compat` and downstream action/viewport/champion route probes. |

## Click-specific handlers

| Function | Lines | Source behavior | Firestaff abstraction |
| --- | ---: | --- | --- |
| `F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox` | `CLIKCHAM.C:1-36` | Secondary hit-test inside a champion status box maps name clicks to leader selection and hand-slot clicks to slot-box handling. | `champion_name_hand_routes_ResolveStatusBoxClick`; covered by `touch_click_zone_matrix_pc34_compat` integration test. |
| `F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE` action-name/icon child routing | `CLIKMENU.C:529-582` | Action parent command enters pass/action rows or champion action icons depending on action-area state. Name rows use inclusive boxes and `G0507_ui_ActionCount`; icon rows use inclusive boxes and `G0305_ui_PartyChampionCount`. | `action_area_routes_pc34_compat.c`; invariant checks action parent/child routes, menu command mapping, gates, and inclusive endpoints. |
| `F0377_COMMAND_ProcessType80_ClickInDungeonView` | `CLIKVIEW.C:365-389,406-445,449-510` | Door button, wall sensor, front-wall knock, grab/drop leader hand object, throw object, fountain/alcove handling. | `dm1_v1_viewport_click_pc34_compat.c` and existing door/viewport click probes; touchscreen should feed command `C080` with source screen coordinates. |
| `F0712_AnyKeyboardOrMouseInput` | `IO2.C:262-280` | PC/I34 variants treat mouse button state as input availability alongside keyboard. | `touch_pointer_input_pc34_compat.c` and the M11 touch pointer dispatch probes normalize touch to mouse-like input. |

## V1 parity guardrails for touchscreen work

1. Touchscreen taps should normalize to a 320x200 DM1 screen point or a 224x136 viewport-local point before route lookup.
2. Touchscreen dispatch must preserve ReDMCSB source order: primary interface table first, secondary movement table second.
3. Left and right button semantics are source data: left is `0x0002`, right is `0x0001`. Touch defaults may emulate left-button clicks, but right-button inventory toggles are distinct routes.
4. Inventory/panel zones marked viewport-relative must not be treated as top-level screen-relative zones without explicit translation by `TOUCHCLICK_Compat_MapViewportLocalPointToDispatch` or the scaled viewport mapper.
5. Routed touch commands must enter `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat`/`DM1_V1_InputCommandQueue_EnqueueEventPc34Compat`, preserving queue capacity, pending-click replay, movement gating, and V1 command dispatch parity.
6. Broad UI refactors should wait until the matrix and queue probes are green for any added route group.
