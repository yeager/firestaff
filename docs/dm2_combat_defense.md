# DM2 V1 — Defense: Armor, Shields, Defense Bonuses

## Source
- `dm2_v1_combat.c` — target_defense in damage formula
- `skproject/SKWIN/DME.h:1505-1560` — AIDefinition struct (creature defense)
- `skproject/SKWIN/defines.h:705-716` — creature attack/defense flags
- `docs/champ_equipment.md` — champion equipment slots
- `docs/dm2_combat.md` — combat overview

---

## Overview: DM2 Defense vs DM1

DM2 defense system builds on DM1's equipment slot model while adding new mechanisms:
- Armor from equipment (same slot system as DM1)
- Defense value from champion stats and equipment
- Creature defense from AIDefinition struct
- Spell-based defense (spell shield, magical shield, fire shield)
- Environmental factors (weather in outdoor combat)

---

## Champion Defense Calculation

### Damage Formula (Defense Subtraction)

```c
// From dm2_v1_combat_resolve_attack()
damage = weapon->base_damage + attacker_strength / 4;
damage -= range_penalty;
damage -= target_defense;  // target's defense rating
return damage > 0 ? damage : 0;
```

Champion defense is subtracted from incoming damage after range penalty. If result <= 0, no damage is dealt.

### Champion Defense Sources

From `champ_equipment.md` and DM2 equipment system:

| Slot | Equipment | Defense Contribution |
|---|---|---|
| C01_SLOT_ACTION_HAND | Weapon (if shield) | +shield defense |
| C02_SLOT_HEAD | Helmet | +armor defense |
| C03_SLOT_TORSO | Body armor | +major defense |
| C04_SLOT_LEGS | Leg armor | +moderate defense |
| C05_SLOT_FEET | Foot armor | +minor defense |
| C00_SLOT_READY_HAND | Off-hand/shield | +shield defense |
| Enchantments | Spell Shield, Magical Shield | +spell defense |
| Wounds | Damage wounds | -defense |

### Shield Defense

Shields occupy C00_SLOT_READY_HAND (left hand). They provide defense bonus:
```
shield_defense = shield->defense_rating
```

When a shield is equipped in ready hand, it adds to the champion's total defense, and also contributes to the "ready hand" defense component used in the DM1 defense formula.

### Armor Defense

Armor pieces (C02-C05) provide defense based on material and quality:
- Leather: low defense
- Chain: moderate defense
- Plate: high defense
- Magical armor: defense + enchantment bonuses

Each armor piece has an `AllowedSlots` bitmask determining which slots it can occupy.

---

## Creature Defense (AIDefinition)

### Defense Field

```c
struct AIDefinition {
    Bit8u  ArmorClass;   // @2 — armor class (displayed to player)
    U8     Defense;      // @8 — defense rating (255 = undestroyable)
    Bit16u w24;          // @24 — resistance (fire/poison)
    ...
}
```

| Field | Description | Notes |
|---|---|---|
| `ArmorClass` | Displayed armor class | Shown to player |
| `Defense` | Actual defense rating | 255 = creature cannot be killed |
| `w24` | Resistance flags | Fire/poison/other resistances |

### Special Defense Flags

From `w0AIFlags` bitfield:

| Bit | Name | Defense Effect |
|---|---|---|
| 5 | `w0_5_5` | Non-material/intangible — physical attacks deal reduced or no damage |
| 9 | `AbsorbsMissile` | Most creatures — projectiles are absorbed instead of hitting |
| 10 | `w0_a_a` | Related to invisibility — ghosts, dragoth |

### Intangible Creatures (w0_5_5)
- Physical weapon attacks deal reduced or zero damage
- Only magic/spell attacks or specific enchantments affect them
- Lord Dragoth has this flag (non-material form)

### Missile Absorption (w0_9_9)
- Ranged attacks (crossbow, thrown, gun, spell projectiles) are absorbed
- Creature is immune to ranged attacks
- Must use melee or area-effect spells to damage

