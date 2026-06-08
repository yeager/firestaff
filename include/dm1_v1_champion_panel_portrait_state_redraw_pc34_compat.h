#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_STATE_REDRAW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_STATE_REDRAW_PC34_COMPAT_H

/*
 * DM1 V1 champion panel portrait/state redraw contract.
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand:243-298
 *   updates the leader hand, dirties load, and calls F0292 for the leader.
 * - ReDMCSB CHAMDRAW.C F0291_CHAMPION_DrawSlot:551-552 reads the ready/action
 *   hand slot object, and 595-655 selects C033 normal, C034 wounded, or C035
 *   acting-hand slot box before blitting.
 * - ReDMCSB CHAMDRAW.C F0292_CHAMPION_DrawState:771-839 redraws a champion
 *   status box before continuing into live state details or the dead branch.
 * - ReDMCSB CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 uses C033/C034 for
 *   mouth and eye status overlays when statistics are recomputed.
 * - ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143 iterates
 *   every active champion and calls F0292 in champion-index order.
 * - ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1249-1252
 *   scans chest slot-box icons before requesting a viewport redraw through
 *   F0292 at 1254-1257.
 * - ReDMCSB DUNVIEW.C:3913-3928 draws the 32x29 champion portrait inside the
 *   224x136 viewport at viewport-local base (96,35).
 * - ReDMCSB DEFS.H:779-781, 1873-1878, 2188-2195, and DATA.C:264-272 define
 *   the two hand slots, C033/C034/C035 graphic cascade, and 4 champion x
 *   2 hand slot origins at championIndex*69 + {4,24}, y=10.
 *
 * Contract only: this helper is a synthetic, no-asset fixture for redraw order
 * and geometry gates. It does not render bitmaps or claim real-asset parity.
 */

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34 4
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATE_COUNT_PC34 8
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TABLE_COUNT_PC34 32

#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_BASE_X_PC34 96
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_BASE_Y_PC34 35
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_STRIDE_X_PC34 22
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_WIDTH_PC34 32
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_HEIGHT_PC34 29

#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_STRIDE_X_PC34 69
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_READY_HAND_LOCAL_X_PC34 4
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_ACTION_HAND_LOCAL_X_PC34 24
#define DM1_V1_CHAMPION_PANEL_STATE_REDRAW_HAND_LOCAL_Y_PC34 10

typedef enum {
    DM1_V1_CHAMPION_PANEL_STATE_OK_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_STATE_WOUNDED_PC34 = 1,
    DM1_V1_CHAMPION_PANEL_STATE_POISONED_PC34 = 2,
    DM1_V1_CHAMPION_PANEL_STATE_HUNGRY_PC34 = 3,
    DM1_V1_CHAMPION_PANEL_STATE_ASLEEP_PC34 = 4,
    DM1_V1_CHAMPION_PANEL_STATE_CONFUSED_PC34 = 5,
    DM1_V1_CHAMPION_PANEL_STATE_PARALYZED_PC34 = 6,
    DM1_V1_CHAMPION_PANEL_STATE_DEAD_PC34 = 7
} dm1_v1_champion_panel_state_redraw_state_pc34_compat_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_STATE_REDRAW_INVALID_PC34 = -1,
    DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C033_FRESH_BLIT_PC34 = 33,
    DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34 = 34,
    DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C035_DEAD_POLYGON_PC34 = 35
} dm1_v1_champion_panel_state_redraw_order_pc34_compat_t;

typedef struct {
    int champion_index;
    dm1_v1_champion_panel_state_redraw_state_pc34_compat_t state;
    dm1_v1_champion_panel_state_redraw_order_pc34_compat_t redraw_order;
    int portrait_x;
    int portrait_y;
    int portrait_w;
    int portrait_h;
    int ready_hand_x;
    int action_hand_x;
    int hand_y;
} dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t;

extern const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t
    dm1_v1_champion_panel_state_redraw_table
        [DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TABLE_COUNT_PC34];

int dm1_v1_champion_panel_state_redraw_order(
    int champion_index,
    dm1_v1_champion_panel_state_redraw_state_pc34_compat_t state);

bool dm1_v1_champion_panel_state_redraw_entry(
    int champion_index,
    dm1_v1_champion_panel_state_redraw_state_pc34_compat_t state,
    const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t **out_entry);

const char *dm1_v1_champion_panel_state_redraw_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_STATE_REDRAW_PC34_COMPAT_H */
