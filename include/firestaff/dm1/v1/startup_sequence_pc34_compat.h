#ifndef FIRESTAFF_DM1_V1_STARTUP_SEQUENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_STARTUP_SEQUENCE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_StartupStage_PC34 {
    DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34 = 1,
    DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34 = 2,
    DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34 = 3,
    DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34 = 4,
    DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34 = 5,
    DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34 = 6
} DM1_V1_StartupStage_PC34;

typedef enum DM1_V1_StartupLaunchPath_PC34 {
    DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34 = 1,
    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34 = 2,
    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34 = 3
} DM1_V1_StartupLaunchPath_PC34;

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage);
int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier);
int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path);
int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed);
int dm1_v1_startup_receipt_phase_pc34(int level_loaded,
                                      int intro_bypassed,
                                      char* out_phase,
                                      int out_phase_size);
int dm1_v1_startup_sequence_source_order_valid_pc34(void);
const char* dm1_v1_startup_sequence_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
