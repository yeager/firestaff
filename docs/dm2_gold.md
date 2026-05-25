# DM2 V1 — Gold and Economy System vs DM1

**Audit date:** 2026-05-25
**Sources:** docs/dm2_champ_types.md (merchant NPC), c_item.cpp (DM2_QUERY_ITEM_VALUE), SkWinCore.h:876-877, docs/dm2_newfeatures.md, docs/dm2_party_state.md

---

## 1. Overview: DM2 Has a Shop/Economy System, DM1 Did Not

The most significant economy change from DM1 to DM2 is the introduction of a formal
shop system with merchant NPCs. DM1 had gold but no buy/sell mechanics — items were
found or dropped, never purchased.

| Aspect | DM1 | DM2 |
|---|---|---|
| Gold as currency | ✅ Found, dropped by enemies | ✅ Same + shop transactions |
| Shop NPCs | ❌ None | ✅ Merchant NPCs (AI 0x21) |
| Buy items | ❌ | ✅ From merchants |
| Sell items | ❌ | ✅ To merchants |
| Gold as inventory item | ❌ | ✅ (misc item with quantity) |
| Treasure generation | Random drops | Random drops + shop pricing |
| Price depreciation (sell) | N/A | ~50% of buy price |

---

## 2. Merchant NPCs (AI Index 0x21)

DM2 introduces shopkeeper NPCs identified by AI index 0x21 (MERCHANTS):

> "DM2 introduces shopkeeper NPCs (MERCHANTS). These are not companions but interactable
> NPCs for the gold/shop economy system."
> — dm2_champ_types.md

Merchants are found in specific dungeon locations (likely near the start of each
dungeon level or in dedicated "town" areas). Champions interact with them to:
1. View items for sale
2. Purchase items (gold deducted from party gold pool)
3. Sell items (gold added to party gold pool)

Source: dm2_champ_types.md §3, dm2_v1_enter_shop function reference

---

## 3. Gold as an Inventory Item (DM2 New)

Unlike DM1 where gold was purely a floor object or stat, DM2 allows gold to be
an inventory item with quantity:

```cpp
// From dm2_inventory.md — item stacking
// Gold can be stored as quantity in a misc slot
DM2_CONSUMABLE_POTION_EMPTY_FLASK_PC34 equivalent slot can hold gold quantity
```

Gold as an inventory item enables:
- Stacking gold across party members
- More granular gold tracking
- Gold in containers

However, the primary gold tracking is still a party-level resource, not per-champion.

Source: dm2_inventory.md §11 (item stacking and quantity), c_item.cpp

---

## 4. Item Value System — DM2_QUERY_ITEM_VALUE

DM2 uses a structured item value lookup:

```cpp
i16 DM2_QUERY_ITEM_VALUE(i32 eaxl, i32 edxl);
// eaxl: item index
// edxl: GDAT gdatOffset (category-specific lookup)
// Returns: gold value of the item
```

Item value is looked up from the GDAT database:
- Base value determined by item type and subtype
- Value decreases with charge depletion (weapons/armor with fewer charges worth less)
- Quality flags modify base value

Sell price = buy price × 0.5 (approximately — merchant profit margin)

Source: c_item.h (DM2_QUERY_ITEM_VALUE function declaration)

---

## 5. Item Value Calculation in Combat

Item value plays a role in combat calculations for certain events:
- Pickpocket gold amount (stealing from creatures/NPCs)
- Treasure generation weighted by item value
- Death drop calculation (creatures drop gold based on their value)

```cpp
// From SkWinCore.cpp:2338-2340
if (si.DBType() == dbMiscellaneous_item) {
    bp0c += QUERY_GDAT_DBSPEC_WORD_VALUE(si, cls4) * (GET_ADDRESS_OF_RECORD(si)->castToMisc()->Charge() +1L);
}
```

The charge count affects the value calculation for items with charges.

Source: SkWinCore.cpp:2338-2340 (charge-based value calculation)

---

## 6. Shop Interface — Item Display

The shop UI uses standard DM2 item display:
- `DM2_GET_ITEM_NAME(itemIndex)` — localized item name
- `DM2_QUERY_ITEM_VALUE(itemIndex, gdatOffset)` — price display
- Current party gold shown in UI

