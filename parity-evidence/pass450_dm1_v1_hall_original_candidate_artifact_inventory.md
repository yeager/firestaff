# pass450_dm1_v1_hall_original_candidate_artifact_inventory

- status: `BLOCKED_PASS450_PANEL_VISIBLE_ORIGINAL_FRAME_AVAILABLE_REMAINING_HALL_FRAMES_MISSING`
- parity claim: **not made**
- frame rows inventoried: 30

## Original data provenance
- `DM PC 3.4 English / I34E` `GRAPHICS.DAT` sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` bytes `363417` resolved `/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT` ok=True
- `DM PC 3.4 English / I34E` `DUNGEON.DAT` sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` bytes `33357` resolved `/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT` ok=True

## ReDMCSB source anchors
- `COMMAND.C:397-403,2322-2323` — Viewport left-click dispatches to the type-80 dungeon-view handler. ok=True
- `CLIKVIEW.C:21-25,348-349,407-431` — PC34 screen click is converted to viewport coordinates and front-wall sensor dispatch. ok=True
- `MOVESENS.C:1392,1501-1502` — C127 wall champion portrait sensor enters candidate champion creation. ok=True
- `DUNGEON.C:2558,2608-2612` — Visible wall portrait ordinal is sourced from the same C127 sensor data. ok=True
- `DUNVIEW.C:525,3913-3928` — D1C front-wall portrait draw/click geometry anchor. ok=True
- `COORD.C:1693-1698` — PC viewport origin maps source portrait center to screen x=111 y=82. ok=True
- `PANEL.C:1619-1636,1654-1656,2376-2385` — Candidate inventory redraw forces the resurrect/reincarnate panel and hides save/rest/close. ok=True
- `REVIVE.C:272-294,744-807` — Candidate append, cancel cleanup, confirm cleanup, sensor disable, and reincarnate branch. ok=True
- `COMMAND.C:228-240,1985-1991,2159-2184,2336-2370` — Panel command dispatch and candidate modal blocking for status/rest/save/close paths. ok=True
- `CHAMDRAW.C:536-545,1210-1212` — Candidate-aware champion slot/hand draw suppression. ok=True

## Existing reviewed frames
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0001-initial.png` sha12 `ceb0c2eec633` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0002-gate01.png` sha12 `014ed52c71a0` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0003-gate02.png` sha12 `014ed52c71a0` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0004-gate03.png` sha12 `014ed52c71a0` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0005-gate04.png` sha12 `3ad66a894ed8` dims=[320, 200] class=`graphics_320x200_unclassified` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0006-gate05.png` sha12 `28907074b501` dims=[320, 200] class=`graphics_320x200_unclassified` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0007-gate06.png` sha12 `6d24a74adfe4` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0008-gate07.png` sha12 `17bd7e878157` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0009-gate08.png` sha12 `17bd7e878157` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0010-gate09.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0011-after_portrait_click.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0012-after_c160_resurrect.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0013-after_confirm_return.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0014-after_f1_probe.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0015-after_f4_probe.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0001-initial.png` sha12 `ab7acc8ca298` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0002-gate01.png` sha12 `014ed52c71a0` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0003-gate02.png` sha12 `014ed52c71a0` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0004-gate03.png` sha12 `014ed52c71a0` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0005-gate04.png` sha12 `3ad66a894ed8` dims=[320, 200] class=`graphics_320x200_unclassified` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0006-gate05.png` sha12 `eded77444a51` dims=[320, 200] class=`graphics_320x200_unclassified` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0007-gate06.png` sha12 `4a9676290eb2` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0008-gate07.png` sha12 `17bd7e878157` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0009-gate08.png` sha12 `17bd7e878157` dims=[320, 200] class=`entrance_menu` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0010-gate09.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0011-after_portrait_click.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0012-after_c161_reincarnate.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0013-after_confirm_return.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0014-after_f1_probe.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0015-after_f4_probe.png` sha12 `48ed3743ab6a` dims=[320, 200] class=`dungeon_gameplay` pass173=`blocked/static-no-party-after-gate` use=`review_only_not_promotable_static_no_party`

## N2 DOSBox original Hall artifact
- root: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509` exists=True ok=True
- status: `NARROWED_ORIGINAL_HALL_PANEL_VISIBLE_CANDIDATE_CLICK_NO_TRANSITION` host=`firestaff-worker` created=`2026-05-09T14:15:00+02:00` entries=11
- promotable/narrowed label: `03_panel_visible_north_front_mirror` use=`panel_visible_original_hall_front_mirror_only_not_candidate_panel_parity`
- DUNGEON.DAT sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`; GRAPHICS.DAT sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`; TITLE sha256 `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`
- pc320 `pc320/03_panel_visible_north_front_mirror_pc320.png` sha256 `766c73a66f4d253f0b9e6e1df7bef2e945191a5f635eff87d9d381ce7d031ec0`
- viewport224x136 `viewport224x136/03_panel_visible_north_front_mirror_viewport224x136.png` sha256 `66a1f82c9a7a039918811efddee03dd07430e53f5dabb72d35adaabbd3d9189f`
- remaining blocker: candidate_select/cancel/resurrect_confirm/reincarnate_confirm/hud_status_after true-stop or transition frames remain missing; candidate clicks in this run did not visibly transition.

## Missing promotable scenes
- `candidate_select_portrait_click_before_panel`
- `candidate_cancel_after_panel`
- `candidate_confirm_resurrect_after_panel`
- `candidate_confirm_reincarnate_after_panel`
- `hud_status_after_cancel`
- `hud_status_after_resurrect`
- `hud_status_after_reincarnate`

## Capture tooling readiness
- local host capture ready: `True`
- platform: `Darwin` `arm64`
- dosbox: `/opt/homebrew/bin/dosbox`
- dosbox-x: `None`
- selected DOSBox: `/opt/homebrew/bin/dosbox`
- xvfb-run: `None` needsXvfb=`False` display=`None`
- stage exists: `True` `/Users/bosse/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`
- external artifact root: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/pass173_source_portrait_route_gate_probe` parentExists=`True`
- configured run base: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/pass173_source_portrait_route_gate_probe`
- missing tools/data: `[]`
- reason: capture-ready
- next step: `FIRESTAFF_ARTIFACT_ROOT=/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/pass173_source_portrait_route_gate_probe FIRESTAFF_DOSBOX=/opt/homebrew/bin/dosbox python3 tools/pass173_source_portrait_route_gate_probe.py`
- post-capture verification: `python3 tools/verify_pass450_dm1_v1_hall_original_candidate_artifact_inventory.py && python3 tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py`
