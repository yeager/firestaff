# Pass329 — DM1 V1 direct-PTY breakpoint arming timing probe

Status: `BLOCKED_PASS329_CODE_STOP_TRANSITION_NOT_EMITTED`

## ReDMCSB anchors

- `DUNVIEW.C` F0128: `{'void F0128_DUNGEONVIEW_Draw_CPSF': 8318, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)': 8610}`
- `DRAWVIEW.C` F0097: `{'void F0097_DUNGEONVIEW_DrawViewport': 709, 'VIDRV_09_BlitViewPort': 857}`
- `COMMAND.C` F0380: `{'void F0380_COMMAND_ProcessQueue_CPSC': 2045, 'F0365_COMMAND_ProcessTypes1To2_TurnParty': 2151, 'F0366_COMMAND_ProcessTypes3To6_MoveParty': 2155}`

## Runtime decision

- Strategies: `['pre_arm_before_route', 'arm_at_stable_load_menu_prompt']`
- Direct hits: `[{'f0128_23AD_40FE': False, 'f0097_2809_1EFF_after_f0128': False}, {'f0128_23AD_40FE': False, 'f0097_2809_1EFF_after_f0128': False}]`
- Blocker: `F0128 breakpoint armed/retained but no strict running-to-23AD:40FE prompt transition emitted`

Manifest: `parity-evidence/verification/pass329_dm1_v1_direct_pty_breakpoint_arming_timing/manifest.json`
