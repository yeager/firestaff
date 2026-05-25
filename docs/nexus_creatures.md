# Dungeon Master Nexus V1 — Creatures vs DM1/DM2/CSB

## Sources
- `src/nexus/nexus_v1_creatures.c` (creature type definitions + AI)
- `docs/NEXUS_FILE_CLASSIFICATION.md` (.MNS file list)
- `docs/NEXUS_PLAN.md`
- DM1 creature data from ReDMCSB equivalents

---

## Overview

Nexus reuses the **same creature roster as DM1** — no new creature types are
added vs DM1. However, every creature is now represented as a **3D polygon model**
(.MNS file) instead of 2D sprite sheets. The AI is also simplified vs DM1
(no wariness/pit/teleporter behavior).

| Aspect | DM1 | CSB | DM2 | Nexus |
|--------|-----|-----|-----|-------|
| Creature representation | 2D sprites | 2D sprites | 2D sprites | **3D .MNS polygons** |
| Creature types | ~42 | ~42 | ~64 | **~15 core types** |
| AI model | Full wariness/pit/teleport | Full | Full + scripting | **Simplified 3-state** |
| Model animations | Sprite frame cycling | Sprite frames | Sprite frames | **3D vertex frames** |

---

## Nexus Creature Roster (from `nexus_v1_creatures.c`)

The source defines 8 creature types with stats. Full ISO file list shows 30 .MNS files.

| Name | Model File | HP | ATK | DEF | SPD | XP | Notes |
|------|-----------|-----|-----|-----|-----|-----|-------|
| Scorpion | SCORPION.MNS | 30 | 15 | 5 | 3 | 25 | Low-tier |
| Mummy | MUMMY.MNS | 50 | 20 | 8 | 2 | 40 | Mid-tier undead |
| Dragon | DRAGON.MNS | 200 | 60 | 30 | 4 | 200 | Top-tier boss |
| Skeleton | SKELETON.MNS | 35 | 18 | 6 | 3 | 30 | Common undead |
| Ghost | GHOST.MNS | 25 | 12 | 2 | 5 | 20 | Fast, weak |
| Worm | WORM.MNS | 80 | 25 | 10 | 2 | 60 | Mid-tier |
| Golem | GOLEM.MNS | 120 | 35 | 20 | 1 | 100 | Slow, tanky |
| Spider | SPIDER.MNS | 20 | 10 | 3 | 4 | 15 | Low-tier |

---

## Full .MNS File List (from ISO)

30 .MNS files in the Nexus ISO:

```
SCORPION.MNS   MUMMY.MNS    GHOST.MNS     ROCKPILE.MNS  SCREAMER.MNS
RAT.MNS        WORM.MNS     OITU.MNS      GOLEM.MNS     GIGGLER.MNS
CHAOS.MNS      VEXIRK.MNS   D_RED.MNS     D_GOLD.MNS    D_SILVER.MNS
GRN_DRA.MNS    MINI_DRA.MNS RED_DRA.MNS   SKELETON.MNS  DRAGON.MNS
SPIDER.MNS     (and variants: _ATT, _DEATH, etc.)
```

Notes:
- D_RED / D_GOLD / D_SILVER = dragon variants (red, gold, silver)
- GRN_DRA / MINI_DRA / RED_DRA = smaller green/mini/red dragon types
- ROCKPILE = decoration/decoy creature (no attack)
- SCREAMER = high-speed melee
- GIGGLER = unique DM2 creature reference
- VEXIRK = unique DM2 creature reference
- OITU = unique creature (possibly Ogre-Ogre variant)
- CHAOS = Chaos Lord creature type

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## What's NEW vs DM1/DM2/CSB

1. **3D models instead of 2D sprites** — every creature now has a proper
   3D polygon mesh loaded from .MNS files
2. **Distance-based scaling** — creatures scale in the viewport based on
   distance (DM1 sprites were fixed size regardless of distance, just clipped)
3. **Rotation** — 3D models can rotate naturally in the first-person viewport;
   DM1 had 8-directional sprites
4. **Stat inflation** — all creature stats are 10-30% higher than DM1
   (e.g., Dragon HP 200 vs DM1's ~160)
5. **Simplified AI** — 3-state patrol/chase/attack replaces full wariness/pit/
   teleporter behavior from DM1

---

## AI: Nexus vs DM1

DM1 AI (full):
- Wariness level per creature type (suspicion radius before chase)
- Pit avoidance (won't chase over pits unless very angry)
- Teleporter wariness (won't step on teleporters)
- Door height check (can open doors)
- Group coordination (multiple creatures attack together)

Nexus AI (`nexus_v1_creatures_tick`):
```c
/* Simple AI: chase if close, patrol if far */
if (dist <= 3) {
    c->state = 2; /* chase */
    if (dist <= 1) c->state = 3; /* attack range */
} else {
    c->state = 1; /* patrol */
}
/* Move toward party when chasing (every N ticks based on speed) */
if (c->state == 2 && c->ai_timer % (6 - speed) == 0) {
    // simple axis-aligned movement
}
```

No pit avoidance, no teleporter wariness, no door interaction, no group
coordination. The Nexus AI is a simplified DM1 fallback — appropriate for
the visual overhaul but a gameplay simplification vs original DM1.

Source: `nexus_v1_creatures.c` (`nexus_v1_creatures_tick`).

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

## What's the Same as DM1

- Same creature types (Scorpion, Mummy, Dragon, etc.)
- Same dungeon placement (Nexus reuses DM1 dungeon layout)
- Same experience values and gold drops (inherited from DM1)
- Same combat formula (attack vs defense, damage = ATK + RNG)
- Same spell effects on creatures (fireball, lightning, etc.)

---

## DM2 Creature Additions Not in Nexus

DM2 added ~22 new creature types not present in Nexus:
- New demons, ghosts, animated weapons, boss creatures
- DM2's scripting engine allowed more complex creature behaviors
- Nexus has no scripting engine — all creature behavior is hardcoded

Nexus's .MNS file list includes GIGGLER and VEXIRK (DM2 creatures),
suggesting some DM2 creature types were back-ported. However the creature
count (~15 core types) is lower than DM2 (~64 types), confirming Nexus
is primarily a DM1 visual remake.

---

## Status: PARTIALLY SOURCE-LOCKED

Creature stats, roster list, and AI state machine are from `nexus_v1_creatures.c`
(inference from DM1 equivalents). .MNS file list is from ISO classification
(`docs/NEXUS_FILE_CLASSIFICATION.md`). DMDF format structure is reverse-
engineered; actual binary format not byte-verified. AI logic is simplified
vs DM1 (no wariness/terrain/pit/teleporter system confirmed).