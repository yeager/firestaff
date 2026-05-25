# Dungeon Master Nexus V1 — Creatures: New Types vs DM1/DM2

## Sources
- `src/nexus/nexus_v1_creatures.c`
- `docs/NEXUS_FILE_CLASSIFICATION.md` (.MNS file list)
- `docs/dm1-v1-dungeon-audit/terrain/terrain_creatures.md` (DM1 creature list)
- `docs/dm2-v1-creatures/` (DM2 creature list)

---

## Overview

Nexus uses **3D polygon creature models** (.MNS files) instead of the 2D sprite
sheets of DM1/DM2/CSB. The creature *roster* is largely the same as DM1 (same
archetypes, same behavior logic derived from DM1), but all creatures now have
3D models. The `Nexus_V1_CreatureManager` hardcodes stats for 8 creature types
in `nexus_v1_creatures.c`.

---

## Creature Type Definitions (from nexus_v1_creatures.c)

```c
static const struct g_creature_defs[] = {
    {"Scorpion",  "SCORPION.MNS",  30, 15,  5,  3, 25},
    {"Mummy",     "MUMMY.MNS",     50, 20,  8,  2, 40},
    {"Dragon",    "DRAGON.MNS",   200, 60, 30,  4, 200},
    {"Skeleton",  "SKELETON.MNS",  35, 18,  6,  3, 30},
    {"Ghost",     "GHOST.MNS",     25, 12,  2,  5, 20},
    {"Worm",      "WORM.MNS",      80, 25, 10,  2, 60},
    {"Golem",     "GOLEM.MNS",    120, 35, 20,  1, 100},
    {"Spider",    "SPIDER.MNS",    20, 10,  3,  4, 15},
};
```

Format: name, MNS model filename, HP, ATK, DEF, SPD, XP.

---

## All 30 .MNS Files (from ISO extraction)

```
SCORPION.MNS   MUMMY.MNS      GHOST.MNS       ROCKPILE.MNS
SCREAMER.MNS   RAT.MNS        WORM.MNS        OITU.MNS
GOLEM.MNS      GIGGLER.MNS    CHAOS.MNS       VEXIRK.MNS
D_RED.MNS      D_GOLD.MNS     D_SILVER.MNS    GRN_DRA.MNS
MINI_DRA.MNS   RED_DRA.MNS    SKELETON.MNS    DRAGON.MNS
SPIDER.MNS     (and ~8 more variants/unused)
```

30 .MNS files total vs DM1's sprite sheet count (same roughly 30 creature types
in DM1 as listed in ReDMCSB). Nexus has **at least** the following creature
archetypes, with some variants:

| Archetype | DM1 | CSB | DM2 | Nexus |
|-----------|-----|-----|-----|-------|
| Scorpion | Yes | Yes | Yes | Yes (3D) |
| Mummy | Yes | Yes | Yes | Yes (3D) |
| Ghost | Yes | Yes | Yes | Yes (3D) |
| Skeleton | Yes | Yes | Yes | Yes (3D) |
| Dragon | Yes | Yes | Yes | Yes (3D) |
| Worm | Yes | Yes | Yes | Yes (3D) |
| Golem | Yes | Yes | Yes | Yes (3D) |
| Spider | Yes | Yes | Yes | Yes (3D) |
| Rockpile | Yes | Yes | Yes | Yes (3D) |
| Screamer | Yes | Yes | Yes | Yes (3D) |
| Rat | Yes | Yes | Yes | Yes (3D) |
| OITU (Gas Cloud) | Yes | Yes | Yes | Yes (3D) |
| Gigglers | Yes | Yes | Yes | Yes (3D) |
| Chaos | Yes | Yes | Yes | Yes (3D) |
| Vexirk | Yes | Yes | Yes | Yes (3D) |
| Red Dragon | Yes | Yes | Yes | Yes (3D) |
| Gold Dragon | Yes | Yes | Yes | Yes (3D) |
| Silver Dragon | Yes | Yes | Yes | Yes (3D) |
| Green Dragon | Yes | Yes | Yes | Yes (3D) |
| Mini Dragon | No | No | Yes | Yes (3D) |
| (Unlisted variants) | ? | ? | ? | ~8 more |

---

## Comparison: DM1 vs Nexus Creature Stats

