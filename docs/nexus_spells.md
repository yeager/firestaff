# Nexus V1 — Spell System vs DM1/DM2

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_magic.c`, `include/nexus_v1_magic.h`, `src/nexus/nexus_v1_champions.c`, `include/nexus_v1_champions.h`, `include/firestaff_spell_ui.h`, `docs/spells_system.md` (DM1), `docs/dm2_magic.md`

---

## 1. Overview

Nexus V1's spell system is **directly derived from Dungeon Master (DM1)**. The rune
vocabulary, power/element/enumeration, and mana-cost formula are all inherited.
DM2 took a different path (tech-magic with gems/crystals/power sources). Nexus V1
stays on the DM1 rune path but does not yet implement full rune combination parsing.

---

## 2. Rune System Comparison

| Feature | DM1 | DM2 | Nexus V1 |
|---|---|---|---|
| Rune grid | 6×6×4×6 | No runes | 6×6×4×6 (same as DM1) |
| Power runes | Lo/Um/On/Ee/Pal/Mon | N/A | Lo/Um/On/Ee/Pal/Mon |
| Element runes | Ya/Vi/Oh/Ful/Des/Zo | N/A | Ya/Vi/Oh/Ful/Des/Zo |
| Form runes | Ven/Ew/Kath/Ir/Bro/Gor | N/A | Ven/Ew/Kath/Ir/Bro/Gor |
| Alignment runes | Ku/Ros/Dain/Neta/Ra/Sar | N/A | Ku/Ros/Dain/Neta/Ra/Sar |
| Spell combinations | 25 implemented | ~50 tech spells | 25 (same as DM1) |
| Magic type | Rune words | Gems/crystals/batteries | Rune words |

DM1 uses a 4-rune composition system: Power + Element + Form(optional) + Alignment(optional).
Theoretically 864 combinations, but only 25 spells are actually defined in the original MENU.C
spell table.

DM2 abandoned runes entirely in favor of a "tech magic" system using physical items:
power gems, crystals, power sources, and electronic spell triggers.

Nexus V1 keeps the DM1 rune system but marks full rune combination parsing as FUTURE.

---

## 3. Nexus V1 Spell Structure

```c
typedef struct {
    int power_level;      /* 0-5 (Lo to Mon) */
    int element;          /* element rune index */
    int form;             /* form rune (optional) */
    int alignment;         /* alignment rune (optional) */
    int mana_cost;
    int damage;
    const char *name;
} Nexus_Spell;
```

Source: `include/nexus_v1_magic.h`

---

## 4. Power Rune Enumeration

```c
typedef enum {
    NEXUS_RUNE_LO = 0, NEXUS_RUNE_UM, NEXUS_RUNE_ON,
    NEXUS_RUNE_EE, NEXUS_RUNE_PAL, NEXUS_RUNE_MON
} Nexus_PowerRune;
/* 0=Lo (weakest) ... 5=Mon (strongest) */
```

Source: `include/nexus_v1_magic.h`

---

## 5. Element Rune Enumeration

```c
typedef enum {
    NEXUS_ELEM_YA = 0, NEXUS_ELEM_VI, NEXUS_ELEM_OH,
    NEXUS_ELEM_FUL, NEXUS_ELEM_DES, NEXUS_ELEM_ZO
} Nexus_ElementRune;
```

Source: `include/nexus_v1_magic.h`

---

## 6. Mana Cost Formula

Nexus V1 uses the DM1 mana cost formula with element multipliers:

```c
static const int g_elem_cost_mult[] = {2, 3, 2, 4, 3, 3};

