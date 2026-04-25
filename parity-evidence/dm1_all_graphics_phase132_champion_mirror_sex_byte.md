# DM1 all-graphics phase 132 — carry champion mirror sex byte

## Problem

Champion mirror text contains sex after the empty third field:

```text
NAME|TITLE||SEX|...
```

Pass 131 parsed name/title only. The next source-backed identity byte is the raw sex marker (`M`/`F`).

## Change

Added `ChampionState_Compat.sex` and carried it through the same pure-data path:

- parser reads the source mirror text sex byte from `NAME|TITLE||SEX|...`
- serialization stores it in previous reserved byte `[156]`
- deserialization restores it
- champion serialized size remains 256 bytes

No gameplay behavior, stat inference, UI, or recruitment side effects were added.

## Gate

```text
PASS: Champion mirror text identity parser accepts NAME|TITLE||... source format
PASS: Champion mirror parser packs source Name[8]
PASS: Champion mirror parser packs source Title[20]
PASS: Champion mirror parser carries source sex byte
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 443/443 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- parse the source stat/skill/luck field bytes without inventing gameplay semantics
- wire parsed identity into real mirror recruitment/champion load
- remove the endgame fallback table once real state always carries name/title
