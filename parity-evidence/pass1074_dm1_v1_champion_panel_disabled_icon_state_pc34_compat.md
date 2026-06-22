# pass1074_dm1_v1_champion_panel_disabled_icon_state_pc34_compat

- Status: PASS1074_DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_LOCKED

## Source-locked anchors

- CHAMPION.C F0330:2208-2255
- CHAMPION.C F0330:2252 M008_SET
- CHAMPION.C F0330:2253-2255 C11_EVENT_ENABLE_CHAMPION_ACTION
- ACTIDRAW.C F0386:201-296
- ACTIDRAW.C F0386_MENUS_DrawActionIcon:282-286
- ACTIDRAW.C F0386_MENUS_DrawActionIcon:234-238
- ACTIDRAW.C F0386_MENUS_DrawActionIcon:262-264
- MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]
- MENU.C:27,157
- m11_collect_v1_status_shield_border_graphics
- M11_GameView_ShouldHatchV1ActionIconCells
- DEFS.H MASK0x8000_ACTION_HAND
- DEFS.H MASK0x0008_DISABLE_ACTION

## Verification runs

- `/Volumes/Extern-disk/openclaw-work/firestaff/coding-worktrees/202606221508270004_q50_20260622_dm1_v1_champion_panel_shield_disabled_icon_gate/builds/nv1-build/test_dm1_v1_champion_panel_disabled_icon_state_pc34_compat`: rc=0 passes=153 fails=0 assertions=153
