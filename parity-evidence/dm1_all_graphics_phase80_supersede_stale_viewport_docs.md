# DM1 all-graphics phase 80 — supersede stale viewport documentation

## Problem

After phases 76–79 migrated probes and removed legacy prototype viewport constants, older tracked documentation still contained unsuffixed statements that Firestaff runtime/probe viewport was `(12,24,196,118)`. Those statements were historically true for passes 34/40/41, but are now stale and could mislead future work.

## Change

Added explicit supersession notes / historical wording to:

- `V1_BLOCKERS.md`
- `parity-evidence/pass40_viewport_lock.md`
- `parity-evidence/pass34_sidepanel_rectangle_table.md`
- `parity-evidence/pass41_status_box_stride.md`

The updated docs now preserve the old pass evidence while clearly stating current normal V1 all-graphics work uses the source DM1 viewport `(0,33,224,136)` and that phase 76–79 migrated/removed prototype probe geometry.

## Gate

```text
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 410/410 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
