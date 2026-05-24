# JOB 3: Source-lock DM1 V1 — Wall Collision

## Collision Check Sequence

Ref: CLIKMENU.C:270-314 — F0366_COMMAND_ProcessTypes3To6_MoveParty:

After computing destination coordinates via F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement (line 269), the square type is fetched:
  L1116_i_SquareType = M034_SQUARE_TYPE(
    AL1115_ui_Square = F0151_DUNGEON_GetSquare(L1121_i_MapX, L1122_i_MapY));

## Square Type Checks (lines 279-291)

### 1. Wall — Always Blocked
  if (L1116_i_SquareType == C00_ELEMENT_WALL) {
      L1117_B_MovementBlocked = C1_TRUE;
  }
C00_ELEMENT_WALL = 0 (DEFS.H:1007). No exceptions.

### 2. Door — Conditional
  if (L1116_i_SquareType == C04_ELEMENT_DOOR) {
      L1117_B_MovementBlocked = M036_DOOR_STATE(AL1115_ui_Square);
      L1117_B_MovementBlocked =
          (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) &&
          (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) &&
          (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);
  }
Blocked unless: state is OPEN (C0), CLOSED_ONE_FOURTH (C1), or DESTROYED (C5).

### 3. Fakewall — Conditional
  if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL) {
      L1117_B_MovementBlocked =
          (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN) &&
           !M007_GET(AL1115_ui_Square, MASK0x0001_FAKEWALL_IMAGINARY));
  }
Blocked unless FAKEWALL_OPEN bit is set or FAKEWALL_IMAGINARY bit is set.

## Damage on Blocked Movement

Ref: CLIKMENU.C:293-308 — When blocked and party has champions:
  L1117_B_MovementBlocked = F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(
      L1124 = F0286_CHAMPION_GetTargetChampionIndex(
          L1121_i_MapX, L1122_i_MapY,
          M021_NORMALIZE(AL1118 + (G0308_i_PartyDirection + 2))),
      1,  // attack power
      MASK0x0008_WOUND_TORSO | MASK0x0010_WOUND_LEGS,
      C2_ATTACK_SELF);
- Two leading party cells take wounds to torso and legs
- First cell: NORMALIZE(movement_arrow_index + party_direction + 2)
- Second cell: NORMALIZE(first_cell + 1)
- Sound M562_SOUND_PARTY_DAMAGED plays from destination square
- Queue is flushed: F0357_COMMAND_DiscardAllInput()
- VBlank wait on PC-34: F0693_WaitVerticalBlank() (line 321)

## Creature Collision

Ref: CLIKMENU.C:312 — After wall/door/fakewall checks:
  if (L1117_B_MovementBlocked = (F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY)
                                  != C0xFFFE_THING_ENDOFLIST)) {
      F0209_GROUP_ProcessEvents29to41(L1121_i_MapX, L1122_i_MapY,
          CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT, 0);
  }
- Any creature on destination square blocks movement
- Creature gets event 31 to become more aggressive/alert

## Edge Cases

- BUG0_43: If last living champion is killed while target is a portrait candidate, F0321 returns 0 and movement is not blocked
- BUG0_85: If party has 0 champions, creature collision check is entirely skipped

## Firestaff Implementation

Source: src/dm1/dm1_v1_movement_command_core_pc34_compat.c
- dm1_v1_record_blocked_wall_or_door_damage_request() records the damage request with correct cell calculations
- Wall/door/fakewall/creature block logic with identical square-type checks
- Source citations: CLIKMENU.C:270-314, F0175_GROUP_GetThing, C0xFFFE_THING_ENDOFLIST

## Status

PASS — Wall collision is a per-square-type switch in F0366. Wall=always, Door=not-open-not-quarter-open-not-destroyed, Fakewall=not-open-and-not-imaginary. Creature collision by F0175_GROUP_GetThing. All block paths lead to damage and queue discard. ReDMCSB fully traced.
