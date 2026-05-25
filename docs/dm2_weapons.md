# DM2 V1 — Weapons: Gun, Bomb, Thrown, and New Weapon Types

**Audit date:** 2026-05-25
**Sources:** include/dm2_v1_combat.h, docs/dm2_combat_weapons.md, skproject/SKWIN/SkWinCore.cpp, docs/dm2_newfeatures.md

---

## 1. DM2_WeaponType Enum

```c
typedef enum {
    DM2_WEAPON_MELEE    = 0,  // swords, axes, etc. — same as DM1
    DM2_WEAPON_THROWN   = 1,  // daggers, rocks — NEW in DM2
    DM2_WEAPON_CROSSBOW = 2,  // carried from DM1
    DM2_WEAPON_GUN      = 3,  // tech weapon — NEW in DM2
    DM2_WEAPON_BOMB     = 4,  // AoE explosive — NEW in DM2
    DM2_WEAPON_MAGIC    = 5,  // staff, spell-based — carried from DM1
} DM2_WeaponType;
```

Source: dm2_v1_combat.h

---

## 2. DM2_V1_WeaponInfo Struct

```c
typedef struct {
    DM2_WeaponType type;       // weapon category
    int base_damage;           // base damage per hit
    int range;                 // max range in tiles (1=melee)
    int ammo_required;         // ammo consumed per shot
    int tech_level;            // 0=magic era, 1+=tech required
} DM2_V1_WeaponInfo;
```

Source: dm2_v1_combat.h, dm2_combat_weapons.md

---

## 3. Ranged Weapon Classification

```c
int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return type == DM2_WEAPON_CROSSBOW ||
           type == DM2_WEAPON_GUN      ||
           type == DM2_WEAPON_THROWN   ||
           type == DM2_WEAPON_BOMB;
}
```

All non-MELEE types are ranged — including thrown weapons and bombs.
This means DM2 has 4 ranged categories vs DM1's 1 (crossbow).

Source: dm2_v1_combat.c (dm2_v1_combat_is_ranged)

---

## 4. Gun Weapons (DM2 New)

### Characteristics
- **Higher base damage** than crossbow
- **Shorter range** than crossbow
- **tech_level required** — champions must have sufficient tech stat
- **Ammo required** — gunpowder/bullets consumed per shot
- **Battery power source** — some guns use charges (type 1)

### DM2 Gun Examples
From skproject/SKWIN/SkGlobal.cpp item references and dm2_magic.md:
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

Source: dm2_combat_weapons.md, dm2_magic.md, SkWinCore.cpp

---

## 5. Bomb Weapons (DM2 New)

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

Source: dm2_combat_weapons.md

---

## 6. Thrown Weapons (DM2 New vs DM1)

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

Source: dm2_combat_weapons.md

---

## 7. Crossbow (Carried from DM1)

- **Base damage:** moderate (carried from DM1)
- **Range:** medium (longer than gun but slower projectile)
- **No tech_level requirement** — purely skill-based
- **Ammo required** — crossbow bolts consumed per shot

Source: dm2_combat_weapons.md

---

## 8. Magic Weapons (Staff — Carried from DM1)

- **type = DM2_WEAPON_MAGIC**
- **No range penalty** — spell-based attacks don't suffer distance degradation
- **No ammo** — mana-powered (uses champion's MP)
- **Base damage** from spell effect power, not weapon physical properties

Source: dm2_combat_weapons.md

---

## 9. Melee Weapons (Carried from DM1)

- **type = DM2_WEAPON_MELEE**
- Swords, axes, spears, etc. — same roster as DM1
- **Range = 1** — must be adjacent to target
- **No tech_level** requirement
- **No ammo** — direct weapon vs creature combat

Source: dm2_combat_weapons.md, DM1 weapon base

---

## 10. Tech Level Gate for Ranged Weapons

A fundamental DM2 innovation: guns and bombs require `tech_level` stat on the champion.

| Champion Tech Level | Access |
|---|---|
| 0 | Melee, crossbow, thrown, magic only |
| 1 | Basic guns, basic bombs |
| 2+ | Advanced guns, advanced bombs, hybrid items |

This creates a progression system where combat effectiveness increasingly depends on
both magic AND tech stats — a major DM2 innovation over DM1.

Source: dm2_magic.md (item affinity system), dm2_champ_changes.md

---

## 11. Ammo Consumption System

All ranged weapons have `ammo_required` field:
```c
int ammo_required;  // field present in DM2_V1_WeaponInfo struct
```

Ammo consumption tracked:
- Quiver/ammo inventory tracking
- Ammo depletion handling
- Re-arm action for ranged champions

GDAT weapon fields for projectile/ammo:
- GDAT_ITEM_WEAPON_PROJECTILE_FLAG (field 0x05)
- Missile strength (fields 0x09 / 0x0D)

Source: dm2_v1_combat.h, dm2_newfeatures.md §4, SkWinCore.cpp:19354

---

## 12. Damage Comparison (Approximate)

| Weapon Type | Base Damage | Range | Tech Req | Notes |
|---|---|---|---|---|
| Melee (sword) | 6-10 | 1 | 0 | Direct damage |
| Thrown (dagger) | 4-6 | 3 | 0 | Consumable/ammo |
| Crossbow | 5-8 | 5 | 0 | Bolt ammo |
| Gun | 8-12 | 3 | 1+ | Battery/ammo |
| Bomb | 10-15 | 2 | 1+ | AoE, all targets |
| Magic (staff) | 6-12 | 6+ | 0 | Mana cost |

Source: dm2_combat_weapons.md

---

## 13. Hybrid Items: Tech + Magic Weapons

DM2 introduces **hybrid weapons** that require both tech AND magic:

```c
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

Source: dm2_magic.md (item affinity system)

---

## 14. Weapon GDAT Extensions (DM2 vs DM1)

DM2 extends the GDAT weapon category with new fields:
- GDAT_CATEGORY_WEAPONS — extended weapon data with projectile flags
- Animation field (06 00 00) — weapon swing/projectile animation
- Knock/hit obstacle sound (85 00 00)

Weapon projectile flag (GDAT_ITEM_WEAPON_PROJECTILE_FLAG, field 0x05):
- Controls whether weapon fires a visible projectile
- Gun/bomb always have projectile flag set
- Crossbow always has projectile flag set
- Melee weapons typically have projectile flag cleared

Source: dm2_newfeatures.md §4, SkWinCore.cpp:19354

---

## 15. Weapon vs DM1: Summary of Changes

| Feature | DM1 | DM2 |
|---|---|---|
| Melee weapons | ✅ Swords, axes, etc. | ✅ Same |
| Crossbow | ✅ | ✅ Carried over |
| Thrown weapons | ❌ | ✅ Daggers, rocks |
| Gun weapons | ❌ | ✅ Tech tier 1+ |
| Bomb weapons | ❌ | ✅ AoE explosives |
| Magic staff | ✅ | ✅ Same |
| Tech level gate | ❌ | ✅ New |
| Hybrid items | ❌ | ✅ New |
| Ammo tracking | Basic (bolts) | ✅ Extended (bullets, charges) |
| Projectile animation | Basic | ✅ Extended with animation field |

---

## Status: PARTIALLY SOURCE-LOCKED

Weapon enum and struct confirmed from `dm2_v1_combat.h`.
Gun/bomb details from `dm2_combat.md` and `dm2_magic.md`.
Tech level gate from item affinity system in `skproject/SKWIN/SkGlobal.cpp`.
GDAT extension confirmed from `dm2_newfeatures.md`.
Damage values are approximate (not byte-verified from GDAT binary).
