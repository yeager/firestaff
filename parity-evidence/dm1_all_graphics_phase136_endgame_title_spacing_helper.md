# DM1 all-graphics phase 136 — source endgame title spacing helper

## Problem

Endgame title x-position logic was inline in the renderer. The source rule is small but important:

- base x = `87 + strlen(Champion.Name) * 6`
- add one character gap (`+6`) unless `Champion.Title[0]` is `,`, `;`, or `-`

Keeping this inline made it harder to guard the punctuation edge cases.

## Change

Added a probe-visible helper:

```c
M11_GameView_EndgameTitleXForSourceText(name, title)
```

The renderer now uses that helper for raw `Champion.Title` placement.

## Gate

```text
PASS INV_GV_165M V1 endgame title x spacing honors source punctuation rule
# summary: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire real mirror recruitment/load into `ChampionState_Compat`
- original overlay comparison captures
