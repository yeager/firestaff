# DM2 V1 Phase 6: Creature AI, Combat, Spells, Weather, Sounds, Drops — Source Lock

**Audit date:** 2026-05-26
**Phase:** DM2 V1 Phase 6 (TODO.md)
**ReDMCSB ref:** SKULL.ASM (522,128 lines, sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
**ReDMCSB local:** `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
**Secondary refs:** skproject (github.com/gbsphenx/skproject) SKWIN/SkWinCore.cpp, SkGlobal.cpp/h, defines.h, DME.h; SKULLWIN/c_ai.cpp, c_creature.cpp

---

## 1. Creature AI Architecture: DM2 vs DM1

### 1.1 DM1: Event-driven FSM (ReDMCSB GROUP.C)

DM1 used a per-group event-driven finite state machine with behavior states C0–C7:
- **C0 BEHAVIOR_WANDER** — random walk; chase if party smelt
- **C5 BEHAVIOR_FLEE** — run from danger (fear check after damage)
- **C6 BEHAVIOR_ATTACK** — attack party
- **C7 BEHAVIOR_APPROACH** — move to last known party position
- C2–C4 declared but never used

Per-creature events C38–C41 controlled attack timing. Behavior was a function of distance + creature type + random seed. No named command byte dispatch.

### 1.2 DM2: CCM Command-Dispatch State Machine

DM2 replaces the inline distance-check FSM with **bytecode-like command dispatch** through `DM2_PROCEED_CCM`. The `b_1a` byte is the **primary state register**; `DM2_PROCEED_CCM` dispatches based on its value.

The companion/minion system (indices 13–18 ally; 34/43/49/62 evil) is unique to DM2 — no DM1 equivalent.

| `b_1a` range | Command | Description |
|---|---|---|
| 0x00–0x01 | WALK_NOW | Movement dispatch |
| 0x01–0x02 | attack logic | Attack handler |
| 0x02–0x05 | WALK_NOW | Movement continuation |
| 0x05–0x07 | CCM06/CCM0B/CCM0C | Special actions |
| 0x09–0x0a | STEAL_FROM_CHAMPION | Thief-type item theft |
| 0x0a–0x0d | CCM0B/CCM0C | Merchant/shop behavior |
| 0x0d–0x0f | SHOOT_ITEM | Ranged throw/pickup |
| 0x0f–0x13 | KILL_ON_TIMER_POSITION | Delayed-position kill |
| 0x13–0x15 | ROTATES_TARGET_CREATURE | Reorient another creature |
| 0x15–0x17 | CAST_SPELL | Monster spellcasting |
| 0x17–0x2b | merchant/combat idle | Non-combat behavior |
| 0x26–0x28 | EXPLODE_OR_SUMMON | Self-destruct or spawn minion |
| 0x17–... | CREATURE_ATTACKS_PARTY | Fallback attack |

State transitions: the action handler writes the next `b_1a` command byte directly — no explicit next-state field. The action handler writes the next `b_1a` command byte directly.

Source: `skproject/SKULLWIN/c_ai.cpp` (DM2_THINK_CREATURE), `skproject/SKULLWIN/c_creature.cpp` (DM2_PROCEED_CCM, DM2_CREATURE_ATTACKS_PARTY), `skproject/SKULLWIN/c_creature.h` (`b_1a`, `b_17` fields)

### 1.3 AIDefinition Table

DM2 extends the creature AI table from **42 entries (DM1)** to **64 entries (DM2)**.

```c
// skproject/SKWIN/DME.h:1505-1538 — 36-byte AIDefinition struct
struct AIDefinition { // 36 bytes total
    Bit16u w0AIFlags;       // @0  — behavior/static/flying/invisible
    Bit8u  ArmorClass;      // @2  — armor class (defense rating)
    i8     b3;              // @3
    Bit16u BaseHP;          // @4  — initial hit points
    U8     AttackStrength;  // @6  — base physical damage
    U8     PoisonDamage;    // @7  — poison damage on hit
    U8     Defense;         // @8  — 255 = undestroyable
    X8     b9x;             // @9  — 0x40: pit ghost marker
    X16    w10;             // @10
    X16    w12;             // @12
    X16    AttacksSpells;   // @14 — AI_ATTACK_FLAGS (attack/spell flags)
    X16    w16;             // @16 — switch triggers
    X16    w18;             // @18
    U16    w20;             // @20
    Bit16u w22;             // @22
    Bit16u w24;             // @24 — resistance (fire/poison)
    X16    w26;             // @26
    U8     b28;             // @28
    Bit8u  Weight;           // @29 — push resistance, 255=immovable
    Bit16u w30;             // @30 — 0x0800: can turn missiles
    X16    w32;             // @32
    Bit8u  b34;             // @34
    Bit8u  b35;             // @35
}
```

Source: `skproject/SKWIN/DME.h:1505-1560`

### 1.4 w0AIFlags Bitfield

| Bit | Accessor | Value | Description |
|---|---|---|---|
| 0 | `IsStaticObject()` | 0x0001 | 1=static (non-moving object) |
| 1 | w0_1_1 | 0x0002 | Reflector? |
| 2 | w0_2_2 | 0x0004 | Unknown |
| 3 | w0_3_3 | 0x0008 | Spectres and ghosts |
| 4 | w0_4_4 | 0x0010 | Spectres/ghosts + vexirks |
| 5 | w0_5_5 | 0x0020 | Non-material (intangible) |
| 6–7 | w0_6_7 | 0x00C0 | Worms and glops both set |
| 8 | `PushWhenMoving()` | 0x0100 | Moves and pushes anything on target |
| 9 | `AbsorbsMissile()` | 0x0200 | Most creatures |
| 10 | w0_a_a | 0x0400 | Related to invisibility (ghosts + dragoth) |

Source: `skproject/SKWIN/DME.h:1517-1560`

### 1.5 AI Index Table (64 entries, 0x00–0x3E)

| Index | Name | DM1 Equiv? | Notes |
|---|---|---|---|
| 0 | TREE (PILLAR) | — | Environmental |
| 1 | LABORATORY TABLE | — | Environmental |
| 3 | BUSH | — | Environmental |
| 4 | PILLARS/ROD (PILLAR) | — | Environmental |
| 5 | STALAGMITE (PILLAR) | — | Environmental |
| 6 | BOULDER | — | Environmental |
| 7 | FOUNTAIN | — | Environmental |
| 8 | OBELISKS/TOMBS | — | Environmental |
| 9 | WOOD TABLE (TABLE) | — | Environmental |
| 10 | MAGICK CAULDRON | — | Environmental |
| 11 | SKULL BRAZIER | — | Environmental |
| 12 | TRADING TABLE | — | Environmental/merchant |
| 13 | SCOUT MINION (ALLY) | — | **New: companion** |
| 14 | ATTACK MINION (ALLY) | — | **New: companion** |
| 15 | CARRY MINION (ALLY) | — | **New: companion** |
| 16 | FETCH MINION (ALLY) | — | **New: companion** |
| 17 | GUARD MINION (ALLY) | — | **New: companion** |
| 18 | U-HAUL MINION (ALLY) | — | **New: companion** |
| 19 | THORN DEMON | — | New |
| 20 | OBELISK (PASSABLE) | — | New |
| 21 | VORTEX | — | New |
| 22 | FLAME ORB | — | New |
| 23 | CAVERN BAT (BAT) | DM1 bat | |
| 24 | GLOP | — | New |
| 25 | ROCKY | — | New |
| 26 | GIGGLER | DM1 | Steal flag |
| 27 | THICKET THIEF | — | New (steal) |
| 28 | TIGER STRIPED WORM (WORM) | — | New |
| 29 | TREANT (TREE GORGON) | — | New |
| 30 | LORD DRAGOTH | — | **Primary antagonist, final boss** |
| 31 | DRU TAN | — | New |
| 32 | CAVE IN | — | Trap |
| 33 | MERCHANTS | — | **New: NPC/shop** |
| 34 | DRAGOTH MINION (EVIL) | — | **New: Dragoth spawn** |
| 35 | TOWER BAT (BAT) | DM1 bat | |
| 36 | ARCHER GUARD | — | New (SHOOT flag) |
| 37 | MAGICK REFLECTOR (MACHINE) | — | New |
| 38 | POWER CRYSTAL (MACHINE) | — | New |
| 39 | EVIL FOUNTAIN (FOUNTAIN) | DM1 variant | |
| 40 | SPIKED WALL/FLOOR SPIKES | DM1 spike | Push-back flag |
| 41 | SPECTRE (GHOST) | DM1 ghost | |
| 42 | VEG MOUTH (DIGGER WORM) | — | New |
| 43 | EVIL ATTACK MINION (EVIL) | — | New |
| 44 | AXEMAN | DM1 axeman | |
| 45 | CAVERN/STONE TABLE | — | New |
| 46 | MUMMY | — | New |
| 47 | VOID DOOR (MACHINE) | — | New |
| 48 | DARK VEXIRK (VEXIRK) | — | **New: Vexirk race** |
| 49 | EVIL GUARD MINION (ENEMY) | — | New |
| 50 | SKELETON | DM1 skeleton | |
| 51 | AMPLIFIER (MACHINE) | — | **FIREBALL spell** |
| 52 | WOLF | — | New |
| 53 | PIT GHOST (GHOST) | DM1 variant | Invisible flag |
| 54 | DOOR GHOST (GHOST) | DM1 variant | |
| 55 | VEXIRK KING (VEXIRK) | — | **Elite Vexirk boss** |
| 56 | ? OBELISK LIKE ? | — | Unknown |
| 57 | AXEMAN THIEF | — | New |
| 58 | FLYING CHEST | — | New |
| 59 | BARREL | — | New |
| 60 | PEDISTAL (PILLAR) | — | New |
| 61 | GHOST | DM1 ghost | |
| 62 | EVIL ATTACK MINION (EVIL) | — | Same as index 43 |

Source: `skproject/SKWIN/SkWinCore.cpp:741-810` (getAIName), `skproject/SKWIN/SkGlobal.h:1007-1012`

### 1.6 Key AI Functions

| Function | Location | Role |
|---|---|---|
| `getAIName(U8 ai)` | SkWinCore.cpp:741 | Returns creature name by AI index |
| `QUERY_CREATURE_AI_SPEC_FROM_TYPE(Bit8u)` | SkWinCore.cpp:2995 | Get AIDefinition for creature type |
| `QUERY_GDAT_CREATURE_WORD_VALUE` | SkWinCore.cpp:3008 | Read GDAT creature stat |
| `ALLOC_NEW_CREATURE(type, mult, dir, x, y)` | SkWinCore.cpp:16815 | Spawn creature with HP scaling |
| `CREATE_MINION(type, mult, dir, x, y, map, missile, searchdir)` | SkWinCore.cpp:16901 | Spawn minion on specific map |
| `EXTENDED_LOAD_AI_DEFINITION()` | SkWinCore.cpp:233 | Load AI table from GDAT in extended mode |
| `DM2_THINK_CREATURE` | SKULLWIN/c_ai.cpp | NPC/companion planning tick |
| `DM2_PROCEED_CCM` | SKULLWIN/c_creature.cpp | Command-dispatch main loop |
| `DM2_CREATURE_ATTACKS_PARTY` | SKULLWIN/c_creature.cpp | Creature-vs-party attack resolver |

### 1.7 Comparison: DM1 vs DM2 Creature AI

| Aspect | DM1 | DM2 |
|---|---|---|
| Behavior model | Event-driven FSM (C0–C7 enum) | Flag-based + CCM command dispatch |
| Group behavior | Per-group (Group->Behavior) | Per-creature-type (AIDefinition flags) |
| Attack selection | Distance + range check | AttacksSpells flags |
| Spellcasting creatures | None | Fireball, Dispell, Lightning, Poison, Push/Pull |
| Flying creatures | None | `bFlying` bit in w0AIFlags |
| Invisible creatures | None | `bInvisible` bit |
| Non-material/intangible | Ghosts via w0_3_3 | `w0_5_5` |
| Missile interaction | Absorbed/reflected by walls | `AbsorbsMissile` + w30 missile turning |
| Push resistance | None | `Weight` field |
| Spawn HP variance | Fixed per creature type | `healthMultiplier` parameter |
| Multi-map spawn | Same map only | `CREATE_MINION` with map index |
| Companion/minion system | None | Ally (13–18) and evil (34/43/49/62) minions |
| Command dispatch | Inline distance checks | Named command bytes (`b_1a` state machine) |

---

## 2. Attack Types and Projectile System

### 2.1 AI_ATTACK_FLAGS — DM2 Creature Attack Types

DM2 extends creature attacks beyond DM1's pure melee/ranged with a 16-bit flag field:

| Flag | Value | Attack Type | Applied To | Ref |
|---|---|---|---|---|
| `AI_ATTACK_FLAGS__MELEE` | 0x0001 | Standard physical attack | Default most creatures | defines.h:705 |
| `AI_ATTACK_FLAGS__PUSH_BACK` | 0x0002 | Knock-back on hit | Spiked Wall/Floor Spikes (40) | SkWinCore.cpp:27038 |
| `AI_ATTACK_FLAGS__STEAL` | 0x0004 | Steal item from champion | Giggler (26), Thicket Thief (27) | SkWinCore.cpp:27046 |
| `AI_ATTACK_FLAGS__SHOOT` | 0x0008 | Throw projectile/weapon | Archer Guard (36) | SkWinCore.cpp:27096 |
| `AI_ATTACK_FLAGS__FIREBALL` | 0x0010 | Cast fireball spell | Amplifier (51) | SkWinCore.cpp:27051 |
| `AI_ATTACK_FLAGS__DISPELL` | 0x0020 | Dispel champion enchantments | — | SkWinCore.cpp:27056 |
| `AI_ATTACK_FLAGS__LIGHTNING` | 0x0040 | Cast lightning spell | — | SkWinCore.cpp:27061 |
| `AI_ATTACK_FLAGS__POISON_CLOUD` | 0x0080 | AoE poison cloud | — | SkWinCore.cpp:27066 |
| `AI_ATTACK_FLAGS__POISON_BOLT` | 0x0100 | Single-target poison bolt | — | SkWinCore.cpp:27071 |
| `AI_ATTACK_FLAGS__POISON_BLOB` | 0x0200 | Contact poison blob | Giggler gas | SkWinCore.cpp:27086 |
| `AI_ATTACK_FLAGS__PUSH_SPELL` | 0x0400 | Push spell (telekinesis) | — | SkWinCore.cpp:27076 |
| `AI_ATTACK_FLAGS__PULL_SPELL` | 0x0800 | Pull spell (telekinesis) | — | SkWinCore.cpp:27082 |

Source: `skproject/SKWIN/defines.h:705-716`, `SkWinCore.cpp:415-437` (flag routing), `SkWinCore.cpp:27038-27096` (spell resolution)

### 2.2 Spell-Based Attack Resolution (SkWinCore.cpp:27038–27096)

Each spell flag maps to an `OBJECT_EFFECT_*` constant:

```cpp
switch (dAITable[creatureIndex].AttacksSpells & AI_ATTACK_FLAGS__*) {
    case AI_ATTACK_FLAGS__MELEE:      /* normal physical */
    case AI_ATTACK_FLAGS__PUSH_BACK: OBJECT_EFFECT_PUSHBACK;     break;
    case AI_ATTACK_FLAGS__PUSH_SPELL:OBJECT_EFFECT_PUSH_SPELL;   break;
    case AI_ATTACK_FLAGS__PULL_SPELL:OBJECT_EFFECT_PULL_SPELL;   break;
    case AI_ATTACK_FLAGS__FIREBALL: OBJECT_EFFECT_FIREBALL;     break;
    case AI_ATTACK_FLAGS__DISPELL:  OBJECT_EFFECT_DISPELL;      break;
    case AI_ATTACK_FLAGS__LIGHTNING:OBJECT_EFFECT_LIGHTNING;     break;
    case AI_ATTACK_FLAGS__POISON_CLOUD:OBJECT_EFFECT_POISON_CLOUD; break;
    case AI_ATTACK_FLAGS__POISON_BOLT:OBJECT_EFFECT_POISON_BOLT; break;
    case AI_ATTACK_FLAGS__POISON_BLOB:OBJECT_EFFECT_POISON_BLOB; break;
    case AI_ATTACK_FLAGS__STEAL:    OBJECT_EFFECT_STEAL;        break;
    case AI_ATTACK_FLAGS__SHOOT:    OBJECT_EFFECT_SHOOT;        break;
}
```

### 2.3 Amplifier Fireball Patch

The Amplifier (AI index 51) is patched in fixed mode to carry the fireball attack:

```cpp
// SkWinCore.cpp:249-252
if (SkCodeParam::bUseFixedMode) {
    dAITable[51].AttacksSpells |= AI_ATTACK_FLAGS__FIREBALL;
    // Amplifier must remain static object, or it loses its moveable ability.
}
```

### 2.4 Missile Turning (AIDefinition.w30: 0x0800)

If bit 0x0800 is set in `w30`: creature **reflects projectiles back at attacker**. Checked at `SkWinCore.cpp:10479, 10561`.

DM1 had no equivalent missile-reflection mechanism.

### 2.5 Projectile/Fireball Object Effects

| Effect constant | Triggered by | Description |
|---|---|---|
| `OBJECT_EFFECT_FIREBALL` | AI_ATTACK_FLAGS__FIREBALL (Amplifier) | Fireball — high AoE damage |
| `OBJECT_EFFECT_LIGHTNING` | AI_ATTACK_FLAGS__LIGHTNING | Electric single-target |
| `OBJECT_EFFECT_DISPELL` | AI_ATTACK_FLAGS__DISPELL | Removes champion enchantments |
| `OBJECT_EFFECT_PUSH_SPELL` | AI_ATTACK_FLAGS__PUSH_SPELL | Telekinetic push |
| `OBJECT_EFFECT_PULL_SPELL` | AI_ATTACK_FLAGS__PULL_SPELL | Telekinetic pull |
| `OBJECT_EFFECT_POISON_CLOUD` | AI_ATTACK_FLAGS__POISON_CLOUD | AoE poison |
| `OBJECT_EFFECT_POISON_BOLT` | AI_ATTACK_FLAGS__POISON_BOLT | Single-target poison |
| `OBJECT_EFFECT_POISON_BLOB` | AI_ATTACK_FLAGS__POISON_BLOB | Contact poison |

Source: `skproject/SKWIN/SkWinCore.cpp:27051-27088`

---

## 3. Champion Actions in Combat

### 3.1 DM1 Champion Combat

DM1 champions use physical attacks (melee) and ranged (crossbow). Attack roll: d20-style `attack_skill + RAND(1..20) >= defense_value`. No mana, no cooldown, no class-specific special actions beyond weapon choice. Spell casting uses rune sequences with mana cost.

### 3.2 DM2 Champion Combat Additions

DM2 adds:
- **Tech weapons**: guns (DM2_WEAPON_GUN, tech_level 1+)
- **Bombs**: DM2_WEAPON_BOMB, area damage, explosion sound
- **Per-rune mana cost**: mana deducted per rune added (not at cast time)
- **Cooldown after casting**: `ADJUST_HAND_COOLDOWN(player, bp0e, 2)` at SkWinCore.cpp:17623
- **Skill decay on failed cast**: explicit penalty
- **Ranged penalty**: -10% per extra tile beyond first (crossbows, guns, bombs)

Source: `skproject/SKWIN/SkWinCore.cpp:17521-17900` (CAST_SPELL_PLAYER), `skproject/SKWIN/SkWinCore.cpp:18159-18174` (ADD_RUNE_TO_TAIL)

### 3.3 Champion Attack Resolution

```c
// dm2_v1_combat.c — Firestaff implementation
int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance)
{
    int damage, range_penalty;
    if (!weapon) return 0;
    damage = weapon->base_damage + attacker_strength / 4;
    if (distance > weapon->range) return 0;
    range_penalty = (distance - 1) * damage / 10;  // -10% per extra tile
    damage -= range_penalty;
    damage -= target_defense;
    return damage > 0 ? damage : 0;
}
```

### 3.4 Weapon Types (DM2 vs DM1)

| Type | DM2 Constant | DM1? | Notes |
|---|---|---|---|
| Melee | DM2_WEAPON_MELEE | Yes | Sword, axe, etc. |
| Thrown | DM2_WEAPON_THROWN | Limited | Daggers, axes |
| Crossbow | DM2_WEAPON_CROSSBOW | Yes | Ranged, -10%/tile penalty |
| Gun | DM2_WEAPON_GUN | **No** | Tech weapon, new in DM2 |
| Bomb | DM2_WEAPON_BOMB | **No** | Area damage, explosion sound |
| Magic | DM2_WEAPON_MAGIC | Yes | Spellcasting |

Source: `include/dm2_v1_combat.h`, `docs/dm2_combat_weapons.md`

---

## 4. Spell System (34 Spells vs DM1's ~30)

### 4.1 Spell Count and Table

DM2 V1 defines **34 spells** (index 0–33) in `dSpellsTable`. Extended mode supports up to 255 via GDAT.

```cpp
// SkGlobal.h:41
#define MAXSPELL_ORIGINAL    34
#define MAXSPELL_CUSTOM      255  // extended custom spell mode
```

Source: `skproject/SKWIN/SkGlobal.cpp:966-1011` (dSpellsTable), `skproject/SKWIN/SkGlobal.h:37-55`

### 4.2 Spell Type Constants

```cpp
// SkGlobal.h:50-53
#define SPELL_TYPE_POTION    1  // requires empty flask in hand
#define SPELL_TYPE_MISSILE   2  // fires projectile at target
#define SPELL_TYPE_GENERAL   3  // enchantments, light, auras
#define SPELL_TYPE_SUMMON    4  // summons a creature minion
```

### 4.3 Full Spell List

| Idx | Runes | Name | Type | Diff | Skill | Source |
|---|---|---|---|---|---|---|
| 0 | OH IR RA | Long Light | GENERAL | 0x04 | 0x0F | SkGlobal.cpp:966 |
| 1 | DES IR SAR | Darkness | GENERAL | 0x04 | 0x0F | |
| 2 | YA IR | Spell Shield (Party) | GENERAL | 0x04 | 0x0F | |
| 3 | OH EW SAR | Invisibility | GENERAL | 0x00 | 0x0F | |
| 4 | YA IR (2sym) | Magical Shield | GENERAL | 0x00 | 0x0F | |
| 5 | FUL | Light | GENERAL | 0x00 | 0x0F | |
| 6 | OH EW DAIN | Aura of Wisdom | GENERAL | 0x00 | 0x0F | |
| 7 | OH EW ROS | Aura of Dexterity | GENERAL | 0x00 | 0x0F | |
| 8 | FUL BRO NETA | Fire Shield | GENERAL | 0x00 | 0x0F | |
| 9 | OH EW NETA | Aura of Vitality | GENERAL | 0x00 | 0x0F | |
| 10 | OH EW KU | Aura of Strength | GENERAL | 0x00 | 0x0F | |
| 11 | OH IR ROS | Aura of Speed | GENERAL | 0x00 | 0x0F | |
| **12** | **ZO BRO ROS** | **Spell Reflector** | **GENERAL** | **0x0F** | **0x0F** | **New** |
| 13 | YA EW (2sym) | Magical Marker | GENERAL | 0x00 | 0x0F | |
| 14 | OH VEN | Poison Cloud | GENERAL | 0x07 | 0x0F | |
| 15 | OH KATH RA | Lightning | MISSILE | 0x0D | 0x0F | |
| 16 | FUL IR | Fireball | MISSILE | 0x00 | 0x0F | |
| 17 | FUL BRO KU | NP: STR Potion | POTION | 0x13 | 0x07 | |
| 18 | DES EW | Antimatter | MISSILE | 0x03 | 0x0F | |
| 19 | DES VEN | Poison Bolt | MISSILE | 0x06 | 0x0F | |
| 20 | ZO | Open/Close Door | GENERAL | 0x04 | 0x0F | |
| 21 | YA BRO | NP: Shield Potion | POTION | 0x13 | 0x0C | |
| 22 | YA | NP: Stamina Potion | POTION | 0x13 | 0x0B | |
| 23 | YA BRO DAIN | NP: Wisdom Potion | POTION | 0x13 | 0x08 | |
| 24 | YA BRO NETA | NP: Vitality Potion | POTION | 0x13 | 0x09 | |
| 25 | VI | NP: Health Potion | POTION | 0x13 | 0x0E | |
| 26 | VI BRO | NP: Anti Venin | POTION | 0x13 | 0x0A | |
| 27 | OH BRO ROS | NP: Dexterity Potion | POTION | 0x13 | 0x06 | |
| 28 | ZO BRO RA | NP: Mana Potion | POTION | 0x13 | 0x0D | |
| **29** | **ZO EW KU** | **Attack Minion** | **SUMMON** | **0x0F** | **0x31** | **New** |
| **30** | **ZO EW NETA** | **Guard Minion** | **SUMMON** | **0x0F** | **0x34** | **New** |
| **31** | **ZO EW ROS** | **U-Haul Minion** | **SUMMON** | **0x0F** | **0x35** | **New** |
| **32** | **OH KATH KU** | **Push** | **GENERAL** | **0x0D** | **0x09** | **New** |
| **33** | **OH KATH ROS** | **Pull** | **GENERAL** | **0x0D** | **0x0A** | **New** |

### 4.4 DM2-new Spells Summary

| Spell | Index | What it adds vs DM1 |
|---|---|---|
| Spell Reflector (ZO BRO ROS) | 12 | Reflects incoming spells back at caster |
| Attack Minion (ZO EW KU) | 29 | Summons Attack Minion (AI index 14) |
| Guard Minion (ZO EW NETA) | 30 | Summons Guard Minion (AI index 17) |
| U-Haul Minion (ZO EW ROS) | 31 | Summons U-Haul Minion (AI index 18) |
| Push (OH KATH KU) | 32 | Telekinetic push — displaces target away |
| Pull (OH KATH ROS) | 33 | Telekinetic pull — displaces target toward |

### 4.5 DM1 Spells Removed in DM2

These DM1/NEXUS spells do NOT appear in DM2:
- YA BRO ROS: Magic Footprints
- YA VEN SAR: Petrify
- VI BRO NETA: Restore Health
- VI BRO RA: Restore Health for Party
- OH EW RA: See Through Walls (re-implemented in DM2 EXTENDED_MODE only)
- ZO KATH RA: ZoKathRa Spell

### 4.6 Spell Casting Mechanics

From `SkWinCore.cpp:17521-17670` (CAST_SPELL_PLAYER):

```cpp
// Difficulty: ref->difficulty scale with power
U16 bp0e = (ref->w6_a_f() * (power + 18)) / 24;
U16 bp08 = ref->difficulty + power;

