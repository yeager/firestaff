# Pass490 — DM1 V1 wall occlusion merge-readiness sweep

Scope: DM1 V1 walls/occlusion/viewport draw order. Primary evidence is ReDMCSB, with Firestaff checked only for matching source shape.

## ReDMCSB anchors

- `DUNVIEW.C:8318-8542` (`F0128_DUNGEONVIEW_Draw_CPSF`) is the master wall/viewport replay order: D4 object lanes, D3L/D3R/D3C, D2L2/D2R2, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C.
- `DUNVIEW.C:6642-6720` (`F0118_DUNGEONVIEW_DrawSquareD3C_CPSF`) draws a center wall and returns unless an alcove wall ornament requires object drawing.
- `DUNVIEW.C:7391-7460` and `DUNVIEW.C:7559-7628` (`F0122`/`F0123`) show left/right D1 side-wall draw and return behavior.
- `DUNVIEW.C:7727-7872` (`F0124_DUNGEONVIEW_DrawSquareD1C`) gives the nearest center wall return and D1C wall bitmap source-row clipping anchor.
- `DUNGEON.C:2466-2707` (`F0172_DUNGEON_SetSquareAspect`) classifies wall/fake-wall/door-facing aspects before the draw functions consume them.

## Salvageable prior work

These existing N2 worktrees are source-aligned and still useful as merge evidence: pass365 side-lane occlusion (`31c6584`), pass368 solid-wall occlusion (`f1a926e`), pass372 door/solid follow-up (`fc62317`), pass395 viewport wall runtime path (`76e7fa9`), pass401 door occlusion (`c363bd1`), pass402 side content order (`1e0dba7`), pass404 center-blocker side-content preservation (`e3ee04e`), and pass442 consolidation (`8a2ce63`).

## Result

Pass490 adds a small current-head verifier/manifest instead of cherry-picking the large pass442 manifest. It verifies the ReDMCSB draw-order/return anchors and Firestaff hooks for nearest center blockers, side-lane occlusion, side contents, and side door occlusion.
