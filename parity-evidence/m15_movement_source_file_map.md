# M15 movement source file map follow-up

Status: **blocked_requested_files_do_not_exist_in_redmcsb; mapped_to_actual_sources**

Worktree: `/home/trv2/work/firestaff-worktrees/n2-dm1v1-merge-readiness-20260505-1228`
Commit checked: `1ac505c`
ReDMCSB source base checked: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
Broader ReDMCSB base checked: `/home/trv2/.openclaw/data/firestaff-redmcsb-source`

## Requested file existence check

The requested split movement files are not ReDMCSB files in the checked local ReDMCSB source trees.

| Requested file | Exact/case-insensitive result under `ReDMCSB_WIP20210206` | Exact/case-insensitive result under broader `firestaff-redmcsb-source` | Replacement source used |
|---|---:|---:|---|
| `MOVESENS.C` | present | present in WIP and non-WIP tree | `MOVESENS.C` |
| `MOVE.C` | absent | absent | `CLIKMENU.C`, `MOVESENS.C`, `COMMAND.C`, `GAMELOOP.C` |
| `MOVEOBJ.C` | absent | absent | object/group/projectile movement paths inside `MOVESENS.C` |
| `MOVESEN2.C` | absent | absent | sensor/list update tail inside `MOVESENS.C` |

Verification command evidence:

```sh
find /home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206 -type f \
  \( -iname MOVE.C -o -iname MOVEOBJ.C -o -iname MOVESEN2.C -o -iname MOVESENS.C \)
# only MOVESENS.C returned

find /home/trv2/.openclaw/data/firestaff-redmcsb-source -type f \
  \( -iname MOVE.C -o -iname MOVEOBJ.C -o -iname MOVESEN2.C -o -iname MOVESENS.C \)
# only Toolchains/Common/Source/MOVESENS.C in WIP and non-WIP trees returned

grep -RIn -E "MOVEOBJ|MOVESEN2|MOVE\\.C" /home/trv2/.openclaw/data/firestaff-redmcsb-source
# no filename mentions returned
```

## Exact actual source map

