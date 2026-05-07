# Pass275 — DM1 V1 debugger BPM runtime hook attempt

Status: `BLOCKED_NO_PROVEN_RUNTIME_HOOK`

## What this pass did

- Re-audited the ReDMCSB command/movement/viewport source seams.
- Used pass273 public-symbol runtime addresses for BP/BPM setup.
- Started `DEBUG DM.EXE -vv -sn -pk` under `/usr/bin/dosbox-debug` with `TERM=vt100`.
- Confirmed debugger-run key workaround: `Escape O t` is vt100 F5; tmux `F5` emits the wrong `^[[15~` sequence in this environment.
- Posted controlled gameplay route input through Xvfb/xdotool.

## Result

- Transcript: `parity-evidence/verification/pass275_dm1_v1_debugger_bpm_runtime_hook_attempt/dosbox_debug_runtime_attempt.clean.txt`
- Listed debugger entries parsed: `1859:0516, 1EA4:010D, 1EA4:01AA, 23AD:40FE`
- Promotion decision: no verified runtime hook is claimed; the transcript does not yet prove the full key→queue→dequeue→mutated tuple→F0128 chain.
- Observed live BPM changes include `G0432/G0433/G0308/G0306/G0307` addresses, but they are not tied to the posted `kp5/kp4/kp6` route because early BPM stops pause the emulator while the synchronous xdotool injector continues posting keys.

## Exact next step

Replace the synchronous route injector with a two-loop driver: one thread posts `kp5/kp4/kp6` only while the emulator is running; the debugger loop must immediately capture CPU/MEMDUMP on each BP/BPM hit and continue with vt100 F5 (`Esc O t`). Then re-run with the same pass273 addresses.
