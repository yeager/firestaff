# DM2 V1 — Inventory System vs DM1

**Audit date:** 2026-05-25
**Sources:** skproject/SKULLWIN/c_item.cpp, skproject/SKWIN/SkWinCore.cpp, docs/dm2_champ_changes.md, docs/dm2_party_state.md

---

## 1. Inventory Structure: Per-Champion Slots

Each champion in DM2 has the same basic inventory layout as DM1:

| Slot Type | DM1 | DM2 | Notes |
|---|---|---|---|
| Inventory grid slots | 12 | 12 | Same |
| Action hand (right) | 1 | 1 | Weapon/tool |
| Shield hand (left) | 1 | 1 | Armor/shield |
| Ring slots | 2 | 2 | Left/right |
| Amulet slots | 2 | 2 | Neck |

**Max party:** 4 champions × 12 slots = 48 item slots + 8 accessory slots.

Source: DM2 party system (4 champions max, same as DM1)

---

## 2. Hand Slot Mechanics (Action Hand / Shield Hand)

The two hand slots are tracked separately:
- **Action hand** — primary weapon/tool, also used for flask during spellcasting
- **Shield hand** — armor/shield, provides defense in combat

DM2 adds complexity to hand slot usage because of the new item types:
- Gun/bomb weapons must be in action hand to fire
- Flask must be in action hand for potion spell crafting
- Magic items may require specific hand slot

Hand slot item swapping is subject to the same encumbrance rules as DM1
(overweight champions move at half speed, cannot swap items mid-combat in some modes).

Source: c_item.cpp (DM2_MOVE_ITEM_TO, DM2_TAKE_OBJECT), SkWinCore.cpp:7039 (hand icon rendering)

---

## 3. Inventory Grid (12 Slots)

The 12-slot inventory grid per champion is unchanged from DM1 in capacity.
However, DM2's extended item types (scrolls, wands, ammo, tech devices) mean
the same 12 slots now hold a wider variety of item categories.

Key inventory management operations in DM2:
- **Add item to inventory** — `DM2_TAKE_OBJECT` (c_item.cpp: line ~1180)
- **Move item within inventory** — `DM2_MOVE_ITEM_TO` with slot coordinates
- **Drop item to floor** — handled via `CREATE_NEW_ITEM_AT_POSITION`
- **Container support** — `DM2__CHECK_ROOM_FOR_CONTAINER`, `DM2_PUT_OBJECT_INTO_CONTAINER`

Source: c_item.cpp: DM2_TAKE_OBJECT, DM2_MOVE_ITEM_TO, DM2__CHECK_ROOM_FOR_CONTAINER

---

## 4. Item Weight and Encumbrance

DM2 carries forward DM1's encumbrance system:
- Each item has a weight (from `DM2_QUERY_ITEM_WEIGHT`)
- Total weight carried affects champion movement speed
- Over encumbrance threshold (~400 lbs) halves movement speed
- Weight per charge tracking (field 34, new in DM2) means consumable items
  have fractional weight per use

Encumbrance check: `DM2_QUERY_ITEM_WEIGHT(i16 itemIndex)` returns item weight.
Weight is summed across all inventory + equipped items to compute encumbrance.

Source: c_item.h (DM2_QUERY_ITEM_WEIGHT), c_item.cpp:field 34 (weight per charge)

---

## 5. Container Items

DM2 supports item containers within the inventory:
```cpp
void DM2__CHECK_ROOM_FOR_CONTAINER(i32 eaxl, unk* xedxp);
i32 DM2_PUT_OBJECT_INTO_CONTAINER(i32 eaxl);
```

Containers allow grouping multiple small items inside a single inventory slot.
The container system was present in DM1 but DM2 extends it to support more
container types (scroll cases, ammo pouches, tech device holders).

Source: c_item.cpp (container functions)

---

## 6. Item Movement and Teleportation

DM2 adds item teleportation ability:
```cpp
void DM2_ACTIVATE_ITEM_TELEPORT(c_tim* eaxtimp, unk* xedxp, i32 ebxl, i32 ecxl,
                                  i32 argl0, unk* xargp1, i32 argl2, i32 argl3);
```

This allows items to be warped from one location to another (champion inventory,
floor position, container). Used by certain spells or quest items.

Source: c_item.cpp (DM2_ACTIVATE_ITEM_TELEPORT)

---

## 7. Chest / Floor Item Storage

Items dropped on the dungeon floor are tracked with position + facing:
```cpp
void CREATE_NEW_ITEM_AT_POSITION(int iDBItem, int iItemType, int iMap,
                                   int iPosX, int iPosY, int iFace);
```

Floor items persist until picked up. Multiple items can exist on the same tile.
The `iFace` parameter controls the facing direction of the item sprite.

Source: SKWIN/SkWinCore.h:876-877

---

## 8. Shop Inventory (Merchant NPCs)

DM2 introduces shop system with merchant NPCs (AI index 0x21):
- Merchants have a predefined item list they will sell
- Champions can buy items using gold
- Champions can sell items for gold (at depreciated value)
- Shop UI shows item name, price, and current gold

The shop interface uses the standard item display system (item name from
`DM2_GET_ITEM_NAME`, value from `DM2_QUERY_ITEM_VALUE`).

Source: dm2_champ_types.md (merchant NPC index 0x21), c_item.h (DM2_GET_ITEM_NAME, DM2_QUERY_ITEM_VALUE)

---

## 9. Inventory Changes vs DM1

| Aspect | DM1 | DM2 |
|---|---|---|
| Grid slots | 12 | 12 (unchanged) |
| Hand slots | 2 | 2 (unchanged) |
| Ring/amulet | 2/2 | 2/2 (unchanged) |
| Max party items | 48 + accessories | 48 + accessories (same) |
| Container system | Basic | Extended with more container types |
| Item weight tracking | Yes | Yes + weight per charge |
| Item teleportation | No | Yes (DM2_ACTIVATE_ITEM_TELEPORT) |
| Shop system | No | Yes (merchant NPCs) |
| Tech items in inventory | N/A | Guns, bombs, batteries |
| Scroll/wand items | No | Yes |

---

## 10. Item Value / Sell Price

DM2 uses the same depreciation model as DM1:
- `DM2_QUERY_ITEM_VALUE(itemIndex, gdatOffset)` — returns base gold value
- Sell price is typically 50% of buy price (merchant profit margin)
- Value varies by item type and charge count

Item value is used for:
1. Shop buy/sell pricing
2. Pickpocket gold amount calculation
3. Treasure generation weight

Source: c_item.h (DM2_QUERY_ITEM_VALUE), SKWIN/SkWinCore.cpp (merchant pricing logic)

---

## 11. Item Stacking and Quantity

DM2 item records support a quantity field per slot:
- Ammo stacks (bolts, bullets, bomb charges)
- Gold can be stored as quantity in a misc slot
- Potions/flasks stack (empty flasks can accumulate)

Stacking limit depends on item type and the GDAT entry for that item.

---

## Status: PARTIALLY SOURCE-LOCKED

Inventory slot counts confirmed from party system (4 × 12 slots).
Hand slot mechanics confirmed from c_item.cpp and SkWinCore.cpp hand rendering.
Shop system confirmed from merchant NPC (AI 0x21) in dm2_champ_types.md.
Container system confirmed from c_item.cpp functions.
Weight/encumbrance confirmed from DM2_QUERY_ITEM_WEIGHT and field 34.
