# Pass 162 — original party/control route unblock

Lane A gate from `DM1_V1_FINISH_PLAN.md`.

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260430-050108-pass162-original-party-route-unblock`
- evidence root: `parity-evidence/verification/pass162_original_party_route_unblock`
- completed: 2
- errors: 0
- buckets: blocked/portrait-c080-no-visible-delta=2

## Source locks

- `DUNGEON.DAT via pass173 helper` n/a: Initial DM1 V1 party pose decodes to map0 x=1 y=3 dir=South (raw 0x0861), directly facing wall square x=1 y=4 with sensor 16 type C127 and sensorData=10; the deterministic route precondition is therefore an entrance gate only, not coordinate navigation.
- `ENTRANCE.C` 857-883: F0441_STARTEND_ProcessEntrance sets G0298_B_NewGame=C099_MODE_WAITING_ON_ENTRANCE and only exits after command/key changes it; keyboard/mouse input is processed before dungeon load.
- `COMMAND.C` 346-352, 557, 2438-2455: C200_COMMAND_ENTRANCE_ENTER_DUNGEON maps to C407_ZONE_ENTRANCE_ENTER / Return and sets G0298_B_NewGame=C001_MODE_LOAD_DUNGEON; resume maps to saved game, credits loops.
- `REVIVE.C` 63-150: F0280_CHAMPION_AddCandidateChampionToParty is the source transition that increments/uses G0305_ui_PartyChampionCount; inventory-looking candidate frames are not enough without this semantic transition.
- `COMMAND.C` 231-238, 509-511: C160/C161 Resurrect/Reincarnate boxes are around y=86-142 or y=90-138, not y=165; pass162 retests the corrected source-box centers x=130/y=115 and x=186/y=115.
- `COMMAND.C` 1-14, 108-114, 397-403, 1465-1662, 2045-2126, 2322-2323: The original command queue stores mouse-derived commands in G0432_as_CommandQueue; C007 viewport left-click maps to C080, F0365 enqueues nonzero mouse commands with X/Y, and F0380 dequeues them before dispatching C080 to F0377.
- `CLIKVIEW.C` 348-349, 407-431: PC click handling subtracts the viewport origin; an empty-hand hit in the C05 front-wall ornament zone calls F0372 to touch the front wall sensor.
- `MOVESENS.C` 1392, 1501-1502: C127_SENSOR_WALL_CHAMPION_PORTRAIT is allowed with no leader and calls F0280_CHAMPION_AddCandidateChampionToParty.
- `DUNVIEW.C / COORD.C` DUNVIEW.C 525,3913-3928; COORD.C 1693-1698: The portrait box is viewport x=96..127/y=35..63; PC viewport origin y=33 gives source screen click center x=111/y=82.

## Route precondition

- Initial party: map0 x=1 y=3 dir=South raw=0x0861.
- Front wall: x=1 y=4 sensor=16 type=C127_SENSOR_WALL_CHAMPION_PORTRAIT sensorData=10.
- No movement is required after the entrance gate; the runtime mismatch is now whether the original C080/F0377 click reaches F0280, not map navigation.

## Result matrix

- `source_gated_portrait_then_resurrect` `DM -vv -sn`: **blocked/portrait-c080-no-visible-delta** — gated gameplay reached, but source portrait click x=111/y=82 produced no visible candidate transition; blocker is now C007/C080 mouse delivery or front-wall hit-state mismatch before F0280 — unique hashes: 48ed3743ab6a, ceb0c2eec633 — `parity-evidence/verification/pass162_original_party_route_unblock/source_gated_portrait_then_resurrect`
- `source_gated_portrait_then_reincarnate` `DM -vv -sn`: **blocked/portrait-c080-no-visible-delta** — gated gameplay reached, but source portrait click x=111/y=82 produced no visible candidate transition; blocker is now C007/C080 mouse delivery or front-wall hit-state mismatch before F0280 — unique hashes: 48ed3743ab6a, ceb0c2eec633 — `parity-evidence/verification/pass162_original_party_route_unblock/source_gated_portrait_then_reincarnate`

## Interpretation

A route is not overlay-ready merely because `pass80_original_frame_classifier` says `dungeon_gameplay`. This pass now gates into real dungeon gameplay first, records the source-proven initial C127 pose, records xdotool focus/geometry/mouse delivery and the source C007->C080->F0380->F0377 queue path, then requires a visible source portrait/C080 candidate transition before C160/C161 and party-control markers. A zero-delta x=111/y=82 portrait click is now narrowed to an in-process original queue trace/breakpoint question: does C080 reach F0380/F0377 at all, or is DOSBox/PC mouse translation dying before the original queue? It is not a request for more map or panel coordinate guessing.
