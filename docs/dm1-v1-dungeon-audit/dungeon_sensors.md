# DM1 V1 Sensor/Actuator Data (C000-C127) — Source Audit

## ReDMCSB Source
- DEFS.H:1200–1253: SENSOR struct (union of Remote/Local)
- DEFS.H:1256–1284: Sensor type constants C000–C127
- DEFS.H:1300–1308: Sensor access macros M039–M048

## SENSOR Struct Layout (DEFS.H:1200–1253, 8 bytes on disk)
Union of Remote and Local — shares first 6 bytes.

Bytes 0-1: THING Next (uint16, little-endian)
Bytes 2-3: Type_Data (uint16):
  bits 6:0 = sensorType (M039_TYPE = Type_Data & 0x007F)
  bits 15:7 = sensorData (M040_DATA = Type_Data >> 7)

Bytes 4-5: Common bitfield (PC LSB-first MEDIA016):
  bf1 = raw[4]|(raw[5]<<8):
  bit 2 = OnceOnly, bits 4:3 = Effect(2), bit 5 = RevertEffect,
  bit 6 = Audible, bits 10:7 = Value(4), bit 11 = LocalEffect, bits 15:12 = OrnamentOrdinal(4)

Bytes 6-7: Remote interpretation:
  bf2 = raw[6]|(raw[7]<<8): bits 5:4=TargetCell(2), bits 10:6=TargetMapX(5), bits 15:11=TargetMapY(5)
Bytes 6-7: Local interpretation:
  bf2 = raw[6]|(raw[7]<<8): bits 11:0=Multiple(12), bits 15:12=bUnreferenced(4)

## Sensor Type Constants
Floor (0-9): C000_DISABLED, C001_THERON_PARTY_CREATURE_OBJECT, C002_THERON_PARTY_CREATURE,
  C003_PARTY, C004_OBJECT, C005_PARTY_ON_STAIRS, C006_GROUP_GENERATOR, C007_CREATURE,
  C008_PARTY_POSSESSION, C009_VERSION_CHECKER
Wall (0-18,127): C001_CLICK, C002_CLICK_WITH_ANY_OBJECT, C003_CLICK_WITH_SPECIFIC_OBJECT,
  C004_CLICK_WITH_SPECIFIC_OBJECT_REMOVED, C005_AND_OR_GATE, C006_COUNTDOWN,
  C007_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT, C008_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION,
  C009_DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT, C010_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION,
  C011_CLICK_REMOVED_ROTATE, C012_OBJECT_GENERATOR_ROTATE, C013_SINGLE_OBJECT_STORAGE_ROTATE,
  C014_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT, C015_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT,
  C016_OBJECT_EXCHANGER, C017_CLICK_REMOVED_REMOVE_SENSOR, C018_END_GAME, C127_PORTRAIT

Effect: C00_EFFECT_SET(0), C01_EFFECT_CLEAR(1), C02_EFFECT_TOGGLE(2), C03_EFFECT_HOLD(3)

## Firestaff Implementation
File: include/memory_dungeon_dat_pc34_compat.h — DungeonSensor_Compat
File: src/memory/memory_dungeon_dat_pc34_compat.c:472–515 — decode_sensor()
All bitfields: sensorType/typeData/onceOnly/effect/revertEffect/audible/value/localEffect/
ornamentOrdinal/targetCell/targetMapX/targetMapY/localMultiple match MEDIA016 bit layout.

STATUS: ALIGNED — Sensor decoding matches ReDMCSB DEFS.H MEDIA016 bitfield layout exactly.
