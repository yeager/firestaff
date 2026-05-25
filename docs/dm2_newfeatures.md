# DM2 V1 New Features vs DM1 — Source-Locked

## Sources

- SKULL.ASM (522,128 lines IDA disassembly, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject (github.com/gbsphenx/skproject) HEAD a962896
- SKWIN/SkGlobal.h: CREATURE_AI_TAB_SIZE, MAXSPELL, GDAT_CATEGORY_LIMIT defines
- SKWIN/SkWinCore.cpp: EXTENDED_LOAD_SPELLS_DEFINITION, EXTENDED_LOAD_AI_DEFINITION
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt

---

## 1. Extended Creature AI Table (42 -> 64)

DM2 increases the creature AI table from 42 (DM1) to 64 entries.

Source:
- skproject/SkGlobal.h: CREATURE_AI_TAB_SIZE 64 (DM2 extended)
- skproject/SkGlobal.h: CREATURE_AI_TAB_SIZE 42 (original/DM1/CSB)

DM2 also increases MAXAI from 62 to 255 in extended mode.

## 2. Extended GDAT Categories (0xF0 vs 0x1D)

DM2 extends GDAT category system from 0x1D (29 categories) to 0xF0 (240 categories).

New DM2-specific GDAT categories:
- GDAT_CATEGORY_SPELL_DEF - spell definitions
- GDAT_CATEGORY_CREATURE_AI - AI behavior per creature type
- GDAT_CATEGORY_CREATURES - creature type data
- GDAT_CATEGORY_WEAPONS - extended weapon data with projectile flags
- GDAT_CATEGORY_DOORS (0x0E) - door properties
- GDAT_CATEGORY_TELEPORTERS (0x18) - teleporter square type
- GDAT_CATEGORY_CHAMPIONS (0x16) - champion character data

## 3. New Spell System (34 original -> 255 custom)

DM2 increases MAXSPELL from 34 to 255 via GDAT custom spell definitions.

Spell types (getSpellTypeName):
- SPELL_TYPE_POTION (1)
- SPELL_TYPE_MISSILE (2)
- SPELL_TYPE_GENERAL (3) - enchantment
- SPELL_TYPE_SUMMON (4)

## 4. Ranged Combat (New Weapon Types)

DM2 adds: crossbow, gun (tech weapon), bomb (thrown explosive).

Weapon GDAT fields:
- 0x09/0x0D: Missile strength
- 0x05: GDAT_ITEM_WEAPON_PROJECTILE_FLAG
- 85 00 00 (SND): Knock (hit obstacle sound)

## 5. New Creature Attack Abilities

AI_ATTACK_FLAGS in SkWinCore.cpp:
- FIREBALL, DISPELL, LIGHTNING, POISON_CLOUD, POISON_BOLT
- POISON_BLOB, PUSH_SPELL, PULL_SPELL, SHOOT, STEAL

## 6. New Door Types

- Second clan door type (0x0A) used for Skullkeep Dragon door
- Animated mirrored door flag (field 20 00 00, DM2 new feature)
- Color key fields for see-through door effects

## 7. Companion / Champion System

Champion GDAT (0x16) sounds: attack, shoot, get hit, eat/drink, death, bump wall.

## 8. Teleporter Square Type (0x18)

Dedicated teleporter GDAT category with teleport sound (89 00 00).

## 9. Extended Item System

New item fields: animation (06 00 00), mana/luck/speed (14-15,33), weight per charge (34).
Potion custom fields: behavior (05), water value (43), spell missile association (4D).

## 10. Larger Asset Files

- GRAPHICS.DAT: DM1 363 KB vs DM2 ~8.6 MB
- DUNGEON.DAT: DM1 33 KB vs DM2 39 KB

## 11. Weather and Lighting

Ambient darkness control (0 = full light, 8 = dark).
Outdoor renderer: weather (clear/rain/fog/storm), time-of-day cycle.

## 12. Fireball Creature Special Case

DM2 patches creature AI index 51 with fireball attack in Skullkeep dungeon.

## STATUS: SOURCE-LOCKED