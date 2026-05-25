# Nexus V1 — Combat Formula: Hit Roll and Damage

## Source
- `src/nexus/nexus_v1_combat.c` — `nexus_v1_attack()`

---

## Attack Resolution: `nexus_v1_attack()`

```c
Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker,
    int weapon_power, int defense)
```

Called when a champion attacks a creature (or vice versa).
Returns `Nexus_CombatResult { hit, damage, critical, experience_gained }`.

---

## Step 1 — Stamina Check

```c
if (attacker->stamina < 3) return r;   // no stamina → no attack
attacker->stamina -= 3;
```

Stamina cost: **3 per attack** (hardcoded). If champion has < 3 stamina,
attack fails silently.

---

## Step 2 — Hit Chance Roll

```c
hit_chance = attacker->dexterity + attacker->fighter_level * 2;
if (hit_chance > 95) hit_chance = 95;
r.hit = (rng(100) < hit_chance);
```

| Component | Source | Notes |
|---|---|---|
| Base hit | dexterity | Champion stat (0–?) |
| Level bonus | fighter_level * 2 | +2% per fighter level |
| Cap | 95% | Hardcoded maximum |
| Roll | rng(100) < hit_chance | Uniform 0–99 |

**Note:** `rng()` is a trivial `rand() % max` — not cryptographically random,
not seeded from any game-tick entropy visible in this file.

---

## Step 3 — Damage Calculation (on hit)

```c
str_bonus = attacker->strength / 5;
damage = weapon_power + str_bonus + rng(str_bonus);
```

| Component | Formula | Notes |
|---|---|---|
| Base damage | weapon_power | Passed in from caller |
| Strength bonus | strength / 5 | +1 per 5 STR |
| Random bonus | rng(str_bonus) | 0 to str_bonus extra damage |

**No range penalty** — unlike DM2 which penalizes damage at distance.
**No minimum damage** here — minimum enforced after defense.

---

## Step 4 — Critical Hit

```c
if (rng(100) < 5) {
    damage *= 2;
    r.critical = 1;
}
```

5% flat chance. On crit: damage doubled, `r.critical = 1`.

---

## Step 5 — Defense Reduction

```c
def_reduce = defense / 2 + rng(defense / 2);
damage -= def_reduce;
if (damage < 1) damage = 1;
```

| Component | Formula |
|---|---|
| Flat reduction | defense / 2 |
| Random reduction | rng(defense / 2) |
| Minimum damage | 1 (clamped) |

Defense is effectively halved on average (deterministic half + random half).

---

## Complete Damage Formula

```
hit_roll      = rng(100) < (dexterity + fighter_level * 2, capped 95)
str_bonus     = strength / 5
raw_damage    = weapon_power + str_bonus + rng(str_bonus)
if crit:      raw_damage *= 2
def_reduce    = defense / 2 + rng(defense / 2)
final_damage  = max(1, raw_damage - def_reduce)
experience   = final_damage
```

---

## Comparison with DM1 and DM2

| Aspect | DM1 | DM2 | Nexus V1 |
|---|---|---|---|
| Hit chance | dex + fighter bonus (no cap confirmed) | weapon_skill + level | dex + fighter_level*2, **cap 95%** |
| Strength bonus | Unknown formula | STR/4 | **STR/5** |
| Critical hit | Not confirmed in DM1 | Unknown | **5% double damage** |
| Defense | Subtract fixed | Subtract fixed | **Half + RNG half** |
| Range penalty | None | -10% per tile | **None** |
| Stamina cost | DM1 stamina model | Per-weapon stamina | **3 flat** |

---

## Creature vs Champion Damage

Creatures in Nexus use their ATK stat directly as damage (no formula):
- Dragon: 60 damage per hit
- Golem: 35 damage per hit
- Scorpion: 15 damage per hit
- etc.

No defense reduction is applied by the creature — champion's defense
is applied only when champions are attacked by creatures.

---

## Status: SOURCE-LOCKED

All formula components are explicit in `nexus_v1_combat.c`.
No byte verification from ISO yet — formula is confirmed from source code.