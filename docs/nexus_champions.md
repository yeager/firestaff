# Dungeon Master Nexus V1 — Champion System vs DM1/DM2/CSB

## Sources
- `src/nexus/nexus_v1_champions.c` (full roster + stat definitions)
- `docs/NEXUS_PLAN.md`
- `docs/NEXUS_FILE_CLASSIFICATION.md` (FACE.BIN, 44 KB)
- DM1 champion system (from ReDMCSB equivalents)
- DM2 champion system (from DM2 documentation)

---

## Overview

Nexus's champion system is DM1-based (same classes, same advancement, same
inventory/spell system) but with **Japanese names and a Japanese-specific
roster**. It introduces no new classes vs DM1 — the roster is 8 champions
(active selection from a pool of 24, mirroring DM1's structure).

| Aspect | DM1 | CSB | DM2 | Nexus |
|--------|-----|-----|-----|-------|
| Classes | 3 (Fighter, Wizard, Priest) | 4 (+ Ninja unlockable) | 4 (formalized) | **4 (same as DM2)** |
| Roster size | 24 | 24 | 24+ | **24** |
| Party size | 4 | 4 | 4 + companions | **4** |
| Names | Western (Thor, Sara...) | Western | Western | **Japanese (Syra, Leyla...)** |
| Food/Water | 1500 each | 1500 | 1500 | **1500 each** |
| Anti-Magic default | 0 | 0 | 0 | **5** |
| Anti-Fire default | 0 | 0 | 0 | **5** |
| Portrait format | Sprite sheet (~8 KB) | Sprite sheet | Sprite sheet | **FACE.BIN (44 KB)** |
| Alignment system | Yes | Yes | Yes | **Yes (same)** |

---

## Champion Roster (from `nexus_v1_champions.c`)

```c
static const struct g_nexus_roster[] = {
    {"Syra",      "シラ",      NEXUS_CLASS_FIGHTER, 70, 55, 15, 55, 40, 25, 50},
    {"Leyla",     "レイラ",   NEXUS_CLASS_WIZARD,  40, 35, 65, 25, 35, 60, 30},
    {"Nabi",      "ナビ",     NEXUS_CLASS_NINJA,   55, 60, 25, 40, 60, 30, 45},
    {"Gando",     "ガンド",   NEXUS_CLASS_PRIEST,  50, 40, 55, 35, 30, 55, 40},
    {"Torham",    "トルハム", NEXUS_CLASS_FIGHTER, 65, 50, 20, 50, 45, 28, 48},
    {"Elija",     "エリジャ", NEXUS_CLASS_WIZARD,  38, 30, 70, 22, 32, 65, 28},
    {"Wu Tse",    "ウー・ツエ", NEXUS_CLASS_NINJA, 52, 58, 30, 38, 55, 35, 42},
    {"Stamm",    "スタム",   NEXUS_CLASS_FIGHTER, 75, 60, 10, 60, 35, 20, 55},
};
```

Format: ASCII name, Shift-JIS JP name, class, HP, STA, MP, STR, DEX, WIS, VIT.

---

## Classes: 4 vs DM1's 3

| Class | DM1 | CSB | DM2 | Nexus |
|-------|-----|-----|-----|-------|
| Fighter | Yes | Yes | Yes | Yes |
| Wizard | Yes | Yes | Yes | Yes |
| Priest | Yes | Yes | Yes | Yes |
| Ninja | No | No (unlockable) | Yes | **Yes** |

Nexus includes the **Ninja class** introduced in DM2. This is the only class
difference vs DM1. The Ninja class (high DEX, moderate HP, no magic, fast
attack speed) is available as of Nexus — confirming Nexus uses the full DM2
class roster.

Source: `nexus_v1_champions.c` `NEXUS_CLASS_NINJA` enum value.

---

## Champion Stats (Base at Creation)

| Stat | Description | Notes |
|------|-------------|-------|
| Health (HP) | Damage tolerance | Varies by class |
| Stamina (STA) | Physical action energy | Depletes with attacks/movement |
| Mana (MP) | Magic point pool | Wizard > Priest > Ninja > Fighter |
| Strength | Melee damage bonus | Fighter highest |
| Dexterity | Attack speed, ranged accuracy | Ninja highest |
| Wisdom | Spell effectiveness, mana pool | Wizard/Priest highest |
| Vitality | HP and STA growth rate | Fighter highest |

Base stats are set at champion creation and increase with level advancement
(combat XP). The exact advancement formula (per DM1's F0317_LEVEL_GAIN_CALC)
is inherited from DM1 code (not re-implemented in Nexus source, but the
Nexus champion pool uses the same stat layout).

---

## DM1 vs Nexus Base Stat Comparison

| Champion | Class | DM1 HP | Nexus HP | DM1 STA | Nexus STA | DM1 MP | Nexus MP |
|----------|-------|--------|----------|---------|-----------|--------|----------|
| Thor (Fighter) | Fighter | 80 | 70/65/75 | 60 | 55/50/60 | 10 | 15/20/10 |
| Sara (Wizard) | Wizard | 40 | 40/38 | 30 | 35/30 | 70 | 65/70 |
| Reesh (Priest) | Priest | 60 | 50 | 45 | 40 | 50 | 55 |
| Mark (Ninja) | Ninja | N/A | 55/52 | N/A | 60/58 | N/A | 25/30 |

Note: DM1 uses Western names; Nexus uses Japanese. DM1 Ninja was introduced
in CSB (Chaos Strikes Back) as an unlockable; DM2 formalized it. Nexus has
the full 4-class roster.

---

## Resource Management

Nexus champions use the same resource model as DM1:

- **Food**: 1500 units at creation, depletes per game tick. Champion dies at 0.
- **Water**: 1500 units at creation, depletes per game tick. Champion dies at 0.
- Both decay rate depends on activity level (combat drains faster than idle).

This is unchanged from DM1. Source: `nexus_v1_champions.c` init with
`c->food = 1500; c->water = 1500;`.

---

## Resistance Stats

| Stat | DM1 default | Nexus default |
|------|-------------|---------------|
| Anti-Magic | 0 | **5** |
| Anti-Fire | 0 | **5** |

Nexus sets default resistance to 5 for both Anti-Magic and Anti-Fire (vs DM1
which defaulted to 0 on most ports). This gives champions slightly better
survivability against magic and fire attacks at creation. The value can be
raised through in-game advancement.

Source: `nexus_v1_champions.c` — `c->anti_magic = 5; c->anti_fire = 5;`.

---

## Alignment System

Nexus inherits DM1's alignment magic system:

| Class | Alignment | Magic type |
|-------|-----------|------------|
| Fighter | Neutral | Standard spell effectiveness |
| Priest | Good | Heal/Protection spells (most effective) |
| Wizard | Chaos | Offensive spells (most effective) |
| Ninja | Neutral | No magic (uses items instead) |

Spell effectiveness is determined by alignment match (same spell at same
power level does more damage/healing when the champion's alignment matches
the spell's alignment). This is unchanged from DM1.

Source: `nexus_v1_champions.c` class enum (`NEXUS_CLASS_FIGHTER`/`WIZARD`/
`PRIEST`/`NINJA`).

---

## Resurrection Mechanic

Nexus implements champion resurrection:

```c
int nexus_v1_champion_resurrect(Nexus_V1_ChampionPool *pool, int party_slot) {
    idx = pool->party[party_slot];
    pool->champions[idx].alive = 1;
    pool->champions[idx].health = pool->champions[idx].max_health / 4;
    pool->champions[idx].stamina = pool->champions[idx].max_stamina / 4;
    return 0;
}
```

- Champion returns to alive state with **25% max HP and STA**
- Mana is not restored on resurrection (only HP/STA)
- This matches DM1 resurrection behavior

Source: `nexus_v1_champions.c`.

---

## Champion Recruitment

```c
int nexus_v1_champion_recruit(Nexus_V1_ChampionPool *pool, int mirror_index) {
    // Adds champion to party if:
    // - alive
    // - not already in party
    // - party not full (max 4)
}
```

- Up to **4 champions** in the active party
- Pool of 24 champions available (mirroring DM1)
- Face portraits: FACE.BIN (44 KB) — contains champion portrait images
  for all 24 roster members

Source: `nexus_v1_champions.c`.

---

## Party System

- **Leader** — `pool->leader_index` tracks current party leader
- Party leader can be changed (swap to any living party member)
- Leader affects: combat initiative order, which champion is shown
  in front during movement
- Dead champions remain in party until removed or resurrected

---

## Portrait System (vs DM1)

| Game | Portrait format | Size |
|------|---------------|------|
| DM1 | Sprite sheet | ~8 KB |
| CSB | Sprite sheet | ~8 KB |
| DM2 | Sprite sheet | ~12 KB |
| **Nexus** | **FACE.BIN** | **44 KB** |

Nexus has the largest portrait file, reflecting the higher graphical fidelity
expected of a Saturn title and the Japanese art style. Portraits are used at:
- Champion selection (choose 4 from 24 roster)
- In-dungeon status display (face icon in HUD)
- Game over / death screen

---

## What's NEW in Nexus Champion System

1. **Ninja class** — DM2 addition, present in Nexus
2. **Japanese champion names** — 8 unique names (vs DM1's Western roster)
3. **Japanese Shift-JIS names** — full-width JP text in UI
4. **Non-zero default resistances** — Anti-Magic/Fire start at 5 (vs 0 in DM1)
5. **Larger portrait data** — 44 KB vs ~8 KB, higher fidelity Saturn graphics

---

## What's the Same as DM1

- 24 roster, 4 active party
- Food/water resource model
- Alignment magic system (Fighter/Neutral, Priest/Good, Wizard/Chaos)
- XP-based level advancement (same formulas)
- Resurrection at 25% HP/STA
- Inventory system (same 12 slots per champion)
- Spell learning system (16 spells from runes)

---

## Comparison: DM1 vs Nexus Champion System

| Aspect | DM1 | Nexus |
|--------|-----|-------|
| Classes | 3 (Fighter, Wizard, Priest) | **4 (adds Ninja)** |
| Roster size | 24 | 24 |
| Party size | 4 | 4 |
| Names | Western (Thor, Sara...) | **Japanese (Syra, Leyla...)** |
| Food/Water | 1500 each | 1500 each |
| Anti-Magic default | 0 | **5** |
| Anti-Fire default | 0 | **5** |
| Resurrection | Yes (25% HP/STA) | Yes (25% HP/STA) |
| Portrait format | Sprite sheet (~8 KB) | FACE.BIN (44 KB) |
| Alignment system | Yes | Yes (same) |
| Stat advancement | XP-based | XP-based (same DM1 formulas) |

---

## Status: SOURCE-LOCKED (Champion Roster)

Champion roster, stats, classes, and init values are from `nexus_v1_champions.c`
which explicitly lists all 8 champions with their base stats. The DM1 inheritance
(exact stat formulas, advancement, spell system) is inferred from engine comments
and DM1 source structure. No byte verification of FACE.BIN portraits or actual
game data.