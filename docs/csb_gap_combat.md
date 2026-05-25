# CSB V1 - GAP: Combat Implementation Gaps

**File:** `docs/csb_gap_combat.md`
**Audit:** Firestaff CSB V1 Audit Runner
**Date:** 2026-05-25
**Reference:** `docs/source-lock/csb_combat.md`

---

## Executive Summary

CSB combat changes vs DM1 are MINIMAL - only one gameplay change beyond bug fixes:
- **Projectile speed normalization** (CHANGE7_20) - full speed on all maps
- **Grey Lord** (0x1a) - new C5_ATTACK_MAGIC creature using existing infrastructure
- 4 bug fixes (BUG0_69, BUG0_09, BUG0_10, CHANGE8_12)

No new attack types, no new weapons, no new armor.

---

## GAP 1: Projectile Speed Normalization (CHANGE7_20)

**What source-lock says:**
- PROJEXPL.C CHANGE7_20_IMPROVEMENT
- DM1 bug: Projectiles moved slower on maps other than the party map
- CSB fix: Projectiles now move at full speed on ALL maps

**Implementation gap:**
Firestaff projectile system uses per-map speed. Need to verify:
1. If currentMapId == projectile.targetMapId: full speed (DM1 and CSB agree)
2. If currentMapId != projectile.targetMapId:
   - DM1 behavior: slower speed (BUGGY)
   - CSB behavior: full speed (CORRECTED)
3. Add GameVariant flag:
   - DM1: apply slowdown factor on non-party maps
   - CSB: apply full speed regardless of map

**Source:** PROJEXPL.C (CHANGE7_20_IMPROVEMENT) · csb_combat.md Part I · csb_mechanics.md Part I

---

## GAP 2: Grey Lord Combat Behavior (New Creature)

**What source-lock says:**
- DEFS.H:1679 - Grey Lord is a C5_ATTACK_MAGIC source (same category as Lord Chaos)
- Attack.cpp:2423 - Grey Lord monster type assignment
- IsLordChaosHere() widened to include Grey Lord proximity checks
- Chaos.cpp - dedicated attack byte sequences for Grey Lord
- Grey Lord is 0x1a (only new creature in CSB vs DM1)

**Implementation gap:**
Grey Lord uses existing C5_ATTACK_MAGIC infrastructure:
1. Add Grey Lord (0x1a) to creature roster
2. C5_MAGICAL type attack, uses existing CastSpell-like infrastructure
3. IsLordChaosHere() now also checks Grey Lord proximity - update detection
4. Grey Lord attack byte sequences in Chaos.cpp - decode/implement
5. Verify Grey Lord is allowed on teleporter-connected maps (BUG0_69 fix)

**Source:** Attack.cpp:2423 · Chaos.cpp · DEFS.H:1679 · csb_creatures.md

---

## GAP 3: Group AI and Creature Teleporter Fix (BUG0_69)

**What source-lock says:**
- GROUP.C CHANGE7_19_FIX: BUG0_69
- Fixed group movement/teleporter handling for Lord Chaos maps
- Related to Lord Chaos allowed map checks

**Implementation gap:**
Group teleporter usage may not handle Lord Chaos/Grey Lord map restrictions.
Need:
1. Group::canUseTeleporter(creatureType, teleporterId) - check map access
2. Lord Chaos (0x18) and Grey Lord (0x1a) map access restrictions
3. BUG0_69 fix ensures groups with these creatures use teleporters correctly

**Source:** GROUP.C (CHANGE7_19_FIX) · csb_combat.md Part II

---

## GAP 4: Dungeon Square Event Fixes (BUG0_09, BUG0_10)

**What source-lock says:**
- DUNGEON.C CHANGE7_17_FIX: BUG0_09 - square event processing
- DUNGEON.C CHANGE7_18_FIX: BUG0_10 - square event processing
- Affects creature group spawning and trigger handling

**Implementation gap:**
Audit DUNGEON.C square event handlers:
1. BUG0_09: Square event trigger timing or condition - fix spawn logic
2. BUG0_10: Related square event condition fix
3. Verify group spawn positions correct after teleporter use

**Source:** DUNGEON.C (CHANGE7_17/18_FIX) · csb_combat.md Part II

---

## GAP 5: Save Game Combat State (CHANGE7_29, CHANGE8_12)

**What source-lock says:**
- CHANGE7_29: New saved game header format (CSBGAME.DAT vs DMSAVE.DAT)
- CHANGE8_12_FIX: Save/load fixes affecting combat state

**Implementation gap:**
Combat state serialization needs CSB-format support:
1. CSBGAME.DAT header format differs from DMSAVE.DAT
2. Combat state fields may have different positions/sizes
3. CHANGE8_12 fixes specific save/load bugs

**Source:** CEDTINC8.C:101-118 · BugsAndChanges.htm (CHANGE7_29,8_12)

---

## What Does NOT Need Implementation

- Magical attack sources (C5_ATTACK_MAGIC) - unchanged except Grey Lord
- Attack type system - unchanged
- Weapon/armor categories - unchanged
- Combat resolution algorithm - unchanged
- Spell power for Ful Ir (standard Fireball) - unchanged (ZOKATHRA delta is magic, not combat)

---

## Summary Table

| Gap | Severity | Description |
|-----|----------|-------------|
| Projectile speed normalization | HIGH | Full speed on all maps (CHANGE7_20) |
| Grey Lord (0x1a) combat | HIGH | New C5_ATTACK_MAGIC creature; attack byte sequences |
| Group teleporter fix (BUG0_69) | MEDIUM | Lord Chaos/Grey Lord map access for teleporters |
| Dungeon square event fixes | MEDIUM | BUG0_09, BUG0_10 - group spawn/trigger |
| CSB save game combat state | MEDIUM | CSBGAME.DAT format; CHANGE8_12 fixes |

---

## Reference Sources

| Source | Content |
|--------|---------|
| docs/source-lock/csb_combat.md | Existing source-lock audit (primary) |
| ReDMCSB PROJEXPL.C (CHANGE7_20) | Projectile speed normalization |
| CSBWin Attack.cpp:2423 | Grey Lord monster type |
| CSBWin Chaos.cpp | Grey Lord attack byte sequences |
| ReDMCSB GROUP.C (CHANGE7_19) | Group teleporter fix |
| ReDMCSB DUNGEON.C (CHANGE7_17/18) | Square event fixes |
| BugsAndChanges.htm | CHANGE7_20,21,29,8_12 |
