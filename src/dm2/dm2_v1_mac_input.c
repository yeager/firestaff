#include "dm2_v1_mac_input.h"

#include <stddef.h>

static void receipt(DM2_V1_MacInputReceipt *out,
                    DM2_V1_MacInputAction action,
                    int runtime_command)
{
    out->accepted = 1;
    out->action = action;
    out->runtime_command = runtime_command;
}

int dm2_v1_mac_input_resolve(uint32_t key,
                             uint32_t modifiers,
                             DM2_V1_MacInputPhase phase,
                             DM2_V1_MacInputReceipt *out)
{
    if (!out || phase < DM2_V1_MAC_INPUT_GAMEPLAY ||
        phase > DM2_V1_MAC_INPUT_CREDITS) {
        return 0;
    }
    out->accepted = 0;
    out->action = DM2_V1_MAC_ACTION_NONE;
    out->runtime_command = DM1_V1_COMMAND_NONE;

    if (modifiers & ~DM2_V1_MAC_MOD_COMMAND) {
        return 0;
    }
    if (modifiers & DM2_V1_MAC_MOD_COMMAND) {
        if (key == 'o' || key == 'O') {
            receipt(out, DM2_V1_MAC_ACTION_OPEN_GAME, DM1_V1_COMMAND_NONE);
        } else if (key == 's' || key == 'S') {
            receipt(out, DM2_V1_MAC_ACTION_SAVE_GAME, DM1_V1_COMMAND_SAVE_GAME);
        } else if (key == 'q' || key == 'Q') {
            receipt(out, DM2_V1_MAC_ACTION_QUIT, DM1_V1_COMMAND_NONE);
        }
        return out->accepted;
    }

    if (phase == DM2_V1_MAC_INPUT_ENTRANCE &&
        (key == DM2_V1_MAC_KEY_RETURN || key == DM2_V1_MAC_KEY_ENTER)) {
        receipt(out, DM2_V1_MAC_ACTION_NEW_GAME, DM1_V1_COMMAND_NONE);
        return 1;
    }
    if (phase == DM2_V1_MAC_INPUT_CREDITS &&
        (key == DM2_V1_MAC_KEY_RETURN || key == DM2_V1_MAC_KEY_ENTER)) {
        receipt(out, DM2_V1_MAC_ACTION_CLOSE_CREDITS, DM1_V1_COMMAND_NONE);
        return 1;
    }
    if (key == DM2_V1_MAC_KEY_RETURN || key == DM2_V1_MAC_KEY_ENTER) {
        receipt(out, DM2_V1_MAC_ACTION_WAKE, DM1_V1_COMMAND_NONE);
        return 1;
    }

    if (key == DM2_V1_MAC_KEY_ESCAPE || key == '`') {
        receipt(out, DM2_V1_MAC_ACTION_FREEZE, DM1_V1_COMMAND_FREEZE_GAME);
        return 1;
    }

    switch (key) {
    case '1': case DM2_V1_MAC_KEY_F1:
        receipt(out, DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_0,
                DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_0); return 1;
    case '2': case DM2_V1_MAC_KEY_F2:
        receipt(out, DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_1,
                DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_1); return 1;
    case '3': case DM2_V1_MAC_KEY_F3:
        receipt(out, DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_2,
                DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_2); return 1;
    case '4': case DM2_V1_MAC_KEY_F4:
        receipt(out, DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_3,
                DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_3); return 1;
    case ' ': receipt(out, DM2_V1_MAC_ACTION_TOGGLE_LEADER,
                      DM1_V1_COMMAND_TOGGLE_INVENTORY_LEADER); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_4: case DM2_V1_MAC_KEY_LEFT: case 'j': case 'J': case 'a': case 'A':
        receipt(out, DM2_V1_MAC_ACTION_TURN_LEFT, DM1_V1_COMMAND_TURN_LEFT); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_5: case DM2_V1_MAC_KEY_UP: case 'k': case 'K': case 's': case 'S':
        receipt(out, DM2_V1_MAC_ACTION_MOVE_FORWARD, DM1_V1_COMMAND_MOVE_FORWARD); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_6: case DM2_V1_MAC_KEY_RIGHT: case 'l': case 'L': case 'd': case 'D':
        receipt(out, DM2_V1_MAC_ACTION_TURN_RIGHT, DM1_V1_COMMAND_TURN_RIGHT); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_1: case 'm': case 'M': case 'z': case 'Z': case DM2_V1_MAC_KEY_DELETE:
        receipt(out, DM2_V1_MAC_ACTION_MOVE_LEFT, DM1_V1_COMMAND_MOVE_LEFT); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_2: case DM2_V1_MAC_KEY_DOWN: case ',': case 'x': case 'X': case DM2_V1_MAC_KEY_END:
        receipt(out, DM2_V1_MAC_ACTION_MOVE_BACKWARD, DM1_V1_COMMAND_MOVE_BACKWARD); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_3: case '.': case 'c': case 'C': case DM2_V1_MAC_KEY_PAGE_DOWN:
        receipt(out, DM2_V1_MAC_ACTION_MOVE_RIGHT, DM1_V1_COMMAND_MOVE_RIGHT); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_7: case 'u': case 'U': case 'q': case 'Q': case DM2_V1_MAC_KEY_F13:
        receipt(out, DM2_V1_MAC_ACTION_WALL_LEFT, DM1_V1_COMMAND_NONE); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_8: case 'i': case 'I': case 'w': case 'W': case DM2_V1_MAC_KEY_F14:
        receipt(out, DM2_V1_MAC_ACTION_WALL_CENTER, DM1_V1_COMMAND_NONE); return 1;
    case DM2_V1_MAC_KEY_NUMPAD_9: case 'o': case 'O': case 'e': case 'E': case DM2_V1_MAC_KEY_F15:
        receipt(out, DM2_V1_MAC_ACTION_WALL_RIGHT, DM1_V1_COMMAND_NONE); return 1;
    default:
        return 0;
    }
}

int dm2_v1_mac_input_enqueue(const DM2_V1_MacInputReceipt *input,
                             struct Dm1V1InputCommandQueuePc34Compat *queue)
{
    if (!input || !queue || !input->accepted ||
        input->runtime_command == DM1_V1_COMMAND_NONE) {
        return 0;
    }
    return DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
        queue, input->runtime_command, 0, 0);
}

const char *dm2_v1_mac_input_source_evidence(void)
{
    return "DMWeb: http://dmweb.free.fr/games/dungeon-master-ii/editions/macintosh/ (English Macintosh keyboard table)";
}
