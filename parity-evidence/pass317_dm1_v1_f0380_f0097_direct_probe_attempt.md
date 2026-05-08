# Pass317 — DM1 V1 late-F0380/F0097 direct runtime probe attempt

Status: `BLOCKED_DIRECT_F0380_F0097_AFTER_LATE_F0380_PROBE`

## Result

- Direct F0380 dequeue hit at `22F4:0699`: `False`.
- Direct F0097 viewport-present hit at `2809:1E31`: `False`.
- Existing F0128 runtime hit at `23AD:40FE` stayed visible: `True`.
- pass317 does not promote any new runtime seam.

## Exact attempted command

`python3 tools/pass317_dm1_v1_late_f0380_f0097_direct_probe_attempt.py --seconds 70`

## Narrowed blocker

Late-F0380 BP probe armed 2809:1E31 and 23AD:40FE first, then 22F4:0699 last under the pass278 DOSBox-debug/tmux/Xvfb/xdotool route. The run again captured only the existing F0128 hit. Reversing the order moved the setup collision to F0097: BPLIST retained 22F4:0699 and 23AD:40FE, but no controlled F0380 dequeue hit was captured before timeout; F0097 was not retained, so no viewport-present hit is promotable.

Next missing input/tool: Need either a debugger sequencing method that can arm/retain BP 22F4:0699 after the setup-time collision while preserving route control, or a validated finer F0097/VIDRV_09_BlitViewPort offset beyond 2809:1E31 that stops after F0128. pass312 must remain sourced from pass278/pass315, not this blocked attempt.

Manifest: `parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt.json`.
