# DM2 V1 — Magic in Combat

## Source
- `skproject/SKWIN/SkWinCore.cpp:17521-17900` — CAST_SPELL_PLAYER
- `skproject/SKWIN/SkGlobal.cpp:966-1011` — dSpellsTable (34 spells)
- `skproject/SKWIN/SkGlobal.h:37-55` — spell constants
- `docs/spells_dm2/dm2_spell_casting.md` — spell casting mechanics
- `docs/spells_dm2/dm2_newspells.md` — DM2 new spells

---

## Overview: DM2 Magic vs DM1

DM2 adds a tech layer alongside magic, and introduces hybrid items. Champions can use pure magic, pure tech, or a combination. Spell combat effects in DM2 include:

- Direct damage spells (fireball, lightning, poison bolt)
- Movement control (push, pull)
- Defense buffs (spell shield, magical shield)
- Utility (invisibility, light, door control)
- Summoning (attack minion, guard minion, u-haul minion)

---

## DM2 Spell Types (from dSpellsTable)

```cpp
// SkGlobal.h:50-53
#define SPELL_TYPE_POTION    1  // requires empty flask
#define SPELL_TYPE_MISSILE   2  // shoots magical projectile
#define SPELL_TYPE_GENERAL   3  // enchantments, light, etc.
#define SPELL_TYPE_SUMMON    4  // summons a creature minion
```

### Spell Execution by Type

From `SkWinCore.cpp:17563`:
```cpp
switch (ref->w6 & 15) {
    case SPELL_TYPE_POTION:   // 1 — requires empty flask in hand
    case SPELL_TYPE_MISSILE:  // 2 — fires projectile at target
    case SPELL_TYPE_SUMMON:   // 4 — creates minion NPC
    case SPELL_TYPE_GENERAL:   // 3 — light, enchantments, auras
}
```

---

## Direct Damage Spells in Combat

### Fireball
- **Rune sequence:** FUL IR
- **Difficulty:** 0x00
- **Type:** SPELL_TYPE_MISSILE
- **Effect:** 0x3823 (fireball object effect, SkWinCore.cpp:27051-27054)
- **Damage:** High AoE damage in blast radius
- Used by: Amplifier creature (AI index 51) — creature uses fireball via AttacksSpells flag

### Lightning
- **Rune sequence:** OH KATH RA
- **Difficulty:** 0x0D
- **Type:** SPELL_TYPE_MISSILE
- **Effect:** 0x0000 — lightning object effect
- **Damage:** Single-target electric damage
- Source: SkWinCore.cpp:27061-27063

### Poison Bolt
- **Rune sequence:** DES VEN
- **Difficulty:** 0x0D
- **Type:** SPELL_TYPE_MISSILE
- **Effect:** 0x0000
- **Damage:** Poison damage over time
- Source: SkWinCore.cpp:27071-27073

### Poison Cloud
- **Rune sequence:** OH VEN
- **Difficulty:** 0x07
- **Type:** SPELL_TYPE_GENERAL (AoE)
- **Effect:** 0x0000
- **Damage:** AoE poison damage in cloud radius
- Source: SkWinCore.cpp:27066-27068

---

## Movement-Control Spells in Combat

### Push
- **Rune sequence:** OH KATH KU
- **Difficulty:** 0x09
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Telekinetic push — displaces target away from caster
- Source: SkWinCore.cpp:27076-27079

### Pull
- **Rune sequence:** OH KATH ROS
- **Difficulty:** 0x0A
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Telekinetic pull — displaces target toward caster
- Source: SkWinCore.cpp:27080-27082

Both spells affect creature or party member positioning. Creatures also have Push/Pull spell attacks via AI_ATTACK_FLAGS (0x0400/0x0800).

---

## Defensive Magic in Combat

### Spell Shield (Party)
- **Rune sequence:** YA IR
- **Difficulty:** 0x04
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Party-wide damage reduction (likely 50%)
- Source: SkGlobal.cpp:967 (index 2)

### Magical Shield
- **Rune sequence:** YA EW (2 symbols)
- **Difficulty:** 0x00
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Personal enchantment — reduces incoming damage
- Source: SkGlobal.cpp:969 (index 4)

