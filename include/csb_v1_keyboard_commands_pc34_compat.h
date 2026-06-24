#ifndef CSB_V1_KEYBOARD_COMMANDS_PC34_COMPAT_H
#define CSB_V1_KEYBOARD_COMMANDS_PC34_COMPAT_H

#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_KEYBOARD_KEY_NONE = 0,
    CSB_V1_KEYBOARD_KEY_F1,
    CSB_V1_KEYBOARD_KEY_F2,
    CSB_V1_KEYBOARD_KEY_F3,
    CSB_V1_KEYBOARD_KEY_F4,
    CSB_V1_KEYBOARD_KEY_ESCAPE,
    CSB_V1_KEYBOARD_KEY_RETURN,
    CSB_V1_KEYBOARD_KEY_ENTER,
    CSB_V1_KEYBOARD_KEY_S,
    CSB_V1_KEYBOARD_KEY_INSERT,
    CSB_V1_KEYBOARD_KEY_UP,
    CSB_V1_KEYBOARD_KEY_CLR_HOME,
    CSB_V1_KEYBOARD_KEY_LEFT,
    CSB_V1_KEYBOARD_KEY_DOWN,
    CSB_V1_KEYBOARD_KEY_RIGHT
} CsbV1KeyboardKeyPc34Compat;

int CSB_V1_KeyboardCommandToMenuInputPc34Compat(
    CsbV1KeyboardKeyPc34Compat key,
    int ctrlDown,
    M12_MenuInput* outInput);

const char* CSB_V1_KeyboardCommandSourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
