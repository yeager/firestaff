# M15 movement source lock probe

Status: **partial_blocked_missing_requested_sources**

Worktree: `/home/trv2/work/firestaff-worktrees/n2-dm1v1-merge-readiness-20260505-1228`
Source base: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## Source availability

- Present: MOVESENS.C
- Missing from requested set: MOVE.C, MOVEOBJ.C, MOVESEN2.C

## Movement command → state → collision → draw sequence

1. **Command gate** (`COMMAND.C:2095-2100`) — movement commands `C003..C006` are deferred while `G0310_i_DisabledMovementTicks` or matching projectile-disabled direction blocks input.
2. **Relative movement mapping** (`CLIKMENU.C:224-233,256,269-270`) — command minus `C003_COMMAND_MOVE_FORWARD` indexes forward/right/back/left step tables, then `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` computes the target coordinate.
3. **Static collision** (`CLIKMENU.C:278-289`) — walls, blocked doors, and closed non-imaginary fake walls set `L1117_B_MovementBlocked`.
4. **Creature collision** (`CLIKMENU.C:291-323`) — group in target square blocks party movement, triggers reaction event 31, discards queued input, and returns.
5. **State transition call** (`CLIKMENU.C:325-329,345-346`) — allowed moves call `F0267_MOVE_GetMoveResult_CPSCE`; then movement cooldown is set from champion movement ticks.
6. **Move-result state** (`MOVESENS.C:438-451,738-741`) — party coordinates and final move-result globals are written after chained destination resolution.
7. **Departure sensors** (`MOVESENS.C:799-807`) — source-square removal/departure sensors run before destination addition.
8. **Arrival sensors/group collision** (`MOVESENS.C:810-822,830-844,869-873,892-897`) — party arrival handles group deletion/drop; group-on-party/group-on-group creates delayed move event; normal arrivals process sensor addition.
9. **Chained teleporters/pits** (`MOVESENS.C:469-493,505-531,538-565,574-606`) — teleporters and pits are resolved in a bounded chain before final sensor/list update.
10. **Draw sequence** (`GAMELOOP.C:80-90,150-151`; `MOVESENS.C:445-450,539-556`) — normal dungeon view draws from current party state in the main loop; pit falling can draw intermediate views inline.

## Blocker

- Requested files MOVE.C, MOVEOBJ.C, MOVESEN2.C are absent from the specified ReDMCSB Source directory and absent under ReDMCSB_WIP20210206 by case-insensitive filename search.

Detailed structured evidence: `parity-evidence/verification/m15_movement_source_lock.json`
