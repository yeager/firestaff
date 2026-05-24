# DM1 V1 Special Squares — Source Audit

## ReDMCSB Source
- DEFS.H:1025-1032: Pit and Teleporter bit mask definitions
- DEFS.H:1009: C02_ELEMENT_PIT = 2
- DEFS.H:1012: C05_ELEMENT_TELEPORTER = 5
- DUNGEON.C:1445-1475: Border wall logic (PIT at map edge = wall)
- DUNGEON.C:1772-1819: F0172 pit/teleporter aspect rendering
- DUNGEON.C:4445-4455: F0509_DUNGEON_MoveAllThingsOnSquare — moves all things on square when pit/teleporter toggled
- CLIKMENU.C:270-314: F0366_COMMAND_ProcessTypes3To6_MoveParty — party movement into special squares
- firestaff_pc34_flattened_amalgam.c:2898: F0202_GROUP_IsMovementPossible pit/teleporter checks
- firestaff_pc34_flattened_amalgam.c:12186: F0128_DUNGEONVIEW_Draw_CPSF pit fall rendering

## Pit Squares

### Encoding (DEFS.H:1025-1027)
- MASK0x0001_PIT_IMAGINARY: pit is imaginary (no damage, can walk over)
- MASK0x0004_PIT_INVISIBLE: pit hidden (drawn as floor until triggered)
- MASK0x0008_PIT_OPEN: pit is open (active, damages/falls creatures)

### Party Movement (CLIKMENU.C:279-291)
Party movement blocked by PIT unless:
  - PIT is imaginary (MASK0x0001 set)
  - OR PIT is closed (!MASK0x0008_PIT_OPEN)
  - OR party has levitation (MASK0x0020_LEVITATION in champion attributes)

### Creature Movement (F0202_GROUP_IsMovementPossible, line 2898)
Creature movement blocked by PIT unless:
  - PIT is imaginary + movement flag allows imaginary pits
  - OR PIT is closed (!MASK0x0008_PIT_OPEN)
  - OR creature has MASK0x0020_LEVITATION attribute

### Pit Damage (firestaff_pc34_flattened_amalgam.c:12186)
When creature/party falls through open non-imaginary pit:
- Falls to destination map at same coordinates
- Damage applied: health = health / 2
- Smoke explosion placed at source (BUG0_66)
- If destination map disallows creature type: removed (no death event)

### Teleporter Squares

### Encoding (DEFS.H:1031-1032)
- MASK0x0004_TELEPORTER_VIBLE: teleporter is visible
- MASK0x0008_TELEPORTER_OPEN: teleporter is active

### Teleporter Thing Data (DUNGEON.C, Thing Type 1, 6 bytes)
- TargetMapIndex (byte 5)
- TargetMapX, TargetMapY (bits in bf1)
- Rotation, AbsoluteRotation (bits in bf1)
- Scope (bits 13-14): MASK0x0001_SCOPE_CREATURES, MASK0x0002_SCOPE_OBJECTS_OR_PARTY
- Audible (bit 15)

### Party Movement (CLIKMENU.C)
Party movement into teleporter: proceeds if teleporter is open and scope includes party.
Party appears at target map X/Y with rotation applied.

### Creature Movement (F0202_GROUP_IsMovementPossible, line 2917)
Creatures with Wariness >= 10 check destination map allowance before entering teleporter.
BUG0_67: G0380_T_CurrentGroupThing not set before creature-type-allowed check, can cause wrong teleport behavior.

### Scope-Based Activation
- MASK0x0001_SCOPE_CREATURES: only creatures can use
- MASK0x0002_SCOPE_OBJECTS_OR_PARTY: party and objects can use
- Both bits set: everything can use

## Wall Squares

### Pure Wall (C00_ELEMENT_WALL)
- Always blocks movement (party and creatures)
- No exceptions — wall is always impassable

### Fakewall (C06_ELEMENT_FAKEWALL)
- BLOCKED unless MASK0x0004_FAKEWALL_OPEN is set
- BLOCKED unless MASK0x0001_FAKEWALL_IMAGINARY is set
- Imaginary fakewall can be walked through (no damage)
- Open fakewall can be walked through (like a door)

## Firestaff Implementation

src/shared/firestaff_pc34_flattened_amalgam.c:2898 (F0202_GROUP_IsMovementPossible):
  - Pit check: imaginary OR closed OR creature levitates
  - Teleporter check: open AND scope includes creatures AND wariness >= 10 check

src/memory/memory_dungeon_dat_pc34_compat.c:516-535 (decode_teleporter):
  - 6-byte thing data decoded: next, bf1 (targetX/Y/rotation/scope/audible), bf2 (targetMapIndex)

## STATUS: ALIGNED
Pit/teleporter/wall special square logic fully source-locked against ReDMCSB.
BUG0_66, BUG0_67 documented in Firestaff amalgam comments.
