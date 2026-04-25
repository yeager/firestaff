# DM1 all-graphics phase 126 — endgame all base skill-title lines

## Problem

Pass 125 drew only the first visible source-style endgame skill-title line (`FIGHTER`). ReDMCSB `ENDGAME.C:F0444_STARTEND_Endgame` iterates all four base skills:

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

The V1 source-backed endgame path now iterates `CHAMPION_SKILL_COUNT` and draws all visible base skill-title lines:

- `FIGHTER`
- `NINJA`
- `PRIEST`
- `WIZARD`

Source behavior retained:

- skips level `<= 1`
- clamps level to `16`
- uses `G0428_apc_SkillLevelNames` spelling/order
- uses x=105 and y increment `+= 8` after the champion name/title row
- uses C13 lightest-gray text color through the current silver/C13 mapping

## Gate

Added invariant:

- `INV_GV_165G` — V1 endgame prints source ninja/priest/wizard skill-title lines

```text
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_165F V1 endgame prints source fighter skill-title line
PASS INV_GV_165G V1 endgame prints source ninja/priest/wizard skill-title lines
# summary: 439/439 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- champion portrait blits in C416–C419
- champion title text after name
- source-derived hidden/lifecycle skill-level computation rather than only stored base skill levels
- endgame timing/music/restart loop
- original overlay comparison captures
