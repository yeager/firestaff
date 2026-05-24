# DM1 V1 Environmental Effects — Source Audit

## Critical Finding: No Standalone Environmental Terrain Types

DM1 V1 does NOT have separate terrain types for water, lava, ice, or similar
environmental hazards. The 7 element types (WALL, CORRIDOR, PIT, STAIRS, DOOR,
TELEPORTER, FAKEWALL) are the complete set of dungeon terrain types.

Environmental effects (water, lava, poison, etc.) are implemented via:

1. SENSOR triggers (DEFS.H:1256-1284)
2. PROJECTILE/EXPLOSION effects (DEFS.H:428, C007_EXPLOSION_POISON_CLOUD)
3. Magic spells (LIGHTNING_BOLT, FIRE, POISON_CLOUD)
4. Projectile launcher sensors (DEFS.H:1273-1275, 1279-1280)

## Sensor-Based Environmental Mechanisms

### Projectile Launcher Sensors (DEFS.H:1273-1280)
- C008_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION
- C010_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION
- C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT
- C015_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT

These sensors fire projectiles at party/creatures on adjacent squares.
Projectile types include fire, poison, lightning.

### Explosion Types (DEFS.H)
- C002_EXPLOSION_LIGHTNING_BOLT
- C007_EXPLOSION_POISON_CLOUD (DEFS.H:1558)
- C0xFF87_THING_EXPLOSION_POISON_CLOUD (0xFF87 = -121)

### Floor Sensors (DEFS.H:1257-1265)
- C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT: triggered when thing enters
- C003_SENSOR_FLOOR_PARTY: party on square triggers event
- C007_SENSOR_FLOOR_CREATURE: creature on square triggers event

## Water/Lava/Ice Effects — Not Standalone Terrain

There is no WATER, LAVA, or ICE element type in DM1 V1.

The creature WATER ELEMENTAL (C20_CREATURE_WATER_ELEMENTAL, DUNGEON.C:925)
is a creature type, not a terrain hazard.

Environmental damage from apparent water/lava/ice squares is achieved by:
- Projectile launcher sensors placed adjacent to those squares
- The projectile strikes party when it enters the affected area
- Explosion applies damage (fire, poison, etc.)

## ReDMCSB Source References

- DEFS.H:1007-1013: 7 element types only — no environmental hazard types
- DEFS.H:1256-1284: Complete sensor type list — environmental via sensors
- DEFS.H:1558: C007_EXPLOSION_POISON_CLOUD
- DEFS.H:428: C0xFF87_THING_EXPLOSION_POISON_CLOUD
- DEFS.H:1273-1280: Projectile launcher sensor types
- DUNGEON.C:925: WATER ELEMENTAL creature name, not terrain
- DUNGEON.C:1197-1198: Poison bolt/cloud projectile handling

## Firestaff Implementation

src/memory/memory_dungeon_dat_pc34_compat.h:79-86:
  DUNGEON_ELEMENT_WALL=0 through DUNGEON_ELEMENT_FAKEWALL=6
  Only 7 terrain types — correct for DM1 V1

Sensor trigger system: include/dm1_v1_sensor_trigger_pc34_compat.h
  DungeonSensor_Compat with trigger types matching DEFS.H:1256-1284

Explosion types: src/memory/memory_projectile_pc34_compat.c
  PROJECTILE_SUBTYPE_LIGHTNING_BOLT, C002_EXPLOSION_LIGHTNING_BOLT

## STATUS: ALIGNED
DM1 V1 has only 7 terrain element types. Environmental hazards are
sensor-driven (projectile launchers) or magic/projective effects,
not standalone terrain types. Firestaff correctly implements only
the 7 element types with sensor-based environmental mechanisms.