### Missile Turning (w30: 0x0800)
- Creature reflects projectiles back at attacker
- Source: SkWinCore.cpp:10479, 10561
- Different from absorption — projectile is redirected, not just negated

---

## Spell-Based Defense

### Spell Shield
- **Runes:** YA IR
- **Effect:** Party-wide damage reduction (likely ~50%)
- **Duration:** Unknown (enchantment)
- **Source:** SkGlobal.cpp:967 (index 2)

### Magical Shield
- **Runes:** YA EW (2 symbols)
- **Effect:** Personal damage reduction enchantment
- **Source:** SkGlobal.cpp:969 (index 4)

### Fire Shield
- **Runes:** FUL BRO NETA
- **Effect:** Reflects fire damage back at attacker
- **Source:** SkGlobal.cpp:973 (index 8)

### Spell Reflector
- **Runes:** ZO BRO ROS
- **Effect:** Reflects any spell back at caster
- **Source:** SkGlobal.cpp:977 (index 12)
- Creatures with `SOUND_CREATURE_REFLECTOR` also reflect spells

---

## Resistance System

### w24 Resistance Flags

AIDefinition.w24 stores creature resistances:

```c
struct AIDefinition {
    ...
    Bit16u w24;  // @24 — resistance/fire/poison
    ...
}
```

| Bit | Resistance |
|---|---|
| Fire resistance | Creature takes reduced fire damage |
| Poison resistance | Creature takes reduced poison damage |
| General resistance | Various damage types |

Creatures with high poison damage (from AttacksSpells flags like POISON_CLOUD, POISON_BOLT) may also have corresponding resistance to avoid self-damage.

---

## Outdoor Combat Defense

DM2 outdoor areas have additional environmental defense factors:

### Weather Effects
- **Fog/Storm:** Reduced visibility — accuracy penalty for ranged attacks
- **Gray sky:** Weather-related visibility reduction
- Weather spells (new in DM2) can create fog/rain to affect combat

### Outdoor Movement Model
- No first-person view — isometric/top-down
- Creatures in outdoor zones have different pathfinding
- Defense calculations may differ in outdoor context

---

## Defense vs Armor Class

DM2 distinguishes between two defense measurements:

| Term | Source | Description |
|---|---|---|
| `ArmorClass` | AIDefinition.b2 | Display value — shown to player, higher = harder to hit |
| `Defense` | AIDefinition.b8 | Actual damage reduction, 255=unkillable |

This parallels D&D-style AC vs actual damage reduction. The `ArmorClass` affects hit chance, while `Defense` affects damage dealt after a hit lands.

---

## DM1 vs DM2 Defense Comparison

| Feature | DM1 | DM2 |
|---|---|---|
| Armor slots | 4 (torso, legs, head, feet) | 5 (torso, legs, head, feet, shield) |
| Shield defense | Ready hand contributes | Dedicated C00 slot |
| Intangible creatures | Limited (ghosts) | Full w0_5_5 flag |
| Missile reflection | None | w30 flag |
| Spell defense | None | Spell Shield, Magical Shield, Fire Shield |
| Resistance system | None | w24 resistance flags |
| Outdoor defense | N/A | Weather effects, different model |
| Undestroyable creatures | None | Defense=255 |

---

## Undestroyable Creatures (Defense=255)

When `Defense` is set to 255 in AIDefinition:
- Creature cannot be killed by conventional damage
- Must use specific spell/effect to defeat
- Used for boss encounters (e.g., Lord Dragoth may have this)

---

## Status

**PARTIALLY SOURCE-LOCKED** — Damage formula defense from `dm2_v1_combat.c`. AIDefinition struct from `DME.h:1505-1560`. Resistance flags from `defines.h:705-716`. Outdoor combat from `dm2_combat.md`.
