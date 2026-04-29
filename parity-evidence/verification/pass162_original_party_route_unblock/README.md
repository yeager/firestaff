# Pass 162 — original party/control route unblock

Lane A gate from `DM1_V1_FINISH_PLAN.md`.

- run base: `<N2_RUNS>/20260429-042959-pass162-original-party-route-unblock`
- evidence root: `parity-evidence/verification/pass162_original_party_route_unblock`
- completed: 3
- errors: 0
- buckets: blocked/static-no-party=3

## Source locks

- `ENTRANCE.C` 857-883: F0441_STARTEND_ProcessEntrance sets G0298_B_NewGame=C099_MODE_WAITING_ON_ENTRANCE and only exits after command/key changes it; keyboard/mouse input is processed before dungeon load.
- `COMMAND.C` 346-352, 557, 2438-2455: C200_COMMAND_ENTRANCE_ENTER_DUNGEON maps to C407_ZONE_ENTRANCE_ENTER / Return and sets G0298_B_NewGame=C001_MODE_LOAD_DUNGEON; resume maps to saved game, credits loops.
- `REVIVE.C` 63-150: F0280_CHAMPION_AddCandidateChampionToParty is the source transition that increments/uses G0305_ui_PartyChampionCount; inventory-looking candidate frames are not enough without this semantic transition.
- `COMMAND.C` 231-238, 509-511: C160/C161 Resurrect/Reincarnate boxes are around y=86-142 or y=90-138, not y=165; pass162 retests the corrected source-box centers x=130/y=115 and x=186/y=115.

## Result matrix

- `pm_f1_panel_clicks_route_recheck` `DM -vv -sn -pm`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 17bd7e878157, 2456e272bbda, 2edc8130117c, 48ed3743ab6a, ceb0c2eec633, e8a02e3f9758 — `parity-evidence/verification/pass162_original_party_route_unblock/pm_f1_panel_clicks_route_recheck`
- `source_enter_zone_then_candidate_buttons` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 17bd7e878157, 28ac2881600f, 307323fbc1f7, 48ed3743ab6a, a914971a8a47, ceb0c2eec633, f3aae170c687 — `parity-evidence/verification/pass162_original_party_route_unblock/source_enter_zone_then_candidate_buttons`
- `source_enter_zone_then_reincarnate_box` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 17bd7e878157, 307323fbc1f7, 342d4e7bdac2, 48ed3743ab6a, 5f1fc0a1c08c, ceb0c2eec633, f938fdba63a1 — `parity-evidence/verification/pass162_original_party_route_unblock/source_enter_zone_then_reincarnate_box`

## Interpretation

A route is not overlay-ready merely because `pass80_original_frame_classifier` says `dungeon_gameplay`. This pass rejects any route containing known static no-party hash `48ed3743ab6a`/`082b4d249740`, then requires both dynamic input deltas and a party/control marker.
