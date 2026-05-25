# DM2 V1 — Movement Mechanics

## Source
-  — party state (position, direction, outdoor flag)
-  — DM2_V1_GameState
-  — outdoor sky/weather renderer
-  — weather and time-of-day config
-  — level geometry access

---

## Party State

```c
typedef struct {
    int party_x, party_y, party_dir;  // grid position + facing direction
    int current_level;
    int outdoor;            // 0=dungeon, 1=outdoor
    int gold;
    int reputation;
    int time_of_day;        // in minutes from midnight (720 = noon)
} DM2_V1_GameState;
```

Key differences from DM1:
-  flag — DM2 tracks whether party is indoors or outdoors (DM1 had no outdoor)
-  — 1440-minute clock, not present in DM1
-  — NPC attitude, not in DM1

---

## Indoor Movement

Indoor levels use the same raycast first-person renderer as DM1:
- 60×60 or smaller grid maps
- 5-bit square type values (same encoding as DM1 for floor/wall/door)
-  — returns raw square type (0–31)

Collision detection: same wall-sliding approach as DM1 (attempted but not enforced in stub).

---

## Outdoor Movement

DM2's signature feature: outdoor exploration in addition to dungeons.

Outdoor maps are flagged as  in the dungeon loader:
```c
typedef enum {
    DM2_LEVEL_OUTDOOR = 0,
    DM2_LEVEL_INDOOR,
    DM2_LEVEL_BUILDING,
} DM2_LevelType;
```

Outdoor differences:
- **No first-person view** — outdoor renderer draws sky, trees, buildings
- Party moves on an outdoor grid (different from dungeon grid)
- Buildings can be entered (transitions to )
- Weather affects movement (rain/fog/storm conditions, tracked in )

---

## Speed and Turning

No explicit speed attribute in the stub state. Assumptions based on SKULL.ASM context:
- Outdoor movement: likely faster tiles-per-turn than indoor (open ground)
- Indoor: same tile-per-turn as DM1 (~1 tile/real-second at normal game speed)
- Turning: same 4-direction (N/E/S/W) as DM1 in dungeons; outdoor may use 8-direction

---

## Day/Night Cycle

```c
state->time_of_day = 720;  // noon, in minutes
```

Time of day drives sky color in the outdoor renderer:
-  → dawn gradient
-  → day sky (blue-gray; rain adds gray tint)
-  → dusk → night (dark blue-black)

Outdoor visibility may degrade at night, but exact mechanics not yet audited.

---

## Source Evidence

> SKULL.ASM: outdoor viewport, sky gradient, building draw
