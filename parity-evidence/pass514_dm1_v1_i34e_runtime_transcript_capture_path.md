# Pass514 - DM1 V1 PC/I34E runtime transcript capture path

Status: BLOCKED_PASS514_F0380_REACHED_WITH_EMPTY_QUEUE

## Source anchors

- COMMAND.C [1357, 1765]: f0361_keyboard_queue_write
- COMMAND.C [2045, 2155]: f0380_pop_count_dispatch
- GAMELOOP.C [166, 215]: keyboard_then_f0380_loop_order
- CLIKMENU.C [135, 269]: turn_step_party_tuple_handlers
- MOVESENS.C [316, 443]: successful_step_party_tuple_commit
- DUNVIEW.C [8318, 8610]: f0128_to_f0097_present_boundary
- DRAWVIEW.C [709, 857]: pc34_f0097_viewport_present

## Capture path

- N2-local original stage: ~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34
- Debugger stack: dosbox-debug + Xvfb + xdotool
- Exact command: python3 tools/verify_pass514_dm1_v1_i34e_runtime_transcript_capture_path.py --run-capture --seconds 45

## Decision

smallest N2 debugger path is wired, but unified capture is blocked because the pass388 keyboard route reaches F0380 with G2153 sampled as zero; exact command: python3 tools/verify_pass514_dm1_v1_i34e_runtime_transcript_capture_path.py --run-capture --seconds 45

## Evidence

- Manifest: parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json
