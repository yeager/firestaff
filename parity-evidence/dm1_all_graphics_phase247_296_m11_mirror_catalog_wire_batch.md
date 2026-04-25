# DM1 all-graphics phases 247–296 — M11 mirror catalog wiring batch

## Goal

Start wiring the real champion mirror catalog into the V1/M11 game-view layer. This is the first actual bridge from the DUNGEON.DAT-backed mirror catalog into game view state and party recruitment helpers.

## Passes / coverage

1. add `ChampionMirrorCatalog_Compat mirrorCatalog` to `M11_GameViewState`
2. add `mirrorCatalogAvailable` state flag
3. build catalog during `M11_GameView_Start` after `F0882_WORLD_InitFromDungeonDat_Compat`
4. expose M11 catalog count helper
5. expose M11 display-name-by-ordinal helper
6. expose M11 display-title-by-ordinal helper
7. expose M11 recruit-by-mirror-ordinal helper
8. expose M11 recruit-by-display-name helper
9. prove M11 starts with 24 real DM1 champion mirror records
10. prove ordinal 0 returns `DAROOU`
11. prove STAMM ordinal title returns `BLADECASTER`
12. prove M11 can recruit STAMM identity through catalog display name
13. prove recruited party state carries source `Name[8]`
14. prove recruited party state carries source `Title[20]`
15. prove M11 ordinal recruit helper is idempotent for already-present champion
16. fix M10 identity-only recruitment count after first-free-slot insertion
17. fix catalog recruitment count after first-free-slot insertion
18–50. run full M11/M10 regression after catalog wiring

## Added M11 API

```c
M11_GameView_GetMirrorCatalogCount(state)
M11_GameView_GetMirrorNameByOrdinal(state, ordinal, outName, outSize)
M11_GameView_GetMirrorTitleByOrdinal(state, ordinal, outTitle, outSize)
M11_GameView_RecruitChampionByMirrorOrdinal(state, ordinal)
M11_GameView_RecruitChampionByMirrorName(state, name)
```

## Important fix

The earlier first-free-slot recruitment helpers were safe for holes, but the count update still needed to be based on actual occupied slots after insertion. Fixed both M10 and catalog recruitment paths to set:

```c
party->championCount = F0638_PARTY_CountOccupiedChampionSlots_Compat(party);
```

instead of incrementing blindly / adding one after recount.

## Gate

```text
PASS INV_GV_400 M11 game view builds champion mirror catalog from DUNGEON.DAT at start
PASS INV_GV_401 M11 mirror catalog exposes display name by ordinal
PASS INV_GV_402 M11 mirror catalog exposes display title by ordinal
PASS INV_GV_403 M11 can recruit champion identity by mirror catalog display name
PASS INV_GV_404 M11 mirror ordinal recruit is idempotent for already-present champion
# summary: 450/450 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire actual mirror click/hit detection to `M11_GameView_RecruitChampionByMirrorOrdinal`
- decode encoded stat/skill/inventory payloads into runtime structures
- decide when identity-only recruitment becomes user-visible vs test/helper-only