### Fire Shield
- **Rune sequence:** FUL BRO NETA
- **Difficulty:** 0x00
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Reflects fire damage back at attacker
- Source: SkGlobal.cpp:973 (index 8)

### Invisibility
- **Rune sequence:** OH EW SAR
- **Difficulty:** 0x00
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Party becomes invisible — prevents creature aggro
- Source: SkGlobal.cpp:968 (index 3)

---

## Summoning Spells in Combat (DM2 New)

DM2 introduces 3 new summoning spells (indices 29-31) that create companion NPCs:

| Index | Runes | Name | Type | AI Index |
|---|---|---|---|---|
| 29 | ZO EW KU | Attack Minion | SUMMON | 14 (Attack Minion) |
| 30 | ZO EW NETA | Guard Minion | SUMMON | 17 (Guard Minion) |
| 31 | ZO EW ROS | U-Haul Minion | SUMMON | 18 (U-Haul Minion) |

### Summon Execution
```cpp
// SkWinCore.cpp:17563
case SPELL_TYPE_SUMMON:  // 4
    // Creates minion NPC at target location
    // Minion has its own stats (HP, attack, defense)
    // Minion fights alongside party
```

### Minion Stats (from dm2-v1-creatures docs)
- Companions have own `BaseHP`, `AttackStrength`, `Defense`
- `Loyalty` stat (0-100) affects behavior
- `Posture`: 0=follow party, 1=guard position, 2=aggressive
- Companion tick runs each game loop iteration

---

## Spell Reflector (DM2 New)

- **Rune sequence:** ZO BRO ROS
- **Difficulty:** 0x0F
- **Type:** SPELL_TYPE_GENERAL
- **Effect:** Reflects incoming spells back at caster
- Source: SkGlobal.cpp:977 (index 12)

Creatures with `SOUND_CREATURE_REFLECTOR (0x03)` also reflect — used by Reflector object and Lord Dragoth.

---

## Spell Casting Mechanics

### Cast Chance Formula
From `SkWinCore.cpp:17535-17555`:

```cpp
U16 bp0e = (ref->w6_a_f() * (power + 18)) / 24;  // skill decay amount
U16 bp08 = ref->difficulty + power;               // global difficulty
U16 bp0c = 0
    + ((RAND() & 7) + (bp08 << 4))
    + ((ref->difficulty * (power - 1)) << 3)
    + (bp08 * bp08);

U16 bp06 = QUERY_PLAYER_SKILL_LV(player, ref->requiredSkill, 1);

// Cast loop: Wizard ability +15 vs difficulty
for (bp0a = bp08 - bp06; (bp0a--) > 0; ) {
    // Roll against min(WizardAbility + 15, 115)
    // If fail: skill penalty, spell fails
}
```

### Mana Cost per Rune

```cpp
// SkWinCore.cpp:18159-18174 (ADD_RUNE_TO_TAIL)
U16 di = RuneManaPower[si * 6 + symbol_0to5];
if (si != 0) {  // if not on POWER rune
    di = (RunePowerMultiplicator[i8(champion->GetRunes()[0]) - RUNE_FIRST] * di) >> 3;
}
if (champion->curMP() >= di) {
    champion->curMP(champion->curMP() - di);
    // rune added to sequence
}
```

Mana is deducted per rune added, not at cast time. Power rune (first rune) has no mana cost but multiplies subsequent rune costs.

### Cooldown After Casting

```cpp
// SkWinCore.cpp:17623
if (bp0e != 0) {
    ADJUST_HAND_COOLDOWN(player, bp0e, 2);
}
```

Successful casting applies a cooldown to the casting hand.

---

## DM2 Spell List (All 34)

