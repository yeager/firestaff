#ifndef REDMCSB_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H
#define REDMCSB_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H

typedef enum TouchClickCoordModePc34Compat {
    TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT = 1,
    TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT = 2
} TouchClickCoordModePc34Compat;

typedef enum TouchClickButtonMaskPc34Compat {
    TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT = 0x0001u,
    TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT  = 0x0002u
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
int TOUCHCLICK_Compat_HitTestInCoordMode(int x, int y, TouchClickCoordModePc34Compat coordMode, unsigned int buttonMask, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_NormalizeScaledScreenPoint(int physicalX, int physicalY, int surfaceW, int surfaceH, int* outScreenX, int* outScreenY);
int TOUCHCLICK_Compat_HitTestScaledScreenPoint(int physicalX, int physicalY, int surfaceW, int surfaceH, unsigned int buttonMask, TouchClickZonePc34Compat* outZone);
int TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(int physicalX, int physicalY, int surfaceW, int surfaceH, unsigned int buttonMask, TouchClickDispatchPc34Compat* outDispatch);
const char* TOUCHCLICK_Compat_GetSourceEvidence(void);

#endif
