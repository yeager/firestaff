# pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate

- status: `BLOCKED_PASS449_HALL_CANDIDATE_FRAMEBUFFER_ORIGINAL_ARTIFACTS_MISSING`
- redmcsb: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- parity claim: **not made**; this is a source-locked evidence path and blocker gate.

## Locked original data
- `dm1_pc34_english_graphics` `DM PC 3.4 English / I34E` `GRAPHICS.DAT` sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` bytes `363417` ok=True
- `dm1_pc34_english_dungeon` `DM PC 3.4 English / I34E` `DUNGEON.DAT` sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` bytes `33357` ok=True

## ReDMCSB source locks
- `PANEL.C:1619-1636` — Candidate panel uses graphic 40 and transparent palette index 6; PC34 route blits it into C101_ZONE_PANEL. ok=True
- `DEFS.H:2194-2201` — Resurrect/reincarnate panel graphic identity is the numbered C040 asset, not filename-only evidence. ok=True
- `DEFS.H:2078-2086` — Transparent color is palette index 6 (dark green). ok=True
- `DEFS.H:3774-3777` — The PC34 panel target is zone C101. ok=True
- `DATA.C:314-319` — The old PC panel bitmap box is viewport-relative x=80..223 y=52..124; when lifted to full screen this is x=80..223 y=85..157 because viewport y origin is 33. ok=True
- `PANEL.C:1654-1656` — Any candidate inventory redraw must show the Hall decision panel. ok=True
- `PANEL.C:2376-2385` — Candidate mode redraws inventory/panel but suppresses save/rest/close affordances. ok=True
- `REVIVE.C:272-294` — Portrait click appends a temporary candidate champion and updates leader/action UI according to party count. ok=True
- `REVIVE.C:744-783` — Cancel closes inventory, clears candidate ordinal, removes the candidate status/HUD slot, and redraws menus. ok=True
- `REVIVE.C:785-807` — Resurrect/reincarnate confirmation clears candidate mode, unlinks possessions, disables the mirror sensor, and branches to reincarnation rename. ok=True
- `COMMAND.C:1985-1991` — Panel choice commands dispatch only from the decision panel and only while the leader hand is empty. ok=True
- `COMMAND.C:228-240` — The Hall decision set is exactly resurrect/reincarnate/cancel. ok=True
- `COMMAND.C:2159-2184` — Status box/inventory toggles and inventory close are blocked while the candidate panel is active. ok=True
- `COMMAND.C:2336-2370` — Rest/wake/save menu paths cannot escape the Hall candidate modal. ok=True
- `CHAMDRAW.C:536-545` — Slot drawing is candidate-aware and prevents unrelated champion slots from overwriting candidate UI. ok=True
- `CHAMDRAW.C:1210-1212` — Changed leader-hand object drawing is suppressed if candidate mode has no inventory champion. ok=True
- `DUNVIEW.C:2463-2467` — Hall/prison map loads panel and portrait graphics together when map creature types allow it. ok=True
- `BASE.C:1341-1369` — Zone blit passes the requested transparent palette index through to the video blitter. ok=True
- `MEMORY.C:2474-2525` — Panel graphic bytes enter the bitmap path through the original memory/graphic expansion routines. ok=True

## Required deterministic capture scenes
- `candidate_select_portrait_click_before_panel`
- `candidate_panel_visible_after_append`
- `candidate_cancel_after_panel`
- `candidate_confirm_resurrect_after_panel`
- `candidate_confirm_reincarnate_after_panel`
- `hud_status_after_cancel`
- `hud_status_after_resurrect`
- `hud_status_after_reincarnate`

## Current artifacts
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/summary.json` exists=True classification=`blocked/static-no-party-after-gate` use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0011-after_portrait_click.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0012-after_c160_resurrect.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/summary.json` exists=True classification=`blocked/static-no-party-after-gate` use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0011-after_portrait_click.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0012-after_c161_reincarnate.png` exists=True dims=[320, 200] use=`review_only_not_promotable`
- `parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json` exists=True use=`review_only_not_promotable`

## Remaining blocker
Original PC34 Hall candidate select/panel/cancel/resurrect/reincarnate/HUD frames are not semantically promotable yet. The pass173 images are review clues only: they remain static/no-party after the gate, so they cannot prove panel pixel parity.

## Non-claims
No original-vs-Firestaff pixel parity, no candidate panel framebuffer parity, and no HUD/status pixel parity is claimed by this pass.
