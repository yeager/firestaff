# DM2 V1 Secrets/Puzzles — Source-Lock Audit

## Sources

- SKULL.ASM (sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject/SkWinCore.cpp (actuator handling, puzzle triggers)
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt
- docs/dm2-v1-overview/dm2_story.md
- include/dm2_v1_companion.h
- include/dm2_v1_game.h

---

## Known Secrets and Puzzles in DM2

### Main Quest: The Skullkeep Artifact

DM2's central narrative is recovering a legendary artifact from within the Skullkeep fortress, guarded by a dragon.

The second clan door type (0x0A) specifically triggers the dragon encounter at the Skullkeep. This door type has:
- Door Strength field (0F 00 00)
- Color key 1 (04 00 00, cyan) — enables see-through rendering behind door
- Color key 2 (0C 00 00, dark green) — secondary transparency

Source: SKWin.GDAT2.InternalCodes.txt DOOR section, docs/dm2-v1-overview/dm2_story.md

### X Teleporter System (Cross-Scene Teleporters)

DM2 introduces cross-scene teleporter puzzles using the SDFSM command system:
- SDFSM_CMD_X_TELEPORTER (4): Cross-teleporter — teleports party to a different scene
- SDFSM_CMD_X_ANCHOR (5): Anchor teleporter — the Sun Clan village anchor point

The SEARCH_DUNGEON_FOR_SPECIAL_MARKER function (SkWinCore.cpp:18904) processes these markers during pathfinding. The ENGAGE_X_TELEPORTER function (SkWinCore.cpp:19041) activates the X-mark floor teleporter at the party position.

Source: SkWinCore.cpp:18904-19072, DME.h:384

### Sun Clan Village Anchor Teleporter

The anchor teleporter (SDFSM_CMD_X_ANCHOR = 5) appears in the Sun Clan village as a special marker. This suggests a puzzle where the party must find and activate the anchor to enable cross-scene teleportation.

Source: SkWinCore.cpp:19032

### Ladder-Based Puzzle Navigation

DM2 extends ladder mechanics with GDAT_WALL_ORNATE__IS_LADDER_UP (0x11) encoding:
- 1 = ladder going up
- absent = ladder going down

The FIND_LADDER_AROUND function (SkWinCore.cpp:9060) searches for ladders within 2 tiles of the party position. Ladder actuators can be enabled/disabled (ACTUATOR_TYPE_SIMPLE_LADDER = 0x1C, noted as beta-only).

Source: SkWinCore.cpp:9060-9108, defines.h:616, 21437-21445

### Item-Triggered Wall Switches

Actuator type ACTUATOR_TYPE_ITEM_WATCHER (0x03) triggers when a specific item is used on a wall square. The GDAT_WALL_ORNATE__IS_ITEM_TRIGGERED (0x0E) and GDAT_WALL_ORNATE__SWITCH_ITEM (0x0E) fields mark walls that respond to item interactions.

Source: defines.h:610-613, SkWinCore.cpp:21368

### Cryocell Champion Display

GDAT_WALL_ORNATE__CRYOCELL (0x5B) renders as a passive device that shows a champion portrait when activated. This could be a puzzle element where triggering the cryocell reveals information or unlocks a path.

Source: defines.h:548, SkWinCore.cpp:13102-13107

### Respawn Cooldown Gem Veins

GDAT_WALL_ORNATE__RESPAWN_COOLDOWN (0x12) controls gem vein respawn timing. This creates a resource-management puzzle where the party must wait or return later for gems to respawn.

Source: defines.h:612, SkWinCore.cpp:21546

### Shop Glass / Alcove System

GDAT_WALL_ORNATE__POSITION (0x05) values:
- 0 = non alcove
- 1 = alcove (secret item alcove)
- 2 = shop glass (merchant interaction)
- 3 = passive device (cryocell)

Alcove-type positions (value 1) likely contain hidden treasures or secrets.

Source: defines.h:596, SKWin.GDAT2.InternalCodes.txt

### Rebirth Altar (DM2 Beta)

GDAT_WALL_ORNATE__IS_REBIRTH_ALTAR = 0x0C, noted as used in DM2 beta. May have been a puzzle element for champion revival or stat restoration.

Source: defines.h:605

### Water Spring Recovery

GDAT_WALL_ORNATE__IS_WATER_SPRING = 0x0B marks healing/rest locations in the dungeon.

Source: defines.h:604

---

## DM1 Secrets Reference (for comparison)

From docs/dm1-v1-dungeon-audit/terrain/terrain_special.md:
- Pit traps (imaginary/open/invisible variants)
- Teleporter squares (visible/open flags, scope-based activation)
- Fake walls (imaginary/open)
- Champion stat boost altars
- Hidden treasure alcoves

---

## DM2 Secret System Differences

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Hidden areas | Fake walls, alcoves | Alcove system (0x05=1), item-triggered walls |
| Teleport puzzles | Single teleporter squares | X teleporter/anchor system, cross-scene |
| Ladder puzzles | Simple up/down | IS_LADDER_UP ornate field, actuator-controlled |
| Item puzzles | None | Item watcher actuators (0x03), item-triggered walls |
| Special rewards | Champion stat boost | Cryocell display, rebirth altar, water spring |
| Puzzle triggers | Manual switch/walk | Item use, actuator type system, GDAT ornate fields |

---

## AI Patching System for Dungeon-Specific Puzzles

DM2 uses runtime AI patches for dungeon-specific behavior. Example from SkWinCore.cpp (EXTENDED_LOAD_AI_DEFINITION):
- if (dungeon == SKULLKEEP) creature 51 gains fireball attack

This means the dragon gains fireball ability specifically in the Skullkeep dungeon. Other dungeons may have different AI patches, creating puzzle/enemy variety.

Source: skproject/SkWinCore.cpp (EXTENDED_LOAD_AI_DEFINITION)

---

## Companion/NPC Puzzle Elements

DM2 introduces companion NPCs who fight alongside the party. Champion GDAT (0x16) provides NPC behavior data. This enables puzzle solutions that require party composition or companion assistance.

Source: include/dm2_v1_companion.h, docs/dm2-v1-overview/dm2_story.md

---

## STATUS: SOURCE-LOCKED
