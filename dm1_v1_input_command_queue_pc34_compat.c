#include "dm1_v1_input_command_queue_pc34_compat.h"
#include <string.h>

/* Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - COMMAND.C:106-121 G0448 movement mouse rows for C001/C003/C002/C006/C005/C004/C080/C083.
 * - COMMAND.C:252-260 and 272-305 G0459 movement keyboard rows for keypad/arrow movement commands.
 * - COMMAND.C:1379-1449 F0358 hit matcher walks mouse rows, checks button mask, returns command.
 * - COMMAND.C:1452-1661 F0359 queues mouse commands; if locked it records G0436..G0439 pending click, otherwise enqueues command/x/y.
 * - COMMAND.C:1692-1707 F0360 replays one pending click after unlock.
 * - COMMAND.C:1709-1813 F0361 queues primary/secondary keyboard commands, then replays pending click.
 * - COMMAND.C:2045-2156 F0380 locks, checks empty/movement-disabled gate, dequeues one command, replays pending click, dispatches turns to F0365 and moves to F0366.
 * - CLIKMENU.C:142-174 F0365 executes turn boundaries; CLIKMENU.C:180-330 F0366 executes move boundaries.
 */

#define DM1_V1_QUEUE_MAX_REGULAR 5u

static int normalize_dir(int value)
{
    while (value < 0) {
        value += 4;
    }
    return value & 3;
}

static int is_move_command(int command)
{
    return command >= DM1_V1_COMMAND_MOVE_FORWARD && command <= DM1_V1_COMMAND_MOVE_LEFT;
}

static int command_for_key(int keyCode)
{
    switch (keyCode) {
    case 0xAB34: case 0x007F:
        return DM1_V1_COMMAND_TURN_LEFT;
    case 0xAB36: case 0x9B3F:
        return DM1_V1_COMMAND_TURN_RIGHT;
    case 0xAB35: case 0x9B41: case 0x9B54:
        return DM1_V1_COMMAND_MOVE_FORWARD;
    case 0xAB33: case 0x9B43:
        return DM1_V1_COMMAND_MOVE_RIGHT;
    case 0xAB32: case 0x9B42: case 0x9B53:
        return DM1_V1_COMMAND_MOVE_BACKWARD;
    case 0xAB31: case 0x9B44: case 0x9B61:
        return DM1_V1_COMMAND_MOVE_LEFT;
    default:
        return DM1_V1_COMMAND_NONE;
    }
}

static int in_box(int x, int y, int left, int right, int top, int bottom)
{
    return x >= left && x <= right && y >= top && y <= bottom;
}

static int command_for_mouse(int x, int y, int buttonMask)
{
    if ((buttonMask & DM1_V1_BUTTON_LEFT) != 0) {
        if (in_box(x, y, 234, 261, 125, 145)) return DM1_V1_COMMAND_TURN_LEFT;
        if (in_box(x, y, 263, 289, 125, 145)) return DM1_V1_COMMAND_MOVE_FORWARD;
        if (in_box(x, y, 291, 318, 125, 145)) return DM1_V1_COMMAND_TURN_RIGHT;
        if (in_box(x, y, 234, 261, 147, 167)) return DM1_V1_COMMAND_MOVE_LEFT;
        if (in_box(x, y, 263, 289, 147, 167)) return DM1_V1_COMMAND_MOVE_BACKWARD;
        if (in_box(x, y, 291, 318, 147, 167)) return DM1_V1_COMMAND_MOVE_RIGHT;
        if (in_box(x, y, 0, 223, 33, 168)) return DM1_V1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    }
    if ((buttonMask & DM1_V1_BUTTON_RIGHT) != 0 && in_box(x, y, 0, 319, 33, 199)) {
        return DM1_V1_COMMAND_TOGGLE_INVENTORY_LEADER;
    }
    return DM1_V1_COMMAND_NONE;
}

