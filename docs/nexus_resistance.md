# Nexus V1 — Magic Resistance

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_champions.c`, `include/nexus_v1_champions.h`, `src/nexus/nexus_v1_magic.c`, `src/nexus/nexus_v1_combat.c`, `docs/spells_resistance.md` (DM1 V1)

---

## 1. Overview

Nexus V1 champions have two resistance stats: `anti_magic` and `anti_fire`.
Both are initialized to 5 for every champion but are **not currently checked**
in any casting or damage function in the Nexus V1 source. The resistance
system is a stub — the stats exist in the champion struct but have no
runtime effect.

---

## 2. Resistance Stats in Champion Struct

```c
typedef struct {
    /* ... */
    int strength, dexterity, wisdom, vitality;
    int anti_magic, anti_fire;  /* resistance stats */
    /* ... */
} Nexus_V1_Champion;
```

Both fields are `int` (not `uint8_t` or similar), allowing values beyond 0-100.

Source: `include/nexus_v1_champions.h`

---

## 3. Initialization

In `nexus_v1_champions.c`, every champion in the roster is initialized with:
```c
c->anti_magic = 5;
c->anti_fire = 5;
```

This is a flat default value — there is no stat-based derivation or
class-based modifier at creation time. All 8 champions in the roster get
the same resistance values.

Source: `src/nexus/nexus_v1_champions.c`

---

## 4. DM1 Resistance Reference

For comparison, DM1's resistance system (from `spells_resistance.md`):
- Creatures have magic resistance rated 0-200 (or 0-999 for bosses)
- Resistance acts as a percentage reduction to incoming spell damage
- High resistance can fully block low-power spells
- Anti-magic champ stats provide partial party protection

Nexus V1 does not yet implement any equivalent:
- No creature resistance values found in `nexus_v1_creatures.c`
- No champion-side resistance check in `nexus_v1_cast_spell`
- No damage reduction path using `anti_magic` or `anti_fire`

Source: `docs/spells_resistance.md`, `src/nexus/nexus_v1_creatures.c` (grep for resistance)

---

## 5. What Uses Anti-Magic and Anti-Fire

**Anti-magic:** Listed as a champion stat, initialized to 5, never read in any
casting or combat function in the Nexus V1 source.

**Anti-fire:** Listed as a champion stat, initialized to 5, never read in any
casting or combat function in the Nexus V1 source.

Grep across all `src/nexus/` files:
```
nexus_v1_champions.c:    c->anti_magic = 5;
nexus_v1_champions.c:    c->anti_fire = 5;
```
No other occurrences of either field name.

Source: `grep -r 'anti_magic\|anti_fire' src/nexus/`

---

## 6. Planned Role (from Header Comments)

The `nexus_v1_champions.h` comment notes:
```
/* DM Nexus has the same champion system as DM1 but with Japanese names.
 * 24 champions in the Hall of Champions, 4 active in party.
 * Stats, skills, spells identical to DM1 engine. */
```

This suggests `anti_magic` and `anti_fire` should behave identically to DM1
once implemented, but the current source does not wire them into any gameplay path.

---

## Status: NOT IMPLEMENTED (STUB)

- Champion resistance stats: **stub** — initialized to 5, never checked
- Creature magic resistance: **not implemented** — no values in `nexus_v1_creatures.c`
- Resistance check during spell casting: **not implemented**
- Fire resistance check in damage: **not implemented**
- No `nexus_v1_resistance_check()` function exists in the codebase

---

## TODO

- Add resistance check to `nexus_v1_cast_spell` for hostile spell effects on champions
- Add creature resistance values to `nexus_v1_creatures.c`
- Add fire resistance check to `nexus_v1_take_damage` for fire-based attacks
- Align default values with DM1 balance (anti_magic 5 seems low for a baseline)
