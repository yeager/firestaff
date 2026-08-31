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
- **BUG7_01 remains in CSB** - CSB's own creature-replacement palette quirks
  persist through Atari ST 2.1; it is distinct from DM's BUG0_04

No new graphic assets, no new sprite sheets, no new animation types.

---

## GAP 1: VBL Handler Fix (BUG0_03)

**What source-lock says:**
- BASE.C CHANGE7_01_FIX: BUG0_03
- DM1 bug: VBL interrupts could be ignored under heavy load
  -> top of dungeon view used wrong color palette
- CSB fix: VBL handler changed so no interrupts can be ignored
  -> palette switching always starts on time

**Implementation status (2026-08-30):**
Partial. The source-defined scheduling discipline is implemented by
`csb_v1_atari_st_vblank_deliver`: every arrival starts palette setup, a
re-entrant arrival increments a pending counter, and the outer handler drains
the counter while its modelled interrupt priority is 3. The implementation is
locked by `csb_v1_atari_st_vblank`, including a nested two-arrival regression.
The native Atari ST `ANIMATE.SCR` title path is now a live consumer: each 50 Hz
tick passes through the model, invalidates the retained source frame, and
re-decodes the original P4B1 palette for that VBlank. The complete original
STX CLI/title/input/menu regression covers this path. After the verified
ANIM.C → FTLCODE handoff, every Atari gameplay tick also delivers the same
model and its palette-start callback installs the source-selected C232
`GRAPHICS.DAT` light palette before host presentation. The generic PC 3.4
`F0693_WaitVerticalBlank_PC34` gate remains a synchronous wait only and is not
used as Atari ST parity evidence. Remaining work is original gameplay-frame
capture comparison across palette changes; an earlier audit incorrectly cited
`F0613_VBL_Process`, which is a champion-text helper.

**Source evidence used for implementation:**
ReDMCSB WIP `BASE.C:E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF` and
`CHANGE7_01_FIX` identify `G0351_i_ConcurrentVerticalBlankExceptionCount`,
the level-4-to-level-3 priority change, and the drain loop. This source was
read from the published WIP archive; no game bytes are embedded or extracted.

**Implementation gap:**
Firestaff must not claim the CSB fix merely because it has a synchronous VBlank
wait. The required behavior is:
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

## GAP 4: BUG7_01 - CSB Creature Palette Quirks

**What source-lock says:**
- CSB-specific BUG7_01 affects the Atari ST 2.0 and 2.1 releases. Its Worm
  uses palette entries 9/10 and therefore intentionally appears with the
  map's last allowed creature replacement (Demon/Dragon on maps 0/9;
  Giant Scorpion/Flying Eye on map 4).
- Slime Devil and Lord Chaos omit part or all of their replacement-color
  declarations; Hellhound inherits unused replacements; Zytaz retains an
  inappropriate Materializer replacement declaration.
- There is no CSB 2.1 fix. This is preservation behavior, not a license to
  manufacture corrected art or palette values.

**Implementation gap:**
BUG7_01 is a CSB source behavior, not a Modern-mode correction target.
1. Bind creature palette inputs to original CSB map/allowed-creature data.
2. Capture the three real Worm map outcomes before marking renderer parity.
3. Preserve source colors in Original mode; any optional Modern correction
   needs separately documented user consent and must never replace Original.

**Source:** ReDMCSB `DUNVIEW.C` · BugsAndChanges.htm#BUG7_01

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
| VBL handler fix (BUG0_03) | PARTIAL / source-modelled | No ignored VBL interrupts; precise palette switching |
| Engine version display (CHANGE7_36) | LOW | v2.0/v2.1 in dialog top-right; CSB only |
| Wall drawing optimization | NONE | Performance only; no functional gap |
| BUG7_01 (CSB creature palette) | OPEN / preservation | Real map-specific source palette evidence and renderer binding required |
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

The authoritative online reproduction and resolution are at
<http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm#BUG0_03>.
It specifies that BUG0_03 affects the Atari ST DM releases, is fixed in Atari
ST CSB 2.0/2.1, and requires that no VBlank interrupt be ignored.
