# Nexus V1 — Armor System

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_combat.c`, `include/nexus_v1_combat.h`, `include/firestaff_item_encyclopedia.h`, `src/ui/firestaff_item_encyclopedia.c`, `include/firestaff_inventory_ui.h`, `docs/nexus_combat_items.md`

---

## 1. Armor Types in the Item Encyclopedia

The Firestaff item encyclopedia defines 6 armor types:

| Name | Attack | Defense | Weight | Notes |
|---|---|---|---|---|
| Leather Jerkin | 0 | 8 | 8 | Light leather protection |
| Mail Aketon | 0 | 14 | 24 | Chainmail vest |
| Plate Armor | 0 | 22 | 40 | Full plate mail — heavy but strong |
| Shield | 0 | 12 | 16 | Wooden shield |
| Helmet | 0 | 6 | 10 | Head protection |
| Boots | 0 | 4 | 6 | Sturdy leather boots |

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 2. Armor in the Combat Formula

Defense is passed as the second parameter to `nexus_v1_attack()`:

```c
Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker, int weapon_power, int defense);
```

Applied as:
```c
def_reduce = defense / 2 + rng(defense / 2);
damage -= def_reduce;
```

Source: `src/nexus/nexus_v1_combat.c`

---

## 3. Defense Reduction Formula

| Component | Formula | Notes |
|---|---|---|
| Minimum reduction | defense / 2 | Always applied (integer division) |
| Random addition | rng(defense / 2) | 0 to floor(defense/2) |
| Total reduce | defense/2 + rng(defense/2) | Applied to raw damage |
| Floor | 1 | Minimum damage even if reduction exceeds raw damage |

Example — Plate Armor (defense=22):
- Minimum reduction: 22/2 = 11
- Maximum reduction: 11 + 11 = 22
- Raw damage 20 becomes 0 to 9 (clamped to 1 minimum)

Example — Leather Jerkin (defense=8):
- Minimum reduction: 8/2 = 4
- Maximum reduction: 4 + 4 = 8
- Raw damage 15 becomes 7 to 11

Source: `src/nexus/nexus_v1_combat.c` (rng: rand() % max)

---

## 4. Armor Slots (UI Layer)

The inventory UI header describes equipment slots:
```
Equipment slots: head, torso, legs, feet, hands, weapon, shield
```

FS_InventoryItem.equipped field:
- 0 = not equipped (in inventory)
- 1 = head, 2 = torso, 3 = legs, 4 = feet, 5 = hands, 6 = weapon, 7 = shield

This is 7 equipment slots + 30 inventory slots per champion.
No per-slot armor tracking was found in the Nexus V1 game-logic source.

Source: `include/firestaff_inventory_ui.h`

---

## 5. Shield Special Case

Shield (defense=12, weight=16) is a separate hand slot item.
Shield defense stacks with body armor defense.

In DM1/DM2, the shield was a dedicated hand slot. This pattern is inherited
by Nexus V1 — the shield hand slot is part of the overall armor system.

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 6. Comparison: Nexus V1 vs DM1 Defense Formula

| Game | Defense Reduction Formula |
|---|---|
| DM1 | defense * random(0.5-1.0) — floating point |
| Nexus V1 | defense / 2 + rng(defense / 2) — integer |

Both produce the same statistical range (defense/2 to defense) but Nexus V1
uses integer arithmetic, causing slight rounding differences at odd defense
values (e.g., defense=15: DM1 gives 7.5-15, Nexus V1 gives 7-15).

Source: `src/nexus/nexus_v1_combat.c`, parity analysis

---

## 7. Armor Stats Summary

| Armor Type | Defense | Weight | Defense/lb |
|---|---|---|---|
| Boots | 4 | 6 | 0.67 |
| Helmet | 6 | 10 | 0.60 |
| Leather Jerkin | 8 | 8 | 1.00 |
| Shield | 12 | 16 | 0.75 |
| Mail Aketon | 14 | 24 | 0.58 |
| Plate Armor | 22 | 40 | 0.55 |

Best defense-per-weight: Leather Jerkin (1.0).
Highest raw defense: Plate Armor (22).

---

## 8. What is NOT in Nexus V1 Armor

| Feature | DM2 | Nexus V1 |
|---|---|---|
| Magic armor variants | Yes | No |
| Accessory rings | Yes | No dedicated entries |
| Accessory amulets | Yes | No dedicated entries |
| Armor degradation | Yes | No |
| Anti-magic armor | Yes | No |
| Fire resistance armor | Yes | No |

Champion anti_magic and anti_fire stats are intrinsic (in `Nexus_V1_Champion`)
— not derived from equipped armor.

Source: `include/nexus_v1_champions.h`

---

## Status: SOURCE-LOCKED

- Defense formula in combat: **source-locked** — `nexus_v1_combat.c`
- Armor item roster: **source-locked** — `firestaff_item_encyclopedia.c`
- Equipment slot layout: **source-locked** — `firestaff_inventory_ui.h`
- Champion anti_magic/anti_fire stats: **source-locked** — `nexus_v1_champions.h`
