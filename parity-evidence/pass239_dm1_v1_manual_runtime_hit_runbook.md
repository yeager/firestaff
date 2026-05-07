# Pass239 — DM1 PC34 manual runtime-hit runbook

Status: `PASS_MANUAL_RUNTIME_HIT_PATH_EXECUTABLE_NO_PROMOTIONS`

## What this makes executable

A human can now enter the DOSBox debugger, set `BPINT 21 4B`, derive the child program load segment, compute numeric CS:IP breakpoints, and bind movement/viewport source seams to observed debugger hits.

## Exact runbook

1. Start from a stock DM1 PC34 directory outside this repo.
2. Copy the verified decompressed fixture as FIRES.EXE only in that scratch directory; never commit FIRES/FIRES.EXENEW.
3. Run /usr/bin/dosbox-debug from a real terminal/console. Do not use stdin-only SSH automation for this pass.
4. At the debugger prompt, set the loader breakpoint: BPINT 21 4B; BPLIST; F5.
5. At DOS C:\>, run FIRES.EXE. When INT 21 AH=4B breaks, note the parent PSP from registers if visible, then continue/step until the child entry line shows CS:0000 with the decompressed FIRES first instruction.
6. Record that child-entry CS as program_load_segment. Because FIRES.EXENEW has entry 0000:0000, PSP = program_load_segment - 0x10.
7. Compute each runtime breakpoint as runtime_cs = program_load_segment + static_cs and runtime_ip = static_ip.
8. Set the movement/viewport breakpoints printed below, press F5, enter/reach a playable viewport, then issue one movement command.
9. For each hit record: exact CS:IP, register dump, 8-16 nearby disassembly lines, command value/state if visible, and whether the sequence proves command accepted -> movement resolver -> viewport draw.
10. Do not edit data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json to verified_runtime_hit unless a debugger-observed hit at the computed CS:IP is captured.

## Candidate breakpoints

- `command_accepted` — `COMMAND.C` / `F0380_COMMAND_ProcessQueue_CPSC` — static `1b7c:06e9` -> `BP 1D69:06E9`; still `candidate_only` until hit.
- `turn_types_1_to_2` — `CLIKMENU.C` / `F0365_COMMAND_ProcessTypes1To2_TurnParty` — static `1771:010d` -> `BP 195E:010D`; still `candidate_only` until hit.
- `move_types_3_to_6` — `CLIKMENU.C` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` — static `1771:01aa` -> `BP 195E:01AA`; still `candidate_only` until hit.
- `move_get_move_result` — `MOVESENS.C` / `F0267_MOVE_GetMoveResult_CPSCE` — static `1126:0516` -> `BP 1313:0516`; still `candidate_only` until hit.
- `viewport_game_loop_draw_call_site` — `GAMELOOP.C` / `F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF` — static `23cc:110e` -> `BP 25B9:110E`; still `candidate_only` until hit.

## Validation

- dosbox-debug: `/usr/bin/dosbox-debug`; BPINT=`True`, BPLIST=`True`, MEMDUMP=`True`
- FIRES.EXENEW SHA256: `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`; entry `0000:0000`
- Captured/example program_load_segment: `01ED`; inferred PSP: `01DD`
- ReDMCSB source seam checks passed: `True`

## Guardrail

No candidate row is a verified_runtime_hit. Promote only after a real DOSBox debugger stop at the computed runtime CS:IP with registers/disassembly/state evidence for the movement-to-viewport chain.

Manifest: `<firestaff-repo>/parity-evidence/verification/pass239_dm1_v1_manual_runtime_hit_runbook/manifest.json`
