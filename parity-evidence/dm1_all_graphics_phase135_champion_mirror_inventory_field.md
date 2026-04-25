# DM1 all-graphics phase 135 — carry encoded champion mirror inventory field

## Problem

The decoded DUNGEON.DAT champion mirror text includes a remaining encoded field after stats/skills:

```text
NAME|TITLE||SEX|ENCODED_STATS|ENCODED_SKILLS|ENCODED_INVENTORY
```

Dropping it would make later real recruitment/load work depend on re-reading text instead of carrying the complete source mirror record forward.

## Change

Added raw source carry-through field:

- `mirrorInventoryText[32]`

The parser copies the field without interpreting or mutating gameplay inventory. Serialization stores it in previously reserved champion bytes:

- `[189..220] mirrorInventoryText[32]`

Champion serialized size remains 256 bytes.

## Gate

```text
PASS: Champion mirror parser carries source sex byte
PASS: Champion mirror parser carries encoded source stat field
PASS: Champion mirror parser carries encoded source skill field
PASS: Champion mirror parser carries encoded source inventory field
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 444/444 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- decode the encoded mirror fields into real champion stats/skills/inventory
- wire parsed mirror records into actual recruitment/load
