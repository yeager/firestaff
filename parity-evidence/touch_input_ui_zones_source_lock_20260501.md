# Touch input UI zones source lock (DM1 V1 parity)

Date: 2026-05-01
Lane: touchscreen support retry, source-locked only.
Primary source audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

## Intent

This is a non-runtime artifact to keep touchscreen support out of movement/front-wall parity lanes until the original DM1 input boundary is explicit.  Any future touch layer should feed the same click coordinates/buttons into the existing command path; it must not special-case dungeon movement, collision, projectile, tick, front-wall, door-field, or viewport rendering code.

## Original source boundary

### Command queues and active input tables

`COMMAND.C:6-16` defines the circular `G0432_as_CommandQueue`, queue indexes, queue lock, and pending click storage. `COMMAND.C:22-24` holds active primary/secondary mouse and primary keyboard input tables. This means touch/click abstraction belongs before command lookup/queueing, not inside movement execution.

### Main interface zones

`COMMAND.C:78-105` defines `G0447_as_Graphic561_PrimaryMouseInput_Interface` for top champion/status/icon areas plus spell/action/freeze zones. Exact DM1 V1 boxes captured there include:

- Champion status boxes: `0..42,0..28`, `69..111,0..28`, `138..180,0..28`, `207..249,0..28`.
- Champion hand/icon quadrants: `274..299,0..13`, `301..319,0..13`, `301..319,15..28`, `274..299,15..28`.
- Spell area: `233..319,42..73`.
- Action area: `233..319,77..121`.
- Freeze pixel trap: `0..1,198..199`.

### Movement and dungeon-view click zones

`COMMAND.C:106-121` defines `G0448_as_Graphic561_SecondaryMouseInput_Movement`. The source-locked movement/click boxes are:

| Command | Box |
| --- | --- |
| `C001_COMMAND_TURN_LEFT` | `234..261,125..145` |
| `C003_COMMAND_MOVE_FORWARD` | `263..289,125..145` |
| `C002_COMMAND_TURN_RIGHT` | `291..318,125..145` |
| `C006_COMMAND_MOVE_LEFT` | `234..261,147..167` |
| `C005_COMMAND_MOVE_BACKWARD` | `263..289,147..167` |
| `C004_COMMAND_MOVE_RIGHT` | `291..318,147..167` |
| `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` | `0..223,33..168` |

`COMMAND.C:323-328` duplicates the four move-arrow highlight boxes in `G0463_aai_Graphic561_Box_MovementArrows`, in command-index order forward/right/backward/left. `MENUDRAW.C:5-18` draws the movement arrows from graphic `C013_GRAPHIC_MOVEMENT_ARROWS`, confirming the UI draw boundary is separate from click dispatch.

### Zone indirection and viewport offsets

`COMMAND.C:1098-1142` initializes mouse-input boxes from zone ids via `F0672_InitializeAllMouseInput()` and `F0673_SetMouseInputBoxFromZone()`. Negative `Box.X1` values mean source zone ids are resolved through `F0638_GetZone()`, with special viewport offsets for `Box.X1 == -2` (`G2067_i_ViewportScreenX`, `G2068_i_ViewportScreenY`) and `Box.X1 == -3` (centered viewport offset). A touch layer must preserve that resolution path instead of baking scaled/translated zones into gameplay code.

### Hit matching and command dispatch

`COMMAND.C:1379-1444` (`F0358_COMMAND_GetCommandFromMouseInput_CPSC`) is the authoritative hit matcher: it checks the active `MOUSE_INPUT` table, inclusive box bounds, zone-indirection cases, and button masks. `COMMAND.C:1452-1662` (`F0359_COMMAND_ProcessClick_CPSC`) is the authoritative click-to-command queue path: it handles queue-full checks, primary lookup first (`G0441_ps_PrimaryMouseInput`), secondary lookup second (`G0442_ps_SecondaryMouseInput`), and only enqueues non-zero commands with the original click `X/Y` saved. `COMMAND.C:1693-1707` replays deferred pending clicks through the same function.

Keyboard input is parallel but separate: `COMMAND.C:1709-1765` starts `F0361_COMMAND_ProcessKeyPress`, locks the command queue, scans keyboard tables, and enqueues a command. Touch must not fake keyboard movement unless intentionally testing keyboard parity; the click path is the source-locked touchscreen seam.

### Dungeon movement execution boundary

`CLIKMENU.C:142-174` executes turn commands (`F0365_COMMAND_ProcessTypes1To2_TurnParty`) after command dispatch: highlights turn boxes/zones, handles stairs, and updates party direction/sensors. `CLIKMENU.C:180-347` executes move commands (`F0366_COMMAND_ProcessTypes3To6_MoveParty`): derives movement arrow index from command, highlights original movement box/zone, computes relative map deltas, performs wall/door/fake-wall/group blocking, handles stairs, and sets movement-disabled ticks.

Touchscreen work must not modify these execution functions for input convenience. If a tap on `263..289,125..145` queues `C003_COMMAND_MOVE_FORWARD`, all movement/collision/tick behavior stays in `CLIKMENU.C` unchanged.

## Implementation guardrails for future code

1. Add touch as an input-provider shim that normalizes device coordinates to original 320x200 screen coordinates and calls/feeds the existing click path equivalent to `F0359_COMMAND_ProcessClick_CPSC(x, y, MASK0x0002_MOUSE_LEFT_BUTTON)`.
2. Keep source zones inclusive (`x >= X1 && x <= X2`, `y >= Y1 && y <= Y2`) as in `COMMAND.C:1437-1439`.
3. Do not widen movement/dungeon-view boxes without an explicit parity probe; enlarged touch affordances would change command selection near boundaries.
4. Do not touch movement collision, tick-orchestrator, projectile, front-wall, door-field, or viewport renderer files for touchscreen support.
5. If scaling from modern viewport/window coordinates is needed, put it before hit matching and cover it with tests that assert the exact DM1 V1 boxes above map to the same commands.
