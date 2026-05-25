# Nexus V1 — Magic in Combat

## Source
- `src/nexus/nexus_v1_magic.c` — spell casting, mana cost

---

## Overview

Nexus V1 inherits the DM1 spell system: 10 flask-based potions (from rune word
combinations), plus a stub for an extended DM1/DM2-style alignment-based magic
system. The extended alignment system is marked `/* FUTURE */` — it is not
operational in V1.

---

## DM1 Spell System (Inherited, Operational)

### Potion Crafting

The DM1 magic system is flask-based: champions combine rune words with an empty
flask to create potions. From `nexus_v1_magic.c`:

```c
int nexus_v1_spell_mana_cost(int power, int elem) {
    return (power + 1) * (power + 1) * g_elem_cost_mult[elem];
}
```

Element cost multipliers:
```
Fire=2, Cold=3, Electricity=2, Acid=4, Poison=3, Physical=3
```

### Mana Cost Formula

```
mana_cost = (power + 1)^2 * element_multiplier
```

| Power Level | 0 | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|---|
| Fire (×2) | 2 | 8 | 18 | 32 | 50 | 72 |
| Cold (×3) | 3 | 12 | 27 | 48 | 75 | 108 |
| Electricity (×2) | 2 | 8 | 18 | 32 | 50 | 72 |
| Acid (×4) | 4 | 16 | 36 | 64 | 100 | 144 |
| Poison (×3) | 3 | 12 | 27 | 48 | 75 | 108 |
| Physical (×3) | 3 | 12 | 27 | 48 | 75 | 108 |

---

## Spell Casting: `nexus_v1_cast_spell()`

```c
int nexus_v1_cast_spell(Nexus_V1_Champion *caster,
    int power, int elem, int form, int align)
```

Preconditions:
1. Champion is alive
2. Champion has sufficient mana (cost check)
3. Champion has priest_level OR wizard_level ≥ power

On success: mana consumed, spell cast (print-only in V1 stub).

On failure: returns -1, prints reason to stdout.

**Note:** `form` and `align` parameters are unused — marked `(void)`.
The alignment-based effects described in the comment are future extensions.

---

## What's Present vs DM2

| Feature | DM2 | Nexus V1 |
|---|---|---|
| Potion crafting | Yes | Yes (DM1 base, same 10 spells) |
| Spell shield / magical shield | Yes | Not implemented (stub only) |
| Direct damage spells (fireball, lightning) | Yes | Not implemented |
| Summoning (minions) | Yes | Not implemented |
| Movement spells (push, pull) | Yes | Not implemented |
| Alignment-based rune words | DM2 had extended rune words | Stub marked `/* FUTURE */` |
| Scroll/wand items | Full scroll+wand system | **Absent** |
| Tech magic (battery-powered items) | Yes | **Absent** |

---

## Creature Spell Attacks in Nexus

Nexus creature AI (`nexus_v1_creatures.c`) has **no spell-casting capability**:
- No fireball, lightning, dispel, push/pull spell states
- Creatures use only direct ATK damage (no magic missile equivalent)
- Compare DM2: Amplifier creature (AI 51) uses `AI_ATTACK_FLAGS__FIREBALL`

This is a significant gap vs DM2's creature magic system.

---

## Status: PARTIALLY SOURCE-LOCKED

- Mana cost formula: **source-locked** — explicit in `nexus_v1_magic.c`
- Skill requirement check: **source-locked** — priest_level or wizard_level ≥ power
- Alignment/form parameters: **stub** — `(void)` casts, no operational effect
- Spell effects (damage, healing, buffs): **stub** — print only
- DM1 potion list: **inherited** — confirmed via DM1 documentation
- DM2 creature magic: **absent** — no spell attack flags in Nexus creature AI