# CSB V1 - GAP: Mechanics Implementation Gaps

**File:** `docs/csb_gap_mechanics.md`
**Audit:** Firestaff CSB V1 Audit Runner
**Date:** 2026-05-25
**Reference:** `docs/source-lock/csb_mechanics.md`

---

## Executive Summary

CSB mechanics are DM1 with targeted changes:
- **24 levels** (vs 14 in DM1) - new dungeon header format
- **Projectile speed normalization** - full speed on all maps
- **Grey Lord** (0x1a) - new C5_ATTACK_MAGIC creature
- **Reincarnation penalty** - HP/MP/STA halved, -1/8th stats except Luck
- **ZOKATHRA spell power variant** - fireball with CSB-specific power
- **No movement changes**, **no class changes**, **no new spell types**

---

## GAP 1: Level Count and Dungeon Header (24 vs 14)

**What source-lock says:**
- M13_PLAN.md:303 - CSB DUNGEON.DAT has different header: 24 levels vs 14
- CEDTINC8.C:101-118 - CSBGAME.DAT vs DMSAVE.DAT routing

**Implementation gap:**
Firestaff DM1 dungeon loader hardcodes 14 levels. CSB requires:
1. Level count from header field, not hardcoded
2. Variant-aware header parser:
   - DM1: parse as DMSAVE.DAT / DM1 DUNGEON.DAT header
   - CSB: parse as CSBGAME.DAT / CSB DUNGEON.DAT header (24 levels)
3. Level array/vector sized dynamically from header
4. Reference to M13_PLAN.md:303 for header field layout

**Source:** M13_PLAN.md:303 · CEDTINC8.C:101-118 · csb_dungeon.md Part II

---

## GAP 2: Teleporter Changes (BUG0_69)

**What source-lock says:**
- GROUP.C CHANGE7_19_FIX: BUG0_69 - group movement/teleporter handling
- Fixed Lord Chaos allowed-map checks
- Grey Lord (0x1a) must also be able to use teleporters

**Implementation gap:**
Current Firestaff teleporter logic may not correctly handle:
1. Creatures with map access restrictions (Lord Chaos, Grey Lord)
2. Group spawning on teleporter arrival (BUG0_09, BUG0_10 fixes)
3. Verify all 27 creature types can/cannot use teleporters per CSB rules

**Source:** GROUP.C (CHANGE7_19) · DUNGEON.C (CHANGE7_17/18) · csb_combat.md Part II

---

## GAP 3: ZOKATHRA Spell Power Variant

**What source-lock says:**
- M13_PLAN.md:337 - CSB changes some spell power values
- ZOKATHRA (Zo Kath Ra) fireball variant has different kinetic energy vs Ful Ir
- DEFS.H:1774 - C7_SPELL_TYPE_OTHER_ZOKATHRA = 7
- MENU.C:1994 - ZOKATHRA icon C197_ICON_JUNK_ZOKATHRA
- CSB spell table still 25 spells (same as DM1)

**Implementation gap:**
ZOKATHRA exists in DM1 but CSB has different power values.
Need:
1. Spell power lookup keyed on (spellName, GameVariant)
2. ZOKATHRA in CSB mode: use CSB-specific power value (not DM1 value)
3. Ful Ir (standard Fireball): same between DM1 and CSB
4. Verify spell.power scales correctly for fireball visual effect

**Source:** M13_PLAN.md:337,346 · DEFS.H:1774 · MENU.C:1994 · Magic.cpp · csb_mechanics.md Part II

---

## GAP 4: Grey Lord Creature Roster (0x1a)

**What source-lock says:**
- csb_creatures.md: Grey Lord is the only new creature type in CSB (0x1a)
- DEFS.H:1679 - C5_ATTACK_MAGIC source (shared with Lord Chaos, Lord Order, etc.)
- Attack.cpp:2423 - Grey Lord monster type assignment
- Chaos.cpp - attack byte sequences
- Grey Lord is a lord-tier creature like Lord Chaos and Lord Order

**Implementation gap:**
Grey Lord is the only creature addition in CSB vs DM1.
Need:
1. Add Grey Lord (0x1a) to Firestaff creature enum/table
2. Grey Lord behavior: C5_ATTACK_MAGIC, casts spells, uses existing infrastructure
3. Attack byte sequences in Chaos.cpp - decode and implement Grey Lord AI
4. Grey Lord map placement: only in CSB dungeons, not DM1
5. Verify Grey Lord projectile/spell targeting is correct

