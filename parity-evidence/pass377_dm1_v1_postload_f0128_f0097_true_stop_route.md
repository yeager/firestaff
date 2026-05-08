# Pass377 — DM1 V1 post-load F0128→F0097 true-stop route

Status: `BLOCKED_PASS377_POSTLOAD_F0128_TRUE_STOP_NOT_RECAPTURED`

## Source audit

- `CLIKMENU.C:142-174,180-237` ok=`True` anchors=`{'void F0365_COMMAND_ProcessTypes1To2_TurnParty': 142, 'G0321_B_StopWaitingForPlayerInput = C1_TRUE': 156, 'F0284_CHAMPION_SetPartyDirection': 138, 'void F0366_COMMAND_ProcessTypes3To6_MoveParty': 180, 'G0465_ai_Graphic561_MovementArrowToStepForwardCount': 220, 'G0466_ai_Graphic561_MovementArrowToStepRightCount': 221}`
- `DUNVIEW.C:8318-8338,8598-8612` ok=`True` anchors=`{'void F0128_DUNGEONVIEW_Draw_CPSF': 8318, 'P0183_i_Direction': 8320, 'P0184_i_MapX': 8320, 'P0185_i_MapY': 8320, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)': 8610}`
- `DRAWVIEW.C:709-724,849-858` ok=`True` anchors=`{'void F0097_DUNGEONVIEW_DrawViewport': 709, 'G0324_B_DrawViewportRequested = C1_TRUE': 721, 'M526_WaitVerticalBlank': 722, 'VIDRV_09_BlitViewPort': 857}`
- `VIDEODRV.C:18-34,940-951,3566-3582` ok=`True` anchors=`{'extern void F8161_VIDRV_09_BlitViewPort': 28, 'F8161_VIDRV_09_BlitViewPort': 28, '/*  9 */': 950, 'F8151_VIDRV_02_Blit': 21}`

## Runtime attempt

- Strategy: `post_load_arm_before_route`
- Method: run to stable load/menu prefix, pause to debugger prompt, arm F0128, then continue into route
- Direct hits: `{'f0128_23AD_40FE': False, 'f0097_2809_1EFF_after_f0128': False}`
- Stage/blocker: `F0128 breakpoint armed but no strict running-to-23AD:40FE prompt transition emitted`
- Transcript: `parity-evidence/verification/pass377_dm1_v1_postload_f0128_f0097_true_stop_route/post_load_arm_before_route.clean.txt`

## Decision

This is a controlled post-load arming attempt from the pass360/pass376 contract. It does not promote viewport parity unless strict post-Running F0128 and later F0097/VIDRV stops both appear in order.

Manifest: `parity-evidence/verification/pass377_dm1_v1_postload_f0128_f0097_true_stop_route/manifest.json`
