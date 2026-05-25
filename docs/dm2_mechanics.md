# DM2 V1 — Game Mechanics: How DM2 Plays vs DM1

**Audit date:** 2026-05-25
**Sources:** SKULL.ASM, skproject SKWIN/SkWinCore.cpp, skproject SkGlobal.h, include/dm2_v1_game.h, docs/dm2_overview.md, docs/dm2_newfeatures.md, docs/dm2_game_loop.md, docs/dm2_champ_changes.md

---

## 1. Overview — What Changed

Dungeon Master II (1993) is a direct sequel to Dungeon Master (1987) but runs on a **completely new engine** - not derived from the DM1/CSB codebase. The shift from DM1 to DM2 is thus less like a patch and more like a new game built on similar design principles.

| Aspect | DM1 (1987) | DM2 (1993) |
|--------|------------|-------------|
| Engine | DM1 engine (FTL) | New Skullkeep engine |
| Setting | Chaos Horde dungeon | Skullkeep fortress + outdoor |
| World scope | Indoor only | Indoor + outdoor exploration |
| World size | ~10 levels | Outdoor + buildings + multi-level dungeon |
| Starting gold | 0g | 100g |
| Starting position | Level 1 entrance | Level 15 center |

---

## 2. Core Game Loop

DM2's game loop (GAME_LOOP in SkWinCore.cpp) is structurally similar to DM1 - it processes input, advances game state, renders - but has several DM2-specific additions:

1. **Timer tick** - int 08h handler (_INT08_HANDLER): movement, AI ticks, time-of-day advance
2. **Mouse input** - IBMIO_MOUSE_HANDLER (int 33h)
3. **Keyboard input** - IBMIO_KBOARD_HANDLER (int 09h)
4. **Event queue** - c_eventqueue for delayed/timed actions
5. **Process timers** - PROCEED_TIMERS() each frame
6. **Render pass** - backbuffer composite + blit to screen

Unlike DM1 which used a fixed 10Hz or 15Hz tick, DM2 has more granular timer handling:
- PROCESS_TIMER_0C - per-champion light/torch timers
- PROCESS_TIMER_RESURRECTION - death/resurrection countdown
- CONTINUE_ORNATE_ANIMATOR - special animation sequences
- CONTINUE_TICK_GENERATOR - game event ticker

---

## 3. Turn System

DM2 maintains the **tile-based turn system** from DM1 (1 action = 1 turn), but with these differences:

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Turn pacing | ~1 real-second/tile | Same (indoor); outdoor TBD |
| Time of day | None | 1440-minute cycle advances with turns |
| Weather | None | Weather zones update each tick |
| Companion AI | N/A | companion_tick() runs each loop |
| Party gold | Tracked separately | In GameState struct |
| Reputation | None | Tracked in GameState |

Outdoor turn flow differs from indoor - the outdoor renderer updates differently (sky, ground, weather overlay) while indoor uses the first-person raycast view.

---

## 4. Champion/Companion System (NEW in DM2)

The most significant gameplay addition is the **companion system** - up to 4 NPC companions who fight alongside the party.

From include/dm2_v1_game.h:
```c
typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            // 0=indoor dungeon, 1=outdoor
    int gold;
    int reputation;
    int time_of_day;        // minutes from midnight (0-1439)
} DM2_V1_GameState;
```

Companion data from dm2_champ_types.md and dm2_champ_classes.md:
- Champions have class types (Warrior, Wizard, etc.) with stat bonuses
- Loyalty stat (0-100) affects companion behavior
- Companion mode: 0=follow party, 1=guard position, 2=aggressive
- Companion tick runs each game loop iteration (companion_tick())
- Companion AI controlled by c_ai (creature AI system)
- Champions can die and be resurrected (RESURRECTOR actuator, 0x7E)

DM1 had **no NPC system at all** - the party was always 1-4 human-controlled champions with no AI companions.

---

## 5. Combat System — Expanded from DM1

### Weapon Types

DM1 had: Melee weapons, Thrown weapons, Crossbows.

DM2 adds:
- **Gun** - tech weapon, requires tech_level on the champion
- **Bomb** - AoE explosive, all creatures in blast radius take damage

```c
int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return type == DM2_WEAPON_CROSSBOW || type == DM2_WEAPON_GUN ||
           type == DM2_WEAPON_THROWN || type == DM2_WEAPON_BOMB;
}
```

