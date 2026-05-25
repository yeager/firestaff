# Nexus V1 — Potion/Flask System

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_magic.c`, `include/firestaff_item_encyclopedia.h`, `src/ui/firestaff_item_encyclopedia.c`, `include/firestaff_inventory_ui.h`, `docs/spells_items.md` (DM1 V1), `docs/dm2_potions.md`

---

## 1. Overview

Nexus V1 inherits the DM1 flask-based potion system unchanged. There is
no dedicated items file — the potion logic lives in the magic system.
No scroll/wand usage system was found in the Nexus V1 source; flask-based
potions are the only consumable magic mechanism.

Source: `src/nexus/nexus_v1_magic.c` (entire file is only 39 lines)

---

## 2. Flask/Potion Item Entries

The item encyclopedia defines two flask-related entries:

| Name | Category | Weight |
|---|---|---|
| Water Flask | `FS_ITEM_CAT_MISC` | 4 |
| Torch | `FS_ITEM_CAT_MISC` | 6 |

The empty flask (C195/C20 in DM1) is the core reagent but does not appear
as a separate encyclopedia entry. Water Flask is what you get when you
fill an empty flask at a fountain.

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 3. Flask-Based Potion Spells (10 Spells)

Same 10 spells from DM1 require an empty flask in hand to cast:

| Spell | Symbols | Effect | Flask Consumed |
|---|---|---|---|
| STRENGTH POTION | Ful Bro Ku | Temporary STR boost | Yes |
| WISDOM POTION | Ya Bro Dain | Temporary WIS boost | Yes |
| VITALITY POTION | Ya Bro Neta | Temporary VIT boost | Yes |
| HEALTH POTION | Vi | Heal missing HP | Yes |
| CURE POISON POTION | Vi Bro | Remove poison | Yes |
| DEXTERITY POTION | Oh Bro Ros | Temporary DEX boost | Yes |
| MANA POTION | Zo Bro Ra | Restore mana | Yes |
| STAMINA POTION | Ya | Restore stamina | Yes |
| POISON POTION | Zo Ven | Poison target | Yes |
| SHIELD POTION | Ya Bro | Shield defense | Yes |

Casting requires: known spell + empty flask in action hand + sufficient mana
+ required skill level. On success: flask is consumed, potion item is created,
then potion is consumed separately for the effect.

Source: `docs/spells_items.md` (DM1 V1), `docs/dm2_potions.md`

---

## 4. Magic System — Mana Cost Formula

```c
static const int g_elem_cost_mult[] = {2, 3, 2, 4, 3, 3};

int nexus_v1_spell_mana_cost(int power, int elem) {
    if (power < 0 || power > 5) return 999;
    if (elem < 0 || elem > 5) return 999;
    return (power + 1) * (power + 1) * g_elem_cost_mult[elem];
}
```

Element multipliers (indices 0-5): {2, 3, 2, 4, 3, 3}.

| Power | Cost (elem 0/2) | Cost (elem 3) |
|---|---|---|
| 0 | 2 | 4 |
| 1 | 8 | 16 |
| 2 | 18 | 36 |
| 3 | 32 | 64 |
| 4 | 50 | 100 |
| 5 | 72 | 144 |

Source: `src/nexus/nexus_v1_magic.c`

---

## 5. Spell Casting Function

```c
int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int elem, int form, int align) {
    int cost, skill_req;
    (void)form; (void)align; /* FUTURE: full rune combination parsing.
     * DM Nexus magic system extends DM1 with alignment-based effects. */

    if (!caster || !caster->alive) return -1;
    cost = nexus_v1_spell_mana_cost(power, elem);
    if (caster->mana < cost) return -1;
    skill_req = power;
    if (caster->priest_level < skill_req && caster->wizard_level < skill_req) return -1;
    caster->mana -= cost;
    return cost;
}
```

Validation: caster alive -> sufficient mana -> skill level (priest or wizard >= power).
The form and align parameters are stubbed — rune parsing is marked FUTURE.

Source: `src/nexus/nexus_v1_magic.c`

---

## 6. Pre-Made Dungeon Potions

The item encyclopedia defines 4 dungeon potion types (direct consume, no flask needed):

| Name | Category | Weight |
|---|---|---|
| Health Potion | `FS_ITEM_CAT_POTION` | 2 |
| Mana Potion | `FS_ITEM_CAT_POTION` | 2 |
| Stamina Potion | `FS_ITEM_CAT_POTION` | 2 |
| Antidote | `FS_ITEM_CAT_POTION` | 2 |

These are consumed directly from inventory, restoring the respective stat.
Empty bottle remains as a reusable flask (C195/C20 equivalent).

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 7. Fountain Interaction (Inherited from DM1)

When a champion interacts with a dungeon fountain:
- Action hand holds empty flask: empty flask -> Water Flask (weight 4)
- Action hand holds something else: no filling occurs
- Water Flask can be drunk, restoring water stat, leaving empty flask behind

Inherited from DM1; confirmed in DM1 PC34 compat layer.

Source: `dm1_v1_fountain_interaction_pc34_compat.c`, `docs/dm2_potions.md`

---

## 8. Two-Step Potion Consumption

Step 1 — Flask Crafting (Spell):
Champion has empty flask -> casts potion spell -> flask is consumed -> potion item created

Step 2 — Potion Consumption (Use):
Champion drinks potion -> effect applied -> potion item consumed -> empty flask returned

This two-step model is the same as DM1, allowing the flask to be a reusable reagent.

Source: `docs/spells_items.md`, `docs/dm2_potions.md`

---

## 9. What is NOT in Nexus V1 Potions

| Feature | DM2 | Nexus V1 |
|---|---|---|
| Scroll system | Yes | No (category exists, no casting code) |
| Wand items | Yes | No |
| Tech potions (battery-powered) | Yes | No |
| Potion per-charge weight | Yes | No (fixed weight per entry) |
| Extended potion variety | Yes | No (only 4 dungeon potion types) |
| Potion mixing | Yes | No |

---

## 10. Missing: Rune Combination Parsing for Potions

The `nexus_v1_cast_spell` function notes:
```c
(void)form; (void)align; /* FUTURE: full rune combination parsing. */
```

The actual rune-word to spell mapping (e.g., Ful+Bro+Ku -> STRENGTH POTION)
is not implemented. Spell parameters are passed directly rather than being
parsed from rune input. This is a gap — rune parsing for the flask system
is marked as future work.

Source: `src/nexus/nexus_v1_magic.c`

---

## Status: PARTIALLY SOURCE-LOCKED

- Mana cost formula: **source-locked** — `nexus_v1_magic.c`
- Spell casting gate (mana + skill check): **source-locked** — `nexus_v1_magic.c`
- Fountain filling: **inherited** from DM1 patterns
- Flask crafting rune parsing: **NOT IMPLEMENTED** — marked FUTURE in source
- Potion item data: **source-locked** — `firestaff_item_encyclopedia.c`
- Two-step consumption flow: **inherited** from DM1
