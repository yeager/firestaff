#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_MOUTH_EYE_RELEASE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_MOUTH_EYE_RELEASE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPMER_ICON_EYE_NOT_LOOKING_PC34 202
#define DM1_V1_CPMER_ICON_EYE_LOOKING_PC34 203
#define DM1_V1_CPMER_ICON_MOUTH_OPEN_PC34 205
#define DM1_V1_CPMER_ZONE_MOUTH_PC34 545
#define DM1_V1_CPMER_ZONE_EYE_PC34 546
#define DM1_V1_CPMER_VIEWPORT_NOT_DUNGEON_VIEW_PC34 0
#define DM1_V1_CPMER_DELAY_TICKS_PC34 8
#define DM1_V1_CPMER_SKILL_GROUP_COUNT_PC34 4
#define DM1_V1_CPMER_INVENTORY_ORDINAL_NONE_PC34 0
#define DM1_V1_CPMER_INVENTORY_ORDINAL_FIRST_PC34 1
#define DM1_V1_CPMER_INVENTORY_ORDINAL_LAST_PC34 4
#define DM1_V1_CPMER_OPERATION_CAPACITY_PC34 12

typedef enum DM1_V1_ChampionPanelMouthEyeReleaseActionPc34Compat {
    DM1_V1_CPMER_ACTION_MOUTH_PRESS_PC34 = 1,
    DM1_V1_CPMER_ACTION_MOUTH_RELEASE_PC34 = 2,
    DM1_V1_CPMER_ACTION_EYE_PRESS_PC34 = 3,
    DM1_V1_CPMER_ACTION_EYE_RELEASE_PC34 = 4
} DM1_V1_ChampionPanelMouthEyeReleaseActionPc34Compat;

typedef enum DM1_V1_ChampionPanelMouthEyeReleasePanelRoutePc34Compat {
    DM1_V1_CPMER_PANEL_ROUTE_NONE_PC34 = 0,
    DM1_V1_CPMER_PANEL_ROUTE_FOOD_WATER_POISON_PC34 = 1,
    DM1_V1_CPMER_PANEL_ROUTE_INVENTORY_PANEL_PC34 = 2,
    DM1_V1_CPMER_PANEL_ROUTE_SKILLS_STATISTICS_PC34 = 3,
    DM1_V1_CPMER_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34 = 4
} DM1_V1_ChampionPanelMouthEyeReleasePanelRoutePc34Compat;

typedef enum DM1_V1_ChampionPanelMouthEyeReleaseOpPc34Compat {
    DM1_V1_CPMER_OP_NONE_PC34 = 0,
    DM1_V1_CPMER_OP_SET_IGNORE_MOUSE_PC34 = 1,
    DM1_V1_CPMER_OP_SET_PRESSING_MOUTH_PC34 = 2,
    DM1_V1_CPMER_OP_RESET_IGNORE_MOUSE_PC34 = 3,
    DM1_V1_CPMER_OP_RESET_PRESSING_MOUTH_PC34 = 4,
    DM1_V1_CPMER_OP_HIDE_POINTER_PC34 = 5,
    DM1_V1_CPMER_OP_SET_HIDE_REQUEST_PC34 = 6,
    DM1_V1_CPMER_OP_DRAW_FOOD_WATER_POISON_PANEL_PC34 = 7,
    DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34 = 8,
    DM1_V1_CPMER_OP_DRAW_INVENTORY_PANEL_PC34 = 9,
    DM1_V1_CPMER_OP_SHOW_POINTER_PC34 = 10,
    DM1_V1_CPMER_OP_SET_PRESSING_EYE_PC34 = 11,
    DM1_V1_CPMER_OP_RESET_PRESSING_EYE_PC34 = 12,
    DM1_V1_CPMER_OP_DISCARD_INPUT_PC34 = 13,
    DM1_V1_CPMER_OP_DELAY_PC34 = 14,
    DM1_V1_CPMER_OP_DRAW_EYE_LOOKING_PC34 = 15,
    DM1_V1_CPMER_OP_DRAW_SKILLS_STATISTICS_PC34 = 16,
    DM1_V1_CPMER_OP_CLEAR_LEADER_HAND_NAME_PC34 = 17,
    DM1_V1_CPMER_OP_DRAW_OBJECT_PANEL_PC34 = 18,
    DM1_V1_CPMER_OP_DRAW_EYE_NOT_LOOKING_PC34 = 19,
    DM1_V1_CPMER_OP_CLEAR_SKILL_RECENTLY_UPGRADED_PC34 = 20,
    DM1_V1_CPMER_OP_DRAW_LEADER_HAND_NAME_PC34 = 21
} DM1_V1_ChampionPanelMouthEyeReleaseOpPc34Compat;

typedef struct DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat {
    bool contract_only;
    const char *mouth_press_anchor;
    const char *mouth_release_anchor;
    const char *eye_press_anchor;
    const char *eye_release_anchor;
    const char *slotbox_anchor;
    const char *chest_anchor;
    const char *command_queue_anchor;
    const char *icon_anchor;
    const char *zone_anchor;
    const char *viewport_anchor;
    const char *scope_note;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat {
    DM1_V1_ChampionPanelMouthEyeReleaseActionPc34Compat action;
    bool leader_empty_handed;
    bool left_button_down;
    bool panel_already_food_water_poisoned;
    bool leader_hand_has_object;
    int leader_hand_thing_before;
    int pending_hand_queue_count;
    int pending_hand_thing_before;
    int inventory_champion_ordinal;
} DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat;

typedef struct DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat {
    bool valid;
    bool contract_only;
    bool early_return;
    bool rejected_action;
    bool rejected_inventory_ordinal;
    bool ignore_mouse_movements;
    bool pressing_mouth;
    bool pressing_eye;
    bool pointer_hidden;
    bool pointer_shown;
    int hide_mouse_pointer_request_count;
    int icon_index;
    int icon_zone;
    int viewport_mode;
    int viewport_draw_count;
    int delay_ticks;
    int skill_recently_upgraded_clear_count;
    int leader_hand_thing_before;
    int leader_hand_thing_after;
    int pending_hand_queue_count_before;
    int pending_hand_queue_count_after;
    int pending_hand_thing_before;
    int pending_hand_thing_after;
    bool leader_hand_consumed;
    bool pending_hand_consumed;
    bool pending_queue_preserved;
    bool stale_panel_after;
    bool object_name_cleared;
    bool object_name_drawn;
    bool object_panel_inspect;
    DM1_V1_ChampionPanelMouthEyeReleasePanelRoutePc34Compat panel_route;
    DM1_V1_ChampionPanelMouthEyeReleaseOpPc34Compat
        operations[DM1_V1_CPMER_OPERATION_CAPACITY_PC34];
    int operation_count;
    const DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat *evidence;
} DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat;

const DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat *
DM1_V1_ChampionPanelMouthEyeRelease_EvidencePc34Compat(void);

const char *
DM1_V1_ChampionPanelMouthEyeRelease_SourceEvidencePc34Compat(void);

void DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input);

int DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(
    const DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input,
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *out_result);

#ifdef __cplusplus
}
#endif

#endif
