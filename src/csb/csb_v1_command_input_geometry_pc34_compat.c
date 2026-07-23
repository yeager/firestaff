#include "csb_v1_command_input_geometry_pc34_compat.h"

#include "dm1_v1_mouse_routes_pc34_compat.h"

#include <string.h>

const char* CSB_V1_CommandInputGeometrySourceEvidencePc34Compat(void)
{
    return "ReDMCSB COMMAND.C G0448_as_Graphic561_SecondaryMouseInput_Movement; "
           "F0358_COMMAND_GetCommandFromMouseInput_CPSC; "
           "F0359_COMMAND_ProcessClick_CPSC; "
           "F0380_COMMAND_ProcessQueue_CPSC; "
           "CLIKMENU.C F0365/F0366 C001-C006";
}

int CSB_V1_CommandInputGeometryFromPointerPc34Compat(
    int screen_x,
    int screen_y,
    int button_mask,
    CSB_V1_CommandInputGeometryResultPc34Compat* out_result)
{
    CSB_V1_CommandInputGeometryResultPc34Compat result;
    int coordinate_space = DM1_V1_MOUSE_SPACE_NONE_PC34;
    int command;

    if (!out_result) {
        return 0;
    }
    memset(&result, 0, sizeof(result));
    result.input = M12_MENU_INPUT_NONE;

    if ((button_mask & DM1_V1_MOUSE_MASK_LEFT_PC34) == 0) {
        *out_result = result;
        return 0;
    }

    command = DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
        DM1_V1_MOUSE_LIST_MOVEMENT_PC34,
        screen_x,
        screen_y,
        button_mask,
        &coordinate_space,
        &result.zone_id);
    if (coordinate_space != DM1_V1_MOUSE_SPACE_SCREEN_PC34) {
        *out_result = result;
        return 0;
    }

    switch (command) {
    case 1: result.input = M12_MENU_INPUT_TURN_LEFT; break;
    case 2: result.input = M12_MENU_INPUT_TURN_RIGHT; break;
    case 3: result.input = M12_MENU_INPUT_UP; break;
    case 4: result.input = M12_MENU_INPUT_STRAFE_RIGHT; break;
    case 5: result.input = M12_MENU_INPUT_DOWN; break;
    case 6: result.input = M12_MENU_INPUT_STRAFE_LEFT; break;
    default:
        *out_result = result;
        return 0;
    }

    result.matched = 1;
    result.command = command;
    *out_result = result;
    return 1;
}