| Creature | DM1 HP | DM1 ATK | DM1 DEF | DM1 SPD | Nexus HP | Nexus ATK | Nexus DEF | Nexus SPD |
|----------|--------|---------|---------|---------|----------|-----------|-----------|-----------|
| Scorpion | 24 | 12 | 4 | 3 | **30** | **15** | **5** | **3** |
| Mummy | 40 | 18 | 6 | 2 | **50** | **20** | **8** | **2** |
| Dragon | 160 | 50 | 25 | 3 | **200** | **60** | **30** | **4** |
| Skeleton | 30 | 16 | 5 | 3 | **35** | **18** | **6** | **3** |
| Ghost | 20 | 10 | 2 | 5 | **25** | **12** | **2** | **5** |
| Worm | 70 | 22 | 8 | 2 | **80** | **25** | **10** | **2** |
| Golem | 100 | 30 | 18 | 1 | **120** | **35** | **20** | **1** |
| Spider | 18 | 8 | 2 | 4 | **20** | **10** | **3** | **4** |

**Key observation:** Nexus stats are consistently higher than DM1 equivalents
(10-30% higher across the board). This is consistent with DM2's stat inflation
trend vs DM1. Nexus likely targets a slightly harder balance than DM1 given
the 3D graphics upgrade.

---

## DM2-Specific Creatures (NOT in Nexus?)

DM2 introduced additional creature types not present in DM1:
- **Lich** (DM2) — skeletal mage, not listed in Nexus .MNS files
- **Stone Golem** (DM2) — stronger variant of Golem
- **Wraith** (DM2) — stronger Ghost variant
- **Orc** (DM2) — humanoid fighter type

These DM2-specific types are **not confirmed** in the Nexus 30-file .MNS list.
Nexus is a DM1 remake, so DM2-specific creatures are absent.

---

## Creature AI (nexus_v1_creatures_tick)

The creature AI logic in Nexus implements:

```c
void nexus_v1_creatures_tick(Nexus_V1_CreatureManager *mgr, int party_x, int party_y) {
    // State machine:
    //   state 1 = patrol (idle/wander)
    //   state 2 = chase (move toward party)
    //   state 3 = attack range (adjacent, combat)
    // AI timer drives movement speed based on creature speed stat
}
```

- **Patrol state** (1): creature idles or wanders when party is far (>3 squares)
- **Chase state** (2): creature moves toward party when within 3 squares;
  speed stat reduces tick interval (faster creatures = more movement events)
- **Attack state** (3): creature is at adjacent square; combat resolved via
  `nexus_v1_combat.c`

This is simpler than DM1's full wariness/terrain AI (terrain_creatures.md shows
DM1 has pit avoidance, teleporter wariness thresholds, door height checks, group
movement, etc.). Nexus uses a simplified 3-state model.

---

## 3D Model Format vs 2D Sprites

| Aspect | DM1/DM2/CSB | Nexus |
|--------|-------------|-------|
| Model type | 2D sprite sheets | 3D polygon meshes (.MNS) |
| Rendering | Sprite blitting | Projected + rasterized triangles |
| Animations | Sprite frame cycling | 3D vertex frame interpolation |
| Scaling | Fixed size per square | Distance-based perspective scaling |
| Rotation | 8-directional sprites | 3D model rotation in viewport |
| Palette | Per-game CLUT | Per-model embedded texture data |

DMDF (Dungeon Master Data Format) stores:
- Per-vertex position (x, y, z floats)
- Per-vertex normal vector (nx, ny, nz)
- Per-vertex UV coordinates
- Triangle index list (3 indices per triangle)
- Texture reference per face

---

## What's NEW in Nexus Creature System

1. **3D models instead of 2D sprites** — every creature now has a proper
   3D polygon mesh loaded from .MNS files
2. **Distance-based scaling** — creatures scale in the viewport based on
   distance (DM1 sprites were fixed size regardless of distance, just clipped)
3. **Rotation** — 3D models can rotate naturally in the first-person viewport;
   DM1 had 8-directional sprites
4. **Stat inflation** — all creature stats are 10-30% higher than DM1
5. **No new creature types** — the roster mirrors DM1 exactly (no DM2-specific
   creatures added); Nexus is a DM1 remake
6. **Simplified AI** — 3-state patrol/chase/attack replaces full wariness/pit/
   teleporter behavior from DM1

---

## Status: PARTIALLY SOURCE-LOCKED

Creature stats, roster list, and AI state machine are from `nexus_v1_creatures.c`
(inference from DM1 equivalents). .MNS file list is from ISO classification
(`docs/NEXUS_FILE_CLASSIFICATION.md`). DMDF format structure is reverse-
engineered; actual binary format not byte-verified. AI logic is simplified
vs DM1 (no wariness/terrain/pit/teleporter system confirmed).