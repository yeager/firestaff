# DM1 all-graphics phase 128 — endgame champion title text

## Problem

The source endgame draws each champion title immediately after the champion name on the same text row. Firestaff V1 had source name/portrait/skill lines, but title text was still missing.

Source anchor in `ENDGAME.C:F0444_STARTEND_Endgame`:

```c
F0443_STARTEND_EndgamePrintString(87, y += 14, C09_COLOR_GOLD, champion->Name);
x = (6 * strlen(champion->Name)) + 87;
first = champion->Title[0];
if ((first != ',') && (first != ';') && (first != '-')) {
    x += 6;
}
F0443_STARTEND_EndgamePrintString(x, y++, C09_COLOR_GOLD, champion->Title);
```

## Change

The V1 source endgame path now prints a champion title after known canonical DM1 champion names using the source x rule:

- `titleX = 87 + strlen(name) * 6`
- add 6 px unless the title starts with `,`, `;`, or `-`
- title y matches the name row (`14 + championIndex * 48`)
- C09/gold text path through the existing name style

The current Phase-10 `ChampionState_Compat` does not yet carry raw `Champion.Title[20]`, so this pass uses a bounded bridge table for the canonical names already exercised in V1 overlay/V2 portrait work (`ALEX`, `NABI`, `HALK`, `STAMM`, `SYRA`, `TIGGY`, `SONJA`). Carrying raw title bytes through the champion state remains tracked separately.

## Gate

Added invariant:

- `INV_GV_165I` — V1 endgame prints source champion title after name

```text
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_165I V1 endgame prints source champion title after name
PASS INV_GV_165F V1 endgame prints source fighter skill-title line
PASS INV_GV_165G V1 endgame prints source ninja/priest/wizard skill-title lines
# summary: 441/441 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- raw DUNGEON.DAT `Champion.Title[20]` carried through `ChampionState_Compat`
- source-derived hidden/lifecycle skill-level computation rather than only stored base skill levels
- endgame timing/music/restart loop
- original overlay comparison captures