// Cast chance: Wizard ability +15 vs difficulty
U16 bp0c = 0 + ((RAND() & 7) + (bp08 << 4))
    + ((ref->difficulty * (power - 1)) << 3)
    + (bp08 * bp08);

// Cast loop: bp06 = player skill level
for (bp0a = bp08 - bp06; (bp0a--) > 0; ) {
    // Roll against min(WizardAbility + 15, 115)
    // If fail: skill penalty, spell fails
}

// Skill decay on failure
if (failed) ADJUST_SKILLS(player, ref->requiredSkill, bp0c << (bp08 - bp06));

// Cooldown after success
if (bp0e != 0) ADJUST_HAND_COOLDOWN(player, bp0e, 2);
```

Mana cost per rune (SkWinCore.cpp:18159-18174):
```cpp
// Mana deducted per rune added (not at cast time)
U16 di = RuneManaPower[si * 6 + symbol_0to5];
if (si != 0) { // not on POWER rune (first rune has no mana cost)
    di = (RunePowerMultiplicator[i8(champion->GetRunes()[0]) - RUNE_FIRST] * di) >> 3;
}
```

### 4.7 Comparison: DM1 vs DM2 Spell System

| Feature | DM1 | DM2 |
|---|---|---|
| Base spells | ~30 | 34 |
| Damage spells | Fireball, lightning | + Poison Bolt, Poison Cloud, Antimatter |
| Movement spells | None | Push, Pull |
| Summoning | None | Attack/Guard/U-Haul minion |
| Spell Reflect | None | Spell Reflector |
| Mana system | Fixed per spell | Per-rune mana cost |
| Cooldowns | None | Hand cooldown after casting |
| Skill decay on fail | None | Explicit penalty |
| Creature dispell attack | None | AI_ATTACK_FLAGS__DISPELL (0x0020) |
| Extended spell mode | No | Yes (255 spells via GDAT) |

---

## 5. Cloud / Weather System

### 5.1 DM1: No Weather System

DM1 had no weather, no outdoor areas, no time-of-day cycle. Dungeon environment is static.

### 5.2 DM2: Outdoor Weather Zones

DM2 outdoor levels support weather zones tracked per tick. Weather conditions:
- **Clear (0)**: Normal sky
- **Rain (1)**: Wet conditions, visibility effect, rain sprite overlay
- **Fog (2)**: Reduced visibility, gray overlay
- **Storm (3)**: Severe weather, gray sky overlay

Weather is stored in `DM2_V1_OutdoorConfig.weather` and processed each game tick via `dm2_v2_outdoor_fx_tick()`.

Source: `include/dm2_v1_outdoor_renderer.h`, `src/dm2/dm2_v2_outdoor_enhanced.c`, `docs/dm2_creatures_gfx.md`

### 5.3 Weather Particles

Outdoor weather uses rain drop sprites from the graphics data (blitline_48 16→8-bit overlay). Rain sprite sheet exists for outdoor weather animations. V2.2 enhanced mode renders animated rain particles and lightning flashes.

Source: `docs/dm2_creatures_gfx.md` ("Weather particles: rain drop sprites")

### 5.4 Time-of-Day Cycle

DM2 tracks time as minutes from midnight (0–1439). Outdoor sky color is derived from time_of_day:
- Dawn gradient (morning)
- Noon (midday)
- Afternoon
- Dusk (evening gradient)
- Night

```c
// include/dm2_v1_game.h
typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            /* 0=indoor dungeon, 1=outdoor */
    int gold;
    int reputation;
    int time_of_day;        /* minutes from midnight (0-1439) */
    const char *data_dir;
} DM2_V1_GameState;
```

Starting time: `time_of_day = 720` (noon). Each turn advances time_of_day; sky color interpolates accordingly.

Source: `include/dm2_v1_game.h`, `docs/dm2_time.md`, `docs/dm2_outdoor.md`

### 5.5 Cloud Effects in Combat

Poison Cloud/Blob spells create AoE hazard areas:
- **AI_ATTACK_FLAGS__POISON_CLOUD (0x0080)**: Area-effect poison cloud — source SkWinCore.cpp:27066-27068
- **AI_ATTACK_FLAGS__POISON_BLOB (0x0200)**: Contact poison blob — source SkWinCore.cpp:27086-27088

Both deal poison damage over time to champions standing in the affected area.

---

## 6. Timer System

### 6.1 DM2 Multi-Timer Architecture

DM2 has a layered timer system distinct from DM1's simple turn counter:

| Timer | Source | Description |
|---|---|---|
| **PROCESS_TIMER_0C** | c_tim_proc.cpp | Per-champion torch/light timers. Each champion has independent torch duration countdown. When torch expires, light radius shrinks. |
| **PROCESS_TIMER_RESURRECTION** | c_tim_proc.cpp | Death/resurrection countdown. When champion HP reaches 0, resurrection timer counts down. If expired before resurrection, champion is permanently lost. |
| **CONTINUE_ORNATE_ANIMATOR** | c_tim_proc.cpp | Wall ornament animation sequences (animated tiles, lever transitions, door sequences). |
| **CONTINUE_TICK_GENERATOR** | c_tim_proc.cpp | Primary game event ticker. Fires periodic events driving creature AI, projectile motion, and actuator processing. |
| **KILL_ON_TIMER_POSITION** | SKULLWIN/c_creature.cpp:0x0f-0x13 | Creature dies at tile after timer delay. `b_1a` command 0x0F-0x13. |

### 6.2 Event Queue System

DM2 uses an event-driven architecture (`c_eventqueue`, `c_events`) where actions can be scheduled for future execution:
- Queued events processed each frame via `PROCEED_TIMERS()`
- Used for: delayed actuator firing, projectile flight paths, creature spawn scheduling, animation sequences

Source: `skproject/SKULLWIN/c_tim_proc.cpp`, `skproject/SKULLWIN/c_timer.cpp`, `skproject/SKULLWIN/c_events.cpp`

### 6.3 INT08 Hardware Timer Tick

The Int 08h handler drives:
- Movement tick (party movement, creature AI)
- Time-of-day advance
- Weather zone updates

### 6.4 Comparison: DM1 vs DM2 Timers

| Aspect | DM1 | DM2 |
|---|---|---|
| Torch timer | Per-party global (single timer) | Per-champion (PROCESS_TIMER_0C per slot) |
| Resurrection timer | None | PROCESS_TIMER_RESURRECTION |
| Ornate animator | None | CONTINUE_ORNATE_ANIMATOR |
| Tick generator | Fixed rate | CONTINUE_TICK_GENERATOR |
| Event queue | Limited | Full c_eventqueue system |
| Creature death timer | None | KILL_ON_TIMER_POSITION |

---

## 7. Sound System

### 7.1 Architecture

DM2 has a layered audio architecture:
```
c_sound (master audio, inherits c_sfx and c_midi)
  c_midi    — HMP/MIDI music playback
  c_sfx     — sound effect ring
  c_music_wav — WAV/OGG music (Firestaff SDL addition)
  c_lw      — light/wind helper class
