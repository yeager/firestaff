# DM2 V1 Champion Changes vs DM1 — Source Lock

**Audit date:** 2026-05-25
**Sources:**
- SKULL.ASM (522,128 lines, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject/SKWIN/SkWinCore.cpp (EXTENDED_LOAD_AI_DEFINITION, getAIName)
- skproject/SkGlobal.h: CREATURE_AI_TAB_SIZE, GDAT_CATEGORY_LIMIT, MAXAI, MAXSPELL
- include/dm2_v1_*.h (Firestaff headers)
- docs/dm2-v1-overview/dm2_newfeatures.md, dm2_overview.md

---

## Finding: DM2 V1 Champions Are NOT DM1-Style Champions

The single most important finding: DM2 V1 does NOT have a champion character progression
system in the DM1 sense. There is no skill/XP/level-up system for party members.
Champions in DM2 are simpler characters without advancement. Instead, DM2 introduces
Companion NPCs (AI-controlled allies) as the primary non-party-member characters.

Source: include/dm2_v1_game.h — "DM2 has a different engine with outdoor areas, shops, NPCs."
Source: include/dm2_v1_companion.h — "DM2 features NPC companions that fight alongside the party."

---

## 1. No Champion Skill/XP System in DM2

DM1 champions have:
- 4 base classes (Fighter/Ninja/Priest/Wizard)
- 20 hidden skills (C04-C19) with experience accumulation
- Level-up formula: while XP >= 500, XP >>= 1, level++
- Stat bonuses on level-up per class
- Temporary XP decay

DM2 V1: None of this exists. The SKULL.ASM disassembly and skproject C++ code show no
champion skill advancement routines. The DM2 game architecture treats characters as
simpler entities.

Evidence:
- SKWIN/SkGlobal.h has no champion skill definitions
- SKWIN/SkGlobal.cpp skill table (lines 581-797) defines skill command metadata ("SK", "LV")
  for the 4 visible base skills only (Fighter/Ninja/Priest/Wizard), not 20 hidden skills
- No XP accumulation code for champions found in SKULL.ASM or skproject sources

---

## 2. New Companion System (DM2-Specific)

DM2 introduces companion NPCs (AI indices 13-18) — fundamentally different from champions:

| AI Index | Name | Role |
|----------|------|------|
| 0x0D (13) | SCOUT MINION | Ally - reconnaissance |
| 0x0E (14) | ATTACK MINION | Ally - combat |
| 0x0F (15) | CARRY MINION | Ally - carry items |
| 0x10 (16) | FETCH MINION | Ally - fetch objects |
| 0x11 (17) | GUARD MINION | Ally - defend position |
| 0x12 (18) | U-HAUL MINION | Ally - transport |

Companion struct (include/dm2_v1_companion.h):
- name[16]
- health, max_health
- attack, defense
- loyalty (0-100)
- ai_behavior (0=follow, 1=guard, 2=aggressive)
- alive

Companions are summoned via spells (indices 29-31):
- ZO EW KU -> Attack Minion (AI 0x31)
- ZO EW NETA -> Guard Minion (AI 0x34)
- ZO EW ROS -> U-Haul Minion (AI 0x35)

Source: docs/spells_dm2/dm2_newspells.md (SkWinCore.cpp:17524-17602)

Companions have NO advancement — they are static entities with loyalty tracking.

---

## 3. New Champion GDAT Category (0x16)

DM2 adds GDAT_CATEGORY_CHAMPIONS (0x16) — champion character data in the GDAT system.

Champion GDAT sounds (dm2_newfeatures.md):
- Attack sound
- Shoot sound
- Get hit sound
- Eat/drink sound
- Death sound
- Bump wall sound

This is for character metadata and sound bindings, NOT a character progression system.
The fact that champions have "shoot" sounds suggests ranged weapon usage by champions
in DM2 (consistent with crossbow/gun weapon types in dm2_v1_combat.h).

---

## 4. New Weapon Types for Champions

DM2 adds ranged weapons that champions can use (dm2_v1_combat.h):
- DM2_WEAPON_CROSSBOW — pre-tech ranged
- DM2_WEAPON_GUN — tech weapon (battery powered)
- DM2_WEAPON_BOMB — thrown explosive

Each weapon type:
- Base damage
- Range (tiles, 1 = melee)
- Ammo required
- Tech level (0 = magic era, 1+ = tech)

Source: include/dm2_v1_combat.h, dm2_newfeatures.md

This is a significant "champion ability" change in DM2 — champions gain access to
ranged combat, which DM1 did not have.

---

## 5. Changed Combat System

DM2 creature attacks include spell abilities (AI_ATTACK_FLAGS) that can affect champions:
- FIREBALL, LIGHTNING, POISON_CLOUD, POISON_BOLT, POISON_BLOB
- DISPELL (removes champion enchantments)
- PUSH_SPELL, PULL_SPELL (creature repositioning)
- STEAL (creature steals from champions)

Source: skproject/SKWIN/defines.h:705-716, SkWinCore.cpp:415-437, 27038-27096

---

## Summary

| Feature | DM1 | DM2 V1 |
|---------|-----|--------|
| Champion advancement (skill/XP/levels) | Yes - 20 skills, 4 classes | NO |
| Companion NPCs | No | Yes - AI indices 13-18, summoned via spells |
| Champion skill system | Yes (C00-C19) | Not present |
| Class system (Fighter/Ninja/Priest/Wizard) | Yes | Not confirmed in DM2 |
| Ranged weapons for champions | No (melee/throw only) | Yes (crossbow, gun, bomb) |
| Champion GDAT 0x16 | No | Yes (character data + sounds) |
| Dispel effect on champions | No | Yes (AI_ATTACK_FLAGS DISPELL) |
| Push/Pull spell effects | No | Yes (AI_ATTACK_FLAGS PUSH/PULL_SPELL) |
| New champion death mechanics | Mirror resurrect/reincarnate | Unknown |

---

## Gap: DM2 Champion Death/Reincarnation

No source-locked documentation found for DM2 champion death mechanics (resurrect/reincarnate).
The DM1 REVIVE.C system has no DM2 equivalent identified in SKULL.ASM or skproject sources.

---

## STATUS: PARTIALLY SOURCE-LOCKED

Primary gap: DM2 champion internal structure (stats, health, skills, death handling)
is not yet source-locked from SKULL.ASM disassembly. The skproject C++ code does not
expose champion data structures with the same granularity as ReDMCSB CHAMPION.C.