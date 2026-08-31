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

**Remaining evidence gap:**
The native scheduler and palette-start callback are covered. Do not promote
that to original-frame pixel parity until a capture across a real palette
transition exists. The capture must identify the original STX member and its
palette timing; a synthetic frame or a generic synchronous VBlank wait is not
acceptable evidence.

**Source:** BASE.C (CHANGE7_01_FIX) · csb_graphics.md Part I

---

## GAP 2: Engine Version Display (CHANGE7_36, CHANGE8_13)

**What source-lock says:**
- DIALOG.C: Engine version 2.0/2.1 printed in top right corner of dialog boxes
- CHANGE8_13: CSB version 2.1 display
- New UI element not present in DM1

**Implementation status (2026-08-31):**
Source modelled. `csb_v1_engine_version_display_pc34_compat` owns the
version-state transition, while the CSB boot/title route sets `v2.1` only after
an admitted CSB profile and resets to the DM1 baseline after cleanup. The
focused `csb_v1_graphics_extras_pc34_compat` and
`csb_v1_boot_title_import_ui_gate_pc34_compat` tests cover both strings and
the no-stale-version boundary. This is state/render-plan evidence; an original
dialog frame capture is still needed before pixel parity is claimed.

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

**Implementation status (2026-08-31):**
BUG7_01 is a CSB source behavior, not a Modern-mode correction target.
`csb_v1_f0093_apply_replacement_palette_pc34` now takes the live current-map
allowed-creature order and applies F0093's final owners to the M11 D2/D3
creature palette before the source sprite is blitted. It first performs the
source reset (slot 9 via replacement set 8; slot 10 via set 12), then applies
the map-ordered final writers. This prevents a prior sprite's local remap from
leaking into an unowned CSB map palette. Slot 9/10 assignments,
including a valid replacement value of zero, therefore follow ReDMCSB rather
than a per-creature approximation. The adapter selects the Atari ST target
table for ST 2.0/2.1/F20 variants and ReDMCSB's separate `G2025`/`F0695`
version-3 table for Amiga and FM Towns variants. The two non-PC source
families decode to the same host indices here, but both differ from PC D3
targets for some replacement sets, so PC values are not applied to either
platform family.
`csb_v1_f0093_replacement_palette_pc34_compat` pins the ordered ownership,
reset and Atari-specific result. The real Atari ST STX startup
and M12→M11 handoff tests verify that the rebuilt native CSB path remains
bootable; they do not substitute for a creature-on-Worm-map frame capture.

Remaining evidence is visual, not a known unbound code path: capture the three
real Worm-map outcomes before claiming renderer/pixel parity. Original mode
must retain source colors; any Modern correction needs separate user consent
and must never replace Original.

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
| Engine version display (CHANGE7_36) | SOURCE MODELLED | v2.0/v2.1 state is bound to the CSB boot/title path; original dialog capture pending |
| Wall drawing optimization | NONE | Performance only; no functional gap |
| BUG7_01 (CSB creature palette) | RUNTIME BOUND / capture pending | Map-ordered F0093 D2/D3 binding is live; original Worm-map overlay evidence remains required |
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
