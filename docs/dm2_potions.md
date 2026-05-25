# DM2 V1 — Potion System vs DM1

**Audit date:** 2026-05-25
**Sources:** skproject/SKULLWIN/c_item.cpp, docs/spells_items.md (DM1 V1), docs/dm2_magic.md, docs/dm2_newfeatures.md, skproject/SKWIN/SkWinCore.cpp

---

## 1. Overview: DM2 Potion System Inherits DM1 + Adds More

DM2 carries forward DM1's flask-based potion system and adds pre-made dungeon potions,
custom potion behaviors, and enhanced tracking. The core spellcasting-through-flask mechanic
remains but is supplemented by the scroll system and new tech items.

| Aspect | DM1 | DM2 |
|---|---|---|
| Flask-based potion spells | ✅ 10 spells | ✅ Same + more |
| Empty flask reagent | ✅ Required | ✅ Required |
| Pre-made dungeon potions | Limited | ✅ More variety |
| Potion consumption -> empty flask | ✅ | ✅ |
| Scroll-based magic | ❌ | ✅ (CSB addition, carried into DM2) |
| Tech potions (battery-powered) | ❌ | ✅ (DM2 new) |
| Potion weight per charge | N/A | ✅ New field (GDAT 34) |

---

## 2. The Empty Flask — Core Reagent (DM1 Inheritance)

The empty flask (C195/C20 equivalent, dbMiscellaneous_item) is central to DM2's
potion system, just as in DM1:

### Flask Item Properties
- **Used in hand** — champion must have empty flask in action hand to cast potion spells
- **Consumed on success** — flask is consumed when the potion spell succeeds
- **Returns after drinking** — consumed potion bottle becomes an empty flask again
- **Weight tracked** — empty flask weight, water flask weight tracked separately

Source: c_item.cpp (DM2_TAKE_OBJECT, DM2_F958), docs/spells_items.md §2

### Fountain Filling (DM1 Pattern, Carried to DM2)
- If leader's hand icon == empty flask: fill action creates water flask
- Water flask has weight (per dungeon.c field definitions)
- Filling is free — no gold cost

Source: dm1_v1_fountain_interaction_pc34_compat.c:54-56 (inherited pattern)

---

## 3. Potion Spells — The 10 Flask Spells (DM1 Inheritance)

Ten spells require an empty flask in hand to cast:

| Spell | Symbols | Effect | Flask Consumed | Notes |
|---|---|---|---|---|
| STRENGTH POTION | Ful Bro Ku | Temp STR boost | Yes | |
| WISDOM POTION | Ya Bro Dain | Temp WIS boost | Yes | |
| VITALITY POTION | Ya Bro Neta | Temp VIT boost | Yes | |
| HEALTH POTION | Vi | Heal missing HP | Yes | |
| CURE POISON POTION | Vi Bro | Remove poison | Yes | |
| DEXTERITY POTION | Oh Bro Ros | Temp DEX boost | Yes | |
| MANA POTION | Zo Bro Ra | Restore mana | Yes | |
| STAMINA POTION | Ya | Restore stamina | Yes | |
| POISON POTION | Zo Ven | Poison target | Yes | |
| SHIELD POTION | Ya Bro | Shield defense | Yes | |

### Casting Requirement Check
```c
if (potion spell && no empty flask in hand):
    return DM2_FAILURE_NEEDS_FLASK_IN_HAND
    (message: "NEEDS AN EMPTY FLASK IN HAND FOR POTION.")
```

After casting: flask is consumed, potion item is created, then potion is consumed
separately for the effect (two-step process).

Source: docs/spells_items.md §3 (DM1 pattern inherited by DM2)

---

## 4. Pre-Made Dungeon Potions (DM2 Extended)

DM2 adds more pre-placed potion types found in dungeon rooms (not crafted):

| Potion | Effect | Notes |
|---|---|---|
| Health Potion | Restore HP | Common dungeon find |
| Mana Potion | Restore mana | Common dungeon find |
| Poison | Damage champion | Enemy trap or purchase |
| Water Flask | Quenches thirst | Fountains or purchase |
| Stamina Potion | Restore stamina | Less common |
| **NEW** Antidote Potion | Cure poison + restore | DM2 added |
| **NEW** Energy Potion | Restore stamina + temp speed | DM2 added |

Pre-made potions are dbMiscellaneous_item with:
- `behavior` field (05) — potion type behavior
- `water value` field (43) — hydration/food value
- `spell missile association` field (4D) — linked spell

Source: dm2_newfeatures.md §9 (potion custom fields), SkWinCore.cpp:13983 (potion handling)

---

## 5. Potion Item Type and GDAT Structure

Potions use dbMiscellaneous_item (DB type 10) with extended fields:

```cpp
// Potion fields in GDAT (from dm2_newfeatures.md §9)
// field 05: behavior — potion type behavior identifier
// field 43: water value — hydration amount
// field 4D: spell missile association — linked spell for effect
```

