# DM2 V1 — Creature AI Changes vs DM1

**Source-locked to:** skproject/SKWIN/SkWinCore.cpp, DME.h:1505-1570, defines.h:680-716, ai_creature.md (DM1 AI reference)

---

## 1. AI Architecture: Fundamental Change

DM2 uses a completely different AI architecture from DM1.

### DM1: Event-driven FSM (ReDMCSB GROUP.C)
- Behavior states: WANDER (C0), FLEE (C5), ATTACK (C6), APPROACH (C7)
- Group-wide behavior stored per-group (not per-creature)
- Per-creature events C38-C41 for attack timing
- Event timeline drives all state transitions

### DM2: AI Table + Runtime Instance (SkWinCore)
- Pre-defined AIDefinition array `dAITable[64]` (hard-coded `dAITableGenuine`)
- Optional GDAT override in extended mode (EXTENDED_LOAD_AI_DEFINITION, SkWinCore.cpp:233-400)
- Per-creature runtime info via `sk1c9a02c3` struct in `glbTabCreaturesInfo[]`
- Creature query via `QUERY_CREATURE_AI_SPEC_FROM_TYPE(creatureType)` → AIDefinition*
- Behavior driven by: w0AIFlags, AttacksSpells, w30 (missile turning), Weight (push resistance)

Source: SkWinCore.cpp:2995, DME.h:1505-1545

---

## 2. AIDefinition Flags (w0AIFlags) — DM2's Behavior Control

DM1 used separate BEHAVIOR enum values (C0-C7). DM2 encodes behavior as bitflags in w0AIFlags:

| Bit | Accessor | Description |
|---|---|---|
| 0 | `IsStaticObject()` | 1=static (non-moving object) |
| 1 | w0_1_1 | Reflector? |
| 2 | w0_2_2 | Unknown |
| 3 | w0_3_3 | Spectres and ghosts |
| 4 | w0_4_4 | Spectres/ghosts + vexirks |
| 5 | w0_5_5 | Non-material (intangible) |
| 6-7 | w0_6_7 | Worms and glops both set |
| 8 | `PushWhenMoving()` | Moves and pushes anything on target |
| 9 | `AbsorbsMissile()` | Absorbs/blocks projectiles |
| 10 | w0_a_a | Related to invisibility (ghosts + dragoth) |

Source: DME.h:1545-1560

### Key differences from DM1:
- **Flying:** `bFlying` bit in w0AIFlags — creatures that fly (DM1 had no equivalent)
- **Invisible:** `bInvisible` bit — only Pit Ghost (index 53) has this
- **Non-material:** `w0_5_5` — intangible creatures (ghosts, Dragoth)
- **Missile absorption:** `AbsorbsMissile` — most creatures block projectiles

---

## 3. Attack/Spell System (AttacksSpells field)

DM1: Attack range checked via F0207_GROUP_IsCreatureAttacking, with basic melee/ranged
DM2: Rich AttacksSpells flags in AIDefinition:

```
X16 AttacksSpells;  // @14 — attack/spell commands
```

Checked via `dAITable[index].AttacksSpells & AI_ATTACK_FLAGS__*` at SkWinCore.cpp:415-437.

Spell attack resolution at SkWinCore.cpp:27038-27096:
- MELEE → physical attack
- PUSH_BACK → knockback
- STEAL → item theft
- FIREBALL → OBJECT_EFFECT_FIREBALL
- DISPELL → remove enchantments
- LIGHTNING → OBJECT_EFFECT_LIGHTNING
- POISON_CLOUD/BOLT/BLOB → poison effects
- PUSH_SPELL/PULL_SPELL → telekinetic
- SHOOT → projectile launch

---

## 4. Behavior State Machine Comparison

### DM1 States (GROUP.C:1372-1378)
- C0 BEHAVIOR_WANDER — random walk, chase if smelt
- C5 BEHAVIOR_FLEE — run from danger
- C6 BEHAVIOR_ATTACK — attack party
- C7 BEHAVIOR_APPROACH — move to last known party position
- C2-C4: declared but never used

