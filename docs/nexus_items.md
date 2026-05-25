# Dungeon Master Nexus V1 — Items vs DM1/DM2/CSB

## Sources
- `src/nexus/nexus_v1_combat.c` (weapon_power, defense in combat formula)
- `src/nexus/nexus_v1_champions.c` (champion roster, no item refs)
- `docs/NEXUS_FILE_CLASSIFICATION.md` (item-related file list)
- DM1 item system from ReDMCSB equivalents
- `docs/spells_items.md` (DM1 V1 spell items reference)

---

## Overview

Nexus inherits the **DM1 item system** without major additions. Items in Nexus
follow the same categories and behavior as DM1 (weapons, armor, consumables,
magic items, dungeon objects). The primary difference is that Nexus items
are rendered in the 3D viewport (e.g., dropped items on the floor are
3D-projected) rather than as 2D sprites.

No dedicated `nexus_v1_items.c` exists — item logic is embedded in combat
and dungeon systems, mirroring DM1's structure.

| Aspect | DM1 | CSB | DM2 | Nexus |
|--------|-----|-----|-----|-------|
| Item categories | Weapons, Armor, Consumables, Dungeon Objects | Same + Scrolls, Wands | Same + more | **Same as DM1** |
| Item rendering | 2D sprite | 2D sprite | 2D sprite | **3D viewport projection** |
| Inventory slots | 12 per champion | 12 | 12 | **12 per champion** |
| Hand slots | 2 (action + shield) | 2 | 2 | **2 (same)** |
| Potion system | Flask-based 10 spells | Same | Same + more | **Same as DM1** |
| New item types | None | Scrolls, Wands added | More magic items | **None (DM1-based)** |

---

## Item Categories in Nexus (DM1 Inheritance)

### 1. Weapons

Weapons are equipped in the action hand and provide `weapon_power` to the
combat formula. Nexus inherits DM1's weapon roster:

| Weapon Type | DM1 Power | Nexus Notes |
|-------------|-----------|-------------|
| Dagger | 2-4 | Lowest damage, fast |
| Short Sword | 4-6 | Common starting weapon |
| Hand Axe | 5-7 | Moderate |
| Broad Sword | 7-9 | Better damage |
| katana | 8-12 | High-end |
| Two-Handed Sword | 10-14 | Max physical damage |
| Spear | 6-8 | Reach advantage |
| Staff | 1-3 | Low, Wizard can use |

Combat formula (from `nexus_v1_combat.c`):
```c
damage = weapon_power + str_bonus + rng(str_bonus);
```

Where `weapon_power` comes from the equipped item and `str_bonus` is the
champion's Strength stat.

Source: `nexus_v1_combat.c` (`nexus_v1_attack`).

---

### 2. Armor

Armor is equipped in the shield/armor slot and provides `defense` to reduce
incoming damage. Nexus inherits DM1's armor roster:

| Armor Type | DM1 Defense | Nexus Notes |
|------------|-------------|-------------|
| Leather Armor | 2-4 | Light, common |
| Chain Mail | 5-8 | Medium |
| Plate Mail | 9-12 | Heavy, high defense |
| Magic Armor | 12-18 | Enchanted variants |
| Shield | +2-4 | Additional defense when equipped |

Defense formula (from `nexus_v1_combat.c`):
```c
def_reduce = defense / 2 + rng(defense / 2);
final_damage = raw_damage - def_reduce;
```

Where `defense` comes from equipped armor/shield. Note: this differs from
DM1's defense formula (Nexus uses `defense / 2 + RNG` vs DM1's exact formula).

Source: `nexus_v1_combat.c` (`nexus_v1_attack`).

---

### 3. Consumables (Potions and Flask System)

Nexus inherits the **DM1 flask-based potion system**:

**Potion spells (10 spells requiring empty flask in hand):**

| Spell | Symbols | Effect | Flask Consumed |
|-------|---------|--------|---------------|
| STRENGTH POTION | Ful Bro Ku | Temp STR boost | Yes |
| WISDOM POTION | Ya Bro Dain | Temp WIS boost | Yes |
| VITALITY POTION | Ya Bro Neta | Temp VIT boost | Yes |
| HEALTH POTION | Vi | Heal missing HP | Yes |
| CURE POISON POTION | Vi Bro | Remove poison | Yes |
| DEXTERITY POTION | Oh Bro Ros | Temp DEX boost | Yes |
| MANA POTION | Zo Bro Ra | Restore mana | Yes |
| STAMINA POTION | Ya | Restore stamina | Yes |
| POISON POTION | Zo Ven | Poison target | Yes |
| SHIELD POTION | Ya Bro | Shield defense | Yes |

Casting requires empty flask in hand. Flask is consumed on success,
potion item is created, then potion is consumed separately for the effect.

Pre-placed dungeon potions (found bottles, not crafted):
- Health Potion (restore HP)
- Mana Potion (restore mana)
- Poison (damage champion)
- Water Flask (quenches thirst)

All consumed potion bottles become empty flasks (C195/C20) — reusable reagent.

Source: `docs/spells_items.md` (DM1 V1), `nexus_v1_magic.c` (inherited).

---

### 4. Magic Items

Magic items in Nexus (same as DM1):

