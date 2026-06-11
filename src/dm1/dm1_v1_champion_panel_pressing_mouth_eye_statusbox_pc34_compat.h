#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PRESSING_MOUTH_EYE_STATUSBOX_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PRESSING_MOUTH_EYE_STATUSBOX_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPMESB_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34 67
#define DM1_V1_CPMESB_STATUS_BOX_HEIGHT_PC34 29
#define DM1_V1_CPMESB_STATUS_BOX_BYTE_COUNT_PC34 \
    (DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34 * \
     DM1_V1_CPMESB_STATUS_BOX_HEIGHT_PC34)

typedef enum DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPressPc34 {
    DM1_V1_CPMESB_PRESS_MOUTH_PC34 = 70,
    DM1_V1_CPMESB_PRESS_EYE_PC34 = 71
} DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPressPc34;

typedef struct DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPc34Contract {
    int contract_only;
    int champion_count;
    int status_box_first_zone;
    int status_box_last_zone;
    int status_box_width;
    int status_box_height;
    int status_box_stride;
    int status_box_fill_color;
    int ready_hand_slot;
    int action_hand_slot;
    int slot_box_size;
    int slot_box_normal_graphic;
    int slot_box_wounded_graphic;
    int slot_box_acting_graphic;
    int poison_label_graphic;
    int poison_label_zone;
    int mouth_command;
    int eye_command;
    const char *draw_slot_anchor;
    const char *draw_state_anchor;
    const char *slotbox_route_anchor;
    const char *leader_rotation_anchor;
    const char *poison_overlay_anchor;
    const char *defs_anchor;
} DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPc34Contract;

typedef struct DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 {
    int active_champion_index;
    int inventory_champion_index;
    int acting_champion_index;
    int leader_empty_handed;
    int poison_banner_active;
    int poison_banner_champion_index;
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPressPc34 press;
} DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34;

typedef struct DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 {
    int tick_index;
    int champion_index;
    int zone_id;
    int screen_x;
    int screen_y;
    int width;
    int height;
    int redraws_status_box;
    int status_box_fill_color;
    int status_box_fill_pixel_count;
    int ready_hand_relative_x;
    int action_hand_relative_x;
    int hand_relative_y;
    int ready_hand_graphic;
    int action_hand_graphic;
    int slot_box_pixel_count;
    uint8_t bytes[DM1_V1_CPMESB_STATUS_BOX_BYTE_COUNT_PC34];
} DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34;

typedef struct DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 {
    int valid;
    int rejected_invalid_champion;
    int rejected_invalid_press;
    int rejected_same_poison_champion;
    int active_champion_index;
    int inventory_champion_index;
    int acting_champion_index;
    int pressing_eye;
    int pressing_mouth;
    int g0331_presses_eye;
    int g0333_presses_mouth;
    int panel_redraw_route;
    int viewport_redraw_requested;
    int status_box_redraw_requested;
    int poison_banner_active;
    int poison_banner_champion_index;
    int poison_overlay_zone;
    int poison_overlay_graphic;
    int poison_overlay_on_different_champion;
    int poison_overlay_touches_status_box_bytes;
    int post_tick_changed_byte_count;
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34
        pre_tick[DM1_V1_CPMESB_CHAMPION_COUNT_PC34];
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34
        post_tick[DM1_V1_CPMESB_CHAMPION_COUNT_PC34];
} DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34;

const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPc34Contract *
dm1_v1_champion_panel_pressing_mouth_eye_statusbox_contract_pc34(void);

const char *
dm1_v1_champion_panel_pressing_mouth_eye_statusbox_source_evidence_pc34(void);

int dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 *input,
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 *out_plan);

#ifdef __cplusplus
}
#endif

#endif
