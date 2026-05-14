# pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate

- status: `PASS_PASS449_CORRECTED_TERMINAL_FRAMEBUFFER_AND_HUD_INPUTS_COMPLETE`
- redmcsb: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- parity claim: **not made**; this is a source-locked evidence path and blocker gate.

## Locked original data
- `dm1_pc34_english_graphics` `DM PC 3.4 English / I34E` `GRAPHICS.DAT` sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` bytes `363417` ok=True
- `dm1_pc34_english_dungeon` `DM PC 3.4 English / I34E` `DUNGEON.DAT` sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` bytes `33357` ok=True

## ReDMCSB source locks
- `PANEL.C:F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1636` — Candidate panel uses graphic 40 and transparent palette index 6; PC34 route blits it into C101_ZONE_PANEL. ok=True
- `DEFS.H:C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE:2194-2201` — Resurrect/reincarnate panel graphic identity is the numbered C040 asset, not filename-only evidence. ok=True
- `DEFS.H:C06_COLOR_DARK_GREEN:2078-2086` — Transparent color is palette index 6 (dark green). ok=True
- `DEFS.H:C101_ZONE_PANEL:3774-3777` — The PC34 panel target is zone C101. ok=True
- `DATA.C:G0032_ai_Graphic562_Box_Panel:314-319` — The old PC panel bitmap box is viewport-relative x=80..223 y=52..124; when lifted to full screen this is x=80..223 y=85..157 because viewport y origin is 33. ok=True
- `PANEL.C:F0347_INVENTORY_DrawPanel:1654-1656` — Any candidate inventory redraw must show the Hall decision panel. ok=True
- `PANEL.C:F0355_INVENTORY_Toggle_CPSE:2376-2385` — Candidate mode redraws inventory/panel but suppresses save/rest/close affordances. ok=True
- `REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty:272-294` — Portrait click appends a temporary candidate champion and updates leader/action UI according to party count. ok=True
- `REVIVE.C:F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:744-783` — Cancel closes inventory, clears candidate ordinal, removes the candidate status/HUD slot, and redraws menus. ok=True
- `REVIVE.C:F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:785-807` — Resurrect/reincarnate confirmation clears candidate mode, unlinks possessions, disables the mirror sensor, and branches to reincarnation rename. ok=True
- `COMMAND.C:F0445_COMMAND_ProcessCommands160To162_ClickInPanel:1985-1991` — Panel choice commands dispatch only from the decision panel and only while the leader hand is empty. ok=True
- `COMMAND.C:G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel:228-240` — The Hall decision set is exactly resurrect/reincarnate/cancel. ok=True
- `COMMAND.C:F0446_COMMAND_ProcessCommands12To27_ClickInChampionStatusBox / F0448_COMMAND_ProcessCommands7To11_ToggleInventory:2159-2184` — Status box/inventory toggles and inventory close are blocked while the candidate panel is active. ok=True
- `COMMAND.C:F0449_COMMAND_ProcessCommands100To149_ClickInMenu:2336-2370` — Rest/wake/save menu paths cannot escape the Hall candidate modal. ok=True
- `CHAMDRAW.C:F0323_CHAMPION_DrawSlot:536-545` — Slot drawing is candidate-aware and prevents unrelated champion slots from overwriting candidate UI. ok=True
- `CHAMDRAW.C:F0333_CHAMPION_DrawChangedObjectIcons:1210-1212` — Changed leader-hand object drawing is suppressed if candidate mode has no inventory champion. ok=True
- `DUNVIEW.C:F0136_DUNGEONVIEW_LoadViewportGraphics:2463-2467` — Hall/prison map loads panel and portrait graphics together when map creature types allow it. ok=True
- `BASE.C:F0658_BlitBitmapIndexToZoneIndexWithTransparency:1341-1369` — Zone blit passes the requested transparent palette index through to the video blitter. ok=True
- `MEMORY.C:F0488_MEMORY_ExpandGraphicToBitmap / F0489_MEMORY_GetNativeBitmapOrGraphic:2474-2525` — Panel graphic bytes enter the bitmap path through the original memory/graphic expansion routines. ok=True

