# Nexus V1 — Combat System vs DM1/DM2

## Source
- `src/nexus/nexus_v1_combat.c` — core combat resolver
- `src/nexus/nexus_v1_creatures.c` — creature AI and spawning
- `src/nexus/nexus_v1_magic.c` — magic system

---

## Overview

Nexus V1 implements a DM1-style turn-based first-person combat system, with
simplified AI and a reduced creature roster. Combat is party-wide: each
living champion can act in turn order, consuming stamina to attack.

---

## What's the Same as DM1

- **First-person view** — combat takes place in the dungeon viewport
- **Turn-based** — party and creatures alternate turns
- **Melee + ranged** — swords/axes at adjacent squares, crossbow at range
- **Stamina cost** — 3 stamina consumed per attack (nexus_v1_combat.c)
- **Critical hits** — 5% chance, double damage
- **Death and XP** — champions die at 0 HP; survivors gain experience
- **No outdoors combat** — Nexus is dungeon-only (no sky/building zones)

---

## What's Different vs DM1

| Aspect | DM1 | Nexus V1 |
|---|---|---|
| Renderer | 2D sprite projection | 3D DMDF polygon rasterizer |
| Creature roster | ~64 types (full) | ~15 core types (reduced) |
| Creature AI | Full awareness (wary, terrain, pits, teleporters) | Simplified chase/patrol |
| Hit chance cap | Not confirmed capped | Capped at **95%** (explicit) |
| Ranged weapon types | Crossbow only | Crossbow only (no gun/bomb in Nexus) |
| Magic system | Rune words, 10 spells | Same 10 spells + alignment extension stub |

---

## What's Missing vs DM2

| Aspect | DM2 | Nexus V1 |
|---|---|---|
| Companion system | Up to 4 NPC companions | **Absent** — party is champions only |
| Tech weapons | Guns, bombs with tech_level | **Absent** — no gun/bomb |
| Outdoor combat | Isometric top-down outdoors | **Absent** — dungeon only |
| Creature spell attacks | Fireball, lightning, dispel, push/pull | **Absent** in Nexus creature AI |
| Scroll/wand items | Full scroll+wand system | **Absent** — inherited from DM1 only |
| Steal mechanic | Creatures can steal items | **Absent** |
| Extended creature AI | Scripted behaviors | **Absent** — hardcoded only |

---

## Turn Order (Inferred from Code)

From `nexus_v1_combat.c` and `nexus_v1_creatures_tick()`:

```
1. Party champion attacks
   - Consume 3 stamina
   - Roll hit chance (dex + fighter_level*2, cap 95%)
   - On hit: roll damage (weapon_power + str/5 + RNG)
   - Critical check (5%, double damage)
   - Apply defense reduction

2. Creature tick (each active creature)
   - Check distance to party (Manhattan)
   - State machine: patrol (>3 tiles) → chase (≤3 tiles) → attack (≤1 tile)
   - Move every N ticks based on creature speed
   - Creature attacks when adjacent

3. Death check
   - Health ≤ 0 → champion/creature dies
   - Party wipe if all champions dead
```

---

## Class System in Combat

Nexus V1 exposes 4 champion classes for skill advancement:

| Class | Relevant Stat | Effect |
|---|---|---|
| Fighter | fighter_level | Hit chance: +2% per level |
| Ninja | ninja_level | (Future — not wired in V1 combat) |
| Priest | priest_level | Required for spell power ≥ N |
| Wizard | wizard_level | Required for spell power ≥ N |

From `nexus_v1_attack()`: hit_chance = dexterity + fighter_level * 2

Experience is awarded per damage dealt: `experience_gained = damage`

---

## Status: PARTIALLY SOURCE-LOCKED

- Combat formula: **source-locked** — explicit code in `nexus_v1_combat.c`
- Stamina cost: **source-locked** — 3 hardcoded
- Critical hit rate: **source-locked** — 5% hardcoded
- Creature AI: **partial** — simplified chase/patrol; awareness system not implemented
- Class skills: **partial** — fighter_level used; ninja_level unused in combat