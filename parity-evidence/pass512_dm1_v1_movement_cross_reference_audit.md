# Pass512 - DM1 V1 movement cross-reference audit

Status: PASS512_DM1_V1_MOVEMENT_CROSS_REFERENCE_AUDIT

Scope: DM1 V1 movement/förflyttning only. This is evidence, not a runtime behavior change.

## Primary ReDMCSB locks

- PASS /Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/GAMELOOP.C:164-219 - keyboard input is drained through F0361 before F0380 processes the queue in the input wait loop
- PASS /Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/COMMAND.C:396-405,636-685,2045-2156 - mouse/key movement tables map to C001..C006 and F0380 dispatches turn/step commands after cooldown filtering
- PASS /Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/CLIKMENU.C:142-347 - F0365/F0366 own turning, relative stepping, collision, stairs, F0267 handoff, and movement cooldown
- PASS /Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/MOVESENS.C:438-497,760-818,1553-1794 - F0267 commits party X/Y and source-before-destination movement sensors; direction-gated sensors read G0308
- PASS /Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C:8318-8338,8468-8542 - F0128 consumes direction/mapX/mapY and derives visible squares with F0150

## Secondary source cross-checks

- PASS /Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/CSBCode.cpp:7845-8337 - CSBWin keeps TurnParty/MoveParty around partyFacing, MoveObject, d.partyX/Y, and partyMoveDisableTimer
- PASS /Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/Code11f52.cpp:2792-2866,2956-2960 - CSBWin MoveObject treats RNnul as party and commits d.partyX/Y
- PASS /Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp:6694-7170 - CSBWin DrawViewport consumes facing/x/y and marks the viewport updated
- PASS /Users/bosse/.openclaw/data/firestaff-csb-source/CSB/src/CSBCode.cpp:7868-8360 - CSB lineage preserves the same TurnParty/MoveParty party tuple and movement-disable semantics
- PASS /Users/bosse/.openclaw/data/firestaff-csb-source/CSB/src/Code11f52.cpp:2780-2920,3011-3012 - CSB lineage MoveObject commits d.partyX/Y for RNnul movement/teleport paths
- PASS /Users/bosse/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp:6701-7176 - CSB lineage DrawViewport consumes facing/x/y and marks viewport update

## Data anchors

- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE sha256 adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745
- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json result PASS with 0 mismatches.

## Current blocker evidence

- PASS parity-evidence/verification/pass509_dm1_v1_movement_n2_reference_anchor/manifest.json status PASS509_DM1_V1_MOVEMENT_N2_REFERENCE_ANCHORED
- PASS parity-evidence/verification/pass511_dm1_v1_movement_original_route_contract/manifest.json status PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED
- PASS parity-evidence/verification/pass508_dm1_v1_movement_remaining_gap_after_pass374/manifest.json status BLOCKED_PASS508_DM1_V1_MOVEMENT_REMAINING_ORIGINAL_OVERLAY_GAP_PROVED

Current blocker remains original-runtime evidence: a non-static PC/I34E keyboard-buffer/F0380 route transcript and representative original movement/HUD/viewport overlay captures tied to before/after party tuple.

Not claimed: pixel parity, viewport rendering changes, binary-level F0380 body breakpoint, or route promotion from Firestaff-only evidence.
