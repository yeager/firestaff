# Nexus V1 — Magic Mechanics

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_magic.c`, `include/nexus_v1_magic.h`, `include/firestaff_spell_ui.h`, `include/firestaff_spell_ref.h`, `docs/spells_system.md` (DM1 V1)

---

## 1. Overview

Nexus V1 magic is rune-word based, identical in structure to DM1. Spells are
cast by composing rune sequences (Power + Element + optional Form + optional
Alignment) and paying a mana cost. Champion skill levels must meet or exceed
the spell's power level.

The key difference from DM1: Nexus V1 does not yet implement rune combination
parsing — the form and alignment parameters of `nexus_v1_cast_spell` are
discarded (`(void)form; (void)align`). Spell selection is currently done by
passing raw power and element indices directly rather than through a rune UI.

---

## 2. Rune Grid — Same 6×6×4×6 Structure as DM1

The Firestaff spell UI (`include/firestaff_spell_ui.h`) defines the full DM1 rune grid:

```
Power runes:   Lo  Um  On  Ee  Pal Mon     (indices 0-5, power levels 0-5)
Element runes: Ya  Vi  Oh  Ful Des Zo      (indices 0-5)
Form runes:    Ven Ew  Kath Ir  Bro Gor     (indices 0-5, optional)
Align runes:   Ku  Ros Dain Neta Ra Sar    (indices 0-5, optional)
```

The spell casting UI (`FS_SpellUI`) follows the DM1 state machine:
```
IDLE -> POWER (select power rune) -> ELEMENT (select element)
-> FORM (optional) -> ALIGN (optional) -> READY (cast)
```

Source: `include/firestaff_spell_ui.h`

---

## 3. Casting Flow in Code

```c
int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int elem, int form, int align) {
    int cost, skill_req;
    (void)form; (void)align; /* FUTURE: full rune combination parsing. */

    if (!caster || !caster->alive) return -1;
    cost = nexus_v1_spell_mana_cost(power, elem);
    if (caster->mana < cost) return -1;
    skill_req = power;
    if (caster->priest_level < skill_req && caster->wizard_level < skill_req) return -1;
    caster->mana -= cost;
    return cost;
}
```

Gate 1: Champion alive?
Gate 2: Mana >= cost?
Gate 3: priest_level OR wizard_level >= power?
On success: mana deducted, cost returned.
On failure: -1 returned.

Source: `src/nexus/nexus_v1_magic.c`

---

## 4. The Flask System — Two-Step Potion Crafting

Nexus V1 inherits the DM1 flask/potion system unchanged. Potions are NOT
direct-consume items (except for 4 pre-made dungeon potions). Instead, they
are crafted in a two-step process:

### Step 1 — Flask Crafting (spell cast)
Champion must have:
- Empty flask (C195/C20 equivalent) in action hand
- Spell known (e.g., STRENGTH POTION = Ful Bro Ku)
- Sufficient mana
- Priest or Wizard skill level >= required level

On success: empty flask is consumed → potion item created in hand.

### Step 2 — Potion Consumption (use)
Champion uses/drinks the potion → effect applied → flask item returned.

The 10 flask-crafting spells:

| Spell Name | Runes | Effect | Reagent |
|---|---|---|---|
| STRENGTH POTION | Ful Bro Ku | +STR temp | Empty flask |
| WISDOM POTION | Ya Bro Dain | +WIS temp | Empty flask |
| VITALITY POTION | Ya Bro Neta | +VIT temp | Empty flask |
| HEALTH POTION | Vi | Heal missing HP | Empty flask |
| CURE POISON POTION | Vi Bro | Remove poison | Empty flask |
| DEXTERITY POTION | Oh Bro Ros | +DEX temp | Empty flask |
| MANA POTION | Zo Bro Ra | Restore mana | Empty flask |
| STAMINA POTION | Ya | Restore stamina | Empty flask |
| POISON POTION | Zo Ven | Poison target | Empty flask |
| SHIELD POTION | Ya Bro | Shield defense | Empty flask |

Source: `docs/spells_items.md` (DM1 V1), `docs/nexus_potions.md`

---

## 5. Fountain Interaction — Flask Filling

Inherited from DM1:
- Champion with empty flask uses/interacts with dungeon fountain → empty flask → Water Flask
- Champion drinks Water Flask → water stat restored → empty flask returned
- Champion with no flask or wrong item: no effect

Source: `docs/dm2_potions.md`, `dm1_v1_fountain_interaction_pc34_compat.c`

---

## 6. Pre-Made Potions — Direct Consume

The item encyclopedia also lists 4 direct-consume potion types (no flask required):

| Name | Effect | Weight |
|---|---|---|
| Health Potion | Restore health directly | 2 |
| Mana Potion | Restore mana directly | 2 |
| Stamina Potion | Restore stamina directly | 2 |
| Antidote | Remove poison directly | 2 |

These consume the bottle/flask on use, leaving no reusable container.
They bypass the two-step flask crafting system entirely.

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 7. Champion Magic Resources

Each champion has three magic-related stats:

```c
int mana, max_mana;          /* magical energy pool */
int priest_level;            /* priest-class skill level */
int wizard_level;            /* wizard-class skill level */
int anti_magic;               /* resistance to hostile magic (default 5) */
```

Starting mana pools (from `nexus_v1_champions.c` roster):
| Class | Starting Mana |
|---|---|
| Fighter | 10-20 |
| Ninja | 25-30 |
| Priest | 55 |
| Wizard | 65-70 |

Skill levels (priest_level/wizard_level) start at 1 and increase through use.
Both stats are checked: either priest_level OR wizard_level >= power passes.

Source: `src/nexus/nexus_v1_champions.c`, `include/nexus_v1_champions.h`

---

## 8. Alignment Runes — Present in Enum, Unused

Nexus V1 defines alignment rune slots (Ku/Ros/Dain/Neta/Ra/Sar) but they are
never used in the current casting code. The `align` parameter is cast but
discarded. This is where DM1 had alignment-specific spell variants.

In DM1, alignment runes modified:
- Damage type (e.g., good vs evil targeting)
- Party-friendly vs enemy-only effects
- Success rate against different creature alignments

This system is noted as FUTURE in the Nexus V1 source.

---

## 9. What is NOT Yet Implemented

| Feature | Status | Source |
|---|---|---|
| Rune word → spell lookup | NOT IMPLEMENTED | nexus_v1_magic.c (form/align discarded) |
| Form rune effects | NOT IMPLEMENTED | Future |
| Alignment rune effects | NOT IMPLEMENTED | Future |
| Rune UI state machine | Defined in firestaff_spell_ui.h, not wired to nexus_v1_cast_spell | Future |
| Spell damage/effect dispatch | NOT IMPLEMENTED | Only mana cost deducted; no actual effect |

Source: `src/nexus/nexus_v1_magic.c` (stub comments)

---

## Status: PARTIALLY SOURCE-LOCKED

- Mana cost formula: **source-locked** — `nexus_v1_magic.c`
- Skill gate (priest/wizard level check): **source-locked** — `nexus_v1_magic.c`
- Flask-based potion crafting: **inherited** from DM1 patterns
- Pre-made dungeon potions: **source-locked** — `firestaff_item_encyclopedia.c`
- Fountain filling: **inherited** from DM1
- Rune parsing for spell dispatch: **NOT IMPLEMENTED** — marked FUTURE
