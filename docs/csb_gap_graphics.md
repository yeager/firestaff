# CSB V1 - GAP: Graphics/UI Implementation Gaps

**File:** `docs/csb_gap_graphics.md`
**Audit:** Firestaff CSB V1 Audit Runner
**Date:** 2026-05-25
**Reference:** `docs/source-lock/csb_graphics.md`

---

## Executive Summary

CSB graphics/UI changes vs DM1 are MINIMAL:
- **VBL handler fix** (BUG0_03) - no ignored interrupts, palette switching on time
- **Engine version display** - new UI element in dialog boxes
- **Left-click champion bar** - UI interaction change (covered in champions gap)
- **BUG0_04 NOT fixed** - Lord Chaos palette bug persists in CSB

No new graphic assets, no new sprite sheets, no new animation types.

---

## GAP 1: VBL Handler Fix (BUG0_03)

**What source-lock says:**
- BASE.C CHANGE7_01_FIX: BUG0_03
- DM1 bug: VBL interrupts could be ignored under heavy load
  -> top of dungeon view used wrong color palette
- CSB fix: VBL handler changed so no interrupts can be ignored
  -> palette switching always starts on time

**Implementation gap:**
Current Firestaff VBL handler may exhibit DM1 palette bug under load.
Need:
1. Verify VBL interrupt handler does not drop or skip interrupts
2. Palette switch triggered at precise VBL timing
3. Under heavy render load: ensure VBL queue does not overflow/overwrite
4. Palette bank selection for dungeon view starts on correct VBL boundary

**Source:** BASE.C (CHANGE7_01_FIX) · csb_graphics.md Part I

---

## GAP 2: Engine Version Display (CHANGE7_36, CHANGE8_13)

**What source-lock says:**
- DIALOG.C: Engine version 2.0/2.1 printed in top right corner of dialog boxes
- CHANGE8_13: CSB version 2.1 display
- New UI element not present in DM1

**Implementation gap:**
Firestaff DM1 does not display engine version in dialog boxes.
Need:
1. Add engine version string to dialog box rendering (top right corner)
2. Version format: "v2.0" or "v2.1" depending on CSB variant
3. Render only in CSB mode (not DM1)
4. Update when dialog is opened/refreshed

**Source:** DIALOG.C (CHANGE7_36,8_13) · csb_graphics.md Part I

---

## GAP 3: Wall Drawing Optimization (CHANGE7_15)

**What source-lock says:**
- DUNVIEW.C CHANGE7_15_OPTIMIZATION
- New function to draw walls in center of dungeon view
- Avoids unnecessary transparency support
- Performance optimization, no visual change

**Implementation gap:**
This is an optimization, not a functional gap. No implementation required unless
performance issues are observed. Document as non-blocking.

**Source:** DUNVIEW.C (CHANGE7_15_OPTIMIZATION)

---

## GAP 4: BUG0_04 - Lord Chaos Palette NOT Fixed

**What source-lock says:**
- DM1 bug (BUG0_04): Lord Chaos front/side/attack bitmaps use color 9
  but no replacement color specified
- On Lord Chaos map: Demon-defined colors used (appropriate but unspecified)
- Zytaz bitmaps also have color 9/10 issues
- **This bug PERSISTS in CSB** - not listed in CSB-specific fixes

**Implementation gap:**
BUG0_04 is an existing design issue, not a CSB gap.
1. Confirm current Firestaff also has this bug (likely yes)
2. Document as known limitation
3. If CSB dungeon has Lord Chaos map: verify Demon-defined palette used correctly
4. No fix required (CSB did not fix it either)

**Source:** csb_graphics.md Part II · csb_dungeon.md

---

## GAP 5: Mouse Pointer Handling Fix (BUG0_00 part)

**What source-lock says:**
- CHANGE7_14_FIX: Mouse pointer handling in DUNVIEW.C, CHEST.C, LOADSAVE.C, MOVESENS.C, STARTUP1.C
- Part of BUG0_00 fix - removal of useless code

**Implementation gap:**
No functional gap - code cleanup fix. Audit for residual useless code from DM1.
Not a blocking issue.

**Source:** DUNVIEW.C, CHEST.C, LOADSAVE.C, MOVESENS.C, STARTUP1.C (CHANGE7_14)

---

## GAP 6: Code-to-Assembly Conversion (CHANGE7_16)

**What source-lock says:**
- Files: CHAMDRAW.C, CHAMPION.C, DUNGEON.C, REVIVE.C, TIMELINE.C
- Some source converted from C to assembly for performance/size
- No visual change, internal optimization

**Implementation gap:**
Not a functional gap. Firestaff uses portable C++ - assembly conversion is
platform-specific. Document as non-gap.

**Source:** CHAMDRAW.C, CHAMPION.C, DUNGEON.C, REVIVE.C, TIMELINE.C (CHANGE7_16)

---

## What Does NOT Need Implementation

- New graphic assets (CSB has same NUM_MONSTER_TYPE = 27)
- New sprite sheets (Grey Lord uses existing infrastructure)
- New animation types
- New UI element graphics beyond engine version display
- Copy protection graphics (CHANGE7_04, CHANGE7_10) - not applicable to Firestaff

---

## Summary Table

| Gap | Severity | Description |
|-----|----------|-------------|
| VBL handler fix (BUG0_03) | MEDIUM | No ignored VBL interrupts; precise palette switching |
| Engine version display (CHANGE7_36) | LOW | v2.0/v2.1 in dialog top-right; CSB only |
| Wall drawing optimization | NONE | Performance only; no functional gap |
| BUG0_04 (Lord Chaos palette) | DOCUMENT | Known limitation; CSB also has it; no fix |
| Mouse pointer fix | NONE | Code cleanup; not blocking |
| Code-to-assembly conversion | NONE | Platform-specific; not applicable |

---

## Reference Sources

| Source | Content |
|--------|---------|
| docs/source-lock/csb_graphics.md | Existing source-lock audit (primary) |
| BASE.C (CHANGE7_01_FIX) | VBL handler fix |
| DIALOG.C (CHANGE7_36,8_13) | Engine version display |
| DUNVIEW.C (CHANGE7_15) | Wall drawing optimization |
| csb_graphics.md Part II | BUG0_04 persistence |
| BugsAndChanges.htm | CHANGE7_01,14,15,16,36,8_13 |
