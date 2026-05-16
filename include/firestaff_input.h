
#ifndef FIRESTAFF_INPUT_H
#define FIRESTAFF_INPUT_H

#include <stdint.h>

/* Firestaff Input — translate SDL/platform events to V1 game commands. */

typedef enum {
    FS_CMD_NONE = 0,
    FS_CMD_MOVE_FORWARD,
    FS_CMD_MOVE_BACKWARD,
    FS_CMD_TURN_LEFT,
    FS_CMD_TURN_RIGHT,
    FS_CMD_STRAFE_LEFT,
    FS_CMD_STRAFE_RIGHT,
    FS_CMD_ACTION,       /* Use/interact */
    FS_CMD_INVENTORY,    /* Toggle inventory */
    FS_CMD_SPELL,        /* Open spell panel */
    FS_CMD_MENU,         /* Escape/menu */
    FS_CMD_SCREENSHOT,
    FS_CMD_CLICK,        /* Mouse click at x,y */
} FS_Command;

typedef struct {
    FS_Command cmd;
    int param1;  /* click x or other param */
    int param2;  /* click y */
} FS_InputEvent;

#define FS_INPUT_QUEUE_SIZE 32

typedef struct {
    FS_InputEvent queue[FS_INPUT_QUEUE_SIZE];
    int head, tail, count;
} FS_InputQueue;

void fs_input_queue_init(FS_InputQueue *q);
void fs_input_queue_push(FS_InputQueue *q, FS_Command cmd, int p1, int p2);
int fs_input_queue_pop(FS_InputQueue *q, FS_InputEvent *out);
int fs_input_queue_count(const FS_InputQueue *q);

/* Keyboard mapping */
FS_Command fs_input_key_to_command(int scancode, int wasd_enabled);

#endif

