# Nexus V1 — Weapons and Items in Combat

## Source
- `src/nexus/nexus_v1_combat.c` — weapon_power parameter in attack formula
- `src/nexus/nexus_v1_magic.c` — potion crafting (flask system)
- No `nexus_v1_items.c` exists — item logic is distributed

---

## Item System Overview

Nexus V1 uses the DM1 item set unchanged. No new weapon or armor types were
added. Item data for combat purposes is injected at call sites:

```c
Nexus_CombatResult r = nexus_v1_attack(attacker, weapon_power, defense);
```

- `weapon_power` is passed from the equipped weapon
- `defense` is the target's defense stat

---

## Weapon Types

| Type | Present | Notes |
|---|---|---|
| Melee (sword, axe) | Yes | DM1 melee weapons |
| Thrown (dagger) | Yes | DM1 thrown weapons |
| Crossbow | Yes | DM1 ranged weapon |
| Gun | **No** | DM2 tech weapon — not in Nexus |
| Bomb | **No** | DM2 AoE weapon — not in Nexus |
| Magic (staff) | Yes | DM1 staff-based magic |

---

## Armor and Defense

Champion defense is applied in the damage formula:
```c
def_reduce = defense / 2 + rng(defense / 2);
damage -= def_reduce;
```

Champion defense likely comes from equipped armor (same as DM1).

---

## What's Missing vs DM2

DM2 introduced many item types not present in Nexus:

| Item | DM2 | Nexus V1 |
|---|---|---|
| Scrolls | Full spell scroll system | **Absent** |
| Wands | Limited-use spell devices | **Absent** |
| Tech items | Battery-powered devices | **Absent** |
| Gun / bomb | Gunpowder-based weapons | **Absent** |
| New rings / amulets | Extended accessory roster | **Absent** |
| Quest items | Unique-effect items | DM1 set only |

Nexus is a DM1 visual remake — it does not incorporate DM2's expanded item roster.

---

## Flask-Based Potion Crafting (Inherited from DM1)

The only item-based magic in Nexus V1 is the DM1 flask system:
- 10 spells, each requiring a specific rune word combination
- Requires empty flask as reagent
- Mana cost scales with power level and element (see `nexus_combat_magic.md`)

---

## Inventory

- 12-slot inventory per champion (same as DM1)
- ~400 lb carry limit before movement penalty (same as DM1)
- Food/water resource tracking (separate from inventory)

---

## Item Placement

DM1 had global item spawns. Nexus uses per-level scripts (`SLEV*.BIN`)
which may control per-level item placement — but no Nexus-specific
item placement code was examined in this audit.

---

## Status: PARTIALLY SOURCE-LOCKED

- Weapon combat integration: **source-locked** — `weapon_power` passed to attack
- Potion crafting: **source-locked** — `nexus_v1_magic.c`
- Armor/defense integration: **inferred** — formula uses defense stat
- DM2 items: **absent** — no gun/bomb/scroll/wand in Nexus code
- No dedicated items source file — item logic embedded in combat/dungeon/magic