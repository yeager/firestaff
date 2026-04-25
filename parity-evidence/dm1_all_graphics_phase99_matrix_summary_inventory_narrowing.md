# DM1 all-graphics phase 99 — summary counts after inventory narrowing

## Problem

Phase 98 moved the inventory-screen row from generic `UNPROVEN` to `KNOWN_DIFF (narrowed)`, but the summary block still had the pass-97 counts:

```text
KNOWN_DIFF = 10
UNPROVEN   = ~36
```

That left the matrix internally stale again.

## Change

Updated the summary counts and bottom line:

- `KNOWN_DIFF`: `10 -> 11`
- `UNPROVEN`: `~36 -> ~35`
- bottom line now mentions the narrowed inventory-screen row backed by deterministic capture evidence

This does **not** claim full inventory parity. It only records that the row is no longer an unknown blob: slot-icon behavior is source-backed/capture-gated, while full layout and panel composition remain open until original overlay evidence exists.

## Gate

Documentation-only pass, backed by the previous gates:

```text
phase 95 capture smoke PASS
phase 96 418/418 invariants passed
phase 96 ctest 5/5 PASS
```
