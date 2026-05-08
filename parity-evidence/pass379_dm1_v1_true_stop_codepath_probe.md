# Pass379 — DM1 V1 true-stop codepath probe

Status: `BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED`

## Runtime result

- Direct hits: `{'f0128_23AD_40FE': False, 'f0097_2809_1EFF': False, 'f0097_entry_2809_1E31': False, 'probe_07FB_01EB': False}`
- Saw running: `True`; route after arming: `True`
- Stage/blocker: `F0128/F0097/07FB breakpoints retained and route input delivered, but no strict candidate stop occurred; final forced pause landed at 280C:14B5, showing the pass377 07FB:01EB pause site is not stable and narrowing the blocker to FIRES CS:IP mapping/source-path rather than prompt or input delivery`
- Transcript: `parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/pass379_codepath_probe.clean.txt`

## Narrowing

07FB:01EB is used as a direct control-path probe because pass377 paused there after route while F0128 was retained. A strict post-Running stop there narrows the active blocker away from input delivery/debugger prompt control and toward candidate F0128/F0097 CS:IP mapping or a missing source-path transition.


## ReDMCSB source anchors

- `DUNVIEW.C:8318` — `F0128_DUNGEONVIEW_Draw_CPSF`; calls `F0097_DUNGEONVIEW_DrawViewport` at `DUNVIEW.C:8606` and `DUNVIEW.C:8610`.
- `DRAWVIEW.C:709` — `F0097_DUNGEONVIEW_DrawViewport`; indirect viewport blit via `VIDRV_09_BlitViewPort` at `DRAWVIEW.C:857`.
- `COMMAND.C:2045` — `F0380_COMMAND_ProcessQueue_CPSC`; movement command branches call `F0365_COMMAND_ProcessTypes1To2_TurnParty` at `COMMAND.C:2151` and `F0366_COMMAND_ProcessTypes3To6_MoveParty` at `COMMAND.C:2155`.

Manifest: `parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/manifest.json`
