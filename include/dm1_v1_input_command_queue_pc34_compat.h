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
    DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_0 = 7,
    DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_1 = 8,
    DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_2 = 9,
    DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_3 = 10,
    DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0 = 12,
    DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_1 = 13,
    DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_2 = 14,
    DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_3 = 15,
    DM1_V1_COMMAND_CLICK_IN_DUNGEON_VIEW = 80,
    DM1_V1_COMMAND_TOGGLE_INVENTORY_LEADER = 83,
    DM1_V1_COMMAND_CLICK_IN_SPELL_AREA = 100,
    DM1_V1_COMMAND_CLICK_IN_ACTION_AREA = 111,
    DM1_V1_COMMAND_CHAMPION_ICON_TOP_LEFT = 125,
    DM1_V1_COMMAND_CHAMPION_ICON_TOP_RIGHT = 126,
    DM1_V1_COMMAND_CHAMPION_ICON_BOTTOM_RIGHT = 127,
    DM1_V1_COMMAND_CHAMPION_ICON_BOTTOM_LEFT = 128,
    DM1_V1_COMMAND_RELEASE_CHAMPION_ICON = 129,
    DM1_V1_COMMAND_SAVE_GAME = 140,
    DM1_V1_COMMAND_FREEZE_GAME = 147,
    DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL = 254
};

enum Dm1V1InputKindPc34Compat {
    DM1_V1_INPUT_KIND_KEY = 1,
    DM1_V1_INPUT_KIND_MOUSE = 2
};

enum Dm1V1UsioDataTypePc34Compat {
    DM1_V1_USIO_DATA_TYPE_NONE = 0,
    DM1_V1_USIO_DATA_TYPE_KEYBOARD = 1,
    DM1_V1_USIO_DATA_TYPE_MOUSE = 2
};

enum Dm1V1ButtonPc34Compat {
    DM1_V1_BUTTON_LEFT = 2,
    DM1_V1_BUTTON_RIGHT = 1,
    DM1_V1_BUTTON_LEFT_UP = 4,
    DM1_V1_BUTTON_RIGHT_UP = 8,
    DM1_V1_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION = 33
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

struct Dm1V1UsioDataPc34Compat {
    int usioType;
    int rawKeyCode;
    int mouseX;
    int mouseY;
    int mouseButtons;
};

struct Dm1V1UsioMouseStatusPc34Compat {
    int mouseButtons;
    int mouseX;
    int mouseY;
};

struct Dm1V1InputCommandQueuePc34Compat {
    struct Dm1V1QueuedCommandPc34Compat commands[7];
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

enum { DM1_V1_PENDING_MOTION_QUEUE_CAPACITY_PC34_COMPAT = 7 };

struct Dm1V1PendingMotionQueuePc34Compat {
    int inputs[DM1_V1_PENDING_MOTION_QUEUE_CAPACITY_PC34_COMPAT];
    unsigned int head;
    unsigned int count;
    unsigned int droppedFullCount;
};

void DM1_V1_InputCommandQueue_InitPc34Compat(struct Dm1V1InputCommandQueuePc34Compat* queue);
void DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(struct Dm1V1InputCommandQueuePc34Compat* queue);
void F0357_COMMAND_DiscardAllInput(struct Dm1V1InputCommandQueuePc34Compat* queue);
int DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1InputEventPc34Compat event);
int DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int command,
    int x,
    int y);
int DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int command,
    int x,
    int y,
    int buttonMask);
int F1173_AddUsioDataToInputQueue(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    const struct Dm1V1UsioDataPc34Compat* usioData);
int F1174_AddPendingUsioDataToInputQueue(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1UsioDataPc34Compat* pendingUsioData,
    int* pendingPresent);
int F1684_GetMouseStatus(
    struct Dm1V1UsioMouseStatusPc34Compat* outMouseStatus,
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F1694_AddMouseInputToQueue(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int mouseX,
    int mouseY,
    int mouseButtons);
int F1128_IsLeftMouseButtonDown(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F2008_IsLeftMouseButtonDown(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F2024_IsLeftMouseButtonDown(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F2009_GetMouseX(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F2010_GetMouseY(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F2047_GetMouseX(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F2048_GetMouseY(
    const struct Dm1V1UsioMouseStatusPc34Compat* callerOwnedMouseStatus);
int F1172_QueueMouseAndKeyboardInput(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    const struct Dm1V1UsioDataPc34Compat* callerOwnedUsioData,
    unsigned int callerOwnedUsioDataCount);
struct Dm1V1InputQueueProcessResultPc34Compat DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int partyDirection,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection);
int DM1_V1_InputCommandQueue_PeekPc34Compat(
    const struct Dm1V1InputCommandQueuePc34Compat* queue,
    struct Dm1V1QueuedCommandPc34Compat* outCommand);
void DM1_V1_PendingMotionQueue_InitPc34Compat(
    struct Dm1V1PendingMotionQueuePc34Compat* queue);
void DM1_V1_PendingMotionQueue_ClearPc34Compat(
    struct Dm1V1PendingMotionQueuePc34Compat* queue);
int DM1_V1_PendingMotionQueue_PushPc34Compat(
    struct Dm1V1PendingMotionQueuePc34Compat* queue,
    int menuInput);
int DM1_V1_PendingMotionQueue_PopPc34Compat(
    struct Dm1V1PendingMotionQueuePc34Compat* queue,
    int* outMenuInput);
unsigned int DM1_V1_PendingMotionQueue_CountPc34Compat(
    const struct Dm1V1PendingMotionQueuePc34Compat* queue);
int DM1_V1_InputSourceIsActivePc34Compat(int active, const char* sourceId);
int DM1_V1_InputMenuTokenIsImmediateTurnPc34Compat(int menuInput);
const char* DM1_V1_InputCommandQueue_SourceEvidencePc34Compat(void);
const char* F0357_COMMAND_DiscardAllInput_SourceEvidence(void);
const char* F1172_QueueMouseAndKeyboardInput_SourceEvidence(void);
const char* F1173_AddUsioDataToInputQueue_SourceEvidence(void);
const char* F1174_AddPendingUsioDataToInputQueue_SourceEvidence(void);
const char* F1684_GetMouseStatus_SourceEvidence(void);
const char* F1694_AddMouseInputToQueue_SourceEvidence(void);
const char* F1128_IsLeftMouseButtonDown_SourceEvidence(void);
const char* F2008_F2024_IsLeftMouseButtonDown_SourceEvidence(void);
const char* F2009_F2010_F2047_F2048_MouseCoordinate_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif
