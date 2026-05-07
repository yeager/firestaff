# Pass316 — DM1 V1 F0380/F0097 direct runtime probe attempt

Status: `BLOCKED_DIRECT_F0380_F0097_AFTER_CODE_ONLY_PROBE`

## Result

- Direct F0380 dequeue hit at `22F4:0699`: `False`.
- Direct F0097 viewport-present hit at `2809:1E31`: `False`.
- Existing F0128 runtime hit at `23AD:40FE` stayed visible: `True`.
- pass316 does not promote any new runtime seam.

## Exact attempted command

`python3 tools/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.py --seconds 75`

## Narrowed blocker

Code-only BP probe armed 22F4:0699, 23AD:40FE, and 2809:1E31 under the pass278 DOSBox-debug/tmux/Xvfb/xdotool route. The run again captured only the existing F0128 hit. The F0380 BP command appears to collide during setup (22F4:0699 text appears while setting subsequent BPs, but BPLIST retains only 2809:1E31 and 23AD:40FE), so this is not promotable as a controlled dequeue hit. F0097 at 2809:1E31 did not stop before timeout.

Next missing input/tool: Need a debugger sequencing method that arms BP 22F4:0699 after the initial game-loop/setup collision without losing route control, or a validated finer F0097/VIDRV_09_BlitViewPort offset beyond 2809:1E31 that stops after F0128. pass312 must remain sourced from pass278/pass315, not this blocked attempt.

Manifest: `parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.json`.
