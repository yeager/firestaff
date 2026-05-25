# DM2 V1 Champion Death/Mechanics — Source Lock

**Audit date:** 2026-05-25
**Sources:**
- SKULL.ASM (no champion death/resurrection routines confirmed)
- skproject/SKWIN/SkWinCore.cpp
- include/dm2_v1_*.h
- include/dm2_v1_save_load.h
- docs/dm2-v1-overview/dm2_newfeatures.md (companion system)
- docs/champ_death.md (DM1 V1 reference — mirror resurrect/reincarnate)
- docs/source-lock/csb_champions.md (CSB reincarnation changes)

---

## Finding: No DM2-Specific Champion Death/Resurrection System Documented

### DM1 V1 Death System (reference)
- Champion dies when CurrentHealth <= 0
- Mirror square: RESURRECT (restore, no stat loss) or REINCARNATE (rename, clear skills, -1/8th stats)
- VIP altar: rebirth with health reduction
- No permanent death in standard game

Source: ReDMCSB REVIVE.C:704-950, champ_death.md

### CSB Death System (CHANGE7_24 — significant modification)
- HP/MP/STA halved on reincarnate (vs preserved in DM1)
- Max stats -1/8th (vs preserved in DM1)
- Luck exempted from stat penalty
- New globals: reincarnateAttributePenalty, reincarnateStatPenalty, randomPoints

Source: csb_champions.md (CSBWin Character.cpp:14,682-687)

### DM2 V1: NO SOURCE-LOCKED DEATH SYSTEM FOUND

Comprehensive search of:
- SKULL.ASM — no RESURRECT, REINCARNATE, REVIVE, DEATH routines found
- skproject SKWIN/*.cpp — no champion death/resurrection logic confirmed
- Firestaff DM2 V1 headers — no death/resurrection function declarations

This is a significant documentation gap.

---

## 1. Companion NPC Death

Companions have alive flag (include/dm2_v1_companion.h):
- int alive; // 0 = dead, non-zero = alive

Companion death is tracked but no resurrection mechanism documented.
Companions are summoned entities — when they die, they are simply gone.
No mirror-square mechanic, no altar revival.

---

## 2. DM2 Save Format Gap

DM2 uses a different save/load format (dm2_v1_save_load.h):
- Extended creature/AI tables
- Companion/NPC state
- Outdoor area state
- Weather/time-of-day state
- Reputation tracking

The save format does not preserve champion death state in the same way DM1 does
(no DMSAVE.DAT equivalent with per-champion mirror/reincarnate data).

Unknown: How DM2 handles dead champions across save/load.

---

## 3. No VIP Altar Equivalent in DM2

DM1 VIP altar (F0283_CHAMPION_ViAltarRebirth) has no confirmed DM2 equivalent.
The altar concept in DM2 is not documented in available sources.

---

## 4. Dispel Effect on Champions (New in DM2)

DM2 creatures can use DISPELL attack (AI_ATTACK_FLAGS__DISPELL = 0x0020):
- Removes champion enchantments
- This is a new way champions can be affected (lose buffs) without dying

Source: skproject/SKWIN/defines.h:705-716, SkWinCore.cpp:27056-27058

---

## 5. Push/Pull Spell Effects (New in DM2)

Creatures can push or pull champions with spells:
- AI_ATTACK_FLAGS__PUSH_SPELL (0x0400)
- AI_ATTACK_FLAGS__PULL_SPELL (0x0800)

This is positional disruption, not death — but affects champion positioning.

---

## Summary

| Death Feature | DM1 V1 | DM2 V1 |
|---------------|--------|--------|
| Death trigger (health <= 0) | Yes | Not confirmed |
| Mirror resurrect (no stat loss) | Yes (REVIVE.C) | Not found |
| Mirror reincarnate (stat penalty) | Yes (REVIVE.C) | Not found |
| VIP altar rebirth | Yes (F0283) | Not found |
| Permanent death | No (standard game) | Unknown |
| Companion death tracking | N/A | Yes (alive flag) |
| Companion resurrection | N/A | Not found |
| Dispel (lose enchantments) | No | YES |
| Push/Pull repositioning | No | YES |

---

## Status: NOT SOURCE-LOCKED — Significant Gap

DM2 champion death mechanics are not documented in available sources.
This is a critical gap for understanding DM2 character permanence system.

Requires: Deep SKULL.ASM analysis for champion-related routines
(address search for champion health, death, revival code patterns).