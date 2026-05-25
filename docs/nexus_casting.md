# Nexus V1 — Spell Casting

**Audit date:** 2026-05-25
**Sources:** `src/nexus/nexus_v1_magic.c`, `include/nexus_v1_magic.h`, `include/nexus_v1_champions.h`, `include/firestaff_spell_ui.h`, `include/firestaff_spell_ref.h`, `src/nexus/nexus_v1_champions.c`

---

## 1. Overview

Nexus V1 spell casting uses the DM1 rune-word system. Champions compose spells
by selecting runes (Power + Element + optional Form + optional Alignment) and
paying a mana cost. The actual effect dispatch system (what happens after mana
is deducted) is not yet implemented — only the mana/skill gate and cost
deduction exist in code.

---

## 2. Champion Requirements to Cast

A champion can attempt to cast if all of these are true:

1. Champion is alive (`alive == 1`)
2. Champion has sufficient mana: `caster->mana >= nexus_v1_spell_mana_cost(power, elem)`
3. Champion has sufficient skill: `caster->priest_level >= power` OR `caster->wizard_level >= power`

```c
int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int elem, int form, int align) {
    int cost, skill_req;
    (void)form; (void)align; /* FUTURE: full rune combination parsing. */

    if (!caster || !caster->alive) return -1;
    cost = nexus_v1_spell_mana_cost(power, elem);
    if (caster->mana < cost) return -1;
    skill_req = power;
    if (caster->priest_level < skill_req && caster->wizard_level < skill_req) return -1;
    caster->mana -= cost;
    return cost;
}
```

Returns: cost deducted on success, -1 on any failure.

Source: `src/nexus/nexus_v1_magic.c`

---

## 3. Spell Casting UI State Machine

The Firestaff shared spell UI defines the full DM1 rune-composition state machine:

```c
typedef enum {
    SPELL_STATE_IDLE = 0,
    SPELL_STATE_POWER,     /* selecting power rune */
    SPELL_STATE_ELEMENT,   /* selecting element */
    SPELL_STATE_FORM,      /* selecting form (optional) */
    SPELL_STATE_ALIGN,     /* selecting alignment (optional) */
    SPELL_STATE_READY,     /* spell composed, ready to cast */
} FS_SpellState;
```

Flow:
```
IDLE (spell panel opened by champion selection)
  -> POWER (click one of 6 power runes: Lo/Um/On/Ee/Pal/Mon)
  -> ELEMENT (click one of 6 element runes: Ya/Vi/Oh/Ful/Des/Zo)
  -> [optional] FORM (click one of 6 form runes: Ven/Ew/Kath/Ir/Bro/Gor)
  -> [optional] ALIGN (click one of 6 align runes: Ku/Ros/Dain/Neta/Ra/Sar)
  -> READY (cast button active)
```

The UI renders 6 rune buttons per category, cursor position tracked as `cursor_pos`.
Cancel last rune (`fs_spell_cancel_last`) steps back one state.

Source: `include/firestaff_spell_ui.h`

---

## 4. Skill Classes and Casting Eligibility

Nexus V1 has 4 champion classes, each with a skill level:

```c
typedef enum {
    NEXUS_CLASS_FIGHTER = 0,
    NEXUS_CLASS_NINJA,
    NEXUS_CLASS_PRIEST,
    NEXUS_CLASS_WIZARD,
    NEXUS_CLASS_COUNT
} Nexus_ChampionClass;
```

Skill level fields per champion:
```c
int fighter_level, ninja_level, priest_level, wizard_level;
```

Casting gate: either `priest_level >= power` OR `wizard_level >= power`.
This means **Fighter and Ninja champions cannot cast spells** (both skill fields
start at 1 but rarely advance). Priests and Wizards are the natural casters,
but any champion who trains the relevant skill can cast.

Source: `include/nexus_v1_champions.h`, `src/nexus/nexus_v1_magic.c`

---

## 5. Champion Roster — Starting Mana and Class

From `nexus_v1_champions.c` roster initialization:

| Name | Class | Max Mana | Pri/Wiz Start |
|---|---|---|---|
| Syra | Fighter | 15 | 1/1 |
| Leyla | Wizard | 65 | 1/1 |
| Nabi | Ninja | 25 | 1/1 |
| Gando | Priest | 55 | 1/1 |
| Torham | Fighter | 20 | 1/1 |
| Elija | Wizard | 70 | 1/1 |
| Wu Tse | Ninja | 30 | 1/1 |
| Stamm | Fighter | 10 | 1/1 |

All champions start with priest_level=1 and wizard_level=1.
Only Priest and Wizard classes advance those skills meaningfully through use.

Source: `src/nexus/nexus_v1_champions.c`

---

## 6. Rune-to-Spell Dispatch — NOT IMPLEMENTED

The critical gap: `nexus_v1_cast_spell` accepts raw power/element/form/align
integers, deducts mana, and returns cost. It does NOT dispatch to any spell
effect. The form and align parameters are explicitly cast to void — no
rune combination logic runs.

The actual rune-to-spell mapping (Ful+Ir -> FIRE FLASH, etc.) lives in the
DM1 spell reference data (`M12_SpellReferenceEntry` table in `spell_reference_m12.h`)
but is not connected to the Nexus V1 casting path.

What should happen after mana is deducted (the spell effect):
- Projectile spells: spawn projectile, deal damage
- Potion spells: consume flask, create potion item
- Area spells: apply effect to party or dungeon
- None of this is implemented in Nexus V1 yet.

Source: `src/nexus/nexus_v1_magic.c` (stub comment: "FUTURE: full rune combination parsing")

---

## 7. Experience and Skill Advancement

When champions deal damage through attacks, they gain experience:
```c
void nexus_v1_gain_experience(Nexus_V1_Champion *champ, Nexus_ChampionClass skill, int amount) {
    if (!champ || amount <= 0) return;
    switch (skill) {
    case NEXUS_CLASS_FIGHTER: champ->fighter_level += amount / 100; break;
    case NEXUS_CLASS_NINJA:   champ->ninja_level += amount / 100;   break;
    case NEXUS_CLASS_PRIEST:  champ->priest_level += amount / 100;  break;
    case NEXUS_CLASS_WIZARD:  champ->wizard_level += amount / 100;  break;
    }
}
```

Experience comes from combat damage (`nexus_v1_attack` sets `experience_gained = damage`).
There is no explicit spell-casting experience in the current source.

Source: `src/nexus/nexus_v1_combat.c`

---

## 8. Anti-Magic Stat

Every champion starts with `anti_magic = 5` (default value set in champion init):
```c
c->anti_magic = 5;
c->anti_fire = 5;
```

This stat is stored but not currently used in any resistance calculation in the
casting code. See `docs/nexus_resistance.md` for full details.

Source: `src/nexus/nexus_v1_champions.c`

---

## Status: PARTIALLY SOURCE-LOCKED

- Mana cost gate: **source-locked** — `nexus_v1_magic.c`
- Skill level gate: **source-locked** — `nexus_v1_magic.c`
- Spell UI state machine: **defined** in `firestaff_spell_ui.h`, **not wired** to casting
- Rune parsing and spell effect dispatch: **NOT IMPLEMENTED** — marked FUTURE
- Experience for spell casting: **NOT IMPLEMENTED** — no spell XP path found
