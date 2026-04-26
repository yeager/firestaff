# DM1 all-graphics phase 2957–2976 — V1 endgame zone helpers

## Scope

Non-HUD UI pass: endgame/victory overlay geometry only. No viewport, inventory, title-cadence, or right-column HUD behavior changed.

## Source anchors

Existing endgame evidence ties `ENDGAME.C:F0444_STARTEND_Endgame` to these DM1 PC 3.4 surfaces:

- `C006_GRAPHIC_THE_END` = graphic `6`, 80×14
- `C346_GRAPHIC_WALL_ORNAMENT_43_CHAMPION_MIRROR` = graphic `346`, 48×43
- `C412..C415` champion mirror zones at `(19, 7 + 48n)`
- `C416..C419` champion portrait zones parented at `(+8,+6)` → `(27, 13 + 48n)`, 32×29
- champion name origin `(87, 14 + 48n)`
- source skill line origin `(105, 23 + 48n + 8m)` for visible skill lines
- restart outer/inner boxes `{103,140,115,15}` / `{105,142,111,11}`
- quit outer/inner boxes `{127,165,67,15}` / `{129,167,63,11}`

## Change

Added public V1 endgame geometry helpers and routed the existing source endgame renderer through them:

- `M11_GameView_GetV1EndgameTheEndZone`
- `M11_GameView_GetV1EndgameChampionMirrorZoneId/Zone`
- `M11_GameView_GetV1EndgameChampionPortraitZoneId/Zone`
- `M11_GameView_GetV1EndgameChampionNameOrigin`
- `M11_GameView_GetV1EndgameChampionSkillOrigin`
- `M11_GameView_GetV1EndgameRestartBox`
- `M11_GameView_GetV1EndgameQuitBox`

This removes another pocket of hardcoded endgame overlay geometry from the draw path and makes the non-HUD UI surface probeable/cherry-pickable.

## Gate

Added invariant:

- `INV_GV_300AP` — exposes source C412–C419, THE END, champion text/skill, and restart/quit geometry.

```text
PASS INV_GV_300AP V1 endgame zones expose source C412-C419, title, text, skill and restart/quit geometry
# summary: 536/536 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

