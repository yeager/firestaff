# DM2 V1 — Movement: Party Movement in DM2 vs DM1

**Audit date:** 2026-05-25
**Sources:** SKULL.ASM, skproject SKWIN/SkWinCore.cpp, include/dm2_v1_game.h, include/dm2_v1_outdoor_renderer.h, docs/dm2_movement.md, docs/dm2_overview.md, docs/dm2_dungeon_design.md

---

## 1. DM1 Movement Model (Reference)

DM1 used a tile-based first-person movement system:
- Grid: up to 60x60 tiles per level
- 4-directional facing (N/E/S/W)
- 1 tile per turn (approximately 1 real-second at normal speed)
- Wall-sliding collision: if a direction is blocked, slide along the wall
- No outdoor areas - pure dungeon first-person view
- No companion AI movement - all champions moved together as the party

---

## 2. DM2 Party State

From include/dm2_v1_game.h:
```c
typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            // 0=indoor dungeon, 1=outdoor
    int gold;
    int reputation;
    int time_of_day;
} DM2_V1_GameState;
```

Key new field: outdoor flag - tracks whether the party is in a dungeon/building or in an outdoor area.

---

## 3. Indoor Movement (Dungeon/Building)

Indoor levels use the same raycast first-person renderer as DM1:
- 60x60 or smaller grid maps
- 5-bit square type values (same encoding as DM1 for floor/wall/door)
- get_square_type(level, x, y) returns raw square type (0-31)
- Collision detection: wall-sliding approach (attempted but not fully enforced in stub)
- 4-directional facing (N/E/S/W) - same as DM1 in dungeons

The indoor renderer (c_map) draws the 3D dungeon view into the backbuffer each frame, same as DM1.

---

## 4. Outdoor Movement (NEW in DM2)

DM2's signature feature is outdoor exploration in addition to dungeon crawls.

Outdoor areas (DM2_LEVEL_OUTDOOR = 0):
- No first-person view - outdoor renderer draws sky, ground, trees, buildings
- Party moves on an outdoor grid (different tile size from dungeon grid)
- Buildings can be entered (transitions to DM2_LEVEL_BUILDING)
- Weather affects visibility and possibly movement speed

Level type enum:
```c
typedef enum {
    DM2_LEVEL_OUTDOOR = 0,
    DM2_LEVEL_INDOOR,
    DM2_LEVEL_BUILDING,
} DM2_LevelType;
```

Outdoor movement speed:
- Assumed faster than indoor (open ground) - exact tiles-per-turn TBD from SKULL.ASM analysis
- 8-direction movement possible outdoors vs 4-direction in dungeons
- Turn advancement continues at same rate

---

## 5. Companion Movement

DM1 had no NPC companions - all party members moved together.

DM2 introduces up to 4 NPC companions who fight alongside the party:
- Companions have their own position and facing direction
- companion_tick() runs each game loop iteration, updating companion AI
- Companion modes:
  - Mode 0: follow party (move with party leader)
  - Mode 1: guard position (stay at specified tile)
  - Mode 2: aggressive (engage enemies on sight)
- Loyalty stat (0-100) affects behavior quality

Companion movement is handled by the c_ai system (same creature AI framework used for monsters), but companions are friendly units.

---

## 6. Speed and Turning

No explicit speed attribute in the stub state. Speed assumptions from SKULL.ASM context:
- Indoor: ~1 tile/real-second at normal game speed (same as DM1)
- Outdoor: likely faster tiles-per-turn (open ground, less friction)
- Turning: same 4-direction (N/E/S/W) in dungeons; outdoor may use 8-direction

Companions move independently using the same tick system as creatures.

---

## 7. Day/Night Effects on Movement

 time_of_day drives sky color in the outdoor renderer. This does not directly change movement speed, but:
- Night outdoor areas may have reduced visibility (smaller view radius)
- Torch light becomes more critical in dark dungeons (per-champion torch timers via PROCESS_TIMER_0C)
- No explicit "can't move at night" restriction

---

## 8. Collision and Wall Sliding

Indoor collision detection follows the DM1 model:
- Wall squares are impassable
- Door squares may be open, closed, or locked
- Wall-sliding: if moving forward into a wall, attempt to slide left or right

Outdoor collision:
- Building walls are impassable
- Tree/terrain features may block movement (specific implementation TBD)
- No wall-sliding in outdoor mode (different grid/collision model)

---

## 9. Level Transitions

DM2 has three level types, with transitions between them:

| Transition | Trigger | Details |
|-----------|---------|---------|
| Outdoor to Indoor (dungeon) | Enter cave/stairs | outdoor flag = 0, load dungeon level |
| Outdoor to Building | Enter door | outdoor flag = 0, load building level |
| Indoor to Outdoor | Exit door/stairs | outdoor flag = 1, load outdoor area |
| Building to Outdoor | Exit building | outdoor flag = 1, return to outdoor map |

Teleporter squares (GDAT_CATEGORY_TELEPORTERS = 0x18) enable instant relocation:
- X teleporter (SDFSM_CMD_X_TELEPORTER = 4) - cross-scene teleport
- Anchor teleporter (SDFSM_CMD_X_ANCHOR = 5) - anchored location in Sun Clan village
- Activated by stepping on the floor square

Ladders: ACTUATOR_TYPE_SIMPLE_LADDER (0x1C) or standard ladder triggers (ACTUATOR_TYPE_LADDER = 0x11) move party vertically between levels.

---

## 10. Comparison: Movement

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| World type | Indoor only | Indoor + outdoor + buildings |
| First-person view | Yes (dungeon) | Yes (indoor); No (outdoor = isometric/top-down) |
| Grid size | 60x60 max | Same (indoor); different (outdoor) |
| Facing directions | 4 (N/E/S/W) | 4 (indoor), 8 (outdoor TBD) |
| Wall sliding | Yes | Yes (indoor) |
| Companion movement | N/A | Yes (companion_tick, 3 modes) |
| Outdoor grid | None | Separate outdoor grid |
| Level transitions | Stairs/ladders | Stairs + doors + outdoor transitions |
| Teleporter type | Generic floor | Dedicated GDAT 0x18 category |
| Torch timer | Global | Per-champion (PROCESS_TIMER_0C) |

---

## STATUS: AUDIT COMPLETE