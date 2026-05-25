# DM2 V1 — Game Loop & Timing

## Source
-  — state init, dungeon load, shop
-  — companion AI tick
-  — game state with time_of_day
-  — time-of-day sky color
-  — save/load

---

## Game State

```c
typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            // 0=indoor dungeon, 1=outdoor
    int gold;
    int reputation;
    int time_of_day;        // minutes from midnight (0–1439)
    const char *data_dir;
} DM2_V1_GameState;
```

---

## Turn System

DM2 maintains the tile-based turn system from DM1 (1 action = 1 turn) but adds:

1. **Outdoor turn flow** — different renderer update vs indoor
2. **Companion tick** —  called each iteration for NPC AI
3. **Time of day advance** —  increments as turns pass (rate TBD)
4. **Weather timer** — weather zones may have duration timers

No explicit tick rate constants in V1 stub code — derived from SKULL.ASM runtime analysis.

---

## Timing Differences from DM1

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Turn pacing | ~1 real-second/tile | Same (indoor); outdoor TBD |
| Time of day | None | 1440-minute cycle advances with turns |
| Weather | None | Weather zones update each tick |
| Companion AI | N/A |  runs each loop |
| Save format | DM1 struct | DM2 struct (larger — includes gold, rep, outdoor) |

---

## Initialization

```c
void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir) {
    state->party_x = 15;
    state->party_y = 15;
    state->party_dir = 0;
    state->gold = 100;
    state->time_of_day = 720;  // noon
}
```

Starting gold: 100g (vs DM1's 0g). Starting position: center of level 15.

---

## Dungeon Load

Hash-based (not path-based) asset resolution:
```c
if (asset_find_by_md5_list(state->data_dir, dm2_dungeon_hashes, ...))
```

Until zip extraction is implemented, dungeon load will fail if only zip archives are present.

---

## Companion Tick

```c
void dm2_v1_companion_tick(DM2_V1_CompanionState *state) {
    (void)state;  // stub: AI behavior not yet implemented
}
```

Companion AI states: follow (0), guard (1), aggressive (2). Tick manages position updates and combat engagement.

---

## Save/Load

```c
int dm2_v1_save_game(const char *path, const void *state, int size);
int dm2_v1_load_game(const char *path, void *state, int max_size);
```

Format is DM2-specific — not compatible with DM1 save files. Includes outdoor flag, gold, reputation, and companion state.

---

## Source Evidence

> SKULL.ASM: DM2 save/load format  
> SKULL.ASM: NPC companion AI, loyalty, trading  
> SKULL.ASM: outdoor viewport, sky gradient, building draw  
> SKULL.ASM: DM2 dungeon loading routines