static int enqueue_command(struct Dm1V1InputCommandQueuePc34Compat* queue, int command, int x, int y)
{
    if (command == DM1_V1_COMMAND_NONE) {
        return 0;
    }
    if (queue->count >= DM1_V1_QUEUE_MAX_REGULAR) {
        queue->droppedFullCount++;
        return 0;
    }
    queue->commands[queue->count].command = command;
    queue->commands[queue->count].x = x;
    queue->commands[queue->count].y = y;
    queue->count++;
    return 1;
}

static void process_pending_click(struct Dm1V1InputCommandQueuePc34Compat* queue)
{
    int x;
    int y;
    int buttons;
    if (!queue->pendingClickPresent) {
        return;
    }
    x = queue->pendingClickX;
    y = queue->pendingClickY;
    buttons = queue->pendingClickButtons;
    queue->pendingClickPresent = 0;
    queue->pendingReplayCount++;
    (void)enqueue_command(queue, command_for_mouse(x, y, buttons), x, y);
}

void DM1_V1_InputCommandQueue_InitPc34Compat(struct Dm1V1InputCommandQueuePc34Compat* queue)
{
    memset(queue, 0, sizeof(*queue));
}

int DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1InputEventPc34Compat event)
{
    int command;
    if (event.kind == DM1_V1_INPUT_KIND_MOUSE && queue->locked) {
        queue->pendingClickPresent = 1;
        queue->pendingClickX = event.x;
        queue->pendingClickY = event.y;
        queue->pendingClickButtons = event.buttonMask;
        return 0;
    }
    command = event.kind == DM1_V1_INPUT_KIND_KEY
        ? command_for_key(event.keyCode)
        : command_for_mouse(event.x, event.y, event.buttonMask);
    return enqueue_command(queue, command, event.x, event.y);
}

struct Dm1V1InputQueueProcessResultPc34Compat DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int partyDirection,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection)
{
    struct Dm1V1InputQueueProcessResultPc34Compat result;
    unsigned int i;
    memset(&result, 0, sizeof(result));
    queue->locked = 1;
    if (queue->count == 0u) {
        queue->locked = 0;
        process_pending_click(queue);
        result.pendingReplayCount = (int)queue->pendingReplayCount;
        return result;
    }
    result.command = queue->commands[0].command;
    if (is_move_command(result.command) &&
        (disabledMovementTicks ||
         (projectileDisabledMovementTicks &&
          lastProjectileDisabledMovementDirection == normalize_dir(partyDirection + result.command - DM1_V1_COMMAND_MOVE_FORWARD)))) {
        result.movementDisabledGate = 1;
        queue->locked = 0;
        process_pending_click(queue);
        result.pendingReplayCount = (int)queue->pendingReplayCount;
        return result;
    }
    for (i = 1; i < queue->count; ++i) {
        queue->commands[i - 1] = queue->commands[i];
    }
    queue->count--;
    result.dequeued = 1;
    queue->locked = 0;
    process_pending_click(queue);
    if (result.command == DM1_V1_COMMAND_TURN_LEFT || result.command == DM1_V1_COMMAND_TURN_RIGHT) {
        result.dispatchedTurn = 1;
    } else if (is_move_command(result.command)) {
        result.dispatchedMove = 1;
    }
    result.pendingReplayCount = (int)queue->pendingReplayCount;
    return result;
}

int DM1_V1_InputCommandQueue_PeekPc34Compat(
    const struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1QueuedCommandPc34Compat* outCommand)
{
    if (queue->count == 0u) {
        return 0;
    }
    if (outCommand != 0) {
        *outCommand = queue->commands[0];
    }
    return 1;
}

const char* DM1_V1_InputCommandQueue_SourceEvidencePc34Compat(void)
{
    return "COMMAND.C:106-121,252-260,272-305,1379-1449,1452-1661,1692-1707,1709-1813,2045-2156; CLIKMENU.C:142-174,180-330; MENUDRAW.C:5-19";
}
