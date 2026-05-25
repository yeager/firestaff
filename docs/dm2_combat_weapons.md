# DM2 V1 — Weapons: Ranged, Gun, Bomb, New Types

## Source
- `dm2_v1_combat.h` — DM2_WeaponType enum, DM2_V1_WeaponInfo struct
- `dm2_v1_combat.c` — dm2_v1_combat_is_ranged(), resolve attack
- `dm2_magic.md` — tech/magic item system
- `skproject/SKWIN/SkGlobal.cpp:966-1011` — weapon/item references

---

## Overview: DM2 Weapon Types vs DM1

| Type | DM1 | DM2 |
|---|---|---|
| Melee | ✅ Sword, axe, etc. | ✅ Same |
| Thrown | ❌ | ✅ Daggers, rocks |
| Crossbow | ✅ | ✅ Carried over |
| Gun | ❌ | ✅ **New** — tech weapon |
| Bomb | ❌ | ✅ **New** — AoE |
| Magic | ✅ Staff | ✅ Staff (unchanged) |

**DM2 adds a tech tier** — guns and bombs that require `tech_level` on the champion.

---

## DM2_WeaponType Enum

```c
typedef enum {
    DM2_WEAPON_MELEE    = 0,  // swords, axes, etc.
    DM2_WEAPON_THROWN   = 1,  // daggers, rocks
    DM2_WEAPON_CROSSBOW = 2,  // carried from DM1
    DM2_WEAPON_GUN      = 3,  // tech weapon (new)
    DM2_WEAPON_BOMB     = 4,  // AoE explosive (new)
    DM2_WEAPON_MAGIC    = 5,  // staff, spell-based
} DM2_WeaponType;
```

---

## DM2_V1_WeaponInfo Struct

```c
typedef struct {
    DM2_WeaponType type;     // weapon category
    int base_damage;         // base damage per hit
    int range;               // max range in tiles (1=melee)
    int ammo_required;        // ammo consumed per shot
    int tech_level;          // 0=magic era, 1+=tech required
} DM2_V1_WeaponInfo;
```

---

## Ranged Weapon Classification

```c
int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return type == DM2_WEAPON_CROSSBOW ||
           type == DM2_WEAPON_GUN      ||
           type == DM2_WEAPON_THROWN   ||
           type == DM2_WEAPON_BOMB;
}
```

All non-MELEE types are ranged — including thrown weapons and bombs.

---

## Gun Weapons (DM2 New)

### Characteristics
- **Higher base damage** than crossbow
- **Shorter range** than crossbow
- **tech_level required** — champions must have sufficient tech stat
- **Ammo required** — gunpowder/bullets consumed per shot
- **Battery power source** (type 1) — some guns use charges

### DM2 Gun Examples
From `skproject/SKWIN/SkGlobal.cpp` item references and `dm2_magic.md`:
- Laser/energy weapons (tech tier 2+)
- Gunpowder firearms (tech tier 1)
- Power source: battery (1 charge per shot) or manual

### Tech Level Requirement
```c
// From dm2_v1_item_can_use() logic
case DM2_ITEM_TECH:
    return champion_tech >= item->tech_level;
```
Gun weapons require `tech_level >= 1` (or higher for advanced guns).

---

## Bomb Weapons (DM2 New)

### Characteristics
- **AoE damage** — all creatures in blast radius take damage
- **Short range** — must be within throw distance
- **tech_level required** — bomb construction/usage requires tech stat
- **Consumes bomb charges** — not individually targeted

### Damage Model
```
// Bombs hit all targets in blast radius simultaneously
// Each affected creature rolls damage independently
damage = weapon->base_damage + attacker_strength / 4;
// Applied to ALL creatures in radius, not one target
```

Unlike single-target weapons, bombs deal damage to every creature within the blast radius.

### Example Bomb Types
- Explosive grenades (tech tier 1)
- Magic-tech hybrid bombs (tech tier 2, magic tier 1)

---

## Thrown Weapons (DM2 New vs DM1)

### Difference from DM1
DM1 had no thrown weapons category. DM2 adds:
- **Daggers** — can be thrown (type=DM2_WEAPON_THROWN)
- **Rocks** — improvised thrown projectiles
- Lower base damage than gun, higher than crossbow
- No ammo consumed (weapon itself is consumed or reusable)

### Thrown Weapon Properties
```c
DM2_WEAPON_THROWN: range = 3, base_damage moderate, ammo_required=0
```

---

## Crossbow (Carried from DM1)

- **Base damage:** moderate (carried from DM1)
- **Range:** medium (longer than gun but slower projectile)
- **No tech_level requirement** — purely skill-based
- **Ammo required** — crossbow bolts consumed per shot

---

## Tech Level Gate

A fundamental DM2 innovation: guns and bombs require `tech_level` stat on the champion.

| Champion Tech Level | Access |
|---|---|
| 0 | Melee, crossbow, thrown, magic only |
| 1 | Basic guns, basic bombs |
| 2+ | Advanced guns, advanced bombs, hybrid items |

This creates a progression system where combat effectiveness increasingly depends on both magic AND tech stats.

---

## Ammo Consumption

All ranged weapons have `ammo_required` field:
```c
// From dm2_v1_combat.c stub
int ammo_required;  // field present in struct
```

Ammo consumption is tracked but not fully enforced in V1 stubs. Full implementation requires:
- Quiver/ammo inventory tracking
- Ammo depletion handling
- Re-arm action for ranged champions

---

## Magic Weapons (Staff)

- **type = DM2_WEAPON_MAGIC**
- **No range penalty** — spell-based attacks don't suffer distance degradation
- **No ammo** — mana-powered
- **Base damage** from spell effect power, not weapon physical properties

---

## Damage Comparison (Approximate)

| Weapon Type | Base Damage | Range | Tech Req | Notes |
|---|---|---|---|---|
| Melee (sword) | 6-10 | 1 | 0 | Direct damage |
| Thrown (dagger) | 4-6 | 3 | 0 | Consumable/ammo |
| Crossbow | 5-8 | 5 | 0 | Bolt ammo |
| Gun | 8-12 | 3 | 1+ | Battery/ammo |
| Bomb | 10-15 | 2 | 1+ | AoE, all targets |
| Magic (staff) | 6-12 | 6+ | 0 | Mana cost |

---

## Hybrid Items: Tech + Magic Weapons

DM2 introduces **hybrid weapons** that require both tech AND magic:

```c
// From dm2_magic.md — item affinity system
typedef enum {
    DM2_ITEM_MAGIC  = 0,  // traditional spells
    DM2_ITEM_TECH   = 1,  // guns, bombs
    DM2_ITEM_HYBRID = 2,  // requires both stats
} DM2_ItemAffinity;

// Hybrid item usage check
case DM2_ITEM_HYBRID:
    return champion_tech >= item->tech_level &&
           champion_magic >= item->magic_level;
```

Example hybrid weapons:
- Enchanted gun (magic-enhanced firearm)
- Runic bomb (magic-tech explosive)

---

## Status

**PARTIALLY SOURCE-LOCKED** — weapon enum and struct confirmed from `dm2_v1_combat.h`. Gun/bomb details from `dm2_magic.md` and `dm2_combat.md`. Tech level gate from item affinity system in `skproject/SKWIN/SkGlobal.cpp`.
