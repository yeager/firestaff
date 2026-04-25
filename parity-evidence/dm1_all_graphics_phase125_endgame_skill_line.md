# DM1 all-graphics phase 125 — endgame skill-title line

## Problem

Pass 123 added champion names at the source endgame coordinates, but `ENDGAME.C:F0444_STARTEND_Endgame` also prints each champion's visible skill-title lines below the name/title header.

Source excerpt:

```c
for (AL1411_i_SkillIndex = C00_SKILL_FIGHTER; AL1411_i_SkillIndex <= C03_SKILL_WIZARD; AL1411_i_SkillIndex++) {
    L1412_i_SkillLevel = min(16, F0303_CHAMPION_GetSkillLevel(...));
    if (L1412_i_SkillLevel == 1)
        continue;
    strcpy(L1422_ac_String, G0428_apc_SkillLevelNames[L1412_i_SkillLevel - 2]);
    strcat(L1422_ac_String, " ");
    strcat(L1422_ac_String, G0417_apc_BaseSkillNames[AL1411_i_SkillIndex]);
    F0443_STARTEND_EndgamePrintString(105, AL1410_i_Y += 8, C13_COLOR_LIGHTEST_GRAY, L1422_ac_String);
}
```

## Change

Added the first bounded source-backed skill-title line in the V1 source endgame path:

- uses source skill level names (`NEOPHYTE`, `NOVICE`, `APPRENTICE`, `JOURNEYMAN`, ...)
- draws `LEVEL FIGHTER` for champion base fighter skill if level > 1
- source coordinate: `x=105`, `y=23 + championIndex*48`
- source colour: C13 lightest gray

This intentionally starts with the fighter line because the current runtime champion state only stores the four base skill levels directly; fuller hidden-skill/base-skill derivation remains separate work.

## Gate

Added invariant:

- `INV_GV_165F` — V1 endgame prints source skill-title line

```text
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_165F V1 endgame prints source skill-title line
# summary: 438/438 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- champion portrait blits in C416–C419
- title text after champion name
- all four base skill-title lines (fighter/ninja/priest/wizard)
- source-derived skill level computation from lifecycle/hidden skills
- endgame timing/music/restart loop
- original overlay comparison captures
