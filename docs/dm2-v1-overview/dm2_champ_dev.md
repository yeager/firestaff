# DM2 V1 Champion Development (Skill/XP/Advancement) — Source Lock

**Audit date:** 2026-05-25
**Sources:**
- SKULL.ASM (no champion XP/skill advancement found)
- skproject/SKWIN/SkGlobal.cpp:581-797 (skill command metadata only)
- include/dm2_v1_*.h
- docs/game_logic_xp.md (DM1 V1 reference)
- docs/dm2-v1-overview/dm2_newfeatures.md

---

## Finding: No Champion Skill/XP/Advancement System in DM2 V1

### DM1 V1 Advancement System (reference)
- F0303_CHAMPION_GetSkillLevel — XP halving loop
- F0304_CHAMPION_AddSkillExperience — XP add + level-up
- XP thresholds: 0, 500, 1000, 2000, ... (each level doubles)
- 16 skill level names (NEOPHYTE through ARCHMASTER+)
- Stat bonuses on level-up per class
- Temporary XP decay (per tick)
- Map difficulty multiplier on XP gains

Source: ReDMCSB CHAMPION.C:736-970, game_logic_xp.md

### DM2 V1: NO SUCH SYSTEM EXISTS

Comprehensive search of:
- SKULL.ASM — no champion XP accumulation, no skill level-up functions
- skproject SKWIN/*.cpp — no champion experience or level-up code
- Firestaff dm2_v1_*.h — no skill/XP/advancement definitions

The DM2 engine architecture treats characters differently. Champions in DM2 are simpler
entities without the multi-tier skill + class + level-up system of DM1.

---

## 1. DM2 Skill Command System (Different Purpose)

The SKWIN/SkGlobal.cpp skill command system handles skill checks for UI commands:

- "SK" - Skill number used (0-3 = Fighter/Ninja/Priest/Wizard)
- "LV" - Minimal skill level required

This is a UI command visibility system — determining when skill commands appear
in the champion action menu based on skill level. It is NOT a character progression system.

Compare DM1: The 20 hidden skills (C04-C19) have actual Experience values that
accumulate and drive level-ups. The visible 4-class skills (C00-C03) are derived.

DM2: The visible 4-class system may exist for command gating, but there is no
evidence of hidden skill tracking or XP accumulation.

---

## 2. Companion NPC Development

Companions (AI indices 13-18) have no development system:

- name[16], health, max_health, attack, defense
- loyalty (0-100), ai_behavior, alive

Stats are fixed at spawn. No loyalty gain/loss system documented.
No level-up, no XP, no skill growth.

dm2_v1_companion_tick() processes companion state but does not advance stats.

Source: include/dm2_v1_companion.h

---

## 3. Minion Summons Are Static

The minion spells (indices 29-31) create creatures with fixed stats:
- Attack Minion (AI 0x31): fixed combat capability
- Guard Minion (AI 0x34): fixed defensive stats
- U-Haul Minion (AI 0x35): fixed carry capacity

No advancement for summoned minions.

Source: docs/spells_dm2/dm2_newspells.md (SkWinCore.cpp:17524-17602)

---

## 4. Gold/Shop Economy Replaces XP Progression

DM2 introduces a gold currency and shop system (dm2_v1_game.h):
- int gold — party currency
- int reputation — NPC standing
- dm2_v1_enter_shop() — buy/sell transactions

DM2 character power progression appears to be:
1. Equipment-based (buy better gear with gold)
2. Item-based (consumables, enchantments)
3. Companion-based (recruit stronger companions)
4. Spell-based (unlock new spell abilities)

Rather than DM1 time-based skill advancement (XP from combat/events).

---

## Summary

| System | DM1 V1 | DM2 V1 |
|--------|--------|--------|
| Champion XP accumulation | Yes (F0303/F0304) | NO |
| Skill level formula (XP halving loop) | Yes (CHAMPION.C:766) | NO |
| Level-up stat bonuses | Yes (per class) | NO |
| Temporary XP decay | Yes (per tick) | NO |
| Map difficulty XP multiplier | Yes (CHAMPION.C:871) | NO |
| Hidden skills (20 total) | Yes (C04-C19) | Not confirmed |
| Skill command visibility gating | Yes (command queue) | Yes (SKWIN/SkGlobal.cpp) |
| Gold/shop economy | No | YES |
| Companion advancement | N/A | NO |
| Minion advancement | N/A | NO |

---

## Status: NOT SOURCE-LOCKED — DM2 V1 Lacks Champion Advancement System

DM2 V1 does not have a champion skill/XP/advancement system equivalent to DM1.
The development model is fundamentally different: equipment/economy-based rather
than time/XP-based.

Gap: DM2 champion stat structures (health, stamina, mana, stats) need deeper
SKULL.ASM analysis. The existing DM2 headers (dm2_v1_*.h) are thin wrappers.