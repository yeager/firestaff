# Nexus V1 — Weapons and Combat Formula

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_combat.c`, `include/nexus_v1_combat.h`, `include/firestaff_item_encyclopedia.h`, `src/ui/firestaff_item_encyclopedia.c`, `docs/nexus_combat_items.md`

---

## 1. Weapon Categories in the Item Encyclopedia

The Firestaff item encyclopedia defines 12 weapon types:

| Name | Attack (Power) | Defense | Weight | Notes |
|---|---|---|---|---|
| Falchion | 30 | 0 | 18 | Curved single-edged sword |
| Rapier | 24 | 4 | 14 | Thrusting blade |
| Mace | 32 | 0 | 30 | Heavy blunt weapon |
| Club | 16 | 0 | 20 | Simple wooden club |
| Staff | 10 | 2 | 12 | Low damage; also used for magic |
| Sword | 34 | 2 | 22 | Standard double-edged sword |
| Axe | 36 | 0 | 26 | Single-bladed battle axe |
| Dagger | 14 | 0 | 6 | Can be thrown |
| Arrow | 10 | 0 | 1 | Ammunition for bows |
| Slayer | 50 | 6 | 28 | Legendary demon-slaying blade |
| Vorpal Blade | 48 | 4 | 20 | Enchanted blade of supreme sharpness |
| Firestaff | 40 | 10 | 16 | The legendary Firestaff of power |

Source: `src/ui/firestaff_item_encyclopedia.c`

---

## 2. Weapon Power in Combat

In `src/nexus/nexus_v1_combat.c`, the weapon attack stat (called `weapon_power`
in the function) is the primary damage source:

```c
Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker, int weapon_power, int defense);
```

The function receives `weapon_power` as a parameter, injected at the call site
from the equipped weapon. No lookup table exists in the combat file —
the caller must look up the equipped weapon power and pass it in.

Source: `src/nexus/nexus_v1_combat.c`, `include/nexus_v1_combat.h`

---

## 3. Full Combat Formula

```c
static int rng(int max) { return max > 0 ? (rand() % max) : 0; }

Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker, int weapon_power, int defense) {
    Nexus_CombatResult r = {0};
    int hit_chance, str_bonus, damage, def_reduce;

    if (!attacker || !attacker->alive) return r;

    /* Consume stamina */
    if (attacker->stamina < 3) return r;
    attacker->stamina -= 3;

    /* Hit chance: dex + fighter level, max 95% */
    hit_chance = attacker->dexterity + attacker->fighter_level * 2;
    if (hit_chance > 95) hit_chance = 95;

    r.hit = (rng(100) < hit_chance);
    if (!r.hit) return r;

    /* Damage = weapon_power + str_bonus + rng(str_bonus) */
    str_bonus = attacker->strength / 5;
    damage = weapon_power + str_bonus + rng(str_bonus);

    /* Critical: 5% chance, double damage */
    if (rng(100) < 5) {
        damage *= 2;
        r.critical = 1;
    }

    /* Defense reduction */
    def_reduce = defense / 2 + rng(defense / 2);
    damage -= def_reduce;
    if (damage < 1) damage = 1;

    r.damage = damage;
    r.experience_gained = damage;
    return r;
}
```

Step-by-step breakdown:

| Step | Formula | Notes |
|---|---|---|
| Stamina cost | stamina -= 3 | Attack fails if stamina < 3 |
| Hit chance | dexterity + fighter_level * 2 | Capped at 95% |
| Miss | Returns hit=0, damage=0 | No damage on miss |
| Strength bonus | strength / 5 | Integer division |
| Base damage | weapon_power + str_bonus + rng(str_bonus) | Random 0-to-str_bonus added |
| Critical hit | 5% chance | Doubles total damage |
| Defense reduce | defense / 2 + rng(defense / 2) | Random 0 to defense/2 |
| Minimum damage | 1 | Clamp after defense |

Source: `src/nexus/nexus_v1_combat.c`

---

## 4. Weapon Types and Roles

### Melee Weapons
Falchion, Sword, Axe, Mace, Club, Rapier — standard physical damage.
Two-handed weapons (Sword, Axe) tend to have higher power but use action hand only.

### Thrown Weapons
Dagger (attack=14) — can be thrown. No separate thrown-weapon code path found.

### Ranged Weapons
Arrow (attack=10) — ammunition item. The ranged weapon that fires it is not in the roster.
No dedicated bow/crossbow weapon type in the current encyclopedia.

### Magic Weapons
Staff (attack=10, defense=2) — low damage but usable by Wizard class.
Firestaff (attack=40, defense=10) — highest defense of any weapon.

### Unique/Legendary Weapons
Slayer (attack=50, defense=6) — highest attack power.
Vorpal Blade (attack=48, defense=4) — close second.
These match the DM1 tradition of named legendary weapons.

---

## 5. Weapon Combat Integration

No dedicated weapon database or equip-system code exists in Nexus V1 source.
Integration is entirely by parameter — the caller resolves `inventory[index]` to
weapon_power and passes it to `nexus_v1_attack()`.

Source: `include/nexus_v1_champions.h`, `include/firestaff_inventory_ui.h`

---

## 6. Experience from Combat

Damage dealt generates experience:
```c
r.experience_gained = damage;
```

Experience is applied to the appropriate class skill (amount / 100 per level-up).

Source: `src/nexus/nexus_v1_combat.c`

---

## 7. What is NOT in Nexus V1 Weapons

| Feature | DM2 | Nexus V1 |
|---|---|---|
| Gun weapons | Yes | No |
| Bomb/explosive weapons | Yes | No |
| Tech-based weapons | Yes | No |
| Per-charge degradation | Yes | No |
| Projectile animation | Yes | No (no ranged code found) |
| Ammo consumption tracking | Yes | No (Arrow is static entry) |

Nexus V1 weapons are pure melee/thrown — no ranged combat system was found.

---

## Status: SOURCE-LOCKED

The combat formula, weapon power parameter interface, and hit/damage/crit mechanics
are all explicitly present in `src/nexus/nexus_v1_combat.c` — fully source-locked.
