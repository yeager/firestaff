# DM2 V1 Champion Classes — Source Lock

**Audit date:** 2026-05-25
**Sources:**
- skproject/SKWIN/SkGlobal.cpp:581-797 (skill command metadata)
- skproject/SkGlobal.h
- include/dm2_v1_*.h
- docs/dm2-v1-overview/dm2_newfeatures.md
- SKULL.ASM (no class definition found)

---

## Finding: DM2 V1 Has No Confirmed Champion Class System

### DM1 Class System (for reference)
DM1 has 4 base classes (Fighter C00, Ninja C01, Priest C02, Wizard C03) each with 4
hidden skills (20 total, C04-C19). Class is implicit from champion definition text.
Class determines stat bonus progression on level-up.

Source: ReDMCSB DEFS.H:757-768, CHAMPION.C:199-270

### DM2: Class System Not Confirmed
No equivalent class definition found in:
- SKULL.ASM disassembly (no "CHAMPION_CLASS" or similar structures)
- skproject C++ sources (no class enum for champions)
- Firestaff DM2 V1 headers (no champion class definitions)

---

## 1. Visible Skill System in DM2

The SKWIN/SkGlobal.cpp skill command metadata table (lines 581-797) defines:

- "SK" - Skill number used. Skills from 0-3 are normal visible skills
  (Fighter, Ninja, Priest, Wizard)
- "LV" - minimal skill level for the command to appear

The comment indicates DM2 still has the 4 base skill categories (Fighter/Ninja/Priest/Wizard).
However this is for the command/skill check system, not necessarily the full DM1 20-skill
hidden system.

Unknown: Whether DM2 has 4 visible skills only, or extends to 20 (or some other number).

---

## 2. Skill Level Names

The skill level name table (SKWIN/SkGlobal.cpp:786-797) defines rank names:
- Index values suggest up to 11+ named ranks (EXPERT at 0x0b = 11)
- Compare DM1: 16 named ranks (NEOPHYTE through ARCHMASTER+)
- Compare CSB: adds NEOPHYTE as rank 0

DM2 skill rank count is not confirmed — the table is partially visible.

---

## 3. No Champion Class Selection in DM2

DM1: Class is determined by champion definition text (name string parsing in REVIVE.C).
DM2: No champion creation/class-selection screen documented for DM2.

DM2 champion data (GDAT 0x16) appears to be simpler — character metadata with sounds,
not the complex multi-tier skill+class system of DM1.

---

## 4. Companion NPCs Have No Classes

Companions (AI indices 13-18) are AI-controlled allies with:
- Fixed attack/defense stats (set at spawn)
- Loyalty (0-100, affects behavior)
- ai_behavior enum: 0=follow, 1=guard, 2=aggressive

They have no class system, no skill system, no advancement.

Source: include/dm2_v1_companion.h

---

## Summary

| Aspect | DM1 | DM2 V1 |
|--------|-----|--------|
| Base classes (Fighter/Ninja/Priest/Wizard) | Yes, C00-C03 | Unconfirmed |
| Hidden skills (20 total) | Yes, C04-C19 | Not confirmed |
| Class-based stat bonuses | Yes | Not confirmed |
| Class implied from champion text | Yes (REVIVE.C parsing) | Not confirmed |
| Skill level ranks | 16 (NEOPHYTE..ARCHMASTER+) | ~11+ (EXPERT at 0x0b) |
| Companion classes | N/A | No |

---

## Status: NOT FULLY SOURCE-LOCKED

DM2 champion class structure requires deeper SKULL.ASM analysis for definitive answers.
The skproject C++ code does not expose champion class/skill internals.