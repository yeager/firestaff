#include "csb_v1_keyboard_commands_pc34_compat.h"

int CSB_V1_KeyboardCommandToMenuInputPc34Compat(
    CsbV1KeyboardKeyPc34Compat key,
    int ctrlDown,
    M12_MenuInput* outInput)
{
    M12_MenuInput mapped = M12_MENU_INPUT_NONE;

    if (!outInput) {
        return 0;
    }

    /* ReDMCSB COMMAND.C:245-260 / 263-305 define the CSB-family primary
     * and movement keyboard tables: F1-F4 select champion inventories,
     * Ctrl-S enters the save/disk menu, Esc freezes, and the movement
     * table maps Insert/Up/Clr Home/Left/Down/Right to turn/step/strafe.
     * COMMAND.C:308-310 maps Return to C146 wake while resting, and
     * COMMAND.C:708-717 maps Esc to unfreeze from the frozen table. */
    switch (key) {
    case CSB_V1_KEYBOARD_KEY_F1:
        mapped = M12_MENU_INPUT_CHAMPION_1_INVENTORY;
        break;
    case CSB_V1_KEYBOARD_KEY_F2:
        mapped = M12_MENU_INPUT_CHAMPION_2_INVENTORY;
        break;
    case CSB_V1_KEYBOARD_KEY_F3:
        mapped = M12_MENU_INPUT_CHAMPION_3_INVENTORY;
        break;
    case CSB_V1_KEYBOARD_KEY_F4:
        mapped = M12_MENU_INPUT_CHAMPION_4_INVENTORY;
        break;
    case CSB_V1_KEYBOARD_KEY_ESCAPE:
        mapped = M12_MENU_INPUT_FREEZE_TOGGLE;
        break;
    case CSB_V1_KEYBOARD_KEY_RETURN:
    case CSB_V1_KEYBOARD_KEY_ENTER:
        mapped = M12_MENU_INPUT_ACCEPT;
        break;
    case CSB_V1_KEYBOARD_KEY_S:
        if (!ctrlDown) {
            return 0;
        }
        mapped = M12_MENU_INPUT_DISK_MENU;
        break;
    case CSB_V1_KEYBOARD_KEY_INSERT:
        mapped = M12_MENU_INPUT_TURN_LEFT;
        break;
    case CSB_V1_KEYBOARD_KEY_UP:
        mapped = M12_MENU_INPUT_UP;
        break;
    case CSB_V1_KEYBOARD_KEY_CLR_HOME:
        mapped = M12_MENU_INPUT_TURN_RIGHT;
        break;
    case CSB_V1_KEYBOARD_KEY_LEFT:
        mapped = M12_MENU_INPUT_STRAFE_LEFT;
        break;
    case CSB_V1_KEYBOARD_KEY_DOWN:
        mapped = M12_MENU_INPUT_DOWN;
        break;
    case CSB_V1_KEYBOARD_KEY_RIGHT:
        mapped = M12_MENU_INPUT_STRAFE_RIGHT;
        break;
    default:
        return 0;
    }

    *outInput = mapped;
    return 1;
}

const char* CSB_V1_KeyboardCommandSourceEvidencePc34Compat(void)
{
    return "ReDMCSB COMMAND.C:245-260,263-305,308-310,708-717";
}
