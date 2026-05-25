# DM1 V1 - Spell Items (Magic Consumables)
Audit date: 2026-05-25 | Source: ReDMCSB WIP20210206

---

## 1. Magic Item Categories

DM1 V1 has three categories of magic-related items:

| Category                  | Item Examples              | How Obtained              |
|---------------------------|----------------------------|---------------------------|
| Potions (spell products)  | Health, Mana, Strength etc | Created by spell + flask  |
| Spell casting reagents    | Empty Flask (C195/C20)     | Found in dungeon, fountains |
| Magic Map                 | Illumulet / Magic Map     | Found in dungeon          |

---

## 2. Empty Flask - The Core Magic Reagent

The empty flask is central to DM1 V1 spellcasting. It is both:
- An inventory item (class C195 / C20)
- A required reagent for 10 potion spells

### Empty flask item constants:
Source: dm1_v1_inventory_consumables_pc34_compat.c:25
  DM1_POTION_WATER_FLASK_PC34 = 15   // water flask (after filling)
  DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34  // empty flask

### Fountain interaction - filling a flask:
Source: dm1_v1_fountain_interaction_pc34_compat.c:54-56
- If leader's hand icon == DM1_V1_ICON_POTION_EMPTY_FLASK:
    Action: DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK
    Result: hand icon changes to DM1_V1_ICON_POTION_WATER_FLASK
- Weight: DUNGEON.C:1108-1127 defines water flask weight and empty flask weight

### Potion consumption - flask returns:
Source: dm1_v1_inventory_consumables_pc34_compat.c:58
  PANEL.C:1850-1917 applies potion effects and converts all potions
  to C20 empty flask without clearing Power

All consumed potion bottles become empty flasks again.

---

## 3. Potion Spells - Requiring Empty Flask

Ten spells require an empty flask in hand to cast:

| Spell                  | Symbols      | Effect               | Flask Consumed |
|------------------------|-------------|----------------------|----------------|
| STRENGTH POTION        | Ful Bro Ku  | Temp STR boost       | Yes            |
| WISDOM POTION          | Ya Bro Dain | Temp WIS boost       | Yes            |
| VITALITY POTION        | Ya Bro Neta | Temp VIT boost       | Yes            |
| HEALTH POTION          | Vi          | Heal missing HP      | Yes            |
| CURE POISON POTION     | Vi Bro      | Remove poison        | Yes            |
| DEXTERITY POTION       | Oh Bro Ros  | Temp DEX boost       | Yes            |
| MANA POTION            | Zo Bro Ra   | Restore mana         | Yes            |
| STAMINA POTION         | Ya          | Restore stamina      | Yes            |
| POISON POTION          | Zo Ven      | Poison target        | Yes            |
| SHIELD POTION          | Ya Bro      | Shield defense       | Yes            |

### Casting check:
Source: dm1_v1_spell_casting_pc34_compat.c:377-382

  if (potion spell && no empty flask in hand):
      return DM1_FAILURE_NEEDS_FLASK_IN_HAND
      (message: " NEEDS AN EMPTY FLASK IN HAND FOR POTION.")

### After casting: flask is consumed, potion item is created.
Source: MENU.C F0412 - flask consumed when spell succeeds.

---

## 4. Potion Item Types

Pre-existing potions found in the dungeon (not crafted):

| Potion         | Icon/Class | Effect when consumed |
|----------------|-----------|---------------------|
| Health Potion  | C195      | Restore HP          |
| Mana Potion    | C195      | Restore mana        |
| Poison         | C195      | Damage champion     |
| Water Flask    | C195      | Quenches thirst     |

### Potion consumption flow:
Source: dm1_v1_inventory_consumables_pc34_compat.c:214-235
- Item type checked: DM1_POTION_WATER_FLASK_PC34
- Effect applied based on potion type
- potionTypeAfter = DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34
- Flask returns to inventory after drinking

---

## 5. Magic Map - For MAP Spell

Some spells (e.g., MAGIC MAP) require a Magic Map in the action hand.
Source reference: dm1_v1_spell_casting_pc34_compat.c:127
  " NEEDS A MAGIC MAP IN ACTION HAND FOR THIS SPELL."

The Magic Map is a dungeon object (class/graphic defined in DUNGEON.DAT)
that can be held in the action hand slot.

---

## 6. Potions vs. Spellcasting - Two Systems

DM1 V1 has two separate potion systems:

### System A - Potion items in dungeon:
- Pre-placed potion bottles found in dungeon rooms
- Consumed from inventory by clicking champion panel
- Flask lost when potion consumed

### System B - Potion spells (10 spells requiring flask):
- Champion must have empty flask in hand when casting
- Flask is consumed, creating a potion item
- Potion then consumed separately for the effect

---

## 7. Scrolls and Wands - NOT in DM1 V1

DM1 V1 does NOT have scroll or wand items for magic.
Magic is cast exclusively through:
1. Champion symbol inscription (the 4-step rune system)
2. Fountain effects (water flask filling)
3. Dungeon objects (light sources, triggers)

The CHAOS STRIKES BACK expansion added scroll/wand items;
they are absent from DM1 V1.

---

## 8. Source Evidence

  dm1_v1_inventory_consumables_pc34_compat.c:25   flask/potion constants
  dm1_v1_inventory_consumables_pc34_compat.c:58   PANEL.C:1850-1917 potion->flask
  dm1_v1_inventory_consumables_pc34_compat.c:214  potion type switch/case
  dm1_v1_fountain_interaction_pc34_compat.c:54-56 empty flask fill action
  dm1_v1_spell_casting_pc34_compat.c:125-127      NEEDS_FLASK / NEEDS_MAP failures
  dm1_v1_spell_casting_pc34_compat.c:377-382       flask requirement check
  DM1_V1_PLAN.md Phase 3 item 12: Potion creation (flask-based spells) - GAP