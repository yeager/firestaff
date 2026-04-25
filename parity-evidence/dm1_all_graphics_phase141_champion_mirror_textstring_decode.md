# DM1 all-graphics phase 141 — decode real DUNGEON.DAT champion TextString records

## Problem

The mirror parser worked on synthetic `|`-separated text. Real DUNGEON.DAT decoded text uses the source separator code as a line separator, while reports display it as `|`. The next step was to prove the parser can consume real decoded TextString records.

## Change

- mirror parser now treats both `|` and decoded newline separators as source field separators
- added `F0607_CHAMPION_ParseMirrorTextString_Compat(things, textStringIndex, champ)`
- the helper decodes a DUNGEON.DAT TextString by offset and parses it as a champion mirror record

## Gate

Real DUNGEON.DAT integration check:

```text
PASS: DUNGEON.DAT TextString parser finds STAMM/BLADECASTER mirror record
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire `F0607` into actual mirror recruitment/champion load
- decode encoded source stats/skills/inventory into runtime structures
