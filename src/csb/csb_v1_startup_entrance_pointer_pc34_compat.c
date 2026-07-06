#include "firestaff/csb/v1/startup_entrance_pointer_pc34_compat.h"

static CSB_V1_StartupEntrancePointerAction_PC34
csb_v1_startup_entrance_pointer_action_for_command(unsigned int command_id)
{
    switch (command_id) {
        case 200u:
            return CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_DUNGEON_PC34;
        case 201u:
            return CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_BONUS_DUNGEON_PC34;
        case 202u:
            return CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_RESUME_PC34;
        case 203u:
            return CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_DRAW_CREDITS_PC34;
        case 216u:
            return CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34;
        default:
            return CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34;
    }
}

int csb_v1_startup_entrance_pointer_action_pc34(
    int credits_active,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_StartupEntrancePointerAction_PC34 *out_action)
{
    EntranceMouseRouteCompat route;
    CSB_V1_StartupEntrancePointerAction_PC34 action =
        CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34;

    if (!out_action) {
        return 0;
    }
    *out_action = CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34;
    if (credits_active) {
        return 1;
    }
    if (!ENTRANCE_Compat_HitTestMouseRoute(
            x,
            y,
            button_mask,
            &route)) {
        return 0;
    }
    action = csb_v1_startup_entrance_pointer_action_for_command(
        route.commandId);
    *out_action = action;
    return action != CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34;
}

int csb_v1_startup_entrance_command_for_pointer_action_pc34(
    CSB_V1_StartupEntrancePointerAction_PC34 action)
{
    switch (action) {
        case CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_DUNGEON_PC34:
            return 200;
        case CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_BONUS_DUNGEON_PC34:
            return 201;
        case CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_RESUME_PC34:
            return 202;
        case CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_DRAW_CREDITS_PC34:
            return 203;
        case CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34:
            return 216;
        case CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34:
        default:
            return 0;
    }
}

const char *csb_v1_startup_entrance_pointer_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C F0806/F0438 entrance mouse-command startup; "
           "shared entrance_mouse_routes_pc34_compat command ids";
}
