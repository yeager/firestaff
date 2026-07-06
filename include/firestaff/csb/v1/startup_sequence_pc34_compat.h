#ifndef FIRESTAFF_CSB_V1_STARTUP_SEQUENCE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_SEQUENCE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CSB_V1_StartupStage_PC34 {
    CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 = 1,
    CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 = 2,
    CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 = 3,
    CSB_V1_STARTUP_STAGE_ENTRANCE_LOAD_BLACK_PC34 = 4,
    CSB_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34 = 5,
    CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34 = 6,
    CSB_V1_STARTUP_STAGE_ENTRANCE_DOOR_OPENING_PC34 = 7,
    CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34 = 8
} CSB_V1_StartupStage_PC34;

typedef enum CSB_V1_StartupInput_PC34 {
    CSB_V1_STARTUP_INPUT_NONE_PC34 = 0,
    CSB_V1_STARTUP_INPUT_ACCEPT_PC34 = 1,
    CSB_V1_STARTUP_INPUT_ACTION_PC34 = 2,
    CSB_V1_STARTUP_INPUT_BACK_PC34 = 3,
    CSB_V1_STARTUP_INPUT_DISK_MENU_PC34 = 4
} CSB_V1_StartupInput_PC34;

typedef enum CSB_V1_StartupEntranceAction_PC34 {
    CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34 = 0,
    CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34 = 1,
    CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34 = 2,
    CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34 = 3
} CSB_V1_StartupEntranceAction_PC34;

const char* csb_v1_startup_stage_name_pc34(CSB_V1_StartupStage_PC34 stage);
int csb_v1_startup_stage_after_pc34(CSB_V1_StartupStage_PC34 later,
                                    CSB_V1_StartupStage_PC34 earlier);
int csb_v1_startup_title_total_ticks_pc34(void);
int csb_v1_startup_title_presents_ticks_pc34(void);
int csb_v1_startup_title_stage_for_frame_pc34(int frame);
int csb_v1_startup_entrance_wait_stage_pc34(void);
int csb_v1_startup_entrance_pre_open_delay_ticks_pc34(void);
int csb_v1_startup_entrance_action_for_input_pc34(
    int credits_active,
    CSB_V1_StartupInput_PC34 input);
int csb_v1_startup_sequence_source_order_valid_pc34(void);
const char* csb_v1_startup_sequence_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
