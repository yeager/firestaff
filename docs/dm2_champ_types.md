# DM2 V1 New Champion Types — Source Lock

**Audit date:** 2026-05-25
**Sources:**
- skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName — 64 AI entries)
- skproject/SKWIN/defines.h:705-716 (AI_ATTACK_FLAGS)
- skproject/SkGlobal.h: CREATURE_AI_TAB_SIZE, MAXAI, GDAT_CATEGORY_LIMIT
- include/dm2_v1_companion.h
- include/dm2_v1_combat.h
- docs/dm2-v1-creatures/dm2_creatures_new.md
- docs/dm2-v1-overview/dm2_newfeatures.md

---

## Finding: DM2 Has No New Champion Classes — Companion NPCs Are Different Entity Type

"Champion types" in DM2 refers to:
1. Companion NPCs — AI-controlled allies (NOT party champion characters)
2. No new party champion character classes confirmed

The term "champion" in DM2 context is ambiguous. DM2 "Champion GDAT (0x16)" is
character metadata for party members, but the actual character types with variety
are the companion NPCs.

---

## 1. Companion NPC Types (AI Indices 13-18)

| Index | Name | Role | Behavior |
|-------|------|------|----------|
| 0x0D (13) | SCOUT MINION | Reconnaissance | Follow party |
| 0x0E (14) | ATTACK MINION | Combat | Aggressive |
| 0x0F (15) | CARRY MINION | Item transport | Follow party |
| 0x10 (16) | FETCH MINION | Object retrieval | Follow party |
| 0x11 (17) | GUARD MINION | Defense | Guard position |
| 0x12 (18) | U-HAUL MINION | Transport | Follow party |

These are summoned entities created via spells (indices 29-31):
- ZO EW KU -> Attack Minion (creature ID 0x31)
- ZO EW NETA -> Guard Minion (creature ID 0x34)
- ZO EW ROS -> U-Haul Minion (creature ID 0x35)

Source: docs/spells_dm2/dm2_newspells.md, skproject/SkWinCore.cpp:17524-17602

---

## 2. Evil Minion Variants

| Index | Name | Notes |
|-------|------|-------|
| 0x22 (34) | DRAGOTH MINION (EVIL) | Dragoth sub-type |
| 0x2B (43) | EVIL ATTACK MINION | Enemy variant |
| 0x31 (49) | EVIL GUARD MINION | Enemy variant |
| 0x3E (62) | EVIL ATTACK MINION | Duplicate of 43 |

These are enemy versions of the companion system — enemy minions that fight for
Dragoth (the primary antagonist).

Source: docs/dm2-v1-creatures/dm2_creatures_new.md

---

## 3. Merchant NPCs (AI Index 0x21 / 33)

| Index | Name | Notes |
|-------|------|-------|
| 0x21 (33) | MERCHANTS | NEW — NPC/shop system |

DM2 introduces shopkeeper NPCs (MERCHANTS). These are not companions but interactable
NPCs for the gold/shop economy system.

Source: dm2_creatures_new.md, dm2_v1_game.h (dm2_v1_enter_shop)

---

## 4. No New Party Champion Classes

Confirmed: No evidence of new champion classes (Fighter/Ninja/Priest/Wizard variants,
or entirely new classes) in DM2 party system.

Unknown:
- Whether DM2 party champions still have the 4-class system
- Whether party champions have any class-like specialization
- The DM2 equivalent of champion portrait sensor / mirror square recruitment

---

## 5. New Creature Types (Not Champions, But Affect Champion Combat)

DM2 adds creature types that challenge champions:

| Index | Name | Threat to Champions |
|-------|------|---------------------|
| 0x13 (19) | THORN DEMON | Unknown attack pattern |
| 0x17 (23) | CAVERN BAT | DM1 bat equivalent |
| 0x18 (24) | GLOP | Unknown |
| 0x19 (25) | ROCKY | Unknown |
| 0x1A (26) | GIGGLER | Steals from champions |
| 0x1B (27) | THICKET THIEF | Stealth-based |
| 0x1C (28) | TIGER STRIPED WORM | Worm-type |
| 0x1D (29) | TREANT (TREE GORGON) | New |
| 0x1E (30) | LORD DRAGOTH | Primary antagonist |
| 0x1F (31) | DRU TAN | New |
| 0x2B (43) | EVIL ATTACK MINION | Combat |
| 0x30 (48) | DARK VEXIRK | New Vexirk race |
| 0x37 (55) | VEXIRK KING | Vexirk boss |

---

## Summary

| Entity Type | DM1 | DM2 V1 |
|------------|-----|--------|
| Party champion classes | 4 (Fighter/Ninja/Priest/Wizard) | Not confirmed |
| Companion NPCs | None | Yes - 6 types, AI 13-18 |
| Merchant NPCs | None | Yes - AI 33 |
| Enemy minions | None | Yes - AI 34,43,49,62 |
| Boss characters | None | Yes - LORD DRAGOTH (AI 30) |
| Stealing creatures | Giggler | Giggler (AI 26) + Thicket Thief (AI 27) |

---

## Status: PARTIALLY SOURCE-LOCKED

Companion NPC system is well documented (skproject SKWIN/SkWinCore.cpp, getAIName).
Party champion class system for DM2 is NOT confirmed — requires deeper SKULL.ASM analysis.