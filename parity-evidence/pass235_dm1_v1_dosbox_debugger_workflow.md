# Pass235 — DM1 PC34 DOSBox debugger workflow

Status: `PASS_ENTRY_CAPTURE_BLOCKED_SOURCE_SEAM_CSIP`

## Captured runtime evidence

- FIRES.EXENEW SHA256: `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`
- Static EXENEW entry: `0000:0000`
- Actual DOSBox debugger entry hit: `01ED:0000`
- Inferred PSP segment: `01DD`
- Transcript: `<firestaff-repo>/parity-evidence/verification/pass235_dm1_v1_dosbox_debugger_workflow/entry_debugger_transcript.clean.txt`

## Source seam audit

- PASS `command_accepted` — `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC`
- PASS `turn_handler` — `CLIKMENU.C:142-179` / `F0365_COMMAND_ProcessTypes1To2_TurnParty`
- PASS `move_handler` — `CLIKMENU.C:180-347` / `F0366_COMMAND_ProcessTypes3To6_MoveParty`
- PASS `move_result` — `MOVESENS.C:316-850` / `F0267_MOVE_GetMoveResult_CPSCE`
- PASS `draw_uses_mutated_tuple` — `GAMELOOP.C:55-95` / `F0002_MAIN_GameLoop_CPSDF`
- PASS `viewport_buffer_composed` — `DUNVIEW.C:8318-8611` / `F0128_DUNGEONVIEW_Draw_CPSF`
- PASS `viewport_present` — `DRAWVIEW.C:709-858` / `F0097_DUNGEONVIEW_DrawViewport`

## Command/movement/viewport CS:IP blocker

Actual loader entry CS:IP is captured, but COMMAND.C F0380, CLIKMENU movement, MOVESENS F0267, GAMELOOP/DUNVIEW/DRAWVIEW viewport breakpoints remain blocked by missing FIRES.EXENEW source-symbol/global-address map. Numeric DOSBox BP/BPM addresses are required before runtime hits can be claimed.

Runbook: `<firestaff-repo>/parity-evidence/verification/pass235_dm1_v1_dosbox_debugger_workflow/dosbox_debugger_runbook.md`
