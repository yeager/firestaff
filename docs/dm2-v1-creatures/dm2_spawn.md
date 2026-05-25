# DM2 V1 — Creature Spawn Patterns

**Source-locked to:** skproject/SKWIN/SkWinCore.cpp:16815-16936 (ALLOC_NEW_CREATURE, CREATE_MINION), DME.h:1993-2030 (sk1c9a02c3 creature record), SKWIN/SkWinCore.cpp:17615-17626

---

## 1. Creature Allocation System

### ALLOC_NEW_CREATURE (SkWinCore.cpp:16815)

```
ObjectID SkWinCore::ALLOC_NEW_CREATURE(
    U16 creaturetype,      // creature AI index (0-63)
    U16 healthMultiplier,  // 1-31 (base is 8): HP = (mult * BaseHP) >> 3
    U16 dir,               // direction
    U16 xx,                // X position
    U16 yy                 // Y position
)
```

Key spawn mechanics:
- **Health multiplier:** `si = (healthMultiplier * BaseHP) >> 3` — spawn HP scales from base HP
  - healthMultiplier range: 1-31, base reference is 8
  - Final HP: `RAND16((si >> 3) + 1) + si` — random variance added
- **Max creatures:** Checked against `glbCreaturesCount` (SkWinCore.cpp:16848)
- Creature placed at trigger position (xx, yy) via `MOVE_RECORD_TO`
- Creature type stored as `CreatureType(U8)` in creature record

Source: SkWinCore.cpp:16815-16872

### CREATE_MINION (SkWinCore.cpp:16901)

Creates minions on a specific map (different from party map):

```
ObjectID SkWinCore::CREATE_MINION(
    U16 creatureType,      // AI index
    U16 healthMultiplier,  // HP scale
    U16 creatureDir,       // direction
    U16 xx, U16 yy,       // position
    U16 zz,               // map index
    ObjectID ww,          // missile record to attach
    i16 dir               // search direction (or -1)
)
```

Minion spawn logic:
- Changes to target map (zz) before spawning
- Searches for valid spawn square using `dir` (spiral search)
- Valid tile check: not wall, not stairs, not locked door, no existing creature
- Tile type checks: `ttWall`, `ttStairs`, `ttTrickWall`, `ttDoor` all excluded
- If primary tile invalid, tries adjacent tiles in spiral pattern
- `0x8000` flag on creatureType: creates associated missile record

Source: SkWinCore.cpp:16901-17000

---

## 2. Spawn Group Size and Rate

### Hard-coded AI Table (dAITableGenuine)

The `dAITableGenuine` static table contains per-creature-type base stats used during spawn:
- `BaseHP` — spawn HP scale factor
- `AttackStrength` — damage output
- `Defense` — damage reduction
- `ArmorClass` — hit chance reduction
- `PoisonDamage` — poison on hit
- `AttacksSpells` — spell attack flags
- `Weight` — push resistance

### Spawn Rate Factors

From `EXTENDED_LOAD_AI_DEFINITION` (SkWinCore.cpp:233-400):
- GDAT category `GDAT_CATEGORY_CREATURE_AI` (0x19) provides per-entry data
- Extended mode reads 36 fields per creature AI index from GDAT
- Fields 0-35 map to AIDefinition struct members (see dm2_creature_abilities.md)

### Spawn in Dungeon Maps

Dungeon map spawn controlled by:
- DUNGEON.DAT square types (wall, floor, door, teleporter, etc.)
- Actuator triggers for creature spawn events
- `ALLOC_CAII_TO_CREATURE` at SkWinCore.cpp:42160 — allocates creature AI instance

---

## 3. Creature Record Structure (sk1c9a02c3)

Creature instances stored in `glbTabCreaturesInfo[]` array (SkWinCore.h:527):

```
struct sk1c9a02c3 { // per-creature runtime info
    // w0: creature instance index
    // w2: hit points (current)
    U16 w4;          // hit points of creature 2
    U16 w6;          // hit points of creature 3
    U16 w8;          // hit points of creature 4
    ...
}
```

Creature index lookup: `glbTabCreaturesInfo[creature->b5_0_7()]`

Source: DME.h:1993-2030, SkWinCore.cpp:3066

---

## 4. Minion System (DM2 Unique)

DM2 introduces a multi-type minion/companion system:

| Minion Type | AI Index | Role |
|---|---|---|
| Scout Minion | 13 | Ally — reconnaissance |
| Attack Minion | 14 | Ally — combat |
| Carry Minion | 15 | Ally — item transport |
| Fetch Minion | 16 | Ally — item fetching |
| Guard Minion | 17 | Ally — defense |
| U-Haul Minion | 18 | Ally — heavy transport |
| Dragoth Minion | 34 | Evil — Dragoth sub-type |
| Evil Attack Minion | 43, 62 | Enemy — combat |
| Evil Guard Minion | 49 | Enemy — defense |

Minions are spawned via `CREATE_MINION` with healthMultiplier and specific map targeting.
The `0x8000` flag on creatureType signals creation of an associated missile record.

Source: getAIName, SkWinCore.cpp:16901

---

## 5. Comparison with DM1

DM1 spawn system (from ReDMCSB GROUP.C):
- Simple per-group spawning
- Spawn rate controlled by dungeon square events
- No companion/minion distinction
- No health multiplier scaling per spawn
- Creature group size limited by group record slots

DM2 spawn adds:
- Health multiplier (1-31) scaling per spawn
- Map-specific minion spawning (CREATE_MINION with map index)
- Multiple minion sub-types with different roles
- Missile-attached creature spawn (0x8000 flag)
- Spiral search for valid spawn tiles

---

## STATUS: SOURCE-LOCKED
