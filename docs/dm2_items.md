# DM2 V1 — Item System: Types, Categories, and Database Structure

**Audit date:** 2026-05-25
**Sources:** skproject/SKULLWIN/c_item.cpp, skproject/SKWIN/SkWinCore.cpp, skproject/SKWIN/DME.h, docs/dm2_newfeatures.md, docs/dm2_champ_changes.md

---

## 1. Item Database Categories (dbWeapon, dbCloth, dbScroll, dbMiscellaneous_item)

DM2 stores items in a structured record system with 4 main database types:

| DB Type Constant | Description | Notes |
|---|---|---|
| `dbWeapon` (5) | Weapons — swords, guns, bombs, thrown | Has charges, damage stats |
| `dbCloth` (6) | Clothing/Armor — protection | Has armor strength, charges |
| `dbScroll` (7) | Scrolls — spell items (CSB/DM2 new) | Has spell association |
| `dbMiscellaneous_item` (10) | Misc — potions, flasks, tools, quest items | Has charges/compass |

Source: SkWinCore.cpp:847-852 (dbWeapon through dbMiscellaneous_item switch handling)

### GDAT Category System

DM2 extends GDAT categories from DM1's 0x1D (29) to 0xF0 (240):
- `GDAT_CATEGORY_WEAPONS` — extended weapon data with projectile flags
- `GDAT_CATEGORY_SPELL_DEF` — spell definitions
- Extended item fields: animation (06 00 00), mana/luck/speed (14-15,33), weight per charge (34)

Source: dm2_newfeatures.md, SkWinCore.cpp:25391-25440

---

## 2. Item Record Structure (c_itemrecord in dm2data.h)

```cpp
class c_itemrecord {
    // 16-bit fields — item identity
    // Contains ItemType() accessor (7-bit field, value 0x00-0x7F)
    // ItemType stored in w2 (word 2) of record
};
```

Item type is a 7-bit value (0-127) per category, allowing granular item classification
within each DB type. The ItemType() field is reused across Weapon, Cloth, Scroll, and Misc
classes via inheritance or pattern.

Source: SKWIN/DME.h:567-568, 588-589, 612-613, 728-729

---

## 3. Item Type Accessor Pattern

Each item subclass has an `ItemType()` accessor:

```cpp
// From DME.h — pattern for Weapon, Cloth, Scroll, Miscellaneous_item
U8 ItemType() const { return (U8)(w2 & 0x007F); }
void ItemType(Bit16u val) {
    w2 = (w2 & ~0x007F) | (val & 0x007F);
}
```

Example from SkWinCore.cpp:41441-41465:
```cpp
case dbWeapon:
    return bp04->castToWeapon()->ItemType();
case dbCloth:
    return bp04->castToCloth()->ItemType();
case dbScroll:
    return bp04->castToScroll()->ItemType();
case dbMiscellaneous_item:
    return bp04->castToMisc()->ItemType();
```

Source: SkWinCore.cpp:41441-41465, DME.h:567-729

---

## 4. Item Cast Helpers

SkWinCore provides typed accessors for each item category:
```cpp
Weapon *SkWinCore::GET_ADDRESS_OF_RECORD5(ObjectID recordLink);
Cloth *SkWinCore::GET_ADDRESS_OF_RECORD6(ObjectID recordLink);
Scroll *SkWinCore::GET_ADDRESS_OF_RECORD7(ObjectID recordLink);
Miscellaneous_item *SkWinCore::GET_ADDRESS_OF_RECORDA(ObjectID recordLink);
```

Source: SkWinCore.cpp:2698-2703

---

## 5. Item Charges System

Weapons, Cloth, and Miscellaneous items all support a `Charges` field:
- **Weapon/Cloth**: `Charges()` — remaining uses before depletion
- **Misc**: `Compass()` (misnamed field reused for charges)

Charge consumption is tracked per item. Weapons consume charges on use,
armor/cloth on damage absorption, misc items on consumption.

```cpp
case dbWeapon:
    si = reinterpret_cast<Weapon *>(bp04)->Charges();
    break;
case dbCloth:
    si = reinterpret_cast<Cloth *>(bp04)->Charges();
    break;
case dbMiscellaneous_item:
    si = reinterpret_cast<Miscellaneous_item *>(bp04)->Compass();
    break;
```

Source: SkWinCore.cpp:2211-2223 (charge read), 2236-2246 (charge write)

---

## 6. Extended Item Fields (DM2 vs DM1)

DM2 adds new fields to the item data structure:

| Field Index | Content | Notes |
|---|---|---|
| 0x06 / 06 00 00 | Animation | New in DM2 |
| 0x14 / 14 00 00 | Mana bonus | New in DM2 |
| 0x15 / 15 00 00 | Luck bonus | New in DM2 |
| 0x33 / 33 00 00 | Speed bonus | New in DM2 |
| 0x34 / 34 00 00 | Weight per charge | New in DM2 |

