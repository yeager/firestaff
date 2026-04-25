# DM1 all-graphics phase 134 — carry encoded champion mirror stat/skill fields

## Problem

Champion mirror text carries more source identity/setup data after `NAME|TITLE||SEX|`:

```text
NAME|TITLE||SEX|ENCODED_STATS|ENCODED_SKILLS|...
```

Earlier passes carried name/title/sex, but dropped the encoded stat/skill fields before the real recruitment/lifecycle path can interpret them.

## Change

Added raw source carry-through fields to `ChampionState_Compat`:

- `mirrorStatsText[16]`
- `mirrorSkillsText[16]`

The parser now copies those fields from decoded DUNGEON.DAT mirror text without interpreting or mutating gameplay stats. Serialization stores them in previously reserved champion bytes:

- `[157..172] mirrorStatsText[16]`
- `[173..188] mirrorSkillsText[16]`

Champion serialized size remains 256 bytes.

## Gate

```text
PASS: Champion mirror text identity parser accepts NAME|TITLE||... source format
PASS: Champion mirror parser packs source Name[8]
PASS: Champion mirror parser packs source Title[20]
PASS: Champion mirror parser carries source sex byte
PASS: Champion mirror parser carries encoded source stat field
PASS: Champion mirror parser carries encoded source skill field
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 444/444 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- decode these fields into real source champion stats/skills when the recruitment path is implemented
- carry the remaining encoded inventory/object field if needed
- wire parsed mirror data into actual champion recruitment/load
