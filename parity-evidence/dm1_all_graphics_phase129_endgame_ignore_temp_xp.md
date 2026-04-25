# DM1 all-graphics phase 129 — endgame ignores temporary XP for skill titles

## Problem

`ENDGAME.C:F0444_STARTEND_Endgame` computes each visible skill title with:

```c
F0303_CHAMPION_GetSkillLevel(championIndex,
    skillIndex | (MASK0x4000_IGNORE_OBJECT_MODIFIERS |
                  MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE))
```

Pass 126 drew all four base skill lines, but it used the currently stored base `skillLevels[]`. That was useful as a bounded visual bridge, but did not encode the important endgame rule that temporary XP must be ignored.

## Change

Added `m11_endgame_source_skill_level(...)` for the V1 endgame path:

- calls `F0848_LIFECYCLE_ComputeSkillLevel_Compat(..., ignoreTemporary=1)`
- clamps to source max level `16`
- skips level `<= 1` through the existing endgame line logic
- currently keeps `ChampionState_Compat.skillLevels[]` as a fallback floor because the full raw champion/title/state carry-through is not complete yet

This narrows the endgame skill-title path toward source behavior without breaking existing Phase-10 state.

## Gate

Added invariant:

- `INV_GV_165J` — V1 endgame skill levels ignore temporary XP

```text
PASS INV_GV_165F V1 endgame prints source fighter skill-title line
PASS INV_GV_165G V1 endgame prints source ninja/priest/wizard skill-title lines
PASS INV_GV_165J V1 endgame skill levels ignore temporary XP
# summary: 442/442 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- raw DUNGEON.DAT `Champion.Title[20]` carried through `ChampionState_Compat`
- remove fallback bridge once base skill levels are always derived from lifecycle/source state
- ignore object modifiers exactly, if/when item modifiers are modeled in skill level computation
- endgame timing/music/restart loop
- original overlay comparison captures