## Required deterministic capture scenes
- `candidate_select`
- `panel_visible`
- `cancel`
- `resurrect_confirm`
- `reincarnate_confirm`
- `hud_status_after`

## Exact framebuffer comparator manifest
- manifest: `parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/hall_candidate_framebuffer_manifest.json`
- schema: `parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/hall_candidate_framebuffer_manifest_schema.json`
- comparator result: `parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/hall_candidate_framebuffer_compare.json` status=`COMPARE_COMPLETE`
- required original data provenance: `GRAPHICS.DAT` and `DUNGEON.DAT` must include exact variant, file/path, bytes, and SHA256; filename-only identity is rejected.
- materialization: `MATERIALIZED_CORRECTED_TERMINAL_FRAMEBUFFER_AND_HUD_INPUTS_COMPLETE` from Firestaff root `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-pass449-firestaff-frames/framebuffer_inputs` and original root `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509`
- partial comparisons executed (diagnostic only; no full pixel parity claim):
  - `candidate_select` `fullframe` differingPixels=58965/64000 maxChannelDelta=255 meanAbsDeltaRgb=104.875698
  - `candidate_select` `hud_status_crop` differingPixels=8174/10560 maxChannelDelta=255 meanAbsDeltaRgb=80.281503
  - `panel_visible` `fullframe` differingPixels=58965/64000 maxChannelDelta=255 meanAbsDeltaRgb=104.878286
  - `panel_visible` `panel_crop` differingPixels=9692/10512 maxChannelDelta=255 meanAbsDeltaRgb=114.116787
  - `cancel` `fullframe` differingPixels=52937/64000 maxChannelDelta=255 meanAbsDeltaRgb=108.546599
  - `cancel` `panel_crop` differingPixels=9738/10512 maxChannelDelta=255 meanAbsDeltaRgb=131.599157
  - `cancel` `hud_status_crop` differingPixels=7225/10560 maxChannelDelta=219 meanAbsDeltaRgb=77.220644
  - `resurrect_confirm` `fullframe` differingPixels=6960/64000 maxChannelDelta=255 meanAbsDeltaRgb=12.779141
  - `resurrect_confirm` `panel_crop` differingPixels=1079/10512 maxChannelDelta=255 meanAbsDeltaRgb=11.379313
  - `resurrect_confirm` `hud_status_crop` differingPixels=2054/10560 maxChannelDelta=255 meanAbsDeltaRgb=19.956881
  - `reincarnate_confirm` `fullframe` differingPixels=54020/64000 maxChannelDelta=255 meanAbsDeltaRgb=109.378255
  - `reincarnate_confirm` `panel_crop` differingPixels=9738/10512 maxChannelDelta=255 meanAbsDeltaRgb=131.599157
  - `reincarnate_confirm` `hud_status_crop` differingPixels=8174/10560 maxChannelDelta=255 meanAbsDeltaRgb=80.281503
  - `hud_status_after` `fullframe` differingPixels=6960/64000 maxChannelDelta=255 meanAbsDeltaRgb=12.779141
  - `hud_status_after` `hud_status_crop` differingPixels=2054/10560 maxChannelDelta=255 meanAbsDeltaRgb=19.956881
