#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include <string.h>

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage) {
    switch (stage) {
        case DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34:
            return "SWSH_LOGO";
        case DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34:
            return "SWSH_RUN_START";
        case DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34:
            return "TITLE_BEGIN";
        case DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34:
            return "TITLE_LAST_FRAME";
        case DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34:
            return "MENU_ELIGIBLE";
        case DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34:
            return "ENTRANCE_WAIT";
    }
    return "UNKNOWN";
}

int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier) {
    return (unsigned int)later > (unsigned int)earlier;
}

int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path) {
    switch (path) {
        case DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34:
            return 0;
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34:
            /* M11_GameView_StartDm1() is a focused test/dev entry point.
             * It intentionally starts the DM1 game view directly and must
             * not be confused with the ReDMCSB SWSH -> TITLE -> ENTRANCE
             * launcher handoff used by normal DM1 startup. */
            return 1;
    }
    return 1;
}

int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed) {
    return sourceId && strcmp(sourceId, "dm1") == 0 && bypassed ? 1 : 0;
}

int dm1_v1_startup_sequence_source_order_valid_pc34(void) {
    /* ReDMCSB startup source order:
     * SWSH.C:39-47 runs START.PRG after the FTL palette program;
     * STARTUP1.C:143 calls TITLE.C F0437_STARTEND_DrawTitle();
     * TITLE.C:319-409 draws PRESENTS, title zoom, STRIKES BACK and final
     * guard; ENTRANCE.C:850-883 then enters the entrance wait loop.
     */
    return dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
                                           DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34);
}

const char* dm1_v1_startup_sequence_source_evidence_pc34(void) {
    return "ReDMCSB SWSH.C:39-47 -> STARTUP1.C:143 -> TITLE.C:319-409 -> ENTRANCE.C:850-883";
}
