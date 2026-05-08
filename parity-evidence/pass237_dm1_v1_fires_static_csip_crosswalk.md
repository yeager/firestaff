# Pass237 — DM1 PC34 FIRES static CS:IP crosswalk

Status: `CANDIDATE_ONLY_RUNTIME_HITS_REQUIRED`

This pass emits debugger follow-up candidates only. Nothing here is a `verified_runtime_hit`.

## MZ / image facts

- FIRES.EXENEW input: `<firestaff-repo>-dm1v1-viewport-walls-source-lock-20260506-0237/parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW`
- sha256: `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`
- MZ header bytes skipped for body disassembly: `10240`
- body size: `167984`
- MZ entry CS:IP: `0000:0000` (not a source seam)

## Candidate breakpoints

- `command_accepted` — `F0380_COMMAND_ProcessQueue_CPSC`
  - static candidate: `1b7c:06e9` / body linear `0x1c2a9`
  - confidence: `medium_high`; classification: `candidate_only`
  - promote blocker: Capture loaded FIRES PSP/load segment and debugger hit at this queue-dispatch prologue or branch after command dequeuing.
  - evidence: function prologue followed by queue-count guard at data 3e78 and queue index at 3ec8
  - evidence: branches compare accepted command in SI against 1/2 and 3..6
  - evidence: dispatch lcall targets at 1c3b6 -> 1771:010d and 1c3ca -> 1771:01aa
- `turn_types_1_to_2` — `F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - static candidate: `1771:010d` / body linear `0x1781d`
  - confidence: `high_static`; classification: `candidate_only`
  - promote blocker: Runtime hit on 1771:010d with command 1/2 and party-direction before/after values.
  - evidence: exact far-call operand from F0380 branch for command values 1 or 2
  - evidence: prologue sets movement redraw flag at 1a7c then selects zone constants 0x44/0x45 (C068/C069 turn arrows)
  - evidence: updates party direction through 0d06:000d before invoking movement result helper 1126:18ba
- `move_types_3_to_6` — `F0366_COMMAND_ProcessTypes3To6_MoveParty`
  - static candidate: `1771:01aa` / body linear `0x178ba`
  - confidence: `high_static`; classification: `candidate_only`
  - promote blocker: Runtime hit on 1771:01aa with command 3..6 and source/destination party coordinates.
  - evidence: exact far-call operand from F0380 branch for command values 3..6
  - evidence: prologue sets movement redraw flag at 1a7c then computes command-3 movement index
  - evidence: adds 0x46 to movement index, matching C070_ZONE_MOVE_FORWARD + movement-arrow index family
- `move_get_move_result` — `F0267_MOVE_GetMoveResult_CPSCE`
  - static candidate: `1126:0516` / body linear `0x11776`
  - confidence: `medium_high`; classification: `candidate_only`
  - promote blocker: Runtime hit on 1126:0516 plus watchpoints proving G0306/G0307 writes for the party path.
  - evidence: exact far-call target repeatedly used with five pushed arguments including 0xffff party thing sentinel
  - evidence: entry decodes thing cell bits with masks 0x3c00/0xc000 and branches on source/destination map state
  - evidence: called from F0365/F0366 static candidates as the legal step / collision resolver
- `viewport_game_loop_draw_call_site` — `F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF`
  - static candidate: `23cc:110e` / body linear `0x24dce`
  - confidence: `low_medium`; classification: `candidate_only`
  - promote blocker: Need FIRES.MAP or debugger stepping from F0128 through DUNVIEW helpers; current static evidence only identifies a viewport-adjacent loop cluster.
  - evidence: function calls input/timeline/viewport-adjacent routines and later conditionally calls 1126:0516 with party coordinates
  - evidence: candidate came from the decompressed-image control-flow cluster around pass234 viewport blocker follow-up
  - evidence: not yet separated into the exact F0098/F0104/F0108/F0109/F0115 draw helpers without FIRES.MAP

## Pattern guards

- `command_dispatch_lcall_turn`: 0x1c3b6
- `command_dispatch_lcall_move`: 0x1c3ca
- `turn_zone_44_45`: 0x1782d
- `move_zone_46_add`: 0x17917
- `move_result_far_call`: 0x00ccd, 0x00faf, 0x013ba, 0x01cec, 0x024bb, 0x044aa, 0x05d96, 0x05ee9, 0x0608d, 0x06116, 0x075f1, 0x07c48, 0x0c736, 0x0ced4, 0x0e749, 0x0e890, 0x15eeb, 0x15f19, 0x15f86, 0x16250, 0x17270, 0x1731b, 0x177c1, 0x179c4, 0x17b30, 0x24eac
- `viewport_cluster_entry`: 0x24dce

## Promotion blocker

Exact blocker to promote any candidate: capture runtime PSP/load segment and debugger-observed hit at the listed static CS:IP (runtime_cs = PSP + 0x10 + static_cs, runtime_ip = static_ip), plus seam-specific state/watchpoint evidence. Until then every row remains candidate_only, not verified_runtime_hit.