### Damage Formula

DM2's damage formula (from SKULL.ASM combat resolver):
```
damage = weapon->base_damage + attacker_strength / 4;
if (distance > weapon->range) return 0;
range_penalty = (distance - 1) * damage / 10;  // -10% per extra tile
damage -= target_defense;
return damage > 0 ? damage : 0;
```

DM1 used a simpler fixed-base-damage formula with no range penalty and no strength bonus.

### Creature Attacks

DM1 creatures used only melee and ranged projectile attacks. DM2 adds comprehensive spell-based attacks:

From dm2_combat_creatures.md - AI_ATTACK_FLAGS in SkWinCore.cpp:
- FIREBALL, DISPELL, LIGHTNING, POISON_CLOUD, POISON_BOLT
- POISON_BLOB, PUSH_SPELL, PULL_SPELL, SHOOT, STEAL

---

## 6. Magic System — Expanded

DM2 increases MAXSPELL from 34 (DM1) to 255 via GDAT custom spell definitions.

Spell types (from dm2_combat_magic.md):
- SPELL_TYPE_POTION (1) - consumed from inventory
- SPELL_TYPE_MISSILE (2) - fires magical projectile at target
- SPELL_TYPE_GENERAL (3) - enchantment buffs
- SPELL_TYPE_SUMMON (4) - summons creatures

GDAT_CATEGORY_SPELL_DEF (0x0F) and GDAT_CATEGORY_CREATURE_AI (0x11) are new DM2 categories for this system.

---

## 7. World Structure — Indoor + Outdoor

DM1 was purely indoor dungeon - a grid of first-person dungeon levels with no outdoor space.

DM2 adds:
- **Outdoor areas** - sky, ground, trees, buildings; uses different renderer (no first-person view)
- **Buildings** - enterable structures in outdoor areas (transitions to indoor mode)
- **Outdoor movement** - different grid from dungeon; 8-direction possibly vs 4-direction in dungeons
- **Weather system** - rain, fog, storm affect outdoor areas; tracked per weather zone
- **Day/night cycle** - time_of_day (0-1439 minutes) drives sky color and possibly creature spawns

Level type enum:
```c
typedef enum {
    DM2_LEVEL_OUTDOOR = 0,
    DM2_LEVEL_INDOOR,
    DM2_LEVEL_BUILDING,
} DM2_LevelType;
```

---

## 8. AI System — Expanded

DM2 expands creature AI from DM1's 42 slots to **64 AI slots**:
- CREATURE_AI_TAB_SIZE 64 (DM2 extended) vs CREATURE_AI_TAB_SIZE 42 (original/DM1/CSB)
- MAXAI increases from 62 to 255 in extended mode
- AI type 34: DRAGOTH ATTACK MINION (summoned projectile creature) - special case in Skullkeep

AI per-type data stored in GDAT_CATEGORY_CREATURE_AI (0x11), loaded via EXTENDED_LOAD_AI_DEFINITION.

---

## 9. Asset Scale

| File | DM1 PC 3.4 | DM2 DOS EN |
|------|-----------|-----------|
| GRAPHICS.DAT | 363 KB | ~8.6 MB |
| DUNGEON.DAT | 33,357 B | 39,437 B |

The 24x larger GRAPHICS.DAT reflects: more creature sprites, outdoor tilesets, spell effects, UI graphics, animated sequences.

---

## 10. Summary of Mechanical Differences

| Feature | DM1 | DM2 |
|---------|-----|-----|
| Engine | Original DM1 | New Skullkeep |
| Turn system | Tile-based | Tile-based (same principle) |
| Time tracking | None | 1440-min day/night cycle |
| Weather | None | Rain, fog, storm |
| NPCs | None | 4 companions with AI |
| Outdoor areas | No | Yes |
| Weapon tiers | Melee/thrown/crossbow | Melee/thrown/crossbow/gun/bomb |
| Magic spells | 16 | 34 original, 255 via GDAT |
| Creature AI slots | 42 | 64 |
| GDAT categories | 0x1D (29) | 0xF0 (240) |
| Trigger system | Hardwired tile types | Generic actuator system |
| Save format | DM1 struct | DM2 struct (gold, rep, outdoor) |
| Starting gold | 0g | 100g |

---

## STATUS: AUDIT COMPLETE