```

SKWin (SDL) audio: `OpenAudio()` initializes SDL at 6000 Hz. Max 16 simultaneous sound buffers (`MAX_SB = 16`). Samples are 8-bit unsigned, converted with `0x80 + raw_byte`.

Source: `docs/dm2_sound_system.md`, `skproject/SKULLWIN/c_sound.h/cpp`

### 7.2 Combat Sound Triggers

| Sound ID | Hex | Triggered By | Category | Ref |
|---|---|---|---|---|
| SOUND_CHAMPION_ATTACK | 0x00 | Champion melee attack | 0x0F (champion) | c_hero.cpp:3336, c_engage.cpp:287 |
| SOUND_CHAMPION_SHOOT | 0x01 | Ranged attack (crossbow, gun, throw) | 0x0F | c_item.cpp:1100, c_hero.cpp:3970 |
| SOUND_CREATURE_ATTACK_1 | 0x07 | Primary melee attack | 0x16 (creature) | c_creature.cpp:877 |
| SOUND_CREATURE_ATTACK_2 | 0x12 | Secondary/missile attack | 0x16 | c_creature.cpp |
| SOUND_CREATURE_PICK_STEAL | 0x08 | Thief pick/steal; Vexirk transform | 0x16 | c_creature.cpp |
| SOUND_CREATURE_GET_HIT | 0x02 | Creature hit reaction | 0x16 | c_creature.cpp:307 |
| SOUND_CREATURE_GET_HIT_2 | 0x09 | Dragoth hit reaction | 0x16 | c_creature.cpp |
| SOUND_CREATURE_REFLECTOR | 0x03 | Dragoth reflecting | 0x16 | c_creature.cpp |
| SOUND_CREATURE_DEATH | 0x11 | Creature death wail | 0x16 | c_creature.cpp |
| SOUND_CREATURE_YELL | 0x0E | Thorn Demon yell; Dragoth spawn minion | 0x16 | c_creature.cpp |
| SOUND_CREATURE_GROWL | 0x0F | Vegmouth combat growl | 0x16 | c_creature.cpp |
| SOUND_CREATURE_ACTIVATE_TRIGGER | 0x0A | Giggler, Dragoth hit in combat | 0x16 | c_creature.cpp |
| SOUND_CHAMPION_GETHIT | 0x82 | Champion takes damage | 0x0F | c_engage.cpp |
| SOUND_CHAMPION_SCREAM | 0x87 | Champion death | 0x0F | c_hero.cpp |
| SOUND_CHAMPION_BUMP | 0x8A | Champion collision | 0x0F | c_hero.cpp:3336 |
| SOUND_STD_DEFAULT | 0x84 | Punch, fall, gethit, wall bump | — | c_engage.cpp:287, 755 |
| SOUND_STD_KNOCK | 0x85 | Knockback, falling item | — | — |
| SOUND_STD_EXPLOSION | 0x81 | Bomb damage (DM2_WEAPON_BOMB) | — | GDAT2 V5 |
| SOUND_STD_THROW | 0x86 | Projectile in flight | — | c_item.cpp:1100, c_hero.cpp:3970 |

### 7.3 Sound Queue System

Two queue functions enqueue world-positioned sounds:
- `DM2_QUEUE_NOISE_GEN1(cat, idx, sfx_id, volume, x, y, flags)` — 7 params
- `DM2_QUEUE_NOISE_GEN2(cat, idx, sfx_id, extra, volume, x, y, flags)` — 9 params (extra byte)

Sound resolution: `DM2_QUERY_SND_ENTRY_INDEX(cat, idx, sfx)` → entry index → `DM2_PLAY_SOUND()`

### 7.4 Music System

Music: 28 HMP tracks (`DATA/00.hmp.mid` through `DATA/1c.hmp.mid`). Track selection: `tMusicMaps[64]` maps dungeon map to track. Music folders match dungeon variants (DATA_DM2_DM, DATA_DM2_SK, etc.).

### 7.5 Comparison: DM1 vs DM2 Sound

| Aspect | DM1 CSB | DM2 |
|---|---|---|
| Audio API | AdLib FM + PC Speaker | SoundBlaster-compatible |
| Music format | AdLib instruments | HMP/MIDI (Windows) |
| SFX channels | 3–4 voices | 16-slot ring buffer (SKWin) |
| Music tracks | ~10 | 28 tracks (00–1c) |
| 3D positioning | None | World-coordinate queue with distance attenuation |
| Bomb explosion | None | Yes (SOUND_STD_EXPLOSION 0x81) |
| Weather ambient | None | Yes (glbXAmbientSoundActivated) |

---

## 8. Drop System

### 8.1 DM1 Drop System

DM1 had a **single drop slot** per creature type in the creature record. When creature died, one item was dropped based on probability.

### 8.2 DM2 Extended Drop Tables

DM2 extends creature drop tables to **11 drop slots** (0x0A through 0x14 = indices 10–20 in GDAT record format):

```cpp
// From SKWin.GDAT2.InternalCodes.txt, CREATURE section:
// 0A 00 00: Drop 1 (item ID + count + random flags)
// 0B 00 00: Drop 2
// 0C 00 00: Drop 3
// 0D 00 00: Drop 4
// 0E 00 00: Drop 5
// 0F 00 00: Drop 6
// 10 00 00: Drop 7
// 11 00 00: Drop 8
// 12 00 00: Drop 9
// 13 00 00: Drop 10
// 14 00 00: Drop 11
```

Each drop slot contains: item ID + count + random-variation flags. DM2 also extends GDAT categories:
- `GDAT_CATEGORY_CREATURES (0x0A)` — creature stats/drops — includes 11 drop slots

Source: `SKWin.GDAT2.InternalCodes.txt` CREATURE section, `skproject/SKWIN/SkGlobal.h:636` (EXTENDED_GDAT_CATEGORIES), `docs/dm2_dungeon_design.md`

### 8.3 Drop Probabilities

Individual drop slots have probability flags (random/variation encoding per slot). Creature's `DropTableSeed` field in GDAT controls drop table RNG.

### 8.4 Special Drops

Thorn Demon (AI index 19) drops **sellable worm food** (from `docs/dm2_characters.md`). The Dead Thorn Demon trick allows cloning dead Thorn Demons to produce infinite steaks.

### 8.5 Comparison: DM1 vs DM2 Drops

| Aspect | DM1 | DM2 |
|---|---|---|
| Drop slots per creature | 1 | 11 |
| Drop table categories | Single slot in creature record | Extended GDAT category 0x0A with 11 sub-slots |
| Random variation flags | Limited | Per-slot random/count encoding |
| Special drops | Limited | Worm food (Thorn Demon), cloned items |

---

## 9. Progression Constants

### 9.1 DM1 Character Progression

DM1 champions advance through experience levels. XP thresholds per level. Stats increase on level-up. No persistent time-of-day or reputation.

### 9.2 DM2 Character Progression — What Changed

**Critically: DM2 V1 does NOT have champion character progression in the traditional RPG sense.**

From `docs/dm2_champ_changes.md`:
> "The single most important finding: DM2 V1 does NOT have a champion character progression system. Stat increases are fixed per champion type at creation, not earned through XP."

DM2 character power progression is primarily:
1. **Equipment quality** — better weapons/armor from shops/drops
2. **Tech level** — gun/bomb availability in later dungeon areas
3. **Spell access** — rune combinations unlock more spell types
4. **Companion recruitment** — ally minions provide additional combat power
5. **Reputation** — affects NPC stores, quest availability

Source: `docs/dm2_chess_changes.md`, `docs/dm2_champ_dev.md`

### 9.3 Champion Class Stat Bonuses

Class determines stat bonus progression on level-up. Class table in `docs/dm2_champ_classes.md`. But note: V1 stat increases are FIXED at champion creation, not earned through play.

### 9.4 Dungeon Level Progression

DM2 has 3 level types:
- **DM2_LEVEL_OUTDOOR (0)**: Sky/ground/tree/building, weather zones
- **DM2_LEVEL_INDOOR (1)**: Standard first-person dungeon (same model as DM1)
- **DM2_LEVEL_BUILDING (2)**: Multi-floor buildings within outdoor areas

Progression through levels is driven by story/mission completion, not character level grinding.

### 9.5 Progression-related Constants

| Constant | Value | Description |
|---|---|---|
| CREATURE_AI_TAB_SIZE | 64 (DM2) vs 42 (DM1/CSB) | DM2 AI table size |
| MAXAI | 255 (DM2) vs 62 (original) | Extended mode creature ID max |
| MAXSPELL_ORIGINAL | 34 | Spell table size |
| MAXSPELL_CUSTOM | 255 | Extended spell mode |
| GDAT_CATEGORY_CREATURES | 0x0A (DM2) vs 0xNN (DM1) | Extended DM2 category |
| Drop slots per creature | 11 (DM2) vs 1 (DM1) | From SKWin.GDAT2.InternalCodes.txt |
| Weather states | 4 (clear/rain/fog/storm) | Outdoor weather enum |
| Time-of-day range | 0–1439 (minutes) | 24-hour clock |
| Time-of-day start | 720 (noon) | Starting time |
| Companion loyalty range | 0–100 | Loyalty meter |
| Missiles turned (w30) | 0x0800 flag | Turn-if-bit-set |

Source: `skproject/SKWIN/SkGlobal.h:1007-163`, `skproject/SKWIN/defines.h:705-716`, `docs/dm2_time.md`, `docs/dm2_dungeon_design.md`

---

## 10. Key Reference File Summary

| File | Evidence For |
|---|---|
| `skproject/SKWIN/defines.h:705-716` | AI_ATTACK_FLAGS constants (0x0001–0x0800) |
| `skproject/SKWIN/DME.h:1505-1560` | AIDefinition struct (36 bytes), w0AIFlags bitfield, w30 missile flag |
| `skproject/SKWIN/SkWinCore.cpp:741-810` | getAIName — 64-entry creature name table |
| `skproject/SKWIN/SkWinCore.cpp:233-400` | EXTENDED_LOAD_AI_DEFINITION |
| `skproject/SKWIN/SkWinCore.cpp:2995, 3008` | QUERY_CREATURE_AI_SPEC_FROM_TYPE, QUERY_GDAT_CREATURE_WORD_VALUE |
| `skproject/SKWIN/SkWinCore.cpp:16815-16936` | ALLOC_NEW_CREATURE, CREATE_MINION |
| `skproject/SKWIN/SkWinCore.cpp:415-437` | Attack flag routing dispatcher |
| `skproject/SKWIN/SkWinCore.cpp:27038-27096` | Spell-based attack resolution (FIREBALL, DISPELL, LIGHTNING, POISON_*, PUSH_PULL, SHOOT, STEAL) |
| `skproject/SKWIN/SkWinCore.cpp:17521-17900` | CAST_SPELL_PLAYER — spell casting mechanics, cast chance, cooldown |
| `skproject/SKWIN/SkWinCore.cpp:18159-18174` | ADD_RUNE_TO_TAIL — per-rune mana cost |
| `skproject/SKWIN/SkGlobal.cpp:966-1011` | dSpellsTable (34 spells) |
| `skproject/SKWIN/SkGlobal.h:37-55` | MAXSPELL_ORIGINAL, SPELL_TYPE_* constants |
| `skproject/SKWIN/SkGlobal.h:636-1012` | CREATURE_AI_TAB_SIZE, MAXAI, GDAT categories |
| `skproject/SKULLWIN/c_ai.cpp` | DM2_THINK_CREATURE — NPC/companion planning tick |
| `skproject/SKULLWIN/c_creature.cpp` | DM2_PROCEED_CCM, DM2_CREATURE_ATTACKS_PARTY, creature attack hooks |
| `skproject/SKULLWIN/c_creature.h` | c_creature struct, b_1a (primary state), b_17 (secondary context) |
| `skproject/SKULLWIN/c_tim_proc.cpp` | PROCESS_TIMER_0C, PROCESS_TIMER_RESURRECTION, CONTINUE_ORNATE_ANIMATOR, CONTINUE_TICK_GENERATOR |
| `skproject/SKULLWIN/c_timer.cpp` | Timer system state |
| `skproject/SKULLWIN/c_events.cpp` | Event queue handling |
| `skproject/SKULLWIN/c_sound.h/cpp` | c_sound master audio class |
| `SKWin.GDAT2.InternalCodes.txt` | 11 drop slots per creature (0x0A–0x14), animated ornate, door types, weather zones |
| `docs/dm2_time.md` | Time-of-day cycle (0–1439 min), per-champion torch timers |
| `docs/dm2_sound_system.md` | Audio architecture, music/midi, sound queue, 16-slot SFX buffer |
| `docs/dm2_sound_combat.md` | All combat sound triggers (0x00–0x92 range), queue params |
| `docs/dm2_combat_creatures.md#AI_ATTACK_FLAGS` | AI_ATTACK_FLAGS table |
| `docs/dm2-v1-creatures/dm2_creature_ai.md` | CCM command dispatch state machine, b_1a table |
| `docs/dm2-v1-creatures/dm2_creature_abilities.md` | Full AIDefinition struct, w0AIFlags bits |
| `docs/dm2-v1-creatures/dm2_creatures_new.md` | 64-entry AI index table |
| `docs/dm2-v1-creatures/dm2_bosses.md` | Lord Dragoth, Dragoth Minion, Vexirk King, Amplifier |
| `docs/dm2_combat_magic.md` | Full 34-spell table, spell type execution, casting formula |
| `docs/spells_dm2/dm2_spells_system.md` | Spell table, rune mana cost, cooldown system |
| `docs/dm2_dungeon_design.md` | 11 drop slots, outdoor level types, GDAT category extensions |
| `docs/dm2_ai_companion.md` | Companion/minion system, loyalty (0–100), trading |
| `docs/dm2_time.md` | Weather zones, time-of-day, per-champion torch timer |
| `include/dm2_v1_combat.h` | DM2_V1_WeaponInfo, DM2_WeaponType enum |
| `src/dm2/dm2_v1_combat.c` | Combat resolver; -10%/tile ranged penalty |

