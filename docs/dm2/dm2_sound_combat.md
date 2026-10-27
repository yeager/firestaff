# DM2 V1 — Combat Audio

## Overview

DM2's combat audio system covers all sound triggers related to combat:
champion attacks, creature attacks, hit reactions, and death sounds.
Sounds fire from the combat resolver in c_engage.cpp and creature AI
in c_creature.cpp, using the QUEUE_NOISE system for world-positioned playback.

## Champion Combat Sounds

### Attack Sounds (c_hero.cpp, c_engage.cpp)

SOUND_CHAMPION_ATTACK (0x00):
- Triggered when a champion initiates a melee attack
- Queue: DM2_QUEUE_NOISE_GEN2(category=0x0F, champion_type, 0x00, volume, x, y, 0)
- c_hero.cpp:3336 — champion attack initiated
- Located: c_engage.cpp:287 — melee weapon swing sound

SOUND_CHAMPION_SHOOT (0x01):
- Triggered for ranged attacks: crossbow, gun, thrown weapons
- c_item.cpp:1100 — item thrown/shoot action
- c_hero.cpp:3970 — ranged attack initiated

### Hit Reaction Sounds

SOUND_CHAMPION_GETHIT (0x82):
- Champion takes damage from creature or world hazard
- Queue: DM2_QUEUE_NOISE_GEN2(category=0x0F, hero_index, 0x82, ...)
- Source: combat resolver hit resolution in c_engage.cpp

### Death Sound

SOUND_CHAMPION_SCREAM (0x87):
- Champion death scream
- Triggered when champion HP reaches 0
- c_hero.cpp — death processing path

### Other Champion Combat Sounds

SOUND_CHAMPION_BUMP (0x8A):
- Champion collides with obstacle or creature
- c_hero.cpp:3336 (param 0x83) — bump collision

SOUND_CHAMPION_FOOTSTEP (0x92):
- SPX custom — footstep during combat movement (not in retail DM2)

## Creature Combat Sounds

### Creature Attack Sounds

SOUND_CREATURE_ATTACK_1 (0x07):
- Primary melee attack of a creature
- c_creature.cpp:877 — creature attack initiated
- Queue: DM2_QUEUE_NOISE_GEN2(category=0x16, creature_type, 0x07, ...)

SOUND_CREATURE_ATTACK_2 (0x12):
- Secondary/missile attack for creatures with two attack types
- c_creature.cpp — secondary attack branch

SOUND_CREATURE_PICK_STEAL (0x08):
- Thief creature pick/steal action
- Vexirk transformation (c_creature.cpp)
- Magic Merchant put item

### Creature Hit Reactions

SOUND_CREATURE_GET_HIT (0x02):
- Creature reacts to being hit by champion
- Rocky, Guard-Archer (shoot)
- c_creature.cpp:307

SOUND_CREATURE_GET_HIT_2 (0x09):
- Secondary hit reaction for Dragoth

SOUND_CREATURE_REFLECTOR (0x03):
- Dragoth reflecting an attack back

### Creature Death Sounds

SOUND_CREATURE_DEATH (0x11):
- Creature death wail/growl
- c_creature.cpp — creature death state machine
- Plays when creature v_1e0b7f (HP) reaches 0

### Creature Special Combat Sounds

SOUND_CREATURE_YELL (0x0E):
- Thorn Demon attack yell
- Dragoth spawns minion

SOUND_CREATURE_GROWL (0x0F):
- Vegmouth combat growl

SOUND_CREATURE_ACTIVATE_TRIGGER (0x0A):
- Giggler, Dragoth hit reaction during combat

## Standard Combat Sounds

### SOUND_STD_DEFAULT (0x84)
Punch, fall, test wall, gethit — used for generic combat interactions:
- c_engage.cpp:287 — melee hit lands
- c_engage.cpp:755 — world object hit reaction

### SOUND_STD_KNOCK (0x85)
Falling item impact, punch knockback sound:
- Used when creature/champion knocked back

### SOUND_STD_EXPLOSION (0x81)
GDAT2 V5 — explosion from bombs or environmental hazards:
- Bomb damage (DM2_WEAPON_BOMB)

### SOUND_STD_THROW (0x86)
Item thrown or shot:
- c_item.cpp:1100 — projectile in flight
- c_hero.cpp:3970 — throw/shoot action

## Combat Audio Queue Parameters

### Champion Attack Queue (c_hero.cpp:3336)
DM2_QUEUE_NOISE_GEN2(
  category = 0x0F,           // SOUND_CHAMPION category
  index    = hero_type,     // champion class/type
  sfx_id   = 0x00,          // SOUND_CHAMPION_ATTACK
  extra    = 0xFE,          // volume/distance modifier
  volume   = parw01,        // calculated volume
  x        = parw02,        // world X position
  y        = parw03,        // world Y position
  flags    = 0x3C           // sound flags
)

### Creature Attack Queue (c_creature.cpp:877)
DM2_QUEUE_NOISE_GEN2(
  category = 0x16,          // SOUND_CREATURE category
  index    = creature_type, // creature type ID
  sfx_id   = 0x07,          // SOUND_CREATURE_ATTACK_1
  extra    = RG3Blo,        // distance
  volume   = parw00,        // volume
  x        = parw01,        // world X
  y        = parw02,        // world Y
  flags    = 0x69           // sound flags
)

## Sound Resolution Chain

1. Combat resolver determines hit/damage outcome (c_engage.cpp)
2. Respective sound category/index/sfx_id triple determined
3. DM2_QUERY_SND_ENTRY_INDEX(cat, idx, sfx) resolves to entry index
4. DM2_PLAY_SOUND(entry_index, sfx_struct*) queues the sound
5. DM2_PROCESS_SOUND(wav) plays from queue in update loop

## Combat Sound Variations

### Melee vs Ranged
- Melee: SOUND_CHAMPION_ATTACK / SOUND_CREATURE_ATTACK_1
- Ranged: SOUND_CHAMPION_SHOOT / SOUND_CREATURE_ATTACK_2 (for archers)
- Thrown: SOUND_STD_THROW

### Weapon Types with Sound
- Melee weapon: champion swing sound + creature react
- Crossbow: SOUND_CHAMPION_SHOOT + bolt impact
- Gun: SOUND_CHAMPION_SHOOT + gunpowder impact
- Bomb: SOUND_STD_EXPLOSION + area damage sound

### Multi-Creature Combat
When multiple creatures are engaged, each creature queues its own
attack sound with its individual world position. The 16-slot SKWin
audio buffer handles overlapping sounds.

## Comparison with DM1 CSB

| Aspect | DM1 CSB | DM2 |
|--------|---------|-----|
| Attack sounds | Single generic | Per-weapon-type |
| Creature attacks | One sound for all | Per-creature-type |
| Hit reactions | Fixed | Per-creature-type |
| Death sounds | Generic | Per-creature-type |
| Ranged sounds | Crossbow only | Crossbow/Gun/Throw |
| Bomb sounds | None | Yes (explosion) |
| 3D positioning | None | World-coordinate queue |
