# DM1 all-graphics phase 113 — dialog matrix after centered message text

## Problem

Pass 112 changed runtime dialog rendering so message text is centered in the source viewport region when the source dialog backdrop is active. The parity matrix still said message centering/wrapping was not source-metric accurate, without recording the new centered-text step.

## Change

Updated the dialog/endgame row in `PARITY_MATRIX_DM1_V1.md`:

- Firestaff state now records pass 112:
  - message text is centered across the source viewport region
  - centering uses text measurement
- Status remains `KNOWN_DIFF (narrowed)` because full dialog/endgame parity is still not done.
- Remaining dialog gaps are now more precise:
  - source choice patch zones / input flow
  - exact C469/C471 vertical placement
  - exact line splitting / source metric behavior
  - endgame still placeholder

Updated the bottom line to mention source backdrop + `V3.4` + centered message text.

## Gate

Documentation-only pass, backed by pass 112:

```text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
# summary: 424/424 invariants passed
ctest: 5/5 PASS
```
