#ifndef FIRESTAFF_CSB_V1_STARTUP_ENTRANCE_POINTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_ENTRANCE_POINTER_PC34_COMPAT_H

#include "entrance_mouse_routes_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CSB_V1_StartupEntrancePointerAction_PC34 {
    CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34 = 0,
    CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_DUNGEON_PC34 = 1,
    CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_BONUS_DUNGEON_PC34 = 2,
    CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_RESUME_PC34 = 3,
    CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_DRAW_CREDITS_PC34 = 4,
    CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34 = 5
} CSB_V1_StartupEntrancePointerAction_PC34;

int csb_v1_startup_entrance_pointer_action_pc34(
    int credits_active,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_StartupEntrancePointerAction_PC34 *out_action);

int csb_v1_startup_entrance_command_for_pointer_action_pc34(
    CSB_V1_StartupEntrancePointerAction_PC34 action);

const char *csb_v1_startup_entrance_pointer_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
