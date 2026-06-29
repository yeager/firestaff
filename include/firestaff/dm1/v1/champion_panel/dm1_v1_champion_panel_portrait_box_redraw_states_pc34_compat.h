#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_REDRAW_STATES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_REDRAW_STATES_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel portrait-box redraw-state matrix.
 *
 * Contract-only, no-asset fixture. This pins the CHAMDRAW.C F0291/F0292/F0296
 * event matrix that decides when the 67x29 C151..C154 status box is repainted,
 * when that repaint reaches the inventory portrait-box blit, and when the
 * redraw only cascades to status-hand/name/action-hand lanes.
 *
 * ReDMCSB source anchors:
 * - CHAMDRAW.C F0291:498-677 maps champion/inventory slot boxes and draws
 *   C033/C034/C035 hand-slot chrome before object icons.
 * - CHAMDRAW.C F0292:757-815 gates the status-box branch, fills the 67x29
 *   status box, calls F0354 only for the inventory champion, then arms only
 *   STATISTICS for that owner; non-inventory champions arm
 *   NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND instead.
 * - CHAMDRAW.C F0292:843-895 applies the PC34 C11 leader / C09 nonleader
 *   name-color cascade after leader changes.
 * - CHAMDRAW.C F0292:898-935 recomputes statistics and C033/C034 mouth/eye
 *   chrome after the status-box branch.
 * - CHAMDRAW.C F0292:1080-1110 redraws the action hand through F0291 and then
 *   clears all nine redraw bits.
 * - CHAMDRAW.C F0293:1117-1143 iterates active champions in index order.
 * - CHAMDRAW.C F0295/F0296:1153-1260 scans changed leader/status/inventory/
 *   chest icons, then requests an F0292 viewport redraw for the inventory
 *   owner when visible icon chrome changes.
 * - CHAMPION.C F0302:662-714 resolves the hand-slot pointer before the final
 *   F0292 redraw of the affected champion.
 * - DEFS.H C113..C116 define the 16x14 champion icon zones, C033/C034/C035
 *   define hand-slot chrome, and C151..C154 define the 67x29 status boxes.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPPBRS_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPPBRS_EVENT_COUNT_PC34 8
#define DM1_V1_CPPBRS_MAX_OPS_PC34 8

#define DM1_V1_CPPBRS_STATUS_BOX_WIDTH_PC34 67
#define DM1_V1_CPPBRS_STATUS_BOX_HEIGHT_PC34 29
#define DM1_V1_CPPBRS_STATUS_BOX_STRIDE_X_PC34 69
#define DM1_V1_CPPBRS_STATUS_BOX_ZONE_BASE_PC34 151
#define DM1_V1_CPPBRS_STATUS_NAME_ZONE_BASE_PC34 159
#define DM1_V1_CPPBRS_STATUS_TEXT_ZONE_BASE_PC34 163

#define DM1_V1_CPPBRS_CHAMPION_ICON_WIDTH_PC34 16
#define DM1_V1_CPPBRS_CHAMPION_ICON_HEIGHT_PC34 14
#define DM1_V1_CPPBRS_CHAMPION_ICON_ZONE_BASE_PC34 113

#define DM1_V1_CPPBRS_HAND_READY_LOCAL_X_PC34 4
#define DM1_V1_CPPBRS_HAND_ACTION_LOCAL_X_PC34 24
#define DM1_V1_CPPBRS_HAND_LOCAL_Y_PC34 10
#define DM1_V1_CPPBRS_HAND_BOX_SIZE_PC34 18

#define DM1_V1_CPPBRS_COLOR_NONLEADER_NAME_PC34 9
#define DM1_V1_CPPBRS_COLOR_LEADER_NAME_PC34 11
#define DM1_V1_CPPBRS_COLOR_STATUS_FILL_PC34 12

#define DM1_V1_CPPBRS_GFX_SLOT_NORMAL_PC34 33
#define DM1_V1_CPPBRS_GFX_SLOT_WOUNDED_PC34 34
#define DM1_V1_CPPBRS_GFX_SLOT_ACTING_PC34 35

#define DM1_V1_CPPBRS_MASK_NAME_TITLE_PC34 0x0080u
#define DM1_V1_CPPBRS_MASK_STATISTICS_PC34 0x0100u
#define DM1_V1_CPPBRS_MASK_LOAD_PC34 0x0200u
#define DM1_V1_CPPBRS_MASK_ICON_PC34 0x0400u
#define DM1_V1_CPPBRS_MASK_PANEL_PC34 0x0800u
#define DM1_V1_CPPBRS_MASK_STATUS_BOX_PC34 0x1000u
#define DM1_V1_CPPBRS_MASK_WOUNDS_PC34 0x2000u
#define DM1_V1_CPPBRS_MASK_VIEWPORT_PC34 0x4000u
#define DM1_V1_CPPBRS_MASK_ACTION_HAND_PC34 0x8000u

#define DM1_V1_CPPBRS_MASK_CLEAR_ALL_PC34 \
    (DM1_V1_CPPBRS_MASK_NAME_TITLE_PC34 | \
     DM1_V1_CPPBRS_MASK_STATISTICS_PC34 | \
     DM1_V1_CPPBRS_MASK_LOAD_PC34 | DM1_V1_CPPBRS_MASK_ICON_PC34 | \
     DM1_V1_CPPBRS_MASK_PANEL_PC34 | \
     DM1_V1_CPPBRS_MASK_STATUS_BOX_PC34 | \
     DM1_V1_CPPBRS_MASK_WOUNDS_PC34 | \
     DM1_V1_CPPBRS_MASK_VIEWPORT_PC34 | \
     DM1_V1_CPPBRS_MASK_ACTION_HAND_PC34)

