# DM2 V1 Special Squares/Triggers — Source-Lock Audit

## Sources

- SKULL.ASM (sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject/SkWinCore.cpp (functions for teleporter, ladder, actuator)
- skproject/SKWIN/defines.h (ACTUATOR_TYPE definitions, GDAT_WALL_ORNATE defines)
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt (GDAT2 internal codes)
- include/dm2_v1_dungeon_loader.h

---

## New Special Square Types in DM2

### Teleporter Squares (GDAT_CATEGORY_TELEPORTERS = 0x18)

DM2 introduces a dedicated Teleporter GDAT category (0x18), distinct from the generic special-square approach in DM1.

Rendered via:  at SkWinCore.cpp:10328

GDAT2 field: 

Source: SkWinCore.cpp:10328, defines.h:433, SKWin.GDAT2.InternalCodes.txt

Teleporter functions in SkWinCore.cpp:
-  (SkWinCore.h:1689) — reads teleporter state at position
-  (SkWinCore.cpp:19041) — activates X-mark floor teleporter at party position
-  (SkWinCore.cpp:18904) — finds special markers including teleporter anchors
-  (SkWinCore.h:1813) — renders teleporter chip

Teleporter scope enum (DME.h:384):
- Scope controls whether creatures, party, or objects can use the teleporter
- X teleporter (SDFSM_CMD_X_TELEPORTER = 4) — cross-scene teleport
- Anchor teleporter (SDFSM_CMD_X_ANCHOR = 5) — anchor in the Sun Clan village

Source: SkWinCore.cpp:18936-18937, DME.h:384

---

## DM1 Special Square Types (for comparison)

From docs/dm1-v1-dungeon-audit/terrain/terrain_special.md (ReDMCSB DEFS.H):

| Element | Value | Notes |
|---------|-------|-------|
| C02_ELEMENT_PIT | 2 | Pit squares (imaginary/open/invisible flags) |
| C05_ELEMENT_TELEPORTER | 5 | Teleporter squares (visible/open flags) |
| C06_ELEMENT_FAKEWALL | 6 | Fake wall (imaginary/open flags) |
| C00_ELEMENT_WALL | 0 | Always impassable |

DM1 special square system uses MASK bitfields in DEFS.H:1025-1032 for pit/teleporter state.

---

## New Trigger/Special Types in DM2

### X Teleporter (Cross-Scene)

DM2 has a cross-scene teleporter system with anchor support. SDFSM_CMD_X_TELEPORTER (4) and SDFSM_CMD_X_ANCHOR (5) are processed in .

Source: SkWinCore.cpp:18936-18937, 19027-19032

### Ladder Actuators

DM2 extends the ladder system.  (SkWinCore.cpp:9060) searches for ladders around party position.

ACTUATOR_TYPE_SIMPLE_LADDER (0x1C) exists but is noted as beta-only. Standard ladder triggers use  (0x11).

Source: SkWinCore.cpp:9060-9089, 21437-21445, defines.h:616

GDAT_WALL_ORNATE__IS_LADDER_UP field: queries whether a wall ornate is a ladder going up vs down.
- Value 1 = ladder going up
- Absent = ladder going down

Source: defines.h:616, SkWinCore.cpp:21602

### Cryocell (Wall Ornate 0x5B)

GDAT_WALL_ORNATE__CRYOCELL = 0x5B — passive device (shows champion portrait). Rendered in DRAW_STATIC_PIC with lever state.

Source: defines.h:548, SkWinCore.cpp:13102-13107

### Water Spring / Rebirth Altar

- GDAT_WALL_ORNATE__IS_WATER_SPRING = 0x0B
- GDAT_WALL_ORNATE__IS_REBIRTH_ALTAR = 0x0C (used for DM2 beta)

Source: defines.h:604-605

### Item-Triggered Switches

GDAT_WALL_ORNATE__SWITCH_ITEM = 0x0E (also GDAT_WALL_ORNATE__IS_ITEM_TRIGGERED = 0x0E)
GDAT_WALL_ORNATE__OVERLAY = 0x0F — shop glass / panel shop overlay

Source: defines.h:610-616

---

## DM2 Actuator Types (New/Changed from DM1)

From defines.h:1182-1204:

| Actuator Type | Value | DM1 Retro? | Notes |
|--------------|-------|-----------|-------|
| ACTUATOR_TYPE_X01 | 0x01 | DM1 | wall switch |
| ACTUATOR_TYPE_ITEM_WATCHER | 0x03 | — | item trigger |
| ACTUATOR_TYPE_MISSILE_SHOOTER | 0x08 | — | projectile trigger |
| ACTUATOR_TYPE_WEAPON_SHOOTER | 0x09 | — | weapon trigger |
| ACTUATOR_TYPE_ITEM_SHOOTER | 0x0E | — | item projectile |
| ACTUATOR_TYPE_DM1_WALL_TOGGLER | 0x0D | DM1 retro | |
| ACTUATOR_TYPE_THE_END | 0x12 | — | end trigger |

Many DM1 actuator types (0x01, 0x04, 0x05, 0x06, 0x07, 0x0C, 0x0F) are marked was unimplemented in DM2.

Source: defines.h:1185-1204

---

## New Door Special Features

### Dragon Door (Type 0x0A)

DM2 uses a second clan door type (0x0A) specifically for the Skullkeep Dragon:
- Door Strength field (0F 00 00)
- Two color keys: 04 00 00 (cyan) and 0C 00 00 (dark green)
- Cyan key enables see-through graphics behind door

Source: SKWin.GDAT2.InternalCodes.txt DOOR section

### Animated Mirrored Door (Field 20 00 00)

DM2 adds animated mirrored door rendering (like DM1 force field) via field 20 00 00.
Noted as unused in standard DM2 dungeons.

Source: SKWin.GDAT2.InternalCodes.txt

---

## Ladder Detection System

 (SkWinCore.cpp:9108)

Ladder state is encoded in wall ornate GDAT data (0x11 field), not in the square type itself. The game queries the wall ornate to determine if a wall square is a ladder.

Source: SkWinCore.cpp:9108, defines.h:616

---

## STATUS: SOURCE-LOCKED