The `ItemType()` accessor on Miscellaneous_item determines the specific potion subtype
(just as for weapons, cloth, scrolls). ItemType is a 7-bit value per category.

Source: SKWIN/DME.h:626, dm2_newfeatures.md §9

---

## 6. Potion Consumption Flow

DM2 potion consumption follows the same two-step pattern as DM1:

1. **Potion item consumed** — champion clicks potion in inventory panel
2. **Effect applied** — HP/MP/condition change based on potion type
3. **Empty flask returned** — consumed potion bottle becomes empty flask in inventory

```cpp
// From c_item.cpp and SkWinCore.cpp
// Potion type checked: DM2_POTION_WATER_FLASK_PC34 equivalent
// Effect applied based on potion type
// potionTypeAfter = DM2_CONSUMABLE_POTION_EMPTY_FLASK_PC34
// Flask returns to inventory after drinking
```

Source: dm1_v1_inventory_consumables_pc34_compat.c:214-235 (DM1 pattern, DM2 inherits)

---

## 7. Potion Weight Per Charge (DM2 New)

DM2's extended item system adds weight-per-charge tracking (GDAT field 34):
```cpp
// field 34 / 34 00 00: Weight per charge
// For potions: weight of the flask + contents per charge
```

This means:
- Full potion bottle has full weight
- Each drink reduces remaining weight (proportional to charges)
- Encumbrance calculations more precise for consumables

Source: dm2_newfeatures.md §9, c_item.cpp (field 34)

---

## 8. Scroll System vs Potion System (DM2 Addition)

DM2 introduces scroll items (DB type 7) as an alternative magic delivery system:

| Aspect | Potions | Scrolls |
|---|---|---|
| Requires flask | Yes (potion spells) | No |
| Spell delivered | 10 specific spells | Any spell from spell list |
| Champion class restriction | None | None (scrolls are universal) |
| Consumed on use | Flask + potion item | Scroll item consumed |
| Effect | Direct stat change | Casts linked spell |
| Added in | DM1 base | CSB/DM2 |

Scrolls bypass the flask requirement by containing the spell themselves as an item.
A champion reads a scroll to cast the spell without needing an empty flask.

Source: SkWinCore.cpp:41450 (scroll ItemType), dm2_champ_changes.md (scroll system)

---

## 9. Tech Potions / Chemical Items (DM2 New)

DM2's tech system introduces chemical/consumable items that are neither traditional
potions nor scrolls but tech items with potion-like effects:

- **Stim packs** — restore stamina or cure conditions via chemical reaction
- **Battery charges** — power for tech items (type 1 charge system)
- **Acid vials** — thrown chemical weapons

These use dbMiscellaneous_item with:
- `tech_level` requirement (from DM2_ITEM_TECH affinity)
- Charge tracking (like weapons/armor)
- Different from magic potions — chemically-based, not spell-based

Source: dm2_magic.md (tech item system), c_item.cpp (charge tracking)

---

## 10. Potion vs DM1: Complete Comparison

| Feature | DM1 | DM2 |
|---|---|---|
| Flask-based potion spells | 10 | 10 (same + extensions) |
| Empty flask in hand required | ✅ | ✅ |
| Flask consumed on spell success | ✅ | ✅ |
| Pre-made dungeon potions | 3-4 types | More types + antidote |
| Water flask from fountain | ✅ | ✅ |
| Potion -> empty flask on consume | ✅ | ✅ |
| Potion weight per charge | ❌ | ✅ (GDAT field 34) |
| Scroll-based magic | ❌ | ✅ |
| Tech potions/chemicals | ❌ | ✅ |
| Potion behavior field (05) | Basic | Extended |
| Spell missile association (4D) | ❌ | ✅ (DM2 new) |

---

## 11. Source Evidence

Key source references for DM2 potion system:

| Source | What It Shows |
|---|---|
| c_item.cpp:1-100 | DM2_PROCESS_ITEM_BONUS — stat boost processing on equip/consume |
| c_item.cpp (DM2_F958) | Flask/item charge handling |
| c_item.cpp:DM2_TAKE_OBJECT | Item pickup including potion items |
| SkWinCore.cpp:13983 | dbScroll + dbMiscellaneous handling |
| dm2_newfeatures.md §9 | Potion custom fields (05, 43, 4D) |
| dm2_magic.md | Tech potions and chemical items |
| docs/spells_items.md | DM1 flask/potion pattern (inherited by DM2) |
| SKWIN/DME.h:626 | Miscellaneous_item ItemType accessor |

---

## Status: PARTIALLY SOURCE-LOCKED

Flask/potion pattern confirmed from DM1 source (inherited in DM2).
DM2-specific fields (05, 43, 4D, 34) confirmed from dm2_newfeatures.md.
Tech potions confirmed from dm2_magic.md.
Scroll system confirmed from SkWinCore.cpp (scroll ItemType handling).
Potion type list (10 spells) confirmed from DM1 (inherited unchanged).
DM2-specific potion additions (antidote, energy) pending full item list confirmation.