- delta buckets: `DELTA_BUCKETS_CLASSIFIED_NO_PARITY_CLAIM`
  - highest-impact finding: HUD/status crop deltas for cancel/resurrect/reincarnate are dominated by compared visual-state buckets: Firestaff/original inputs alternate between all-black, Firestaff gray status HUD, and original brown Hall top band. This is a capture/semantic-stop alignment blocker before renderer pixel parity can be interpreted.
  - HUD/status summary: rows=5 bucketMismatches=5 zeroDeltaRows=0
  - parity eligibility: eligibleRows=12 maskedRows=3 maskedRowKeys=['cancel.hud_status_crop', 'reincarnate_confirm.hud_status_crop', 'resurrect_confirm.hud_status_crop']
  - semantic-stop mask: `FIRESTAFF_HUD_STATUS_SIDE_MASKED_PENDING_SOURCE_STOP_ALIGNMENT` policy=exclude affected HUD/status crop rows from renderer parity interpretation until Firestaff source-stop-aligned terminal HUD inputs exist
  - source refs: `REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty:272-294`, `REVIVE.C:F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:744-807`, `PANEL.C:F0355_INVENTORY_Toggle_CPSE:2376-2385`, `COMMAND.C:F0445_COMMAND_ProcessCommands160To162_ClickInPanel:1985-1991`
- `candidate_select` `firestaff` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/candidate_select/fullframe.png` hashField=`scenes.candidate_select.firestaff.fullframe.sha256` exists=True
- `candidate_select` `firestaff` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/candidate_select/hud_status_crop.png` hashField=`scenes.candidate_select.firestaff.hud_status_crop.sha256` exists=True
- `candidate_select` `original` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/candidate_select/fullframe.png` hashField=`scenes.candidate_select.original.fullframe.sha256` exists=True
- `candidate_select` `original` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/candidate_select/hud_status_crop.png` hashField=`scenes.candidate_select.original.hud_status_crop.sha256` exists=True
- `panel_visible` `firestaff` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/panel_visible/fullframe.png` hashField=`scenes.panel_visible.firestaff.fullframe.sha256` exists=True
- `panel_visible` `firestaff` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/panel_visible/panel_crop.png` hashField=`scenes.panel_visible.firestaff.panel_crop.sha256` exists=True
- `panel_visible` `original` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/panel_visible/fullframe.png` hashField=`scenes.panel_visible.original.fullframe.sha256` exists=True
- `panel_visible` `original` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/panel_visible/panel_crop.png` hashField=`scenes.panel_visible.original.panel_crop.sha256` exists=True
- `cancel` `firestaff` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/cancel/fullframe.png` hashField=`scenes.cancel.firestaff.fullframe.sha256` exists=True
- `cancel` `firestaff` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/cancel/panel_crop.png` hashField=`scenes.cancel.firestaff.panel_crop.sha256` exists=True
- `cancel` `firestaff` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/cancel/hud_status_crop.png` hashField=`scenes.cancel.firestaff.hud_status_crop.sha256` exists=True
- `cancel` `original` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/cancel/fullframe.png` hashField=`scenes.cancel.original.fullframe.sha256` exists=True
- `cancel` `original` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/cancel/panel_crop.png` hashField=`scenes.cancel.original.panel_crop.sha256` exists=True
- `cancel` `original` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/cancel/hud_status_crop.png` hashField=`scenes.cancel.original.hud_status_crop.sha256` exists=True
- `resurrect_confirm` `firestaff` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/resurrect_confirm/fullframe.png` hashField=`scenes.resurrect_confirm.firestaff.fullframe.sha256` exists=True
- `resurrect_confirm` `firestaff` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/resurrect_confirm/panel_crop.png` hashField=`scenes.resurrect_confirm.firestaff.panel_crop.sha256` exists=True
- `resurrect_confirm` `firestaff` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/resurrect_confirm/hud_status_crop.png` hashField=`scenes.resurrect_confirm.firestaff.hud_status_crop.sha256` exists=True
- `resurrect_confirm` `original` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/resurrect_confirm/fullframe.png` hashField=`scenes.resurrect_confirm.original.fullframe.sha256` exists=True
- `resurrect_confirm` `original` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/resurrect_confirm/panel_crop.png` hashField=`scenes.resurrect_confirm.original.panel_crop.sha256` exists=True
- `resurrect_confirm` `original` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/resurrect_confirm/hud_status_crop.png` hashField=`scenes.resurrect_confirm.original.hud_status_crop.sha256` exists=True
- `reincarnate_confirm` `firestaff` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/reincarnate_confirm/fullframe.png` hashField=`scenes.reincarnate_confirm.firestaff.fullframe.sha256` exists=True
- `reincarnate_confirm` `firestaff` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/reincarnate_confirm/panel_crop.png` hashField=`scenes.reincarnate_confirm.firestaff.panel_crop.sha256` exists=True
- `reincarnate_confirm` `firestaff` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/reincarnate_confirm/hud_status_crop.png` hashField=`scenes.reincarnate_confirm.firestaff.hud_status_crop.sha256` exists=True
- `reincarnate_confirm` `original` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/reincarnate_confirm/fullframe.png` hashField=`scenes.reincarnate_confirm.original.fullframe.sha256` exists=True
- `reincarnate_confirm` `original` `panel_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/reincarnate_confirm/panel_crop.png` hashField=`scenes.reincarnate_confirm.original.panel_crop.sha256` exists=True
- `reincarnate_confirm` `original` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/reincarnate_confirm/hud_status_crop.png` hashField=`scenes.reincarnate_confirm.original.hud_status_crop.sha256` exists=True
- `hud_status_after` `firestaff` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/hud_status_after/fullframe.png` hashField=`scenes.hud_status_after.firestaff.fullframe.sha256` exists=True
- `hud_status_after` `firestaff` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/firestaff/hud_status_after/hud_status_crop.png` hashField=`scenes.hud_status_after.firestaff.hud_status_crop.sha256` exists=True
- `hud_status_after` `original` `fullframe` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/hud_status_after/fullframe.png` hashField=`scenes.hud_status_after.original.fullframe.sha256` exists=True
- `hud_status_after` `original` `hud_status_crop` path=`parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/framebuffer_inputs/original/hud_status_after/hud_status_crop.png` hashField=`scenes.hud_status_after.original.hud_status_crop.sha256` exists=True

