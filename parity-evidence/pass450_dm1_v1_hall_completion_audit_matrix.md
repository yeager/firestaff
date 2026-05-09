# pass450_dm1_v1_hall_completion_audit_matrix

- status: `PASS_WITH_BLOCKERS`
- scope: DM1 V1 Hall of Champions only
- evidence-backed completion: **80.0%** (8/10 matrix rows non-blocked)
- parity claim: **not 100%**; original PC34 Hall candidate framebuffer/HUD parity remains blocked.
- redmcsb: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## Locked original data
- `dm1_pc34_english_graphics` `DM PC 3.4 English / I34E` `GRAPHICS.DAT` sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` bytes `363417` ok=True
- `dm1_pc34_english_dungeon` `DM PC 3.4 English / I34E` `DUNGEON.DAT` sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` bytes `33357` ok=True

## Remaining-gap matrix

| area | status | ReDMCSB locks | evidence | gap |
|---|---|---|---|---|
| portrait click route | `VERIFIED_SOURCE_AND_RUNTIME` | `CLIKVIEW.C:347-349`, `CLIKVIEW.C:406-432`, `MOVESENS.C:1501-1503`, `DUNVIEW.C:525-525`, `COORD.C:1693-1749` | `tools/verify_dm1_v1_hall_of_champions_full_source_lock.py`, `tools/verify_v1_champion_portrait_click_source_path.py`, `tools/verify_v1_champion_portrait_click_geometry.py`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | none for stated source/runtime scope |
| candidate append | `VERIFIED_SOURCE_AND_RUNTIME` | `REVIVE.C:272-294` | `tools/verify_dm1_v1_hall_of_champions_full_source_lock.py`, `test_dm1_v1_resurrection_pc34_compat.c`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | none for stated source/runtime scope |
| panel active/display | `VERIFIED_SOURCE_AND_RUNTIME_PANEL_STATE_ONLY` | `PANEL.C:1619-1636`, `PANEL.C:1654-1656`, `DEFS.H:2194-2201`, `DEFS.H:3774-3777` | `tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | original panel pixels/crops blocked |
| cancel removal | `VERIFIED_SOURCE_AND_RUNTIME` | `REVIVE.C:744-783` | `tools/verify_dm1_v1_hall_of_champions_full_source_lock.py`, `test_dm1_v1_resurrection_pc34_compat.c`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | none for stated source/runtime scope |
| resurrect confirm | `VERIFIED_SOURCE_AND_RUNTIME` | `REVIVE.C:785-807` | `tools/verify_dm1_v1_hall_of_champions_full_source_lock.py`, `test_dm1_v1_resurrection_pc34_compat.c`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | none for stated source/runtime scope |
| reincarnate effects | `VERIFIED_SOURCE_AND_RUNTIME_CORE_EFFECTS` | `REVIVE.C:806-835` | `test_dm1_v1_resurrection_pc34_compat.c`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | none for stated source/runtime scope |
| mirror/sensor disable semantics | `VERIFIED_SOURCE_AND_UNIT` | `REVIVE.C:785-799`, `REVIVE.C:801-804`, `DUNGEON.C:2568-2583`, `MOVESENS.C:1390-1395` | `tools/verify_dm1_v1_hall_mirror_sensor_disable_source_lock.py`, `test_dm1_v1_resurrection_pc34_compat.c` | none for stated source/runtime scope |
| HUD/status text and modal blockers | `SOURCE_LOCKED_PARTIAL_RUNTIME` | `COMMAND.C:2159-2184`, `COMMAND.C:2336-2370`, `CHAMDRAW.C:536-545`, `CHAMDRAW.C:1210-1212`, `REVIVE.C:744-783` | `tools/verify_dm1_v1_hall_of_champions_full_source_lock.py`, `tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py`, `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c` | HUD/status text pixel parity and original crops blocked |
| graphics/palette/framebuffer parity | `BLOCKED_ORIGINAL_PROMOTABLE_FRAMES_MISSING` | `PANEL.C:1619-1636`, `DEFS.H:2078-2086`, `DATA.C:314-319`, `BASE.C:1341-1369`, `MEMORY.C:2474-2525` | `tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py`, `parity-evidence/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.md` | promotable original PC34 true-stop frames/crops missing |
| original PC34 frame/crop availability | `BLOCKED_ORIGINAL_TRUE_STOP_AND_SEMANTIC_FRAMES_MISSING` | `GAMELOOP.C:80-90`, `DRAWVIEW.C:709-722` | `parity-evidence/verification/pass173_source_portrait_route_gate_probe`, `parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json` | promotable original PC34 true-stop frames/crops missing |

## Hall runtime probe artifacts
- `parity-evidence/verification/pass450_dm1_v1_hall_completion_audit_matrix/hall_runtime_probe/dm1_v1_hall_walkaround_runtime_probe.json` exists=True sha256 `d785fffc6e0ce676d69203b663c577879931e2e60e7db8d5d0af6941991453f4` bytes `6268`
- `parity-evidence/verification/pass450_dm1_v1_hall_completion_audit_matrix/hall_runtime_probe/dm1_v1_hall_walkaround_runtime_probe.md` exists=True sha256 `a6b409569c1686e57db4de43a29608aa9b225280c5c060e6afcb6b0f3b22e7ac` bytes `2697`

## Required original scenes still missing
- `candidate_select_portrait_click_before_panel`
- `candidate_panel_visible_after_append`
- `candidate_cancel_after_panel`
- `candidate_confirm_resurrect_after_panel`
- `candidate_confirm_reincarnate_after_panel`
- `hud_status_after_cancel`
- `hud_status_after_resurrect`
- `hud_status_after_reincarnate`

## Bottom line
Hall source/runtime behavior is mostly locked, including portrait route, candidate append, cancel, confirm, reincarnate core effects, and first-sensor disable semantics. The blocker is not semantics; it is promotable original PC34 evidence: labelled true-stop fullframes/crops for the candidate panel/HUD scenes plus comparator JSON.
