# Pass601 — DM1 V1 Gameplay Systems Source-Lock

Status: PASS601_DM1_V1_GAMEPLAY_SYSTEMS_SOURCE_LOCKED

## Scope

Extended source-lock of DM1 V1 gameplay systems against ReDMCSB primary source.
This pass covers combat, creature AI, and timeline event integration — the core
gameplay loop beyond movement/viewport/walls.

## ReDMCSB Primary References

### CHAMPION.C — Combat and Champion Management
- F0309_CHAMPION_GetMaximumLoad (1157-1177): load capacity formula
- F0310_CHAMPION_GetMovementTicks (1180-1215): overload movement penalty, BUG0_72
- F0313_CHAMPION_GetWoundDefense (1305-1375): armour/wound defense calculation
- F0314_CHAMPION_WakeUp (1382-1415): party wake from sleep
- F0318_CHAMPION_DropAllObjects (1527-1550): death object drop
- F0319_CHAMPION_Kill (1552-1687): champion death pipeline
- F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds (1689-1800): damage application
- F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage (1803-1949): damage computation

### GROUP.C — Creature AI and Group Management
- F0175_GROUP_GetThing (52-67): group lookup by position
- F0176_GROUP_GetCreatureOrdinalInCell (69-106): cell-based creature lookup
- F0177_GROUP_GetMeleeTargetCreatureOrdinal (109-160): melee target resolution
- F0180_GROUP_StartWandering (311-337): wandering behavior init
- F0181_GROUP_DeleteEvents (340-372): event cleanup
- F0182_GROUP_StopAttacking (374-388): attack cancel
- F0183_GROUP_AddActiveGroup (389-448): active group registration
- F0184_GROUP_RemoveActiveGroup (450-479): active group removal
- F0190_GROUP_GetDamageCreatureOutcome (769-930): creature damage resolution
- F0191_GROUP_GetDamageAllCreaturesOutcome (932-990): group-wide damage
- F0193_GROUP_StealFromChampion (1013-1080): creature theft mechanic
- F0196_GROUP_InitializeActiveGroups (1135-1174): active group init
- F0197/F0198 view/smell blocked (1176-1238): line-of-sight/smell pathing
- F0199_GROUP_GetDistanceBetweenUnblockedSquares (1239-1314): pathfinding
- F0200_GROUP_GetDistanceToVisibleParty (1315-1414): party detection
- F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal (1417-1455): scent tracking
- F0202_GROUP_IsMovementPossible (1457-1554): movement legality
- F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal (1556-1575): direction scan
- F0204_GROUP_IsArchenemyDoubleMovementPossible (1576-1590): boss movement
- F0209_GROUP_ProcessEvents29to41 (1850-2466): main AI event processing
- F0229_GROUP_SetOrderedCellsToAttack (1628-1700): cell attack ordering

### TIMELINE.C — Event Scheduling
- F0233_TIMELINE_Initialize_CPSE (50-100): heap init
- F0234-F0236: heap maintenance (compare, index lookup, fix placement)
- F0237_TIMELINE_DeleteEvent (393-419): event removal
- F0238_TIMELINE_AddEvent_GetEventIndex_CPSE (487-660): event insertion

## Non-Claims
- No pixel-level rendering parity is claimed.
- No original DOS runtime comparison was used.
- DANNESBURK was not used.
