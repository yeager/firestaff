# C080 / Hall of Champions candidate handoff — source lock and Firestaff status

## ReDMCSB source audit

Primary source: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

The expected handoff chain is source-locked as:

`COMMAND.C C080 -> CLIKVIEW.C F0377/F0372 -> MOVESENS.C C127 -> REVIVE.C F0280`.

Exact citations:

- `COMMAND.C:106-114` maps movement-mode left-click in screen box `0..223,33..168` to `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`.
- `COMMAND.C:397-403` maps source zone `C007_ZONE_VIEWPORT` to `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`.
- `COMMAND.C:2322-2323` dispatches dequeued `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` to `F0377_COMMAND_ProcessType80_ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY)`.
- `CLIKVIEW.C:311` defines `F0377_COMMAND_ProcessType80_ClickInDungeonView`.
- `CLIKVIEW.C:348-349` normalizes PC coordinates by subtracting `G2067_i_ViewportScreenX` / `G2068_i_ViewportScreenY`.
- `CLIKVIEW.C:407-431` tests dungeon-view clickable cells through `C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT`; if not facing an alcove it calls `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor()`.
- `CLIKVIEW.C:5` defines `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor`.
- `MOVESENS.C:1392` allows `C127_SENSOR_WALL_CHAMPION_PORTRAIT` even when `G0411_i_LeaderIndex == CM1_CHAMPION_NONE`.
- `MOVESENS.C:1501-1502` maps `C127_SENSOR_WALL_CHAMPION_PORTRAIT` to `F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)`.
- `REVIVE.C:63-68` defines `F0280_CHAMPION_AddCandidateChampionToParty`.
- `DUNVIEW.C:525` defines the legacy portrait-on-wall box as viewport-relative `x=96..127, y=35..63`.
- `DUNVIEW.C:3913-3928` copies the front-wall draw/click zone into `C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT` and blits the champion portrait.
- `COORD.C:1693-1698` gives PC viewport origin `x=0, y=33`, so the safe portrait click center is screen `x=111, y=82`.
- `COMMAND.C:231-238` locks the PC resurrect/reincarnate/cancel boxes; safe centers are C160 `x=130,y=115` and C161 `x=186,y=115` after candidate state exists.

## Firestaff implementation status

No broad movement/viewport/touch rewrite was needed. The existing Firestaff path already matches the audited source chain narrowly:

- `m11_game_view.c:5537-5559` routes V1 chrome viewport clicks through the source mouse matrix and only accepts movement-list `C007 -> C080` before calling `m11_process_v1_c080_click`.
- `m11_game_view.c:7328-7334` normalizes click coordinates viewport-relative, matching `CLIKVIEW.C:F0377`.
- `m11_game_view.c:7352-7361` opens the source-backed mirror candidate panel only for the portrait box `x=96..127,y=35..63` with a front-cell mirror ordinal.
- `m11_game_view.c:5385-5408` handles the candidate panel before generic viewport clicks and maps C160/C161/C162 boxes to resurrect/reincarnate/cancel.
- `m11_game_view.c:5004-5035` selects the front mirror candidate; `m11_game_view.c:5038-5067` confirms it into the party.
- `m11_game_view.c:7437-7462` resolves the front mirror ordinal from source mirror `TextString` records in the front cell.

## Verification run

Commands run on N2 in `/home/trv2/work/firestaff`:

```sh
ctest --test-dir build -R "v1_champion_(recruit_source_path|portrait_click_source_path|portrait_click_geometry)|m11_game_view_probe" --output-on-failure
./build/firestaff_m11_game_view_probe | rg "INV_GV_40[567]|INV_GV_407A0|INV_GV_434|INV_GV_07I"
```

Results:

- `ctest`: 3/3 passed (`v1_champion_recruit_source_path`, `v1_champion_portrait_click_source_path`, `v1_champion_portrait_click_geometry`).
- Focused M11 probe excerpts passed:
  - `INV_GV_434` / `434A` / `434B` / `434C`: viewport C007 maps to C080 and respects boundaries.
  - `INV_GV_407A0`: non-portrait C007/C080 click does not use Firestaff procedural shortcuts or open the mirror panel.
  - `INV_GV_407A`: source portrait click center `x111/y82` opens the mirror candidate panel.
  - `INV_GV_407B`: source C160 center `x130/y115` confirms the candidate panel.
  - `INV_GV_407C`: source C161 center `x186/y115` confirms the candidate panel.
  - `INV_GV_407`: mirror panel resurrect command recruits the selected champion.
  - `INV_GV_07I0` / `07I1`: C080 front-door path ignores non-button clicks and only toggles on the source D1C door-button zone.

## Current blocker classification

Firestaff-side C080/Hall-of-Champions candidate handoff is not blocked in the current tree; it is source-locked and verified.

The remaining blocker described by `parity-evidence/verification/pass162_original_party_route_unblock/README.md` is narrower and belongs to the original DOSBox/runtime lane: the stock-original route reaches a `dungeon_gameplay` frame, but the `x=111,y=82` portrait click still produces no visible candidate transition. That now requires an in-process original queue/breakpoint trace to decide whether the mouse event reaches `COMMAND.C` enqueue/dequeue, `F0380 -> F0377`, or dies in DOSBox/PC mouse translation before ReDMCSB-visible logic. It is not a Firestaff movement/viewport/touch implementation gap.