| Movement concern | Actual file | Function | Exact lines | What this covers |
|---|---|---|---:|---|
| Movement command queue cooldown gate | `COMMAND.C` | `F0380_COMMAND_ProcessQueue_CPSC` | `2045-2100` | Reads queued command, identifies `C003_COMMAND_MOVE_FORWARD..C006_COMMAND_MOVE_LEFT`, defers movement when `G0310_i_DisabledMovementTicks` or matching projectile movement disable is active. |
| Main-loop command dispatch timing | `GAMELOOP.C` | `F0002_MAIN_GameLoop_CPSDF` | `164-217` | Keyboard input collection and `F0380_COMMAND_ProcessQueue_CPSC()` call after draw/tick processing. |
| Normal dungeon redraw after movement | `GAMELOOP.C` | `F0002_MAIN_GameLoop_CPSDF` | `80-90` | Draws `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)`. |
| Movement cooldown decrement | `GAMELOOP.C` | `F0002_MAIN_GameLoop_CPSDF` | `150-154` | Decrements movement and projectile-disabled movement ticks once per main-loop pass. |
| Forward/right/back/left command mapping | `CLIKMENU.C` | file-scope movement arrow tables used by `F0366_COMMAND_ProcessTypes3To6_MoveParty` | `224-233`, `256` | Maps command offset from `C003_COMMAND_MOVE_FORWARD` to relative forward/right step counts. |
| Target square coordinate resolution | `CLIKMENU.C` | `F0366_COMMAND_ProcessTypes3To6_MoveParty` | `264-270` | Reads current party square, handles stairs precondition, converts relative movement to target map coordinates. |
| Stair movement special cases | `CLIKMENU.C` | `F0366_COMMAND_ProcessTypes3To6_MoveParty`; helper `F0364_COMMAND_TakeStairs` | `124-139`, `264-276`, `325-328` | Backing into stairs, entering stairs, and delegation to `F0267_MOVE_GetMoveResult_CPSCE`/level-change helper. |
| Static collision blocker | `CLIKMENU.C` | `F0366_COMMAND_ProcessTypes3To6_MoveParty` | `278-289` | Blocks walls, non-open/non-destroyed doors, and closed non-imaginary fake walls. |
| Collision damage and group blocker | `CLIKMENU.C` | `F0366_COMMAND_ProcessTypes3To6_MoveParty` | `291-323` | Applies bump damage on blocked geometry; checks target group with `F0175_GROUP_GetThing`; triggers reaction event 31; discards input and returns if blocked. |
| Allowed party movement state transition | `CLIKMENU.C` | `F0366_COMMAND_ProcessTypes3To6_MoveParty` | `325-346` | Calls `F0267_MOVE_GetMoveResult_CPSCE` with source/destination and sets `G0310_i_DisabledMovementTicks`; clears projectile-disabled ticks. |
| Core movement/sensor routine entry | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `316-331` | Single ReDMCSB routine that replaces the requested split `MOVE*.C`/`MOVESEN2.C` files for party/group/projectile movement result computation. |
| Moving thing classification and projectile-impact precheck | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE`; helpers `F0264_MOVE_IsLevitating`, `F0266_MOVE_IsKilledByProjectileImpact` | `136-166`, `195-308`, `405-435` | Determines party/group/projectile/object class, levitation, and projectile impact before chained movement. |
| Party state mutation before chain resolution | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `438-451` | Writes `G0306_i_PartyMapX`, `G0307_i_PartyMapY`, required teleporter scope, draw-while-falling gate, and baseline direction. |
| Teleporter chain resolution | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE`; helpers `F0262_MOVE_GetTeleporterRotatedGroupResult`, `F0263_MOVE_GetTeleporterRotatedProjectileThing` | `33-131`, `469-535` | Bounded chain over teleporters; updates destination map/coords, party coords, direction, audible state, group/projectile/object orientation/cell handling. |
| Pit/fall chain resolution and inline draw | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `538-606` | Handles open pits, level changes, falling draw via `F0128_DUNGEONVIEW_Draw_CPSF`, stamina/damage, and party coord update. |
| Final move-result globals | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `738-741` | Writes `G0397_i_MoveResultMapX`, `G0398_i_MoveResultMapY`, `G0399_ui_MoveResultMapIndex`, `G0401_ui_MoveResultCell`. |
| No-op/self-square result short-circuit | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `752-763` | Returns no movement when final party direction/cell did not change on same source square. |
| Party scent/update preparation | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `763-797` | Updates party scent data and restores source map context before source-square removal. |
| Departure sensor/list processing | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `799-807` | Processes source-square party/object/group removal; unlinks levitating things directly. |
| Party arrival sensor/group collision result | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `810-822` | On party arrival, drops/deletes a group occupying the destination and processes destination sensor addition or schedules map change. |
| Group/object arrival and delayed group collision | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE`; helper `F0265_MOVE_CreateEvent60To61_MoveGroup` | `169-193`, `823-887` | For group movement, delays if destination is party/another group, manages active group membership, links/sensor-adds, starts wandering, and returns move result. |
| Non-group projectile/object arrival sensors | `MOVESENS.C` | `F0267_MOVE_GetMoveResult_CPSCE` | `892-897` | Links projectile-like things or processes destination sensor addition for ordinary moved things. |

## Blocker status

**Blocked only on the requested filenames, not on movement-source traceability.** `MOVE.C`, `MOVEOBJ.C`, and `MOVESEN2.C` are absent under both the WIP tree and broader local ReDMCSB source base, and there are no text mentions of those filenames in the checked ReDMCSB source/docs. The corresponding movement responsibilities are consolidated in `MOVESENS.C`, with command/collision entry points in `CLIKMENU.C`, command queue gating in `COMMAND.C`, and draw/tick sequencing in `GAMELOOP.C`.

Next work should cite the actual files above instead of the missing split names unless a different, non-ReDMCSB historical source package is provided.
