# DM1 V1 Key Usage Door Source Audit

Status: blocked as a direct locked-door key behavior. The ReDMCSB PC path does not show a distinct "use key on locked door" route. Key-like object use is source-backed only through wall click sensors/object mechanisms, not through door squares.

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## Audited Files

- `COMMAND.C`
- `CLIKVIEW.C`
- `MOVESENS.C`
- `DUNGEON.C`
- `DUNVIEW.C`
- `PANEL.C`
- `DEFS.H`
- `TIMELINE.C`

## Evidence

- `COMMAND.C:2322-2324` dispatches dungeon viewport clicks to `F0377_COMMAND_ProcessType80_ClickInDungeonView`.
- `CLIKVIEW.C:365-390` handles a front door only when the leader hand is empty, the clicked zone is the door-button/wall-ornament zone, and `DOOR->Button` is set; it schedules `C10_EVENT_DOOR` with `C02_EFFECT_TOGGLE`.
- `CLIKVIEW.C:391-401` handles the non-empty leader hand branch while the square ahead is a door by attempting `F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown`; there is no object-type check for keys and no door unlock/toggle path in this branch.
- `CLIKVIEW.C:449-499` routes non-empty leader-hand object use to wall squares only: drop into wall object pile/alcove, fountain refill, then `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor` for wall sensors.
- `MOVESENS.C:1309-1538` implements `F0275_SENSOR_IsTriggeredByClickOnWall`; specific-object/removing object behavior is in wall sensor cases `C003`, `C004`, `C011`, `C017`, `C013`, and `C016`. These compare/use `G4055_s_LeaderHandObject.Thing` against sensor data on wall-sensor cells.
- `DEFS.H:1266-1284` labels those mechanisms as wall ornament click/player click sensors, while wall event sensors are separate event-driven mechanisms.
- Focused source search for `key.*door`, `door.*key`, `locked door`, `unlock.*door`, `lock.*door`, `keyhole`, and `key hole` across `COMMAND.C`, `CLIKVIEW.C`, `MOVESENS.C`, `DUNGEON.C`, `PANEL.C`, `DEFS.H`, `TIMELINE.C`, and `DUNVIEW.C` found no direct key-door route.

## Decision

Do not implement a direct key-on-locked-door action for DM1 V1. The supported behavior is:

1. Door button on a front door toggles the door when `DOOR->Button` is present and the leader hand is empty.
2. Keys/specific objects can trigger wall sensors or wall object mechanisms when a wall sensor is present.

The root TODO item should stay open or be rewritten by main as a source-lock blocker unless a different primary source proves a direct locked-door key path.
