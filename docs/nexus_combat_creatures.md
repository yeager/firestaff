# Nexus V1 — Creature Combat

## Source
- `src/nexus/nexus_v1_creatures.c` — creature definitions, spawn, AI tick

---

## Creature Roster

Nexus V1 defines a reduced creature set (~15 core types, vs DM1's ~64):

| Name | MNS File | HP | ATK | DEF | SPD | XP |
|---|---|---|---|---|---|---|
| Scorpion | SCORPION.MNS | 30 | 15 | 5 | 3 | 25 |
| Mummy | MUMMY.MNS | 50 | 20 | 8 | 2 | 40 |
| Dragon | DRAGON.MNS | 200 | 60 | 30 | 4 | 200 |
| Skeleton | SKELETON.MNS | 35 | 18 | 6 | 3 | 30 |
| Ghost | GHOST.MNS | 25 | 12 | 2 | 5 | 20 |
| Worm | WORM.MNS | 80 | 25 | 10 | 2 | 60 |
| Golem | GOLEM.MNS | 120 | 35 | 20 | 1 | 100 |
| Spider | SPIDER.MNS | 20 | 10 | 3 | 4 | 15 |

Source: `g_creature_defs[]` table in `nexus_v1_creatures.c`

---

## Spawning

```c
int nexus_v1_creature_spawn(Nexus_V1_CreatureManager *mgr,
    int type_idx, int x, int y, int dir);
```

- Creatures are spawned at specific dungeon coordinates (x, y)
- Each has a facing direction (dir)
- Active creatures stored in `Nexus_V1_CreatureManager.active[]`
- Limit: `NEXUS_MAX_ACTIVE_CREATURES`
- Initial state: alive=1, state=1 (patrol), ai_timer=0

---

## Creature AI State Machine

Nexus V1 uses a simplified 3-state AI (vs DM1's full awareness system):

| State | Name | Trigger | Behavior |
|---|---|---|---|
| 1 | Patrol | distance > 3 tiles | No movement toward party |
| 2 | Chase | distance ≤ 3 tiles | Move toward party every N ticks |
| 3 | Attack range | distance ≤ 1 tile | Adjacent — creature attacks |

Movement tick interval = `6 - creature_speed`:
- Fast creature (SPD 5): moves every 1 tick
- Slow creature (SPD 1): moves every 5 ticks

Chase logic (Manhattan distance):
```c
if (abs(party_x - c->x) > abs(party_y - c->y))
    c->x += (party_x > c->x) ? 1 : -1;
else
    c->y += (party_y > c->y) ? 1 : -1;
```

---

## Creature Attack

Creature attack damage is the creature's fixed ATK stat (from the type table).
No formula or roll is performed — ATK is used directly.

**What's absent vs DM2:**
- No spell-casting creatures (fireball, lightning, dispel, push/pull)
- No awareness system (wary state, terrain, pit avoidance, teleporter routing)
- No steal mechanic
- No group coordination / scripting engine

---

## DM2 Creature Additions Not in Nexus

DM2 added ~22 new creature types with more complex behaviors:
- Spell-using creatures (Amplifier with fireball)
- Stealing creatures (Thicket Thief)
- Push/pull spell creatures
- DM2 creatures GIGGLER and VEXIRK appear in Nexus .MNS file list —
  some DM2 types were back-ported, but no spell-casting AI was carried over

---

## Status: PARTIALLY SOURCE-LOCKED

- Creature stats (HP/ATK/DEF/SPD/XP): **source-locked** — hardcoded table
- AI state machine: **source-locked** — 3-state patrol/chase/attack
- Awareness/wary/terrain system: **absent**
- Spell-casting creatures: **absent**
- .MNS file list from ISO classification (`docs/NEXUS_FILE_CLASSIFICATION.md`)