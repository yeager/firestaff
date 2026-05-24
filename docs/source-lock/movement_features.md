# JOB 4: Source-lock DM1 V1 — Pit/Dungeon Feature Collision

## Pit Handling

Ref: MOVESENS.C:538 — inside F0267_MOVE_GetMoveResult_CPSCE teleporter chain loop:
```
if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) &&
    !L0713_B_ThingLevitates &&
    M007_GET(AL0708_i_DestinationSquare, MASK0x0008_PIT_OPEN) &&
    !M007_GET(AL0708_i_DestinationSquare, MASK0x0001_PIT_IMAGINARY))
```
Conditions for pit fall:
1. Square type = C02_ELEMENT_PIT (DEFS.H:1009)
2. Thing does NOT levitate (party never levitates; F0264_MOVE_IsLevitates returns false for party)
3. MASK0x0008_PIT_OPEN bit is set
4. MASK0x0001_PIT_IMAGINARY bit is NOT set

Pit fall chain:
- L0719_i_TraversedPitCount++ (tracks multiple pits in a row)
- If party and L0719 > 0: F0174_DUNGEON_SetCurrentMapAndPartyMap + F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF
- F0128_DUNGEONVIEW_Draw_CPSF for visual update (BUG0_28/BUG0_71 comments re: fast computers)
- F0154_DUNGEON_GetLocationAfterLevelChange to resolve landing square on lower level
- F0173_DUNGEON_SetCurrentMap to switch maps
- If party has champions: stamina cost OR leg/feet wounds (20 HP, attack type C2_ATTACK_SELF)
- If creature: F0191_GROUP_GetDamageAllCreaturesOutcome for fall damage

Levitation: F0264_MOVE_IsLevitating (MOVESENS.C:145+) returns true for:
- Groups with MASK0x0020_LEVITATION attribute
- Projectiles (always levitate; BUG0_26 notes explosion should too but does not)

## Teleporter Handling

Ref: MOVESENS.C:475-537 — teleporter chain loop within F0267:
- Loop iterates up to 100 times (S21E+; 1000 for older versions)
- Checks teleporter OPEN bit (MASK0x0008_TELEPORTER_OPEN)
- Checks scope: MASK0x0001 (creatures) or MASK0x0002 (party/objects) vs thing type
- Updates P0560/P0561 to target coordinates; sets current map to target map index
- If party: updates G0306/G0307/G0308 (position + direction)
- If creature: F0262_MOVE_GetTeleporterRotatedGroupResult rotates group
- If projectile: F0263_MOVE_GetTeleporterRotatedProjectileThing adjusts direction
- Audible flag plays M560_SOUND_BUZZ
- Chain continues from new destination; L0724_B_DestinationIsTeleporterTarget breaks early if at target

## Stairs Handling

Ref: CLIKMENU.C:264-274 — stairs on current square, moving backward:
- AL1118_ui_MovementArrowIndex == 2 means moving backward
- F0364_COMMAND_TakeStairs(MASK0x0004_STAIRS_UP ? up : down) is called
- Returns immediately from F0366 (no F0267 call)

Ref: CLIKMENU.C:271-277 — moving forward INTO a stairs square:
- L1116_i_SquareType == C03_ELEMENT_STAIRS
- F0267 with CM1_MAPX_NOT_ON_A_SQUARE (suppresses departure sensors)
- G0306/G0307 update to L1121/L1122
- F0364_COMMAND_TakeStairs with stairs direction bit

Stairs sensor behavior (CLIKMENU.C comment on F0366):
- Non-stairs to stairs: departure fires, arrival suppressed on stairs square, re-fired from destination level
- Stairs to non-stairs: departure suppressed, arrival fires normally
- Stairs to stairs (turn or back-step): normal departure/arrival

## Door Interaction

Door squares (C04_ELEMENT_DOOR) are a pre-movement barrier in F0366 (lines 282-285):
- Checked BEFORE F0267_MOVE_GetMoveResult_CPSCE is called
- Blocked unless door state is OPEN (C0), CLOSED_ONE_FOURTH (C1), or DESTROYED (C5)
- This means doors are solid like walls unless explicitly open or quarter-open

## Sensor Processing

F0276_SENSOR_ProcessThingAdditionOrRemoval called:
- After successful destination resolution in F0267
- Departure sensors: C1_TRUE (isAddition=false), C0_FALSE (isFirstTime=false)
- Arrival sensors: C1_TRUE, C1_TRUE (isFirstTime=true)
- BUG0_31: C005_SENSOR_FLOOR_PARTY_ON_STAIRS never fires for party on stairs due to wrong variable in condition

## Firestaff Implementation

Source: src/dm1/dm1_v1_movement_pipeline_pc34_compat.c
- pipeline_delete_group_on_square_before_enter_sensors — removes creature before sensors fire
- Sensor chain modeled on F0276_SENSOR_ProcessThingAdditionOrRemoval
- Stairs dispatch (DM1_V1_STAIRS) handled in command core, matching F0364_COMMAND_TakeStairs path
- dm1_v1_movement_command_core_pc34_compat.c records the blocked-movement damage request

Source citations: MOVESENS.C:475-650, CLIKMENU.C:264-277, F0264_MOVE_IsLevitating, F0276_SENSOR_ProcessThingAdditionOrRemoval, F0364_COMMAND_TakeStairs, F0154_DUNGEON_GetLocationAfterLevelChange.

## Status

PASS — Pit fall is non-blocking: if open+real pit under party, F0267 chain processes it after teleporter chain. Party does not levitate so always falls. Stairs are a pre-movement exit from F0366 via F0364. Teleporter chains up to 100 iterations. All feature interactions ReDMCSB-traced.