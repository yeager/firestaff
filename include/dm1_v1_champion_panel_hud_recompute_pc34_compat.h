/*
 * ReDMCSB source-lock anchors:
 * - CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 recomputes champion stat
 *   bars and the inventory mouth warning from food/water/poison state.
 * - PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:1945-1949 marks
 *   STATISTICS/PANEL and immediately calls F0292 for the champion panel.
 * - CHAMDRAW.C F0292_CHAMPION_DrawState:1060-1091 dispatches panel redraw
 *   and the action-hand/action-icon partial redraw path.
 */
#ifndef DM1_V1_CHAMPION_PANEL_HUD_RECOMPUTE_PC34_COMPAT_H
#define DM1_V1_CHAMPION_PANEL_HUD_RECOMPUTE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

int dm1_v1_champion_panel_hud_recompute(int champion_index,
                                         int poison_state,
                                         int food_state,
                                         int water_state,
                                         int *redraw_action_hand,
                                         int *redraw_action_icon,
                                         int *redraw_bars);

#ifdef __cplusplus
}
#endif

#endif
