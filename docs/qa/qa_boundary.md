# DM1 V1 Boundary Conditions Audit

## Dungeon Size Limits

From dm1_v1_dungeon_loader_pc34_compat.h:
- DM1_MAX_LEVELS: 16
- DM1_MAX_MAP_W: 32
- DM1_MAX_MAP_H: 32
- Maximum tiles per level: 32x32 = 1024

Loader validation: bounds check on w/h before fread, level_count bounded.
Edge cases NOT tested: 0x0 map, max 32x32, level count = 0 or 16, partial tile read.

## Champion Count

- DM1_MAX_CHAMPIONS: 4

Edge cases NOT tested: 0 champions alive, 1 champion solo, 4 champion full party,
champion at 0 HP but alive (resurrection candidate), full vs empty inventory.

## Item/Inventory Limits

- DM1_MAX_TORCH_SLOTS: (DM1_PARTY_MAX_CHAMPIONS * DM1_CHAMPION_HAND_SLOTS)
- DM1_MAX_LIGHT_EVENTS: 16

Edge cases NOT tested: inventory full (no empty slot for pickup), all torch slots
occupied, light event queue full (16 events).

## Group/Creature Limits

- DM1_MAX_CREATURES_IN_GROUP: 4

Edge cases NOT tested: group at max capacity, group splitting on creature death,
empty group, fixed possession drops.

## Skill/Experience Limits

- DM1_MAX_SKILL_LEVEL: 16

Edge cases NOT tested: skill at max level (16), experience overflow at 32-bit limit,
all skills at max level (champion mastery).

## Movement/Turn Timing

Edge cases NOT tested: double-tap movement, movement blocked by wall/door/creature,
movement into pit/teleporter, turn during turn animation, input during room transition.

## Error Handling

File loading: NULL checks on fopen, partial fread caught, fseek validation.
Memory allocation: NULL check on malloc in save_load.c.
All error paths have proper cleanup with free().

## Empty/Zero States

NOT tested: empty dungeon (0 levels), map with no sensors/creatures,
party with no items, no light sources (darkness), empty event timeline.

## Save/Load Edge Cases

NOT tested: corrupted save (checksum mismatch), truncated save, save from different
dungeon version, save during room transition, save with projectile in flight.

## Combat Edge Cases

NOT tested: damage overflow, armor exceeding damage, 0 HP champion targeted,
dead champion in party, projectile expires mid-flight, spell fails.
