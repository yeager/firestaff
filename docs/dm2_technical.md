# DM2 V1 Technical Improvements — Source-Locked

## Sources

- SKULL.ASM (522,128 lines IDA disassembly, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject (github.com/gbsphenx/skproject) HEAD a962896
- SKWIN/SkGlobal.h, SkWinCore.cpp
- include/dm2_v1_*.h (Firestaff DM2 V1 headers)
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt

---

## Overview

DM2 is a complete ground-up sequel with a new engine, not derived from DM1.
The SKULL.EXE (DOS executable) implements a fundamentally different architecture
while maintaining the first-person dungeon crawler genre identity.

Source: include/dm2_v1_game.h: "DM2 has a different engine with outdoor areas, shops, NPCs."

---

## 1. New Engine Architecture

DM2 uses a completely new engine (Skullkeep/SkWin) distinct from DM1's FTL engine.

From skproject/SKWIN/README.md:
- SKWIN: Old SKWin code ported from PC-9821 (Japan)
- SKWINDOS: Old Skull code ported from PC-DOS
- SKULLWIN: Old Skull code ported from PC-DOS

Engine variants (v0/v4/v5) reflect different platform ports:
- v0: Minimal test application base
- v4: PC-9821 (Japan) version
- v5: PC-DOS version

Source: skproject/SKWIN/README.md

---

## 2. Asset File Sizes (Major Expansion)

| File | DM1 PC 3.4 | DM2 DOS EN | Ratio |
|------|-----------|-----------|-------|
| GRAPHICS.DAT | 363,417 bytes | ~8.6 MB | ~24x |
| DUNGEON.DAT | 33,357 bytes | 39,437 bytes | ~1.18x |

The ~24x GRAPHICS.DAT increase reflects:
- Outdoor environment art (sky, ground, trees, buildings)
- Extended creature artwork
- New UI elements (champion sheets, shops, maps)
- Animation frames for items and creatures

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## 3. Extended Mode: 64-bit Addressing and Large Tables

DM2 enables extended mode with larger data structures:

Source: skproject/SkGlobal.h:
- GDAT_CATEGORY_LIMIT: 0xF0 (240 categories, DM2) vs 0x1D (29, original)
- CREATURE_AI_TAB_SIZE: 64 (DM2) vs 42 (original)
- MAXAI: 255 (DM2) vs 62 (original)
- MAXSPELL: MAXSPELL_CUSTOM=255 vs MAXSPELL_ORIGINAL=34

EXTENDED_LOAD_AI_DEFINITION and EXTENDED_LOAD_SPELLS_DEFINITION load
custom tables from GDAT when in DM2 extended mode.

Source: skproject/SkWinCore.cpp

---

## 4. Outdoor Rendering System

DM2's outdoor renderer is entirely new, with no equivalent in DM1:

include/dm2_v1_outdoor_renderer.h:
- Sky texture field
- Ground texture field
- Tree density
- Building count
- Weather system (0=clear, 1=rain, 2=fog, 3=storm)
- Time-of-day cycle (0.0-1.0 float) affecting sky color

Outdoor combat uses different movement rules than indoor dungeon combat.

Source: include/dm2_v1_outdoor_renderer.h, include/dm2_v1_combat.h

---

## 5. Ranged Combat System

DM2 adds ranged combat with three new weapon categories:

DM2_WEAPON_CROSSBOW - pre-tech ranged
DM2_WEAPON_GUN - tech weapon (battery powered)
DM2_WEAPON_BOMB - thrown explosive

Each weapon type has:
- Base damage
- Range (tiles, 1 = melee)
- Ammo required
- Tech level (0 = magic era, 1+ = tech)

Source: include/dm2_v1_combat.h

---

## 6. Tech/Magic Hybrid System

DM2 uniquely combines technology and magic in items:

DM2_ITEM_MAGIC (0) - traditional magic items
DM2_ITEM_TECH (1) - technology items (guns, devices)
DM2_ITEM_HYBRID (2) - items combining both (magic-powered devices)

Item power sources:
- 0 = manual
- 1 = battery
- 2 = mana
- 3 = hybrid

Source: include/dm2_v1_tech_magic.h

---

## 7. Companion / NPC System

DM2 introduces companion NPCs that fight alongside the party:

include/dm2_v1_companion.h - companion AI and behavior
include/dm2_v1_game.h: "companion NPCs, outdoor combat with different movement rules"

Champion GDAT (0x16) provides character data and sounds.

Source: include/dm2_v1_companion.h, include/dm2_v1_combat.h

---

## 8. Shop and Economy System

DM2 adds:
- Gold currency (vs DM1's treasure-as-progress model)
- Reputation tracking (party standing with NPCs)
- Buy/sell transactions
- Shop interface

Source: include/dm2_v1_game.h

---

## 9. Day/Night Cycle and Weather

Two major atmospheric systems:

**Time-of-day:**
- Float value 0.0-1.0
- Affects sky color in outdoor renderer
- Controls ambient lighting

**Weather:**
- 0 = clear
- 1 = rain
- 2 = fog
- 3 = storm

Weather zones are part of the dungeon level data (dm2_v1_dungeon_loader.h).

Source: include/dm2_v1_outdoor_renderer.h, include/dm2_v1_dungeon_loader.h

---

## 10. Enhanced Graphics Pipeline

DM2's graphics system extends DM1's with:

**Extended GDAT structure:**
- 240 category slots vs 29
- Per-category entry data (word values, text, images)
- Custom spell definitions in GDAT (EXTENDED_LOAD_SPELLS_DEFINITION)
- Custom creature AI in GDAT (EXTENDED_LOAD_AI_DEFINITION)

**New ornate types:**
- Animated wall/floor ornates (up to 10 frames)
- Ladder detection via ornate property
- Teleporter chip rendering

**Color key door effects:**
- Two color keys per door (04 00 00 cyan, 0C 00 00 dark green)
- Enable see-through effects and "what's behind" rendering

Source: SKWin.GDAT2.InternalCodes.txt, SkWinCore.cpp

---

## 11. Sound System Extensions

DM2 extends the sound event system:

**New sound event categories:**
- Teleport sound (89 00 00 in Teleporter GDAT)
- Merchant sounds (accept, refuse, think)
- Potion/drink sounds
- Weather-related ambient sounds

**Sound coordinate system:**
QUEUE_NOISE_GEN2 with GDAT2 category, primary index, entry, default index, x, y, tick, 96, 80

Source: SKWin.GDAT2.InternalCodes.txt, SkWinCore.cpp

---

## 12. Save/Load Format

DM2 uses a DM2-specific save/load format:

include/dm2_v1_save_load.h - DM2-specific savegame handling

Different from DM1's save format due to:
- Extended creature/AI tables
- Companion/NPC state
- Outdoor area state
- Weather/time-of-day state
- Reputation tracking

Source: include/dm2_v1_save_load.h

---

## 13. Combat AI Improvements

DM2 creature AI supports more attack types:

| Attack Flag | Effect |
|------------|--------|
| MELEE | Standard melee |
| SHOOT | Ranged shot |
| FIREBALL | Casts fireball (Skullkeep special: AI 51 gets this) |
| LIGHTNING | Lightning bolt |
| POISON_CLOUD | Poison cloud |
| POISON_BOLT | Poison projectile |
| POISON_BLOB | Poison blob |
| DISPELL | Removes champion enchantments |
| PUSH_SPELL | Push-back spell |
| PULL_SPELL | Pull spell |
| STEAL | Steals from party |

Source: skproject/SkWinCore.cpp AI_ATTACK_FLAGS

---

## 14. Viewport and Rendering

DM2 supports two distinct renderers:

1. **Indoor first-person** - Similar to DM1's raycasting approach
   - Wall/floor/ceiling rendering
   - Creature display
   - Door/window effects

2. **Outdoor landscape** - New top-down/external view
   - Sky gradient rendering
   - Ground plane with texture
   - Building exteriors
   - Tree/foliage rendering

DM2 can transition between outdoor and indoor views seamlessly.

Source: include/dm2_v1_outdoor_renderer.h, include/dm2_v1_game.h

---

## 15. Extended Animations

DM2 item animation system (SKWin.GDAT2.InternalCodes.txt):
- 06 00 00: Animation field (e.g. 0x0504 = 4 animated frames)
- Range from 18 08 00 to 1B 08 00 for 4-frame animations

Creature animations:
- Spawn sound (0E 00 00)
- Death sound (11 00 00)
- Multiple attack variants (12 00 00: second attack)

Source: SKWin.GDAT2.InternalCodes.txt

---

## 16. AI Table Patch System

DM2 uses runtime patches for dungeon-specific AI behavior:

Example from SkWinCore.cpp:
```
if (dungeon == SKULLKEEP)
    dAITable[51].AttacksSpells |= AI_ATTACK_FLAGS__FIREBALL;
```

This allows customizing creature abilities per dungeon without
rebuilding the GDAT.

Source: skproject/SkWinCore.cpp (EXTENDED_LOAD_AI_DEFINITION)

---

## STATUS: SOURCE-LOCKED