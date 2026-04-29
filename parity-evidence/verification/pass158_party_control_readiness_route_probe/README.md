# Pass 158 — party/champion-control readiness route probe

This consolidated pass probes champion selection / roster confirmation / party-control readiness before any dungeon movement claim.

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260429-012914-pass158-party-control-readiness-route-probe`
- evidence root: `parity-evidence/verification/pass158_party_control_readiness_route_probe`
- scenarios: 5
- completed: 5
- errors: 0
- buckets: blocked/static-no-party=5

## Source/evidence guidance used

- ReDMCSB Documentation/BugsAndChanges.htm BUG0_60 explicitly describes the original UI flow: start a new game, click a champion portrait, then wait for Reincarnate/Resurrect/Cancel.
- ReDMCSB Documentation/BugsAndChanges.htm BUG0_53/BUG0_43 describe candidate-champion inventory/spell states before adding/canceling; therefore inventory/spell-looking frames are not automatically party-control ready.
- pass151/pass153/pass157 evidence marks 48ed/082b dungeon frames as static no-party placeholders; this pass treats those as blocked unless recruit/readiness probes escape them.

## Action matrix

- `082b_seed_portrait_center_resurrect_confirm` `DM -vv -sn`: **blocked/static-no-party** — readiness probes collapsed to known no-party static dungeon hash — final `48ed3743ab6a`/`dungeon_gameplay` — readiness: f1_readiness=48ed3743ab6a/dungeon_gameplay, f4_readiness=48ed3743ab6a/dungeon_gameplay — `parity-evidence/verification/pass158_party_control_readiness_route_probe/082b_seed_portrait_center_resurrect_confirm`
- `082b_seed_portrait_center_reincarnate_confirm` `DM -vv -sn`: **blocked/static-no-party** — readiness probes collapsed to known no-party static dungeon hash — final `48ed3743ab6a`/`dungeon_gameplay` — readiness: f1_readiness=48ed3743ab6a/dungeon_gameplay, f4_readiness=48ed3743ab6a/dungeon_gameplay — `parity-evidence/verification/pass158_party_control_readiness_route_probe/082b_seed_portrait_center_reincarnate_confirm`
- `pm_f1_candidate_inventory_resurrect_then_readiness` `DM -vv -sn -pm`: **blocked/static-no-party** — readiness probes collapsed to known no-party static dungeon hash — final `48ed3743ab6a`/`dungeon_gameplay` — readiness: f1_readiness=48ed3743ab6a/dungeon_gameplay, f4_readiness=48ed3743ab6a/dungeon_gameplay — `parity-evidence/verification/pass158_party_control_readiness_route_probe/pm_f1_candidate_inventory_resurrect_then_readiness`
- `pm_f1_candidate_inventory_reincarnate_then_readiness` `DM -vv -sn -pm`: **blocked/static-no-party** — readiness probes collapsed to known no-party static dungeon hash — final `48ed3743ab6a`/`dungeon_gameplay` — readiness: f1_readiness=48ed3743ab6a/dungeon_gameplay, f4_readiness=48ed3743ab6a/dungeon_gameplay — `parity-evidence/verification/pass158_party_control_readiness_route_probe/pm_f1_candidate_inventory_reincarnate_then_readiness`
- `enter_f2_roster_candidate_buttons_then_readiness` `DM -vv -sn`: **blocked/static-no-party** — readiness probes collapsed to known no-party static dungeon hash — final `48ed3743ab6a`/`dungeon_gameplay` — readiness: f1_readiness=48ed3743ab6a/dungeon_gameplay, f4_readiness=48ed3743ab6a/dungeon_gameplay — `parity-evidence/verification/pass158_party_control_readiness_route_probe/enter_f2_roster_candidate_buttons_then_readiness`

## Evidence contents

Each scenario directory contains raw PNG captures, `summary.json`, `pass158_rows.json`, `pass158_driver.log`, annotated bboxes, crops for viewport/right/lower/top/movement/candidate-button regions, and an initial→result diff when pixels changed.
