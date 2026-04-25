# DM1 all-graphics phase 97 — parity matrix summary refresh

## Problem

The summary block in `PARITY_MATRIX_DM1_V1.md` still said "after Pass 36 honesty lock" and counted only 11 matched rows, even after the recent all-graphics promotions:

- equipment/item icons promoted by phases 84–95
- rune / C011 spell label cells promoted by phase 96

That made the matrix internally inconsistent: individual rows were updated, but the summary counts and bottom line were stale.

## Change

Updated the summary block to:

- title it as an all-graphics pass-97 refresh
- increase `MATCHED` from 11 to 13
- reduce `UNPROVEN` from ~38 to ~36
- explicitly list the newly matched areas:
  - equipment/item icon resolver + action/inventory palette split
  - rune/C011 spell label cells
- update the bottom line to mention source-bound action/inventory object icons and C011 spell label cells, while keeping the remaining blockers honest:
  - viewport content/draw-order parity
  - original screenshot overlays
  - audio/timing
  - pointer/held-object icons
  - exact inventory/spell-panel placement overlays

## Gate

Documentation-only pass. It is backed by the immediately preceding gates:

```text
phase 96 capture smoke PASS
phase 96 418/418 invariants passed
phase 96 ctest 5/5 PASS
```
