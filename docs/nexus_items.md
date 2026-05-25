# Nexus V1 — Item System: Categories and Types

**Audit date:** 2026-05-25
**Sources:** `include/firestaff_item_encyclopedia.h`, `src/ui/firestaff_item_encyclopedia.c`, `include/firestaff_inventory_ui.h`, `src/nexus/nexus_v1_champions.c`, `src/nexus/nexus_v1_combat.c`, `docs/nexus_combat_items.md`

---

## 1. Item Categories

Nexus V1 defines 7 item categories via `FS_ItemCategory`:

| Enum Value | Category | Notes |
|---|---|---|
| `FS_ITEM_CAT_WEAPON` | Weapons | Melee, ranged, thrown |
| `FS_ITEM_CAT_ARMOR` | Armor | Body, shields, helmets, boots |
| `FS_ITEM_CAT_POTION` | Potions | Consumable stat boosters |
| `FS_ITEM_CAT_SCROLL` | Scrolls | Spell containers (CSB/DM2 carried forward) |
| `FS_ITEM_CAT_CONTAINER` | Containers | Chests, sacks |
| `FS_ITEM_CAT_MISC` | Misc | Torch, compass, food, flasks, etc. |
| `FS_ITEM_CAT_KEY` | Keys | Gold, silver, skeleton keys |

Source: `include/firestaff_item_encyclopedia.h`, `src/ui/firestaff_item_encyclopedia.c`

---

## 2. Item Entry Structure

```c
typedef struct {
    const char *name;
    FS_ItemCategory category;
    const char *description;
    int weight;
    int attack;     /* 0 for non-weapons */
    int defense;    /* 0 for non-armor */
} FS_ItemEntry;
```

This is a **shared Firestaff UI structure**, not specific to Nexus V1. It is used
by the item encyclopedia UI to display item stats. The Nexus V1 core (in `src/nexus/`)
does not use this struct — it passes raw `weapon_power` and `defense` integers directly
to the combat function.

Source: `include/firestaff_item_encyclopedia.h`

---

## 3. Complete Item Roster

The item encyclopedia (`src/ui/firestaff_item_encyclopedia.c`) defines the following
representative items, drawn from the general DM franchise:

### Weapons

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Falchion | 30 | 0 | 18 |
| Rapier | 24 | 4 | 14 |
| Mace | 32 | 0 | 30 |
| Club | 16 | 0 | 20 |
| Staff | 10 | 2 | 12 |
| Sword | 34 | 2 | 22 |
| Axe | 36 | 0 | 26 |
| Dagger | 14 | 0 | 6 |
| Arrow | 10 | 0 | 1 |
| Slayer | 50 | 6 | 28 |
| Vorpal Blade | 48 | 4 | 20 |
| Firestaff | 40 | 10 | 16 |

### Armor

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Leather Jerkin | 0 | 8 | 8 |
| Mail Aketon | 0 | 14 | 24 |
| Plate Armor | 0 | 22 | 40 |
| Shield | 0 | 12 | 16 |
| Helmet | 0 | 6 | 10 |
| Boots | 0 | 4 | 6 |

### Potions

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Health Potion | 0 | 0 | 2 |
| Mana Potion | 0 | 0 | 2 |
| Stamina Potion | 0 | 0 | 2 |
| Antidote | 0 | 0 | 2 |

### Scrolls

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Scroll | 0 | 0 | 1 |

### Containers

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Chest | 0 | 0 | 10 |
| Sack | 0 | 0 | 2 |

### Keys

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Gold Key | 0 | 0 | 1 |
| Silver Key | 0 | 0 | 1 |
| Skeleton Key | 0 | 0 | 1 |

### Misc

| Name | Attack | Defense | Weight |
|---|---|---|---|
| Torch | 8 | 0 | 6 |
| Compass | 0 | 0 | 2 |
| Rabbit Foot | 0 | 0 | 1 |
| Corn | 0 | 0 | 3 |
| Water Flask | 0 | 0 | 4 |

---

## 4. No Dedicated Nexus V1 Item File

Nexus V1 has **no `nexus_v1_items.c`** source file. Item logic is distributed:

| File | Role |
|---|---|
| `src/nexus/nexus_v1_combat.c` | Weapon power and defense in combat formula |
| `src/nexus/nexus_v1_magic.c` | Flask/potion crafting (inherited from DM1) |
| `src/nexus/nexus_v1_champions.c` | Champion struct with inventory array |
| `src/ui/firestaff_item_encyclopedia.c` | UI-facing item encyclopedia (shared) |
| `include/firestaff_inventory_ui.h` | Inventory UI layout and item structures |

The `Nexus_V1_Champion` struct holds items as:
```c
uint8_t inventory[30];  /* item indices — 30 slots, not DM1s 12 */
```

Source: `include/nexus_v1_champions.h` (30-slot inventory, differs from DM1s 12-slot grid)

---

## 5. Category Access Functions

```c
int fs_item_encyclopedia_count(void);
const FS_ItemEntry *fs_item_encyclopedia_get(int index);
int fs_item_encyclopedia_count_in_category(FS_ItemCategory cat);
const char *fs_item_category_name(FS_ItemCategory cat);
```

These functions provide read-only access to the static item encyclopedia.
They are used by the UI layer for the in-game item browser/encyclopedia.

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 6. Nexus V1 vs DM1 vs DM2

| Aspect | DM1 | Nexus V1 | DM2 |
|---|---|---|---|
| Item categories | 6 | 7 (scrolls added) | 7+ (scrolls, tech) |
| Inventory slots/champion | 12 | **30** (`uint8_t[30]`) | 12 |
| Hand slots | 2 | 2 | 2 |
| Flask system | Yes | Yes (inherited) | Yes (inherited) |
| Scroll system | No | **Yes** (carried from CSB) | Yes |
| Tech items | No | No | Yes |
| Gun/bomb weapons | No | No | Yes |
| New rings/amulets | No | No | Yes |
| Item encyclopedia UI | No | Yes | Yes |

**Notable:** Nexus V1's `inventory[30]` array is **larger than DM1's 12 slots**.
This may be a Firestaff extension of the champion struct, or it may include
hand/ring/amulet slots as part of the 30-element array. No dedicated
Nexus V1 inventory-management code was found in the source audit.

---

## Status: PARTIALLY SOURCE-LOCKED

- Category enum and item roster: **source-locked** — `firestaff_item_encyclopedia.c`
- Champion inventory array: **source-locked** — `nexus_v1_champions.h` (`uint8_t inventory[30]`)
- Combat integration (weapon_power, defense): **source-locked** — `nexus_v1_combat.c`
- Flask/potion magic: **inherited** from DM1 patterns in `nexus_v1_magic.c`
- Item encyclopedia UI functions: **source-locked** — `firestaff_item_encyclopedia.c`
- **NOTE:** 30-slot inventory vs DM1's 12-slot — Firestaff extension, needs verification
