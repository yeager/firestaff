# DM2 V1 — Combat Formula

## Source
- `SKULL.ASM` — combat damage calculation
- `dm2_v1_combat.c` — Firestaff resolver implementation
- `dm2_v1_combat.h` — weapon/struct definitions

---

## Hit Resolution

DM2 hit resolution follows a skill-based check (from SKULL.ASM combat routines):

1. **Determine attack type** — melee vs ranged; creature vs champion
2. **Range check** — if ranged and `distance > weapon->range`, attack misses (0 damage)
3. **Hit roll** — compare attacker's attack rating vs target's defense rating

### Champion vs Creature Attack Resolution

```
attack_roll = attacker_attack_skill + RAND(1..20)
defense_value = target_defense_rating
if (attack_roll >= defense_value) → HIT
else → MISS
```

This is a d20-style roll: the attacker's skill acts as a bonus, and the target's defense sets a difficulty threshold. If `attack_roll >= defense_value`, the attack lands.

---

## Damage Calculation

### Champion Ranged/Weapon Damage (SKULL.ASM)

```c
int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance)
{
    int damage, range_penalty;
    if (!weapon) return 0;

    // Base damage + strength bonus
    damage = weapon->base_damage + attacker_strength / 4;

    // Range check
    if (distance > weapon->range) return 0;

    // Range penalty: -10% of base per extra tile
    range_penalty = (distance - 1) * damage / 10;
    damage -= range_penalty;

    // Defense subtraction
    damage -= target_defense;

    return damage > 0 ? damage : 0;
}
```

### Formula Components

| Component | Description | Notes |
|---|---|---|
| `weapon->base_damage` | Weapon base damage | Set per weapon type in struct |
| `attacker_strength / 4` | Strength bonus | Integer division; +1 per 4 STR |
| `distance` | Tiles to target | 1 = adjacent |
| `(distance - 1) * damage / 10` | Range penalty | -10% per extra tile beyond first |
| `target_defense` | Target armor/defense | Applied after range penalty |
| `max(0, damage)` | Minimum damage | 0 if negative result |

### Example Calculation

```
Weapon: Gun (base_damage=8, range=4)
Attacker STR: 20
Target defense: 3
Distance: 3 tiles

damage = 8 + (20/4)              = 13
range_penalty = (3-1) * 13 / 10  = 2
damage = 13 - 2                    = 11
damage = 11 - 3                   = 8
Final damage: 8
```

---

## Creature vs Champion Damage

Creatures deal fixed damage based on their `AttackStrength` and `PoisonDamage` values in the AIDefinition struct:

```c
// From AIDefinition struct (DME.h:1505-1538)
struct AIDefinition {
    Bit16u w0AIFlags;    // behavior flags
    Bit8u  ArmorClass;   // armor class (defense)
    i8     b3;           // unknown
    Bit16u BaseHP;       // initial HP
    U8     AttackStrength;  // @6 — base attack damage
    U8     PoisonDamage;    // @7 — poison damage on hit
    U8     Defense;          // @8 — 255=undestroyable
    ...
    X16    AttacksSpells;   // attack/spell flags
}
```

Creature attack damage is `AttackStrength` ± variability from creature type, plus any applicable spell damage from `AttacksSpells` flags (fireball, lightning, etc.).

---

## Comparison: DM1 vs DM2 Damage Formulas

| Aspect | DM1 | DM2 |
|---|---|---|
| Base damage | Fixed per weapon | Fixed per weapon |
| Strength bonus | None | +STR/4 |
| Range penalty | None | -10% per extra tile |
| Defense application | Subtract after range | Subtract after range |
| Creature attacks | Melee only | Melee + spell-based |
| Spell damage | Direct spell effect | Routed through AI_ATTACK_FLAGS |

**Key DM2 addition:** Strength no longer just affects hit chance — it directly adds to damage output in the formula (`attacker_strength / 4`).

**DM1 context:** DM1 had no range penalty and no strength-to-damage conversion. Weapon base damage was used directly, and only defense was subtracted.

---

## Critical Hit / Special Hit

Not observed in DM2 V1 combat resolver stub. Future versions may extend with critical hit logic based on `QUERY_PLAYER_SKILL_LV` or weapon properties.

---

## Status

**PARTIALLY SOURCE-LOCKED** — damage formula from `dm2_v1_combat.c` is confirmed against SKULL.ASM. Hit roll resolution detailed from SKULL.ASM combat path. Creature damage from AIDefinition struct at DME.h:1505.
