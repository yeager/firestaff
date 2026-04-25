# DM1 all-graphics phase 138 — guard champion mirror sex byte

## Problem

The mirror text parser carried whatever byte appeared in the source sex field. In real DM1 champion mirror text this field is a source marker, `M` or `F`; accepting arbitrary values would let malformed/synthetic records masquerade as source data.

## Change

`F0606_CHAMPION_ParseMirrorTextIdentity_Compat` now rejects mirror text where the sex byte is not `M` or `F`.

## Gate

```text
PASS: Champion mirror parser rejects non-source sex byte
Status: PASS
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```
