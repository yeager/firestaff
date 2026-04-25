# DM1 all-graphics phase 133 — remove invented endgame title fallback

## Problem

The V1 endgame overlay originally bridged missing `Champion.Title[20]` state with a bounded canonical name → title fallback table. After passes 130–132, title bytes are now part of party state and can be parsed from source mirror text, so the renderer should not invent champion titles from names.

DM1 `ENDGAME.C:F0444_STARTEND_Endgame` draws `Champion.Title` directly.

## Change

- removed the endgame canonical title lookup table
- endgame title drawing now uses only raw `ChampionState_Compat.title`
- updated tests to provide raw source title bytes explicitly
- added negative invariant proving name-only state does not invent a title

## Gate

```text
PASS INV_GV_165I V1 endgame prints raw source champion title after name
PASS INV_GV_165L V1 endgame does not invent champion title from name alone
# summary: 444/444 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire real recruitment/load so normal gameplay party state receives source title bytes
- parse source champion stat/skill text fields
- original overlay capture comparison
