# Pass238 — DM1 PC34 runtime debugger keystroke blocker

Status: `NO_RUNTIME_HITS_CAPTURED_NO_PROMOTIONS`.
Classification: `blocked/debugger-keystroke-control-required`.

## Result

No pass237 `candidate_only` CS:IP was promoted to `verified_runtime_hit`. The N2 DOSBox debugger is present, but stdin/SSH automation did not reach the stopped debugger command prompt or acknowledge `BPINT 21 4B` / `BPLIST`.

Sanitized attempt transcript: `parity-evidence/verification/pass238_dm1_v1_runtime_debugger_keystroke_blocker/stdin_bpint_attempt_sanitized.txt`.

## Candidate runtime formulas

- `command_accepted` — static `1b7c:06e9` -> runtime `{load_segment + 0x1b7c}:0x06e9`; still `candidate_only`.
- `turn_types_1_to_2` — static `1771:010d` -> runtime `{load_segment + 0x1771}:0x010d`; still `candidate_only`.
- `move_types_3_to_6` — static `1771:01aa` -> runtime `{load_segment + 0x1771}:0x01aa`; still `candidate_only`.
- `move_get_move_result` — static `1126:0516` -> runtime `{load_segment + 0x1126}:0x0516`; still `candidate_only`.
- `viewport_game_loop_draw_call_site` — static `23cc:110e` -> runtime `{load_segment + 0x23cc}:0x110e`; still `candidate_only`.

## Exact manual runbook blocker

1. Create a scratch DOS directory outside the repo; copy the stock DM1 PC34 files and copy the local verified FIRES.EXENEW fixture as FIRES.EXE (do not commit it).
2. Start /usr/bin/dosbox-debug from a real terminal/console, not through stdin-only SSH automation, mounted to that scratch directory.
3. Press Alt+Pause/Break to force the debugger command prompt if the status line says Running.
4. At the debugger prompt enter: BPINT 21 4B; BPLIST; F5.
5. At the DOS prompt run FIRES.EXE. When EXEC breaks, continue/step until the child entry shows the decompressed FIRES first instruction at CS:0000; record that CS as load_segment.
6. Compute runtime breakpoints with runtime_cs = load_segment + static_cs, runtime_ip = static_ip.
7. Set at least these breakpoints for the movement-to-viewport chain: BP {load+0x1b7c}:06e9; BP {load+0x1771}:01aa; BP {load+0x1126}:0516; BP {load+0x23cc}:110e.
8. Run the route (one movement command after reaching a playable viewport). On every hit, record CS:IP, registers, nearby disassembly, and enough state to prove command_accepted -> movement_applied -> viewport_present.
9. Only after an actual breakpoint hit may a symbol-map entry become verified_runtime_hit; static CS:IP candidates alone remain candidate_only.

## Guardrail

The symbol map remains unpromoted. Static decompressed-image offsets and formulas are not runtime-hit proof.

Evidence manifest: `parity-evidence/verification/pass238_dm1_v1_runtime_debugger_keystroke_blocker/manifest.json`.
