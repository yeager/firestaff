# DM2 V1 Level Design — Source-Lock Audit

## Sources

- SKULL.ASM (sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject/SKWIN/defines.h
- skproject/SkWinCore.cpp
- include/dm2_v1_dungeon_loader.h
- include/dm2_v1_outdoor_renderer.h
- docs/dm2-v1-overview/dm2_dungeon_design.md

---

## How DM2 Level Design Differs from DM1

### Level Type Architecture

DM1: Single dungeon type — indoor first-person corridors and rooms. No outdoor world.

DM2: Three distinct level types (include/dm2_v1_dungeon_loader.h:19-21):
1. **OUTDOOR (0)**: Full landscape — sky textures, ground plane, trees, buildings, weather zones
2. **INDOOR (1)**: Standard DM1-style first-person dungeon (wall/floor/ceiling, corridors, rooms)
3. **BUILDING (2)**: Multi-floor structures within outdoor areas — transition renderer between outdoor and indoor

### Level Count Scaling

DM1: Variable map count (uint8, practical max ~16 maps for 10 levels). Each map is a rectangular grid.

DM2: Up to 30 levels with type metadata per level. Level count and types stored in  struct (include/dm2_v1_dungeon_loader.h:25-29).

Source: include/dm2_v1_dungeon_loader.h:15-16

### Map Descriptor Compatibility

DM2 map descriptors follow the same 16-byte structure as DM1 (DEFS.H:1048-1116):
- Bitfield A: Level(6) + Width-1(5) + Height-1(5)
- Bitfield B: WallOrnament(4) + RandomWall(4) + Floor(4) + RandomFloor(4)
- Bitfield C: DoorOrnament(4) + CreatureType(4) + Unref(4) + Difficulty(4)
- Bitfield D: FloorSet(4) + WallSet(4) + DoorSet0(4) + DoorSet1(4)

DM2 extends the level field via extended mode (0xF0 categories) to support up to 30 levels.

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## New Room/Level Features in DM2

### Outdoor Environment Design

Outdoor levels render (include/dm2_v1_outdoor_renderer.h):
- Sky texture index (per-level field)
- Ground texture (per-level field)
- Tree density (per-level field)
- Building count (per-level field)
- Weather zones: clear(0), rain(1), fog(2), storm(3)

Sky color modulated by time-of-day cycle (float 0.0-1.0).

Source: include/dm2_v1_outdoor_renderer.h

### Multi-Floor Buildings

Level type BUILDING (2) enables buildings with multiple indoor floors embedded in the outdoor world. Each building has its own indoor map grid, distinct from the surrounding outdoor terrain.

Source: include/dm2_v1_dungeon_loader.h:20

### Extended Ornate System

DM2 extends wall/floor ornate rendering (SKWin.GDAT2.InternalCodes.txt):
- Animated wall/floor ornates (field 0D 00 00): up to 10 cycled animated frames
- Activation sound fires at start of each animation cycle
- Alcove/shop glass/passive device positioning via GDAT_WALL_ORNATE__POSITION (0x05): 0=non-alcove, 1=alcove, 2=shop glass, 3=passive device
- Window ornate type (GDAT_WALL_ORNATE__WINDOW = 0x63)
- Do-not-flip flag (GDAT_WALL_ORNATE__DO_NOT_FLIP = 0x07)

Source: SKWin.GDAT2.InternalCodes.txt, defines.h:595-596, 622

### New Wall/Floor Ornate Fields

| Field | Value | Description |
|-------|-------|-------------|
| GDAT_WALL_ORNATE__IS_LADDER_UP | 0x11 | 1=ladder up, absent=ladder down |
| GDAT_WALL_ORNATE__RESPAWN_COOLDOWN | 0x12 | Gem vein respawn timer |
| GDAT_WALL_ORNATE__DATA_13 | 0x13 | Multi-word ornate data |
| GDAT_WALL_ORNATE__DATA_F0/F1/F2/FD | 0xF0-0xFD | Extended ornate variants |
| GDAT_WALL_ORNATE__DATA_6E/6F | 0x6E/0x6F | Used in DRAW_WALL |

Source: defines.h:604-631

---

## Secret Areas / Hidden Design

### Alcove System

GDAT_WALL_ORNATE__POSITION = 0x05 with values 1 (alcove), 2 (shop glass), 3 (passive device) indicates special wall positions that may contain hidden items or interactions.

Source: defines.h:596, SKWin.GDAT2.InternalCodes.txt

### Cryocell Chambers

Wall ornate type GDAT_WALL_ORNATE__CRYOCELL = 0x5B is a passive device that shows a champion portrait when triggered. This may be a secretinteraction in the dungeon.

Source: defines.h:548, SkWinCore.cpp:13102

### Item-Triggered Secrets

GDAT_WALL_ORNATE__SWITCH_ITEM (0x0E) and GDAT_WALL_ORNATE__IS_ITEM_TRIGGERED (0x0E): walls that only activate when a specific item is used on them.

Source: defines.h:610-613

### Water Spring / Rebirth Altar

GDAT_WALL_ORNATE__IS_WATER_SPRING = 0x0B and GDAT_WALL_ORNATE__IS_REBIRTH_ALTAR = 0x0C (DM2 beta) indicate special rest/healing locations.

Source: defines.h:604-605

---

## Level Design Comparison Table

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Level types | Indoor only | Outdoor + Indoor + Building |
| Level count | ~16 maps max | 30 levels max |
| Map size | Variable (up to 64) | DM2_V1_MAX_MAP_SIZE=64 |
| Outdoor rendering | None | Sky, ground, trees, buildings |
| Weather system | None | Clear/rain/fog/storm per zone |
| Day/night cycle | None | 0.0-1.0 float affects sky |
| Multi-floor buildings | No | Yes (type 2) |
| Extended ornaments | No | Yes (animated, ladder, alcove) |
| Door types | Standard | Standard + Dragon (0x0A) |

---

## GDAT Category Extensions for Level Design

DM2 extends GDAT categories from 0x1D to 0xF0 (240 categories), enabling per-dungeon customization of:
- Spell definitions (GDAT_CATEGORY_SPELL_DEF = 0x02)
- Creature AI behaviors (GDAT_CATEGORY_CREATURE_AI = 0x19)
- Champion NPC data (GDAT_CATEGORY_CHAMPIONS = 0x16)
- Teleporter squares (GDAT_CATEGORY_TELEPORTERS = 0x18)

Source: defines.h:432-439, SkGlobal.h:636-638

---

## STATUS: SOURCE-LOCKED
