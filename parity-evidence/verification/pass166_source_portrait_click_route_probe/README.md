# Pass 166 — source portrait click route probe

- run base: `<N2_RUNS>/20260429-045405-pass166-source-portrait-click-route-probe`
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

- `enter_portrait11182_then_resurrect` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 1339aaf0473c, 17bd7e878157, 307323fbc1f7, 342d4e7bdac2, 48ed3743ab6a, 84171db7b6d4, ab7acc8ca298 — `parity-evidence/verification/pass166_source_portrait_click_route_probe/enter_portrait11182_then_resurrect`
- `enter_portrait11182_then_reincarnate` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 125549ae51c3, 17bd7e878157, 307323fbc1f7, 48ed3743ab6a, a914971a8a47, ceb0c2eec633, d16f5430cfd0 — `parity-evidence/verification/pass166_source_portrait_click_route_probe/enter_portrait11182_then_reincarnate`

## Interpretation

This is the first runtime probe after the ReDMCSB C127/F0280 source route and portrait geometry were locked by pass164/pass165. A pass requires no static no-party hash, a visible portrait-click candidate transition, post-choice input deltas, and a party/control marker.
