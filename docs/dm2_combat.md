# DM2 V1 — Combat Mechanics

## Source
-  — SKULL.ASM combat resolver  
-  — weapon types and structures

---

## Weapon Types (New vs DM1)

DM1 had melee and ranged (crossbow). DM2 adds a tech tier:

| Type | Enum | Notes |
|------|------|-------|
| Melee |  | Same as DM1 |
| Thrown |  | Daggers, rocks |
| Crossbow |  | Carried over from DM1 |
| **Gun** |  | **New — tech weapon, requires tech_level** |
| **Bomb** |  | **New — AoE, area damage** |
| Magic |  | Spell-based attacks |

Key difference: guns and bombs require  on the champion.

---

## Damage Formula

```c
damage = weapon->base_damage + attacker_strength / 4;
if (distance > weapon->range) return 0;       // out of range
range_penalty = (distance - 1) * damage / 10; // -10% per extra tile
damage -= range_penalty;
damage -= target_defense;
return damage > 0 ? damage : 0;
```

Comparison with DM1:
- DM1: fixed base damage, no range penalty, defense subtraction same
- DM2: introduces strength bonus () and per-tile range penalty
- DM2 bombs: AoE — all creatures in blast radius take damage (not individually targeted)

---

## Ranged Combat

```c
int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return type == DM2_WEAPON_CROSSBOW || type == DM2_WEAPON_GUN ||
           type == DM2_WEAPON_THROWN || type == DM2_WEAPON_BOMB;
}
```

- Guns: higher base damage than crossbow, shorter range, tech_level gate
- Bombs: short range, but AoE — strategic for groups
- All ranged weapons consume ammo (field present but not enforced in stub)

---

## Companion Combat (New in DM2)

DM2 adds up to 4 NPC companions () who fight alongside the party:
- Companions have their own  and  stats
-  stat (0–100) affects behavior
- : 0=follow party, 1=guard position, 2=aggressive
- Companion tick runs each game loop iteration ()

---

## Outdoor Combat

DM2 outdoor areas (sky, trees, buildings) use a different movement/combat model:
- No first-person view — isometric/top-down outdoor renderer
- Creatures in outdoor zones have different pathfinding
- Weather affects visibility and possibly accuracy (fog/storm → gray sky)

---

## Source Evidence

> SKULL.ASM: combat damage calculation, ranged path
