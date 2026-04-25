# DM1 all-graphics phase 139 — mirror parser clears stale source fields

## Problem

`F0606_CHAMPION_ParseMirrorTextIdentity_Compat` can be called on an existing champion slot. If a malformed mirror record was parsed after a valid one, stale title/sex/encoded field bytes could remain in the destination.

## Change

The parser now clears the source mirror fields at the start of parsing:

- `Name[8]`
- `Title[20]`
- `sex`
- encoded stats/skills/inventory text fields

Then it repopulates only from the current source record.

## Gate

```text
PASS: Champion mirror parser rejects non-source sex byte
PASS: Champion mirror parser clears stale sex before rejecting malformed record
Status: PASS
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```
