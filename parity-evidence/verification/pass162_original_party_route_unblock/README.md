# Pass 162 — original party/control route unblock

Lane A gate from `DM1_V1_FINISH_PLAN.md`.

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260429-042749-pass162-original-party-route-unblock`
- evidence root: `parity-evidence/verification/pass162_original_party_route_unblock`
- completed: 2
- errors: 0
- buckets: blocked/static-no-party=2

## Source locks

- `ENTRANCE.C` 857-883: F0441_STARTEND_ProcessEntrance sets G0298_B_NewGame=C099_MODE_WAITING_ON_ENTRANCE and only exits after command/key changes it; keyboard/mouse input is processed before dungeon load.
- `COMMAND.C` 346-352, 557, 2438-2455: C200_COMMAND_ENTRANCE_ENTER_DUNGEON maps to C407_ZONE_ENTRANCE_ENTER / Return and sets G0298_B_NewGame=C001_MODE_LOAD_DUNGEON; resume maps to saved game, credits loops.
- `REVIVE.C` 63-150: F0280_CHAMPION_AddCandidateChampionToParty is the source transition that increments/uses G0305_ui_PartyChampionCount; inventory-looking candidate frames are not enough without this semantic transition.

## Result matrix

- `pm_f1_panel_clicks_route_recheck` `DM -vv -sn -pm`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 17bd7e878157, 48ed3743ab6a, ab7acc8ca298, cb59cad5c225, dfd13eb966a2, f8d841d4357c — `parity-evidence/verification/pass162_original_party_route_unblock/pm_f1_panel_clicks_route_recheck`
- `source_enter_zone_then_candidate_buttons` `DM -vv -sn`: **blocked/static-no-party** — known static no-party hash present: 48ed3743ab6a — unique hashes: 014ed52c71a0, 17bd7e878157, 307323fbc1f7, 342d4e7bdac2, 48ed3743ab6a, a693ae7bf1b6, ceb0c2eec633, f3aae170c687 — `parity-evidence/verification/pass162_original_party_route_unblock/source_enter_zone_then_candidate_buttons`

## Interpretation

A route is not overlay-ready merely because `pass80_original_frame_classifier` says `dungeon_gameplay`. This pass rejects any route containing known static no-party hash `48ed3743ab6a`/`082b4d249740`, then requires both dynamic input deltas and a party/control marker.
