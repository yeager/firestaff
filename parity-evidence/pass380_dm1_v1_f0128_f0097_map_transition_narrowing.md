# Pass380 — DM1 V1 F0128/F0097 map vs transition narrowing

Status: `BLOCKED_PASS380_SOURCE_PATH_TRANSITION_NARROWED_MAP_BINDING_HELD`

## Decision

FIRES CS:IP mapping held: F0128 and F0097 rebase from FIRES.MAP with loader +0733, and the VIDRV call bytes remain bound at 20D6:1EFF -> 2809:1EFF. pass379 delivered route input after arming and retained the candidate breakpoints, but stopped nowhere at F0128/F0097/07FB; its forced pause decodes into IMAGE_TEXT near F0683, not the DUNVIEW/DRAWVIEW symbols. The remaining blocker is therefore the ReDMCSB source-path transition from delivered route input into F0380/F0365/F0366, G0321_B_StopWaitingForPlayerInput, and the next outer-loop F0128 draw; do not retarget F0128/F0097 CS:IP from pass379 alone.

## Evidence

- Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- FIRES map rebase: F0128 `1C7A:40FE` -> `23AD:40FE`; F0097 `20D6:1E31` -> `2809:1E31`.
- VIDRV call byte binding: `20D6:1EFF` -> `2809:1EFF` remains `26 ff 5f 24`.
- pass379 final forced pause decodes to `IMAGE_TEXT` near `F0683_COPYPIXELLINETOSCREENWITHT`; this is not F0128/F0097.

Manifest: `parity-evidence/verification/pass380_dm1_v1_f0128_f0097_map_transition_narrowing/manifest.json`
