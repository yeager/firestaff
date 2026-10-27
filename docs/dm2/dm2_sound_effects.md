# DM2 V1 — Sound Effects

## Overview

DM2 has a rich sound effect system based on a GDAT-backed lookup table.
Unlike DM1's simpler fixed sound set, DM2 supports up to 64 sound entries
per category, with creature-specific, champion-specific, and world interaction
sounds. Sounds are queued with 3D world coordinates for distance-based attenuation.

## Sound Category System

### SOUND_CHAMPION_* (Category 0x0F)
Champion-specific sounds triggered by player actions:

0x00 SOUND_CHAMPION_ATTACK   — Melee swing (c_hero.cpp:3336)
0x01 SOUND_CHAMPION_SHOOT    — Ranged shot (crossbow, gun, throw)
0x82 SOUND_CHAMPION_GETHIT   — Takes damage (c_hero.cpp)
0x83 SOUND_CHAMPION_EAT_DRINK — Consuming food/water
0x87 SOUND_CHAMPION_SCREAM   — Death scream
0x8A SOUND_CHAMPION_BUMP     — Collision/bump (c_hero.cpp:3336, param 0x83)
0x92 SOUND_CHAMPION_FOOTSTEP — SPX custom (not in retail DM2)

### SOUND_CREATURE_* (Category 0x16)
Creature sounds from c_creature.cpp:

0x00 SOUND_CREATURE_MOVE         — Movement (c_creature.cpp:307)
0x01 SOUND_CREATURE_TURN          — Turning (Minion)
0x02 SOUND_CREATURE_GET_HIT       — Hit reaction (Rocky, Guard-Archer shoot)
0x03 SOUND_CREATURE_REFLECTOR     — Reflecting attack (Dragoth)
0x04 SOUND_CREATURE_JUMP          — Jump (Rocky)
0x05 SOUND_CREATURE_SWING_STEP    — Rocky put-down / land after jump
0x06 SOUND_CREATURE_XXX           — Rocky, Dragoth (hard hit), Wolf
0x07 SOUND_CREATURE_ATTACK_1      — Melee attack (c_creature.cpp:877)
0x08 SOUND_CREATURE_PICK_STEAL    — Pick/steal (Thief), Vexirk transform, merchant put
0x09 SOUND_CREATURE_GET_HIT_2    — Dragoth hit
0x0A SOUND_CREATURE_ACTIVATE_TRIGGER — Giggler, Dragoth (hit?)
0x0B SOUND_CREATURE_CONSIDER      — Merchant
0x0C SOUND_CREATURE_ACCEPT        — Merchant
0x0D SOUND_CREATURE_REFUSE        — Merchant
0x0E SOUND_CREATURE_YELL           — Thorn Demon, Dragoth spawn minion
0x0F SOUND_CREATURE_GROWL         — Vegmouth
0x10 SOUND_CREATURE_SPAWN         — Rocky, CaveIn, Minion, Mummy (c_creature.cpp)
0x11 SOUND_CREATURE_DEATH         — Creature death
0x12 SOUND_CREATURE_ATTACK_2      — Secondary attack (Thorn Demon)

### SOUND_STD_* (Category 0x0E)
Standard world interaction sounds:

0x81 SOUND_STD_EXPLOSION     — GDAT2 V5 explosion
0x84 SOUND_STD_DEFAULT       — Punch, fall, test wall, gethit (c_engage.cpp:287)
0x85 SOUND_STD_KNOCK         — Falling item, punch knock
0x86 SOUND_STD_THROW          — Throw/shoot item (c_item.cpp:1100)
0x88 SOUND_STD_ACTIVATION     — GDAT2 V5 activation (c_engage.cpp:755)
0x89 SOUND_STD_TELEPORT       — GDAT2 V5 teleport
0x00 SOUND_STD_ACTIVATION_MESSAGE — Message tick (PC9821 only)
0x01 SOUND_STD_SPELL_MESSAGE  — Spell message (PC9821 only)
0x02 SOUND_STD_TELEPORT_MESSAGE — Teleporter message (PC9821 only)

### SOUND_DOOR_* (Category 0x0D)
Door interaction sounds:

0x8E SOUND_DOOR_STEP   — Step on door (footstep on door square)
0x8F SOUND_DOOR_CLOSE  — Door closing
0x90 SOUND_DOOR_OPENED — SPX custom (not in retail DM2)

### SOUND_MINION_* (Category 0x0C)
0x8B SOUND_MINION_TRANSFORMS — Minion transformation

### SOUND_ITEM_* (Category variable)
0x60 SOUND_ITEM_TAKE      — SPX custom
0x61 SOUND_ITEM_PUT_DOWN   — SPX custom

### Special
0xA0 SOUND_ACTIVATION_LOOP — SPX custom for animated ornates with default loop sound

## Sound Queue System

### QUEUE_NOISE_GEN1
Parameters: (category, index, sound_id, volume, x, y, flags)
Used for champion actions, attacks, and world interactions.
Source: c_engage.cpp:287 (attack), c_hero.cpp:3336 (champion sounds),
        c_item.cpp:1100 (throw/shoot item)

### QUEUE_NOISE_GEN2
Parameters: (category, index, sound_id, extra, volume, x, y, flags)
Extended version with extra byte parameter. Used for creature sounds
and more complex interactions.
Source: c_creature.cpp:877 (creature attack), c_hero.cpp (movement)

## Creature Sound Assignment

Creature sounds are assigned at creature spawn/type initialization and
triggered based on creature state machine transitions. The GDAT lookup
resolves (category, index, sound_id) tuples to actual audio sample indices.

### Common Triggers
- Movement: SOUND_CREATURE_MOVE (0x00)
- Attack: SOUND_CREATURE_ATTACK_1 (0x07) or ATTACK_2 (0x12)
- Hit reaction: SOUND_CREATURE_GET_HIT (0x02)
- Death: SOUND_CREATURE_DEATH (0x11)
- Spawn: SOUND_CREATURE_SPAWN (0x10)

## Champion Sound Triggers

Champion sounds fire from the action processing pipeline:
- Attack action starts: SOUND_CHAMPION_ATTACK
- Ranged attack: SOUND_CHAMPION_SHOOT
- Taking damage: SOUND_CHAMPION_GETHIT
- Death: SOUND_CHAMPION_SCREAM
- Eating/drinking: SOUND_CHAMPION_EAT_DRINK
- Bump into wall: SOUND_CHAMPION_BUMP

## GDAT Sound Entry System

### DM2_QUERY_SND_ENTRY_INDEX
Signature: DM2_QUERY_SND_ENTRY_INDEX(i8 cat, i8 idx, i8 sfx_id) -> i16
Resolves a (category, index, sound_id) triple to a sound entry index.
Uses the global sound list populated from GDAT2 V5 dungeon files.

### glbSoundList[64]
64-entry sound entry lookup table. Populated from the GDAT2 V5 section
of the dungeon data file at load time.

### dm2sound.v1dfda4[64]
64-entry sound index table. Used at runtime to map sound requests
to the appropriate sample.

## Comparison with DM1 CSB

| Aspect | DM1 CSB | DM2 |
|--------|---------|-----|
| Sound categories | 4–5 basic | 10+ categories |
| Per-creature sounds | No | Yes (creature type ID) |
| Sound ID count | ~20 fixed | 64 per category |
| GDAT lookup | No | Yes (GDAT2 V5) |
| 3D positioning | No | Distance-based attenuation |
| Champion sounds | Attack only | Full HP/Stamina/Mana reactions |
| Creature deaths | Generic | Per-creature-type death sound |
| Queue system | Immediate play | Deferred world-position queue |