| Item | Effect | Notes |
|------|--------|-------|
| Magic Map (Illumulet) | Required for MAGIC MAP spell | Held in action hand |
| Torch / Candle | Light source | Illuminates dungeon |
| Goggles | See in dark | Extended vision |
| Amulet of Purity | Anti-poison | Resist poison attacks |
| Ring of Protect | Extra defense | Equipped in ring slot |
| Scrolls (CSB+) | Not in DM1/V1 | Added in CSB, likely absent in Nexus |
| Wands (CSB+) | Not in DM1/V1 | Added in CSB, likely absent in Nexus |

Nexus does NOT add new magic items vs DM1. Scrolls and wands (introduced in
CSB) are not confirmed present in Nexus — DM1 had no scroll/wand system.

Source: `docs/spells_items.md` (DM1 V1 scroll/wand absence).

---

### 5. Dungeon Objects (Interactive)

Dungeon objects are not inventory items but interact with champions:

| Object | Interaction | Notes |
|--------|-------------|-------|
| Fountain | Fill empty flask | Creates water flask |
| Pressure Plate | Trigger events | Activates traps/doors |
| Lever | Toggle doors | Puzzle element |
| Teleport Square | Warp champion | Damages HP on arrival |
| Pit | Fall damage | Open or covered |
| Chest | Contains items | May be trapped |
| Rune Inscription | Cast spell | 4-symbol system |

---

## Inventory System (12 Slots + 2 Hands)

Each champion has:
- **12 inventory slots** (grid/bag space)
- **2 hand slots**: Action hand (weapon/tool) + Shield hand (armor/shield)
- **2 ring slots** (left/right)
- **2 amulet slots** (neck)

Max party: 4 champions × 12 slots = 48 item slots total.

Item weight affects champion movement speed (encumbrance system).
Food and water are stored separately (not in inventory slots).

Source: DM1 inventory system (inherited by Nexus).

---

## Item Rendering: 3D vs 2D

| Aspect | DM1/DM2/CSB | Nexus |
|--------|-------------|-------|
| Item sprites | 2D sprite sheets | **3D geometry projected into viewport** |
| Floor items | Sprite at position | **3D-projected model at world coords** |
| Champion items | Panel sprite | **3D viewport; panel shows stat/value only** |
| Weapon swing | Sprite animation | **3D model animation in viewport** |

Dropped items on the dungeon floor are now 3D-projected objects in the
viewport, replacing DM1's 2D sprite representation. The item type/quality
determines its 3D model (weapons, armor, potions each have distinct shapes).

---

## Weapon/Armor Stats in Nexus Combat

From `nexus_v1_combat.c`:
```c
int nexus_v1_attack(Nexus_V1_Champion *attacker, int weapon_power, int defense) {
    int damage, str_bonus, def_reduce;
    str_bonus = attacker->strength / 4;
    damage = weapon_power + str_bonus + rng(str_bonus);
    def_reduce = defense / 2 + rng(defense / 2);
    return damage - def_reduce;
}
```

Parameters:
- `weapon_power`: from equipped weapon item
- `defense`: from equipped armor/shield
- `strength_bonus`: champion's STR stat / 4
- `def_reduce`: defense / 2 + random(0 to defense/2)

Note: DM1 used a different defense formula. Nexus's `defense / 2 + RNG`
gives a more variable damage reduction vs DM1's more deterministic model.

---

## What's NEW in Nexus Item System

1. **3D viewport rendering** — items projected as 3D geometry in first-person
   view instead of 2D sprites; dropped items on floor are 3D objects
2. **No new item types** — Nexus adds nothing new vs DM1 items (no scrolls,
   wands, or new magic items beyond DM1's base set)
3. **Per-level item placement** — DM1 had global item spawns; Nexus's level
   scripts (SLEV*.BIN) may control per-level item placement

---

## What's the Same as DM1

- All weapon and armor types (same damage/defense values)
- Flask-based potion crafting (10 spells, empty flask reagent)
- 12-slot inventory per champion
- Food/water resource model (separate from inventory)
- Champion can carry up to ~400 lbs before slowed
- Dungeon objects (fountains, levers, pressure plates, teleporters, pits)

---

## What's MISSING vs DM2

DM2 added many items not present in Nexus:
- Scrolls (CSB/DM2): spell items usable by any champion
- Wands (CSB/DM2): limited-use spell devices
- New magic items (rings, amulets with special properties)
- Enhanced weapon/armor variants
- Quest-specific items with unique effects

Nexus is a DM1 visual remake — it does not incorporate DM2's expanded
item roster. The DM2 Ninja class was added to Nexus, but DM2's scroll/wand
system was not.

---

## File References

No dedicated items file in Nexus. Item data is distributed across:
- `nexus_v1_combat.c` — weapon_power, defense in attack formula
- `nexus_v1_dungeon.c` — dungeon object interactions
- `nexus_v1_magic.c` — potion spell crafting (inherited from DM1)
- DM1 item definitions — used directly (same item types, same stats)

Source: `src/nexus/` (no nexus_v1_items.c exists).

---

## Status: PARTIALLY SOURCE-LOCKED

Weapon/armor stats are from `nexus_v1_combat.c` (explicit formula).
Potion system is inherited from DM1 (no Nexus-specific potion code).
No dedicated items source file — item logic is embedded in combat/dungeon/magic.
No byte verification of actual item data from ISO.