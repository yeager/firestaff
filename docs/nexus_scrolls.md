# Nexus V1 — Scrolls and Potions

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_magic.c`, `include/firestaff_item_encyclopedia.h`, `src/ui/firestaff_item_encyclopedia.c`, `include/firestaff_inventory_ui.h`, `docs/spells_items.md` (DM1 V1), `docs/dm2_potions.md`, `docs/nexus_potions.md`

---

## 1. Overview

Nexus V1 inherits the DM1 flask/potion system as the primary consumable magic
mechanism. Scrolls appear as a category in the item encyclopedia but have no
active implementation — no scroll casting code, no scroll item usage function,
no rune-parsing connection to the spell system. Potions are split into two
models: flask-crafting (10 spells, two-step) and pre-made dungeon potions
(4 types, direct consume).

---

## 2. Scrolls — Category Without Implementation

### Item Category

```c
typedef enum {
    FS_ITEM_CAT_WEAPON = 0,
    FS_ITEM_CAT_ARMOR,
    FS_ITEM_CAT_POTION,
    FS_ITEM_CAT_SCROLL,   /* <-- defined but unused */
    FS_ITEM_CAT_CONTAINER,
    FS_ITEM_CAT_MISC,
    FS_ITEM_CAT_KEY,
} FS_ItemCategory;
```

Source: `include/firestaff_item_encyclopedia.h`

### Scroll Item Entry

The item encyclopedia defines exactly one scroll entry:

| Name | Category | Weight |
|---|---|---|
| Scroll | `FS_ITEM_CAT_SCROLL` | 1 |

This is a generic placeholder — no spell name, no rune sequence, no effect
description. It cannot be used for anything. The single entry exists only
to populate the encyclopedia UI.

Source: `src/ui/firestaff_item_encyclopedia.c`

### Scroll Casting — Not Implemented

Unlike DM1 where scrolls are usable items containing spell charges, Nexus V1
has no scroll usage path:
- No `nexus_v1_use_scroll()` function
- No scroll charge tracking
- No scroll casting from inventory
- No rune lookup from scroll items

The `FS_ITEM_CAT_SCROLL` category exists as a skeleton for future work.

Source: grep across `src/nexus/` — no scroll functions found

---

## 3. Potions — Two Systems

### 3a. Flask-Crafting Potions (Two-Step, 10 Spells)

This is the primary potion system, inherited directly from DM1.

**Step 1 — Casting (Spell):**
Champion must have:
- Empty flask in action hand (C195/C20 equivalent)
- Spell known with correct rune sequence
- Sufficient mana
- Skill level (Priest or Wizard) >= required power

On success: empty flask consumed, potion item created.

**Step 2 — Consuming (Use):**
Potion item used from inventory → stat effect applied → flask returned as empty.

The 10 flask-crafting spells:

| Spell | Runes | Effect | Stat Modified |
|---|---|---|---|
| STRENGTH POTION | Ful Bro Ku | +STR temp | Strength |
| WISDOM POTION | Ya Bro Dain | +WIS temp | Wisdom |
| VITALITY POTION | Ya Bro Neta | +VIT temp | Vitality |
| HEALTH POTION | Vi | Heal missing HP | Health |
| CURE POISON POTION | Vi Bro | Remove poison | Poison status |
| DEXTERITY POTION | Oh Bro Ros | +DEX temp | Dexterity |
| MANA POTION | Zo Bro Ra | Restore mana | Mana |
| STAMINA POTION | Ya | Restore stamina | Stamina |
| POISON POTION | Zo Ven | Poison target | Poison status |
| SHIELD POTION | Ya Bro | Shield defense | Defense |

**Key properties:**
- Flask is consumed on crafting, returned on consume
- Spell must be known (rune combination must be registered)
- Form rune is always `Bro` (Bror) for all potion spells
- Alignment rune differentiates healing vs damage vs stat-boost variants

Source: `docs/spells_items.md` (DM1 V1), `docs/nexus_potions.md`

### 3b. Pre-Made Dungeon Potions (Direct Consume, 4 Types)

These bypass the flask system entirely:

| Name | Category | Weight | Effect |
|---|---|---|---|
| Health Potion | `FS_ITEM_CAT_POTION` | 2 | Restore health directly |
| Mana Potion | `FS_ITEM_CAT_POTION` | 2 | Restore mana directly |
| Stamina Potion | `FS_ITEM_CAT_POTION` | 2 | Restore stamina directly |
| Antidote | `FS_ITEM_CAT_POTION` | 2 | Remove poison directly |

Consuming one of these items does NOT return a flask — the bottle is empty
and discarded. Weight 2 per item is the same for all four.

These represent dungeon treasure (found in dungeon rooms, not crafted).

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 4. Flask Items

### Water Flask

| Property | Value |
|---|---|
| Category | `FS_ITEM_CAT_MISC` |
| Weight | 4 |
| Notes | Filled at dungeon fountains |

Filling flow (inherited from DM1):
```
Empty flask + Fountain interaction -> Water Flask (weight 4)
Water Flask + Consume -> Water restored, empty flask returned
```

Empty flask (C195/C20): no separate item entry, implied by "flask consumed" event.

Source: `src/ui/firestaff_item_encyclopedia.c`, `dm1_v1_fountain_interaction_pc34_compat.c`

### Empty Flask in Inventory

The `Nexus_V1_Champion` inventory is 30 `uint8_t` item indices:
```c
uint8_t inventory[30];  /* item indices — 30 slots, not DM1's 12 */
```

The champion roster initialization does not pre-populate any flask items.
Champions start without any items in inventory.

Source: `include/nexus_v1_champions.h`, `src/nexus/nexus_v1_champions.c`

---

## 5. Inventory Slot Count Difference

| Game | Inventory Slots |
|---|---|
| DM1 | 12 |
| Nexus V1 | 30 |

Nexus V1 gives champions 30 item slots vs DM1's 12. This is a significant
 QoL expansion — flask-crafting requires only 1 slot for the flask/potion,
 so the extra space makes囤积 less necessary.

Source: `include/nexus_v1_champions.h`

---

## 6. Comparison with DM2

| Feature | DM2 | Nexus V1 |
|---|---|---|
| Potion system | Per-charge battery model | Flask-crafting + pre-made |
| Scroll system | Implemented | Stub (category exists, no code) |
| Wand items | Implemented | No |
| Tech potions | Yes (battery-powered) | No |
| Fountain filling | No | Yes (DM1-style) |
| Two-step flask crafting | No | Yes |
| Scroll casting | Yes | No |

Source: `docs/dm2_potions.md`

---

## 7. Missing Implementation: Rune Parsing for Flask Spells

The flask-crafting spell system depends on matching a rune sequence
(e.g., Ful+Bro+Ku = STRENGTH POTION) to a known spell. This is NOT
implemented in Nexus V1:

```c
int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int elem, int form, int align) {
    int cost, skill_req;
    (void)form; (void)align; /* FUTURE: full rune combination parsing. */
    /* ... mana/skill checks only ... */
}
```

The form and alignment rune indices are discarded. The rune-word to spell
lookup must be implemented before any flask-crafting potion can actually be
created through the spell system.

Source: `src/nexus/nexus_v1_magic.c`

---

## Status: PARTIALLY SOURCE-LOCKED

- Flask-crafting system: **inherited** from DM1 patterns
- Pre-made dungeon potions: **source-locked** — `firestaff_item_encyclopedia.c`
- Scroll item category: **stub** — one generic entry, no usage code
- Scroll casting: **NOT IMPLEMENTED**
- Fountain filling: **inherited** from DM1
- Inventory: **source-locked** — 30 slots in `nexus_v1_champions.h`
- Rune parsing for flask spells: **NOT IMPLEMENTED** — marked FUTURE
