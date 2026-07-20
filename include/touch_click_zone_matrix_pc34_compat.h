#ifndef REDMCSB_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H
#define REDMCSB_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H

typedef enum TouchClickCoordModePc34Compat {
    TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT = 1,
    TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT = 2
} TouchClickCoordModePc34Compat;

typedef enum TouchClickButtonMaskPc34Compat {
    TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT = 0x0001u,
    TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT  = 0x0002u,
    /* Synthetic keyboard-activation bit, not a physical mouse button.
     * The physical receiver (IBMIO_MOUSE_EVENT_RECEIVER,
     * SkWinCore.cpp:38776-38802) only ever queues codes 1/2/4/8.  Bit
     * 0x10 enters the queue when _1031_0781 (SkWinCore.cpp:32143-32154)
     * resolves a zone by event code and re-queues the zone's own mask
     * byte as the "button" through FIRE_QUEUE_MOUSE_EVENT
     * (SkWinCore.cpp:8547); _1031_0781 is invoked from the menu/dialog
     * keyboard loop _0aaf_0067 on Enter / default-item activation
     * (SkWinCore.cpp:39105, 39111).  The queued value passes the
     * (button & 0x13) gate in IBMIO_USER_INPUT_CHECK
     * (SkWinCore.cpp:15301-15303) and matches zones through
     * (ww & (w4 & 0xff)) in _1031_0a88 (SkWinCore.cpp:12144). */
    TOUCH_CLICK_BUTTON_KEYBOARD_PC34_COMPAT = 0x0010u
} TouchClickButtonMaskPc34Compat;

typedef struct TouchClickZonePc34Compat {
    unsigned int commandId;
    unsigned int zoneIndex;
    TouchClickCoordModePc34Compat coordMode;
    unsigned int buttonMask;
    int x;
    int y;
    int w;
    int h;
    const char* groupName;
    const char* sourceEvidence;
} TouchClickZonePc34Compat;

typedef struct TouchClickDispatchPc34Compat {
    int screenX;
    int screenY;
    unsigned int buttonStatus;
    unsigned int commandId;
    unsigned int zoneIndex;
    TouchClickCoordModePc34Compat coordMode;
    const char* groupName;
} TouchClickDispatchPc34Compat;

unsigned int TOUCHCLICK_Compat_GetZoneCount(void);
int TOUCHCLICK_Compat_GetZone(unsigned int ordinal, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_HitTest(int screenX, int screenY, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_HitTestWithButton(int screenX, int screenY, unsigned int buttonMask, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(int screenX, int screenY, unsigned int buttonMask, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_HitTestInCoordMode(int x, int y, TouchClickCoordModePc34Compat coordMode, unsigned int buttonMask, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_GetSourceViewportRect(int* outX, int* outY, int* outW, int* outH);
int TOUCHCLICK_Compat_NormalizeScaledScreenPoint(int physicalX, int physicalY, int surfaceW, int surfaceH, int* outScreenX, int* outScreenY);
int TOUCHCLICK_Compat_NormalizeScaledViewportPoint(int physicalX, int physicalY, int surfaceW, int surfaceH, int* outViewportX, int* outViewportY);
int TOUCHCLICK_Compat_HitTestScaledScreenPoint(int physicalX, int physicalY, int surfaceW, int surfaceH, unsigned int buttonMask, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(int viewportX, int viewportY, unsigned int buttonMask, TouchClickDispatchPc34Compat* outDispatch);
int TOUCHCLICK_Compat_MapDungeonViewportLocalPointToDispatch(int viewportX, int viewportY, unsigned int buttonMask, TouchClickDispatchPc34Compat* outDispatch);
int TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(int physicalX, int physicalY, int surfaceW, int surfaceH, unsigned int buttonMask, TouchClickDispatchPc34Compat* outDispatch);
int TOUCHCLICK_Compat_MapScaledViewportPointToDispatch(int physicalX, int physicalY, int surfaceW, int surfaceH, unsigned int buttonMask, TouchClickDispatchPc34Compat* outDispatch);
int TOUCHCLICK_Compat_MapScaledDungeonViewportPointToDispatch(int physicalX, int physicalY, int surfaceW, int surfaceH, unsigned int buttonMask, TouchClickDispatchPc34Compat* outDispatch);
const char* TOUCHCLICK_Compat_GetSourceEvidence(void);

#endif
