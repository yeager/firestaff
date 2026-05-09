# Pass 166 — source portrait click route probe

- run base: `<N2_RUNS>/20260429-074347-pass166-source-portrait-click-route-probe`
- evidence root: `parity-evidence/verification/pass166_source_portrait_click_route_probe`
- completed: 2
- errors: 0
- buckets: blocked/static-no-party=2

## Source locks

- `DUNVIEW.C` 525: G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63}.
- `COORD.C` 1693,1698: PC viewport origin is x=0,y=33, making viewport center x=111,y=49 become screen x=111,y=82.
- `CLIKVIEW.C` 348-349,407-431: F0377 subtracts viewport origin, tests C05, and C05 calls F0372.
- `MOVESENS.C` 1392-1502: F0275 allows C127 with no leader and calls F0280_CHAMPION_AddCandidateChampionToParty.
- `COMMAND.C` 231-238,509-511: C160/C161 button centers remain x=130,y=115 and x=186,y=115 after candidate state.

## Result matrix

- `enter_portrait11182_then_resurrect` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 1339aaf0473c, 17bd7e878157, 307323fbc1f7, 48ed3743ab6a, 84171db7b6d4, a914971a8a47, c6f457763e2e — `parity-evidence/verification/pass166_source_portrait_click_route_probe/enter_portrait11182_then_resurrect`
- `enter_portrait11182_then_reincarnate` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 125549ae51c3, 17bd7e878157, 307323fbc1f7, 48ed3743ab6a, 4c74e1f91f3c, a914971a8a47, ceb0c2eec633 — `parity-evidence/verification/pass166_source_portrait_click_route_probe/enter_portrait11182_then_reincarnate`

## Interpretation

This is the first runtime probe after the ReDMCSB C127/F0280 source route and portrait geometry were locked by pass164/pass165. A pass requires no static no-party hash, a visible portrait-click candidate transition, post-choice input deltas, and a party/control marker.


## 2026-05-09 implementation lock

Added a pure compat gate for the source route in `dm1_v1_resurrection_pc34_compat.{h,c}`:

- `F0866_RESURRECTION_RouteChampionPortraitClick_Compat` models `COMMAND.C:2318-2324` C080 dispatch -> `CLIKVIEW.C:406-431` C05 front-wall ornament hit -> `CLIKVIEW.C:21-25` front-square/opposite-wall sensor touch -> `MOVESENS.C:1392-1395,1501-1503` C127 -> `REVIVE.C:124-132,272-276` candidate add.
- `F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat` models `REVIVE.C:744-785` cancel and `REVIVE.C:785-806` resurrect/reincarnate finalization; C160/C161/C162 are invalid unless candidate state already exists.

Narrow gate: `ctest --test-dir build -R dm1_v1_resurrection --output-on-failure` passes with `Results: 88/88 passed`.