| Index | Runes | Name | Type | Diff | Skill |
|---|---|---|---|---|---|
| 0 | OH IR RA | Long Light | GENERAL | 0x04 | 0x0F |
| 1 | DES IR SAR | Darkness | GENERAL | 0x04 | 0x0F |
| 2 | YA IR | Spell Shield | GENERAL | 0x04 | 0x0F |
| 3 | OH EW SAR | Invisibility | GENERAL | 0x00 | 0x0F |
| 4 | YA IR (2sym) | Magical Shield | GENERAL | 0x00 | 0x0F |
| 5 | FUL | Light | GENERAL | 0x00 | 0x0F |
| 6 | OH EW DAIN | Aura of Wisdom | GENERAL | 0x00 | 0x0F |
| 7 | OH EW ROS | Aura of Dexterity | GENERAL | 0x00 | 0x0F |
| 8 | FUL BRO NETA | Fire Shield | GENERAL | 0x00 | 0x0F |
| 9 | OH EW NETA | Aura of Vitality | GENERAL | 0x00 | 0x0F |
| 10 | OH EW KU | Aura of Strength | GENERAL | 0x00 | 0x0F |
| 11 | OH IR ROS | Aura of Speed | GENERAL | 0x00 | 0x0F |
| 12 | ZO BRO ROS | Spell Reflector | GENERAL | 0x0F | 0x0F |
| 13 | YA EW (2sym) | Magical Marker | GENERAL | 0x00 | 0x0F |
| 14 | OH VEN | Poison Cloud | GENERAL | 0x07 | 0x0F |
| 15 | OH KATH RA | Lightning | MISSILE | 0x0D | 0x0F |
| 16 | FUL IR | Fireball | MISSILE | 0x00 | 0x0F |
| 17 | FUL BRO KU | NP: STR Potion | POTION | 0x13 | 0x07 |
| 18 | DES EW | Antimatter | MISSILE | 0x03 | 0x0F |
| 19 | DES VEN | Poison Bolt | MISSILE | 0x06 | 0x0F |
| 20 | ZO | Open/Close Door | GENERAL | 0x04 | 0x0F |
| 21 | YA BRO | NP: Shield Potion | POTION | 0x13 | 0x0C |
| 22 | YA | NP: Stamina Potion | POTION | 0x13 | 0x0B |
| 23 | YA BRO DAIN | NP: Wisdom Potion | POTION | 0x13 | 0x08 |
| 24 | YA BRO NETA | NP: Vitality Potion | POTION | 0x13 | 0x09 |
| 25 | VI | NP: Health Potion | POTION | 0x13 | 0x0E |
| 26 | VI BRO | NP: Anti Venin | POTION | 0x13 | 0x0A |
| 27 | OH BRO ROS | NP: Dexterity Potion | POTION | 0x13 | 0x06 |
| 28 | ZO BRO RA | NP: Mana Potion | POTION | 0x13 | 0x0D |
| 29 | ZO EW KU | Attack Minion | SUMMON | 0x0F | 0x31 |
| 30 | ZO EW NETA | Guard Minion | SUMMON | 0x0F | 0x34 |
| 31 | ZO EW ROS | U-Haul Minion | SUMMON | 0x0F | 0x35 |
| 32 | OH KATH KU | Push | GENERAL | 0x0D | 0x09 |
| 33 | OH KATH ROS | Pull | GENERAL | 0x0D | 0x0A |

---

## Dispel Mechanics

Dispell attack flag (AI_ATTACK_FLAGS__DISPELL = 0x0020) removes champion enchantments:
- Removes: Spell Shield, Magical Shield, Fire Shield, Invisibility, Aura of Wisdom/Dexterity/Strength/Vitality/Speed, Spell Reflector
- Does not remove potion effects or innate stat bonuses
- Source: SkWinCore.cpp:27056-27058

---

## Comparison: DM1 vs DM2 Magic Combat

| Feature | DM1 | DM2 |
|---|---|---|
| Damage spells | Fireball, lightning | Fireball, lightning, poison bolt, poison cloud, antimatter |
| Movement spells | None | Push, Pull |
| Summoning | None | Attack/Guard/U-Haul minion |
| Reflect | None | Spell Reflector |
| Mana system | Fixed per spell | Per-rune mana cost |
| Cooldowns | None | Hand cooldown after casting |
| Skill decay on fail | None | Explicit penalty |
| Dispel | None | Creature dispell attack |
| Extended spell mode | No | Yes (255 spells via GDAT) |

---

## Status

**SOURCE-LOCKED** — Spell table from `SkGlobal.cpp:966-1011`, casting mechanics from `SkWinCore.cpp:17521-17900`, rune mana from `SkWinCore.cpp:18159-18174`, spell types from `SkGlobal.h:37-55`.
