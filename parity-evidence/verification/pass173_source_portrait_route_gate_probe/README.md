# Pass 173 / pass 4 — gated source portrait route probe

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260429-080530-pass173-source-portrait-route-gate-probe`
- evidence root: `parity-evidence/verification/pass173_source_portrait_route_gate_probe`
- completed: 2
- errors: 0
- buckets: blocked/static-no-party-after-gate=2
- ReDMCSB source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## ReDMCSB source audit

This pass is source-first. The runtime clicks below are derived from these ReDMCSB anchors, not from emulator guessing.

- `DUNGEON.DAT via pass4 helper:n/a` — initial party location decodes to map0 x=1 y=3 dir=South; C127 sensor 16 is on wall square x=1 y=4, so the initial dungeon pose faces a champion portrait sensor.
- `COMMAND.C:397-403,2322-2323` — left-click in C007_ZONE_VIEWPORT dispatches C080_COMMAND_CLICK_IN_DUNGEON_VIEW and calls F0377_COMMAND_ProcessType80_ClickInDungeonView.
- `CLIKVIEW.C:348-349,407-431` — PC build subtracts viewport origin from screen coordinates; empty-hand click in C05 front-wall ornament/door-button zone calls F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor.
- `CLIKVIEW.C:21-25` — F0372 computes the square in front of the party and calls F0275_SENSOR_IsTriggeredByClickOnWall(frontX,frontY,oppositeDirection).
- `MOVESENS.C:1392,1501-1502` — F0275 allows C127_SENSOR_WALL_CHAMPION_PORTRAIT even with no leader, then calls F0280_CHAMPION_AddCandidateChampionToParty(sensorData).
- `REVIVE.C:63+` — F0280_CHAMPION_AddCandidateChampionToParty is the candidate-state creation entrypoint before C160/C161 are meaningful.
- `DUNGEON.C:2558,2608-2612` — while drawing wall square aspects, the same C127 sensor data sets G0289_i_DungeonView_ChampionPortraitOrdinal for visible champion portraits.
- `DUNVIEW.C:525,3913-3928` — portrait source box is {96,127,35,63}; drawing the D1C front wall copies/uses the C05 clickable zone and blits M635_ZONE_PORTRAIT_ON_WALL.
- `COORD.C:1693-1698` — PC viewport origin is x=0,y=33, so viewport portrait center (111,49) maps to screen x=111,y=82.
- `COMMAND.C:231-237,509-510` — candidate panel commands are C160/C161; old PC boxes include resurrect/reincarnate ranges around centers x=130,y=115 and x=186,y=115.

## Route precondition

- DM1 V1 initial party: map0 x=1 y=3 dir=South.
- Front wall square: map0 x=1 y=4 contains sensor 16 type C127 wall champion portrait.
- Therefore no movement is required; only entrance gate must be passed before clicking x=111,y=82.

## Results

- `gate_click_portrait_then_resurrect`: **blocked/static-no-party-after-gate** — known no-party hash present after gate: 48ed3743ab6a — `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect`
- `gate_click_portrait_then_reincarnate`: **blocked/static-no-party-after-gate** — known no-party hash present after gate: 48ed3743ab6a — `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate`
