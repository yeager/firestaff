# DM2 V1 — Magic System

## Source
-  — tech/magic item resolver
-  — affinity types and item struct

---

## DM2's Unique Feature: Tech/Magic Hybrid System

DM1 had only magic. DM2 adds a technology layer and — critically — **hybrid items** that require both.

---

## Item Affinities

```c
typedef enum {
    DM2_ITEM_MAGIC = 0,   // traditional spells, mana-powered
    DM2_ITEM_TECH,        // guns, bombs, mechanical devices
    DM2_ITEM_HYBRID,      // combines tech + magic (new in DM2)
} DM2_ItemAffinity;
```

---

## Tech Items (New in DM2)

Guns and bombs are tech items:
- Require 
- Power source: battery (charges) or manual
- Ammo required (crossbow bolts, gunpowder, bomb components)

```c
typedef struct {
    int item_id;
    DM2_ItemAffinity affinity;
    int tech_level;
    int magic_level;
    int power_source; /* 0=manual, 1=battery, 2=mana, 3=hybrid */
    int charges;
} DM2_V1_TechMagicItem;
```

Power cost by source:
| Source | Cost |
|--------|------|
| 0 — manual | 0 (free) |
| 1 — battery | 1 charge |
| 2 — mana |  |
| 3 — hybrid |  |

---

## Magic Items (Extended from DM1)

DM2 magic items use mana as power source (source=2). Requirements:
- 
- Mana cost =  per use

---

## Hybrid Items (New in DM2)

Hybrid items (source=3) require **both** stats:
```c
int dm2_v1_item_can_use(const DM2_V1_TechMagicItem *item,
    int champion_tech, int champion_magic)
{
    switch (item->affinity) {
        case DM2_ITEM_TECH:   return champion_tech >= item->tech_level;
        case DM2_ITEM_MAGIC: return champion_magic >= item->magic_level;
        case DM2_ITEM_HYBRID: return champion_tech >= item->tech_level &&
                                      champion_magic >= item->magic_level;
        default: return 0;
    }
}
```

This is a fundamental DM2 innovation — champions must level both tech and magic to unlock the most powerful items.

---

## Companion Magic

Companions () do not cast spells — they are melee/ranged NPCs only. Magic is champion-exclusive.

---

## New Spell Types (from SKULL.ASM context)

Based on SKULL.ASM evidence, new DM2 spells include:
- Weather spells (cause rain, fog — affects outdoor combat)
- Building/door control spells
- Extended creature summoning

Exact spell list not yet extracted in V1 stubs.

---

## Source Evidence

> SKULL.ASM: tech/magic item routines
