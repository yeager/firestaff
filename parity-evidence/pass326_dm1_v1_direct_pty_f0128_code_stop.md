# Pass326 — direct PTY strict F0128 code-stop proof

Status: `BLOCKED_PASS326_DIRECT_PTY_F0128_CODE_STOP_NOT_PROVEN`

## ReDMCSB anchors audited first

- DUNVIEW.C / F0128: `{'void F0128_DUNGEONVIEW_Draw_CPSF': 8318, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)': 8610}`
- DRAWVIEW.C / F0097: `{'void F0097_DUNGEONVIEW_DrawViewport': 709, 'VIDRV_09_BlitViewPort': 857}`
- COMMAND.C / F0380: `{'void F0380_COMMAND_ProcessQueue_CPSC': 2045, 'F0365_COMMAND_ProcessTypes1To2_TurnParty': 2151, 'F0366_COMMAND_ProcessTypes3To6_MoveParty': 2155}`

## Proof harness

`tools/run_dosbox_debug_pty.py` owns `dosbox-debug` with pexpect under Xvfb. It arms `BP 23AD:40FE`, sends F5 as `ESC O t`, executes a bounded DM1 V1 in-game route, and records a real stop only when the PTY transcript shows `(Running)` followed by debugger prompt `->` and post-running code-view address `23AD:40FE`.

BPLIST/setup echoes are recorded as a negative control, not a stop source.

## Next blocker

Promote this direct-PTY primitive into the F0097/F0380 runtime-evidence probes: after strict F0128 stop, arm 2809:1EFF/2809:1E31 and 22F4:0699 without tmux scraping.

Manifest: `parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/manifest.json`
Transcript: `parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/dosbox_debug_pty.clean.txt`
