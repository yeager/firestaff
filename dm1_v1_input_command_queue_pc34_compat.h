#ifndef DM1_V1_INPUT_COMMAND_QUEUE_PC34_COMPAT_H
#define DM1_V1_INPUT_COMMAND_QUEUE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum Dm1V1InputCommandPc34Compat {
    DM1_V1_COMMAND_NONE = 0,
    DM1_V1_COMMAND_TURN_LEFT = 1,
    DM1_V1_COMMAND_TURN_RIGHT = 2,
    DM1_V1_COMMAND_MOVE_FORWARD = 3,
    DM1_V1_COMMAND_MOVE_RIGHT = 4,
    DM1_V1_COMMAND_MOVE_BACKWARD = 5,
    DM1_V1_COMMAND_MOVE_LEFT = 6,
    DM1_V1_COMMAND_CLICK_IN_DUNGEON_VIEW = 80,
    DM1_V1_COMMAND_TOGGLE_INVENTORY_LEADER = 83
};

enum Dm1V1InputKindPc34Compat {
    DM1_V1_INPUT_KIND_KEY = 1,
    DM1_V1_INPUT_KIND_MOUSE = 2
};

enum Dm1V1ButtonPc34Compat {
    DM1_V1_BUTTON_LEFT = 2,
    DM1_V1_BUTTON_RIGHT = 1
};

struct Dm1V1QueuedCommandPc34Compat {
    int command;
    int x;
    int y;
};

struct Dm1V1InputEventPc34Compat {
    int kind;
    int keyCode;
    int x;
    int y;
    int buttonMask;
};

struct Dm1V1InputCommandQueuePc34Compat {
    struct Dm1V1QueuedCommandPc34Compat commands[5];
    unsigned int count;
    int locked;
    int pendingClickPresent;
    int pendingClickX;
    int pendingClickY;
    int pendingClickButtons;
    int pendingClickCommand;
    unsigned int pendingReplayCount;
    unsigned int droppedFullCount;
};

struct Dm1V1InputQueueProcessResultPc34Compat {
    int command;
    int dispatchedTurn;
    int dispatchedMove;
    int movementDisabledGate;
    int dequeued;
    int pendingReplayCount;
};

void DM1_V1_InputCommandQueue_InitPc34Compat(struct Dm1V1InputCommandQueuePc34Compat* queue);
int DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1InputEventPc34Compat event);
int DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int command,
    int x,
    int y,
    int buttonMask);
struct Dm1V1InputQueueProcessResultPc34Compat DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int partyDirection,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection);
int DM1_V1_InputCommandQueue_PeekPc34Compat(
    const struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1QueuedCommandPc34Compat* outCommand);
const char* DM1_V1_InputCommandQueue_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