### DM2 Behavior Model
DM2 does NOT use a behavior enum like DM1. Instead:
1. **Static objects** (w0_0_0): don't move (Obelisks, Braziers, Tables, etc.)
2. **Mobile creatures:** `QUERY_CREATURE_AI_SPEC_FROM_TYPE()` → flags control movement
   - `bFlying`: can fly over obstacles
   - `w0_5_5`: non-material — no physical collision
   - `PushWhenMoving`: pushes back anything in path
3. **AI action selection:** driven by `AttacksSpells` flags checked at runtime
4. **Trigger system:** `w16/w18` fields control trigger activation
5. **Missile interaction:** `w30` (0x0800) = can turn missiles back

Source: DME.h:1545, SkWinCore.cpp:16815-16936, 2995-3044

---

## 5. New AI Capabilities in DM2

### Missile Turning (w30:0x0800)
- Creatures with this flag can redirect missiles back at the party
- Checked at SkWinCore.cpp:10479, 10561
- DM1 had no equivalent

### Push Resistance (Weight field)
- `Weight` (AIDefinition.b29): push resistance
- 255 = immovable (cannot be pushed)
- Used for heavy objects vs lightweight creatures

### Multi-trigger Creatures (w16/w18)
- `w16` and `w18` fields: "Can switch triggers"
- DM1 had no equivalent trigger-switching mechanism

### Health Scaling (spawn-time)
- `ALLOC_NEW_CREATURE`: healthMultiplier parameter scales HP at spawn
- `si = (healthMultiplier * BaseHP) >> 3`
- DM1: HP fixed at creature type value; DM2: HP varies per spawn

### Map-specific Spawning (CREATE_MINION)
- Spawn on any map (not just party map)
- Different from DM1's group-spawn-in-same-map

---

## 6. Comparison: DM1 vs DM2 AI

| Aspect | DM1 | DM2 |
|---|---|---|
| Behavior model | Event-driven FSM (C0-C7 enum) | Flag-based (AIDefinition w0AIFlags) |
| Group behavior | Per-group (Group->Behavior) | Per-creature-type (AIDefinition flags) |
| Attack selection | Distance + range check | AttacksSpells flags |
| Spellcasting | None | Fireball, Dispell, Lightning, Poison, Push/Pull |
| Flying creatures | None | `bFlying` bit |
| Invisible creatures | None (Pit Ghost was invisible via w0_6_7?) | `bInvisible` bit |
| Non-material/intangible | Ghosts via w0_3_3 | `w0_5_5` |
| Missile interaction | Absorbed/reflected by walls | `AbsorbsMissile` + missile turning |
| Push resistance | None | `Weight` field |
| Spawn HP variance | Fixed per creature type | `healthMultiplier` parameter |
| Multi-map spawn | Same map only | `CREATE_MINION` with map index |
| Trigger system | Simple event | Multi-trigger (w16/w18 fields) |

---

## 7. Key Source Functions

| Function | Location | Role |
|---|---|---|
| `getAIName(U8 ai)` | SkWinCore.cpp:741 | Returns creature name by AI index |
| `QUERY_CREATURE_AI_SPEC_FROM_TYPE(Bit8u)` | SkWinCore.cpp:2995 | Get AIDefinition for creature type |
| `QUERY_GDAT_CREATURE_WORD_VALUE` | SkWinCore.cpp:3008 | Read GDAT creature stat |
| `ALLOC_NEW_CREATURE(type, mult, dir, x, y)` | SkWinCore.cpp:16815 | Spawn creature with HP scaling |
| `CREATE_MINION(type, mult, dir, x, y, map, missile, searchdir)` | SkWinCore.cpp:16901 | Spawn minion on specific map |
| `EXTENDED_LOAD_AI_DEFINITION()` | SkWinCore.cpp:233 | Load AI table from GDAT in extended mode |
| `IsStaticObject()` | DME.h:1545 | w0_0_0 check: 1 = non-moving |
| `bFlying`, `bInvisible` | DME.h:1519-1523 | Movement/invisibility flags |
| `w0_5_5` | DME.h:1553 | Non-material (intangible) |

---

## STATUS: SOURCE-LOCKED