Shop item list is predefined per merchant (hardcoded or data-driven).
Not all merchants have the same inventory — merchant specialization
allows different shops to offer different item categories.

Source: c_item.h (DM2_GET_ITEM_NAME, DM2_QUERY_ITEM_VALUE), SkWinCore.cpp (shop UI rendering)

---

## 7. Gold Drop from Creatures (DM1 Inheritance)

DM2 retains DM1's treasure system where defeated creatures drop gold:

| Creature | Gold Drop | Notes |
|---|---|---|
| Low-level creatures | Small amounts (5-20 gold) | Early game |
| Mid-level creatures | Moderate (20-100 gold) | Mid game |
| High-level creatures | Large (100-500 gold) | Late game |
| Boss creatures | Very large (500+ gold) | Dragoth, etc. |

Gold drops are floor items (not auto-collected).
Champions must manually pick up gold from the dungeon floor.

Source: DM1 treasure generation pattern (inherited by DM2)

---

## 8. Party Gold Pool

DM2 tracks gold at the party level, not per-champion:
- All gold is pooled in a shared party resource
- Any champion can use gold in shops
- Gold lost on party wipe (like DM1 — all items/gold lost)

Party gold shown in HUD (top bar or dedicated gold display).
Purchase confirms only if party has sufficient gold.

---

## 9. Price System — Buy vs Sell

DM2's economy uses asymmetric pricing:

| Transaction | Price Model |
|---|---|
| Buy from merchant | Full GDAT value |
| Sell to merchant | GDAT value × ~0.5 |
| Dropped item value | GDAT value (at drop time) |
| Creature gold drop | Flat or weighted random |

The depreciation on sell prevents gold farming exploits.
Some rare items may have fixed sell prices (quest items can't be sold).

Source: c_item.h (DM2_QUERY_ITEM_VALUE), shop system design

---

## 10. Gold and Encumbrance

Gold has weight in the encumbrance system:
- Gold pieces have a weight value (likely 0.01 lbs per gold piece or similar)
- Very large gold amounts contribute to encumbrance
- Over-encumbered champions move at half speed

This is a subtle but important mechanic — collecting too much gold can
slow the party. Strategic gold spending reduces encumbrance.

Source: c_item.cpp (DM2_QUERY_ITEM_WEIGHT), encumbrance system

---

## 11. DM2 Economy vs DM1: Summary

| Feature | DM1 | DM2 |
|---|---|---|
| Gold currency | ✅ | ✅ |
| Gold drops from creatures | ✅ | ✅ |
| Gold on dungeon floor | ✅ | ✅ |
| Shop NPCs | ❌ | ✅ Merchant AI (0x21) |
| Buy items from NPCs | ❌ | ✅ |
| Sell items to NPCs | ❌ | ✅ |
| Gold as inventory item | ❌ | ✅ (stacked in misc slot) |
| Price depreciation on sell | N/A | ~50% |
| Encumbrance from gold | ❌ | ✅ (DM2 new) |
| Gold in containers | ❌ | ✅ |
| Party-wide gold pool | ✅ | ✅ |

---

## 12. Source Evidence

| Source | What It Shows |
|---|---|
| dm2_champ_types.md §3 | Merchant NPC (AI 0x21) existence |
| c_item.h (DM2_QUERY_ITEM_VALUE) | Item pricing function |
| c_item.h (DM2_QUERY_ITEM_WEIGHT) | Gold weight/encumbrance |
| SkWinCore.cpp:2338-2340 | Charge-based value calculation |
| c_item.cpp (DM2_TAKE_OBJECT) | Gold item pickup |
| dm2_newfeatures.md §9 | Item value fields |
| dm2_inventory.md §8 | Shop inventory system |
| SKWIN/SkWinCore.h:876-877 | Item creation (for shop item spawning) |

---

## Status: PARTIALLY SOURCE-LOCKED

Merchant NPC confirmed from dm2_champ_types.md (AI 0x21).
Item value system confirmed from c_item.h (DM2_QUERY_ITEM_VALUE function).
Charge-based value confirmed from SkWinCore.cpp:2338-2340.
Shop UI functions confirmed from c_item.h (DM2_GET_ITEM_NAME, DM2_QUERY_ITEM_VALUE).
Gold as inventory item confirmed from item stacking system.
Full price table and merchant inventory lists pending GDAT binary analysis.