## N2 DOSBox original Hall panel-visible artifact
- root: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509` exists=True ok=True
- status: `NARROWED_ORIGINAL_HALL_PANEL_VISIBLE_CANDIDATE_CLICK_NO_TRANSITION` host=`firestaff-worker` created=`2026-05-09T14:15:00+02:00` entries=11
- use: original Hall/front-mirror visible context only; not a candidate panel framebuffer comparator input
- DUNGEON.DAT sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`; GRAPHICS.DAT sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`; TITLE sha256 `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`
- pc320 `pc320/03_panel_visible_north_front_mirror_pc320.png` sha256 `766c73a66f4d253f0b9e6e1df7bef2e945191a5f635eff87d9d381ce7d031ec0`
- viewport224x136 `viewport224x136/03_panel_visible_north_front_mirror_viewport224x136.png` sha256 `66a1f82c9a7a039918811efddee03dd07430e53f5dabb72d35adaabbd3d9189f`
- remaining blocker: candidate_select/cancel/resurrect_confirm/reincarnate_confirm/hud_status_after original true-stop frames and Firestaff-paired comparator inputs are still missing; N2 candidate clicks did not transition visibly.

## Current artifacts
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/summary.json` exists=True classification=`blocked/static-no-party-after-gate` use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0011-after_portrait_click.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0012-after_c160_resurrect.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/summary.json` exists=True classification=`blocked/static-no-party-after-gate` use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0011-after_portrait_click.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0012-after_c161_reincarnate.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json` exists=True use=`review_only_not_promotable`

## Remaining blocker
Corrected original `candidate_select`, `panel_visible`, `cancel`, `resurrect_confirm`, `reincarnate_confirm`, generic `hud_status_after`, and terminal-scoped HUD crops are now staged from initial-south corrected runs. Remaining work is pixel-delta parity triage, not missing framebuffer/HUD inputs.

## Non-claims
No original-vs-Firestaff pixel parity, no candidate panel framebuffer parity, and no HUD/status pixel parity is claimed by this pass.