Potion custom fields (DM2-specific):
- `behavior` (05) — potion type behavior
- `water value` (43) — hydration/food value
- `spell missile association` (4D) — linked spell for potion effect

Source: dm2_newfeatures.md §9

---

## 7. Item Affinity System (Magic vs Tech vs Hybrid)

DM2 introduces an item affinity system defining how items are used:

```cpp
typedef enum {
    DM2_ITEM_MAGIC  = 0,  // traditional magic items (staff, scroll, wand)
    DM2_ITEM_TECH   = 1,  // tech items (guns, bombs, battery-powered)
    DM2_ITEM_HYBRID = 2,  // requires both tech_level AND magic_level
} DM2_ItemAffinity;
```

Usage check:
```cpp
case DM2_ITEM_TECH:
    return champion_tech >= item->tech_level;
case DM2_ITEM_HYBRID:
    return champion_tech >= item->tech_level &&
           champion_magic >= item->magic_level;
```

Source: dm2_magic.md (item affinity system), c_item.cpp (DM2_PROCESS_ITEM_BONUS)

---

## 8. Item Bonus Processing (c_item.cpp)

`DM2_PROCESS_ITEM_BONUS` applies item effects to champions at equip time:
- HP/MP recalculation (stat boosts on equip)
- Light level recalculation if item affects illumination (bit 0x2000)
- Ability score bonuses (strength, wisdom, etc.)
- Walk speed modification (bonus to champion speed from misc items)
- Re equip validation via `DM2_IS_ITEM_FIT_FOR_EQUIP`

```cpp
i32 DM2_RETRIEVE_ITEM_BONUS(i32 itemIndex, i32 gdatOffset, i32 flag2, i32 flag3);
// Returns signed bonus value for item+stat+champion combo
```

Source: c_item.cpp:1-100+ (DM2_RETRIEVE_ITEM_BONUS, DM2_PROCESS_ITEM_BONUS)

---

## 9. Item Value and Weight

Item economic properties:
```cpp
i16 DM2_QUERY_ITEM_VALUE(i32 eaxl, i32 edxl);  // gold value
i16 DM2_QUERY_ITEM_WEIGHT(i16 eaxw);           // encumbrance weight
```

Item value is looked up from GDAT database (gdatOffset passed). Weight affects
champion movement speed (encumbrance system from DM1, carried forward).

Source: c_item.h (function declarations)

---

## 10. Item Placement and Spawning

DM2 supports creation of items in the dungeon:
```cpp
void CREATE_NEW_ITEM_AT_POSITION(int iDBItem, int iItemType, int iMap, int iPosX, int iPosY, int iFace);
void CREATE_NEW_ITEM_FOR_PLAYER(int iDBItem, int iItemType, int iChampionNumber);
```

Item spawning via SLEV*.BIN level scripts or dungeon generation.
Items on floor are tracked with position and facing (iFace).

Source: SKWIN/SkWinCore.h:876-877

---

## 11. Item Categories in DM2 vs DM1

| Category | DM1 | DM2 |
|---|---|---|
| Weapons (melee) | ✅ | ✅ Same |
| Weapons (ranged) | Crossbow only | ✅ Crossbow + Gun + Bomb + Thrown |
| Armor/Cloth | ✅ | ✅ Same + more enchantments |
| Potions | Flask-based 10 spells | ✅ Same + pre-made potions |
| Scrolls | ❌ (added in CSB) | ✅ Full scroll system |
| Wands | ❌ (added in CSB) | ✅ Wand system |
| Misc items | Limited | ✅ Expanded (tools, quest items, ammo) |
| Gold as item | ❌ | ✅ Shop economy with gold as item |

---

## 12. Summary: What's New in DM2 Item System

1. **Tech item category** — guns, bombs requiring `tech_level` stat
2. **Hybrid items** — require both magic and tech stats
3. **Scroll item type** — spell scrolls (DB type 7) new in DM2
4. **Wand item type** — wand devices (part of misc or new type)
5. **Extended GDAT item fields** — animation, mana/luck/speed bonuses
6. **Charge tracking per item** — weapons/armor/misc all track charges
7. **Shop system with merchant NPCs** — gold economy, buy/sell
8. **Item weight per charge** — granular encumbrance tracking

---

## Status: PARTIALLY SOURCE-LOCKED

Item database structure confirmed from skproject source (c_item.cpp, SkWinCore.cpp, DME.h).
GDAT category extensions confirmed from dm2_newfeatures.md.
Tech/hybrid item affinity confirmed from dm2_magic.md.
Scroll/wand full implementation details pending source confirmation.