int nexus_v1_spell_mana_cost(int power, int elem) {
    if (power < 0 || power > 5) return 999;
    if (elem < 0 || elem > 5) return 999;
    return (power + 1) * (power + 1) * g_elem_cost_mult[elem];
}
```

| Power | Cost ×1 (elem 0/2) | ×1.5 (elem 1/4/5) | ×2 (elem 3) |
|---|---|---|---|
| 0 (Lo) | 2 | 3 | 4 |
| 1 (Um) | 8 | 12 | 16 |
| 2 (On) | 18 | 27 | 36 |
| 3 (Ee) | 32 | 48 | 64 |
| 4 (Pal) | 50 | 75 | 100 |
| 5 (Mon) | 72 | 108 | 144 |

Element multipliers: Ya=2, Vi=3, Oh=2, Ful=4, Des=3, Zo=3.

Source: `src/nexus/nexus_v1_magic.c`

---

## 7. Spell List — Same 25 Spells as DM1

Nexus V1 inherits the complete DM1 spell table (from MENU.C:50..77 via
`spell_reference_m12.h` and `firestaff_spell_ref.h`):

| Spell | Power | Element | Form | Align | School | Kind |
|---|---|---|---|---|---|---|
| FIRE WISP | 1 | Ful | — | — | Fire | Projectile |
| FIRE FLASH | 3 | Ful | Ir | — | Fire | Projectile |
| FIREBALL | 5 | Ful | Ir | — | Fire | Projectile |
| GUST | 1 | Oh | — | — | Air | Projectile |
| ZAP | 3 | Oh | Ir | — | Air | Projectile |
| LIGHTNING BOLT | 5 | Oh | Ir | — | Air | Projectile |
| ROCK | 1 | Des | — | — | Earth | Projectile |
| TREmor | 3 | Des | Ir | — | Earth | Projectile |
| EARTHQUAKE | 5 | Des | Ir | — | Earth | Projectile |
| WATER GUSH | 1 | Zo | — | — | Water | Projectile |
| WATER SPOUT | 3 | Zo | Ir | — | Water | Projectile |
| TIDAL WAVE | 5 | Zo | Ir | — | Water | Projectile |
| HEAL | 1 | Ya | Bro | — | Body | Potion |
| GREATER HEAL | 3 | Ya | Bro | — | Body | Potion |
| FULL HEAL | 5 | Ya | Bro | — | Body | Potion |
| CURE POISON | 1 | Vi | Bro | — | Mind | Potion |
| IDENTIFY | 1 | Vi | — | Dain | Mind | Other |
| PROTECT | 1 | Ya | — | Neta | Mind | Other |
| SHIELD | 1 | Ya | Bro | — | Mind | Other |
| MIND SHIELD | 3 | Vi | — | Neta | Mind | Other |
| TELEPORT | 1 | Vi | Ir | — | Mind | Other |
| MAGIC MAP | 1 | Vi | Ir | Dain | Mind | Magic Map |
| LEVITATE | 1 | Oh | Bro | — | Air | Other |
| TIME MEASUREMENT | 1 | Des | Bro | Neta | Earth | Other |
| FLAME PATH | 1 | Ful | Bro | Dain | Fire | Other |

Plus 10 flask-crafting potion spells (Strength, Wisdom, Vitality, Health, Dexterity,
Mana, Stamina, Poison, Shield, Cure Poison variants — see `docs/nexus_potions.md`).

Source: `docs/spells_system.md`, `include/spell_reference_m12.h`

---

## 8. What's Different from DM1

| Aspect | DM1 | Nexus V1 |
|---|---|---|
| Champion names | Western (Jason, Andre...) | Japanese (Syra, Leyla, Nabi...) |
| Language | English | Japanese |
| Champion count | 24 | 24 (same roster) |
| Spell count | 25 + 10 flask | 25 + 10 flask (identical) |
| Mana formula | (p+1)²×elem_mult | Identical |
| Rune parsing | Hard-coded in MENU.C | Stubbed (form/align = noop) |
| Skill check | priest/wizard level | priest/wizard level |

---

## Status: PARTIALLY SOURCE-LOCKED

- Rune enumerations: **source-locked** — `nexus_v1_magic.h`
- Mana cost formula: **source-locked** — `nexus_v1_magic.c`
- Spell list: **inherited** from DM1 reference data
- Rune combination parsing for actual spell lookup: **NOT IMPLEMENTED** — form and align params are discarded, cast function takes raw power/elem indices
