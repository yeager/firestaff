
#include "firestaff_input.h"
#include <string.h>

void fs_input_queue_init(FS_InputQueue *q) {
    if (q) memset(q, 0, sizeof(*q));
}

void fs_input_queue_push(FS_InputQueue *q, FS_Command cmd, int p1, int p2) {
    if (!q || q->count >= FS_INPUT_QUEUE_SIZE) return;
    int idx = (q->head + q->count) % FS_INPUT_QUEUE_SIZE;
    q->queue[idx].cmd = cmd;
    q->queue[idx].param1 = p1;
    q->queue[idx].param2 = p2;
    q->count++;
}

int fs_input_queue_pop(FS_InputQueue *q, FS_InputEvent *out) {
    if (!q || q->count <= 0) return 0;
    if (out) *out = q->queue[q->head];
    q->head = (q->head + 1) % FS_INPUT_QUEUE_SIZE;
    q->count--;
    return 1;
}

int fs_input_queue_count(const FS_InputQueue *q) {
    return q ? q->count : 0;
}

FS_Command fs_input_key_to_command(int scancode, int wasd_enabled) {
    /* SDL scancodes: Up=82, Down=81, Left=80, Right=79
     * WASD: W=26, A=4, S=22, D=7 */
    switch (scancode) {
        case 82: return FS_CMD_MOVE_FORWARD;
        case 81: return FS_CMD_MOVE_BACKWARD;
        case 80: return FS_CMD_TURN_LEFT;
        case 79: return FS_CMD_TURN_RIGHT;
        case 41: return FS_CMD_MENU;     /* Escape */
        case 44: return FS_CMD_ACTION;   /* Space */
        case 43: return FS_CMD_INVENTORY;/* Tab */
        default: break;
    }
    if (wasd_enabled) {
        switch (scancode) {
            case 26: return FS_CMD_MOVE_FORWARD;  /* W */
            case 22: return FS_CMD_MOVE_BACKWARD; /* S */
            case 4:  return FS_CMD_STRAFE_LEFT;   /* A */
            case 7:  return FS_CMD_STRAFE_RIGHT;  /* D */
            default: break;
        }
    }
    return FS_CMD_NONE;
}