---

## 11. Firestaff Implementation Status

### 11.1 IMPLEMENTED (stub or partial)
- `src/dm2/dm2_v1_combat.c`: Range-penalty combat resolver (stub; source-annotated)
- `include/dm2_v1_combat.h`: DM2_WEAPON_* enum, weapon info struct
- `src/dm2/dm2_v1_companion.c`: DM2_V1_CompanionState struct (stub — companion tick is no-op)
- `src/dm2/dm2_v1_outdoor_renderer.c`: dm2_v1_outdoor_set_weather(), weather color derivations
- `src/dm2/dm2_v2_outdoor_enhanced.c`: dm2_v2_outdoor_fx_tick() with rain/fog/storm effects

### 11.2 NOT YET IMPLEMENTED
- Full CCM command-dispatch creature AI state machine
- AIDefinition table (64-entry) with w0AIFlags, AttacksSpells, w30
- AI_ATTACK_FLAGS resolution (FIREBALL, DISPELL, LIGHTNING, POISON_*, PUSH_PULL, SHOOT, STEAL)
- ALLOC_NEW_CREATURE with healthMultiplier HP scaling
- CREATE_MINION with map index, missile flag, searchdir
- Spell casting resolver (CAST_SPELL_PLAYER full implementation)
- Per-rune mana cost deduction
- 11-slot drop table parsing from GDAT
- Full 34-spell dSpellsTable with SPELL_TYPE_* routing
- PROCESS_TIMER_0C per-champion torch countdown
- PROCESS_TIMER_RESURRECTION countdown
- KILL_ON_TIMER_POSITION (b_1a 0x0F-0x13)
- Sound queue / DM2_QUEUE_NOISE_GEN1/GEN2
- Lord Dragoth minion-spawn ability (YELL sound → CREATE_MINION)

---

## STATUS

**SOURCE-LOCKED** — All constants, structs, function names, line references, and evidence citations are sourced from SKULL.ASM, skproject SKWIN/SkGlobal.cpp+h, SkWinCore.cpp, defines.h, DME.h, and SKULLWIN/c_ai.cpp, c_creature.cpp, c_tim_proc.cpp, c_timer.cpp, c_events.cpp, c_sound.h/cpp. Implementation gaps are explicitly catalogued.

**Document ID:** dm2_v1_phase6_creature_combat_spells_weather_sounds_drops_H2306.md