**Source:** Attack.cpp:2423 · Chaos.cpp · DEFS.H:1679 · csb_creatures.md

---

## GAP 5: Champion Reincarnation Penalty (CHANGE7_24)

**What source-lock says:**
- CSB:REVIVE.C CHANGE7_24: New reincarnation rules
- Character.cpp:14 - three new globals:
  - reincarnateAttributePenalty (default 2, max 16)
  - reincarnateStatPenalty (default 8, max 16)
  - randomPoints (default 3, max 25)

| Stat | DM1 | CSB CHANGE7_24 |
|------|-----|----------------|
| Current HP/MP/STA | Full | Halved |
| Max HP/MP/STA | Full | -1/8th |
| Other stats | Full | -1/8th |
| Luck | Full | No change |

**Implementation gap:**
Covered in detail in csb_gap_champions.md GAP 2.
Brief mechanics note:
1. Penalty scales with reincarnateAttributePenalty / reincarnateStatPenalty globals
2. randomPoints: champion gains random bonus stat points on reincarnation
3. Luck completely exempt from all penalties
4. All stats respect minimum values after penalty

**Source:** CSB:REVIVE.C (CHANGE7_24) · Character.cpp:14,682-687 · csb_champions.md

---

## GAP 6: Magic System Unchanged (No Gap)

**What source-lock says:**
- Same 25-spell table (DM1_SPELL_COUNT = 25)
- Same 4 schools and 4 classes
- Same casting mechanism
- Same mana/stat requirements
- Same spell filtering

**Implementation gap:**
None. DM1 magic system carries forward to CSB unchanged.
ZOKATHRA power delta (GAP 3 above) is the only magic-system delta.

**Source:** Magic.cpp:844-954 · CSB.h:1727-1739 · csb_mechanics.md Part II

---

## GAP 7: Movement System Unchanged (No Gap)

**What source-lock says:**
- 8-directional movement: identical between DM1 and CSB
- Collision, sensors, teleporter behavior: unchanged
- No changes to speed, turn rate, step size

**Implementation gap:**
None. Movement system is identical between DM1 and CSB.

**Source:** DUNGEON.C · MOVESENS.C · csb_mechanics.md Part III

---

## GAP 8: Class System Identical (No Gap)

**What source-lock says:**
- Fighter/Ninja/Priest/Wizard - unchanged
- Level-up formulas identical
- MaxHP/MaxMP/MaxStamina calculations unchanged

**Implementation gap:**
None. Class system is carry-forward from DM1.

**Source:** DEFS.H:757-768 · CHAMPION.C · Magic.cpp:LevelUp() · csb_mechanics.md Part IV

---

## Summary Table

| Gap | Severity | Description |
|-----|----------|-------------|
| 24-level dungeon header | HIGH | Variant-aware loader; level count from header |
| Teleporter/Grey Lord fix | MEDIUM | BUG0_69: Grey Lord map access for teleporters |
| ZOKATHRA spell power | MEDIUM | CSB-specific power value for fireball variant |
| Grey Lord (0x1a) | HIGH | New C5_ATTACK_MAGIC creature; attack sequences |
| Reincarnation penalty | HIGH | HP/MP/STA halved, -1/8th stats; Luck exempt |
| Movement system | NONE | Identical to DM1 |
| Class system | NONE | Identical to DM1 |
| Magic system | NONE | Identical except ZOKATHRA power |

---

## Reference Sources

| Source | Content |
|--------|---------|
| docs/source-lock/csb_mechanics.md | Existing source-lock audit (primary) |
| M13_PLAN.md:303,337,346 | 24 levels, ZOKATHRA power |
| CEDTINC8.C:101-118 | CSBGAME.DAT vs DMSAVE.DAT |
| GROUP.C (CHANGE7_19) | Teleporter fix |
| DUNGEON.C (CHANGE7_17/18) | Square event fixes |
| CSBWin Attack.cpp:2423 | Grey Lord type |
| CSBWin Chaos.cpp | Grey Lord attack sequences |
| CSB:REVIVE.C (CHANGE7_24) | Reincarnation rules |
| BugsAndChanges.htm | CHANGE7_20,24 |