#define DM1_V1_CPPBRS_MASK_NON_INVENTORY_CONTINUATION_PC34 \
    (DM1_V1_CPPBRS_MASK_NAME_TITLE_PC34 | \
     DM1_V1_CPPBRS_MASK_STATISTICS_PC34 | \
     DM1_V1_CPPBRS_MASK_WOUNDS_PC34 | \
     DM1_V1_CPPBRS_MASK_ACTION_HAND_PC34)

typedef enum {
    DM1_V1_CPPBRS_EVENT_PARTY_LEADER_ROTATION_PC34 = 0,
    DM1_V1_CPPBRS_EVENT_HAND_SLOT_SWAP_PC34 = 1,
    DM1_V1_CPPBRS_EVENT_STATUS_HAND_ROTATION_PC34 = 2,
    DM1_V1_CPPBRS_EVENT_MIRROR_CANDIDATE_OPEN_CLOSE_PC34 = 3,
    DM1_V1_CPPBRS_EVENT_CHEST_OPEN_CLOSE_PC34 = 4,
    DM1_V1_CPPBRS_EVENT_RESURRECT_PENDING_PC34 = 5,
    DM1_V1_CPPBRS_EVENT_CANDIDATE_PICK_PC34 = 6,
    DM1_V1_CPPBRS_EVENT_INVENTORY_OWNER_STATUS_BOX_PC34 = 7
} dm1_v1_cppbrs_event_pc34_compat_t;

typedef enum {
    DM1_V1_CPPBRS_OP_NONE_PC34 = 0,
    DM1_V1_CPPBRS_OP_F0302_RESOLVE_HAND_POINTER_PC34 = 302,
    DM1_V1_CPPBRS_OP_F0296_SCAN_CHANGED_ICONS_PC34 = 296,
    DM1_V1_CPPBRS_OP_F0291_DRAW_SLOT_PC34 = 291,
    DM1_V1_CPPBRS_OP_F0292_STATUS_FILL_PC34 = 1292,
    DM1_V1_CPPBRS_OP_F0354_PORTRAIT_BLIT_PC34 = 354,
    DM1_V1_CPPBRS_OP_F0292_NAME_COLOR_CASCADE_PC34 = 2292,
    DM1_V1_CPPBRS_OP_F0292_STATISTICS_CHROME_PC34 = 3292,
    DM1_V1_CPPBRS_OP_F0292_ACTION_HAND_PC34 = 4292,
    DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34 = 5292
} dm1_v1_cppbrs_op_pc34_compat_t;

typedef struct {
    int champion_index;
    int status_box_zone;
    int status_name_zone;
    int status_text_zone;
    int status_left;
    int status_top;
    int status_right;
    int status_bottom;
    int ready_hand_left;
    int action_hand_left;
    int hand_top;
    int hand_right;
    int hand_bottom;
    int champion_icon_zone;
    int champion_icon_width;
    int champion_icon_height;
} dm1_v1_cppbrs_geometry_pc34_compat_t;

typedef struct {
    dm1_v1_cppbrs_event_pc34_compat_t event;
    const char *name;
    int leader_before;
    int leader_after;
    int owner_before;
    int owner_after;
    int redraw_champion;
    uint16_t input_mask;
    uint16_t continuation_mask;
    bool calls_f0296;
    bool calls_f0302;
    bool calls_f0291;
    bool calls_f0292;
    bool fills_status_box;
    bool calls_f0354;
    bool name_color_cascade;
    bool statistics_chrome;
    bool action_hand_redraw;
    bool f0296_chrome_transition;
    bool f0296_suppressed_by_candidate;
    bool chest_panel_path;
    bool mirror_candidate_path;
    bool resurrect_pending_path;
    bool candidate_pick_path;
    bool inventory_owner_status_box_path;
    int operation_count;
    dm1_v1_cppbrs_op_pc34_compat_t operations[DM1_V1_CPPBRS_MAX_OPS_PC34];
    int leader_name_color[DM1_V1_CPPBRS_CHAMPION_COUNT_PC34];
} dm1_v1_cppbrs_event_row_pc34_compat_t;

typedef struct {
    bool contract_only;
    bool disjoint_from_pass673_portrait_state;
    bool disjoint_from_pass683_name_color;
    bool disjoint_from_pass764_second_leader_slot_priority;
    bool disjoint_from_pass765_status_hand_rotation;
    bool disjoint_from_f0354_blit_gate;
    dm1_v1_cppbrs_geometry_pc34_compat_t
        geometry[DM1_V1_CPPBRS_CHAMPION_COUNT_PC34];
    dm1_v1_cppbrs_event_row_pc34_compat_t rows[DM1_V1_CPPBRS_EVENT_COUNT_PC34];
    uint32_t deterministic_hash;
} dm1_v1_cppbrs_model_pc34_compat_t;

const char *dm1_v1_cppbrs_source_pc34(void);

bool dm1_v1_cppbrs_build_model_pc34(
    dm1_v1_cppbrs_model_pc34_compat_t *out_model);

const dm1_v1_cppbrs_event_row_pc34_compat_t *
dm1_v1_cppbrs_find_event_pc34(
    const dm1_v1_cppbrs_model_pc34_compat_t *model,
    dm1_v1_cppbrs_event_pc34_compat_t event);

#ifdef __cplusplus
}
#endif

#endif
