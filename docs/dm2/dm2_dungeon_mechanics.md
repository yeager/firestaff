# DM2 V1 — Dungeon Mechanics

## Source
-  — level loading, square access
-  — level types, max constants
-  — sky/weather rendering
-  — shop entry, hash-based file loading

---

## Level Type System (New vs DM1)

DM1 had only one level type (indoor dungeon). DM2 has three:

```c
typedef enum {
    DM2_LEVEL_OUTDOOR = 0,   // sky, ground, trees
    DM2_LEVEL_INDOOR,         // standard dungeon
    DM2_LEVEL_BUILDING,      // inside buildings (sub-level of outdoor)
} DM2_LevelType;
```

Max levels: 30 (), max dimension: 64 ().

---

## Dungeon Format

Enhanced DM1 dungeon.dat format with level type byte prepended:

| Field | Size | Notes |
|-------|------|-------|
| Level count | 2 bytes | Max 30 |
| Per level: type | 1 byte | outdoor/indoor/building |
| Per level: width | 1 byte | |
| Per level: height | 1 byte | |
| Per level: offset | 4 bytes | offset into raw data |
| Level data | variable | 5-bit square types (same as DM1) |

Square type access:
```c
int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d,
    int level, int x, int y)
{
    // returns raw square type & 0x1F — same 5-bit encoding as DM1
}
```

---

## Outdoor Levels (DM2 Signature Feature)

- Sky texture index () — unique sky art per outdoor zone
- Weather zones:  — separate from indoor dungeon weather
- Weather types: 0=clear, 1=rain, 2=fog, 3=storm
- Ground texture — different from dungeon floor tiles
- Tree density — outdoor decoration/pathfinding data

Outdoor sky color driven by time-of-day (see dm2_movement.md):
```c
if (cfg->weather >= 2) return 0xFF666666; // fog/storm = gray
```

---

## Building Levels

Buildings are a sub-type of outdoor level ():
- Stored in the same dungeon.dat with different level type flag
- Accessed by entering a building in an outdoor zone
- Indoor first-person renderer used inside buildings

---

## Triggers and Puzzles

Not yet fully reverse-engineered in V1 stubs. Based on SKULL.ASM context:
- Level transition triggers: stepping on special squares loads adjacent level
- Door squares: same encoding as DM1 (type 2 = door)
- Water/lava squares: same damage-over-time as DM1
- New DM2-specific: NPC interaction squares (shops, conversations)

---

## File Size Comparison

| File | DM1 | DM2 |
|------|-----|-----|
| GRAPHICS.DAT | 363 KB | **8.6 MB** (×24 — massive art increase) |
| DUNGEON.DAT | 33 KB | **39 KB** (+6 KB for outdoor levels) |

The large graphics file reflects the outdoor scenes and building art unique to DM2.

---

## Known DM2 File Hashes

```c
static const char *const dm2_dungeon_hashes[] = {
    6caccd7875009e82fe2e28e7f6d6adc0,  /* DM2 PC English DUNGEON.DAT */
    NULL
};
static const char *const dm2_graphics_hashes[] = {
    25247ede4dabb6a71e5dabdfbcd5907d,  /* PC English */
    b4d733576ea60c41737f79f212faf528,  /* PC French */
    e52ab5e01715042b16a4dcff02052e5d,  /* PC German/English JewelCase */
    NULL
};
```

---

## Source Evidence

> SKULL.ASM: DM2 dungeon loading routines
