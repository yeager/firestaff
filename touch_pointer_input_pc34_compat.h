#ifndef FIRESTAFF_TOUCH_POINTER_INPUT_PC34_COMPAT_H
#define FIRESTAFF_TOUCH_POINTER_INPUT_PC34_COMPAT_H

#include "touch_click_zone_matrix_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"

typedef enum TouchPointerActionPc34Compat {
    TOUCH_POINTER_ACTION_DOWN_PC34_COMPAT = 1,
    TOUCH_POINTER_ACTION_UP_PC34_COMPAT = 2,
    TOUCH_POINTER_ACTION_MOVE_PC34_COMPAT = 3,
    TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT = 4
} TouchPointerActionPc34Compat;

typedef enum TouchPointerSpacePc34Compat {
    TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT = 1,
    TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT = 2,
    TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT = 3
} TouchPointerSpacePc34Compat;

typedef struct TouchPointerEventPc34Compat {
    TouchPointerActionPc34Compat action;
    TouchPointerSpacePc34Compat space;
    int x;
    int y;
    int surfaceW;
    int surfaceH;
    unsigned int buttonMask;
} TouchPointerEventPc34Compat;

typedef struct TouchPointerDispatchPc34Compat {
    int shouldDispatchClick;
    int screenX;
    int screenY;
    unsigned int buttonStatus;
    unsigned int commandId;
    unsigned int zoneIndex;
    TouchClickCoordModePc34Compat coordMode;
    const char* groupName;
} TouchPointerDispatchPc34Compat;

int TOUCHPOINTER_Compat_TranslateEvent(const TouchPointerEventPc34Compat* event,
                                       TouchPointerDispatchPc34Compat* outDispatch);
int TOUCHPOINTER_Compat_EventFromScaledTap(int physicalX,
                                           int physicalY,
                                           int surfaceW,
                                           int surfaceH,
                                           unsigned int buttonMask,
                                           TouchPointerEventPc34Compat* outEvent);
int TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(
    const TouchPointerEventPc34Compat* event,
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    TouchPointerDispatchPc34Compat* outDispatch);
const char* TOUCHPOINTER_Compat_GetSourceEvidence(void);

#endif
