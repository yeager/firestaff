# DM1 all-graphics phase 140 — require source empty field before sex

## Problem

DM1 champion mirror records use a specific source text shape:

```text
NAME|TITLE||SEX|...
```

The parser accepted malformed `NAME|TITLE|SEX|...` records by silently clearing sex and returning success.

## Change

`F0606_CHAMPION_ParseMirrorTextIdentity_Compat` now rejects records that do not include the source empty field before sex (`||M` / `||F`).

## Gate

```text
PASS: Champion mirror parser rejects missing empty source field before sex
Status: PASS
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```
