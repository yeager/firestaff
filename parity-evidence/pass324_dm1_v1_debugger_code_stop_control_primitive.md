# Pass324 — DM1 V1 debugger code-stop/control primitive

Status: `PASS_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE_FOUND`

## ReDMCSB anchors audited first

- `DUNVIEW.C` F0128: `{'void F0128_DUNGEONVIEW_Draw_CPSF': 8318, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)': 8610}`
- `DRAWVIEW.C` F0097 / VIDRV: `{'void F0097_DUNGEONVIEW_DrawViewport': 709, 'VIDRV_09_BlitViewPort': 857}`
- `COMMAND.C` F0380 movement command dispatch: `{'void F0380_COMMAND_ProcessQueue_CPSC': 2045, 'F0365_COMMAND_ProcessTypes1To2_TurnParty': 2151, 'F0366_COMMAND_ProcessTypes3To6_MoveParty': 2155}`

## Result

A reliable primitive exists without tmux pane scraping: own the DOSBox-debug terminal with pexpect. The control/state syntax is:

- stopped prompt: `->`
- run control: F5, emitted as vt100 bytes `ESC O t`
- running marker: `(Running)`
- actual code stop: `(Running)` followed by reappearance of `->` plus refreshed code-view disassembly. DOSBox-debug 0.74-3 did **not** print a stable `breakpoint hit` line in this environment.

BPLIST/setup text is a negative control only. `Breakpoint list:` and `00. BP ...` are not stops.

Manifest: `parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive/manifest.json`
Transcript: `parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive/dosbox_debug_pty.clean.txt`
