/*
 * ReDMCSB IO2.C F0712_AnyKeyboardOrMouseInput, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0712_ANY_KEYBOARD_OR_MOUSE_INPUT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0712_ANY_KEYBOARD_OR_MOUSE_INPUT_PC34_COMPAT_H

#include <stdbool.h>

#include "f0706_get_mouse_state_pc34_compat.h"
#include "redmcsb_f0539_input_cconis_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const ReDMCSB_F0706_IODriverPc34 *io_driver;
    RedmcsbF0539KeyboardInputPresentPc34Compat keyboard_input_present;
    void *keyboard_context;
} RedmcsbF0712InputStatePc34Compat;

/*
 * Applies IO2.C F0712's PC 3.4 path: query IODRV_13, then return true when
 * its button word is nonzero or IODRV_01 reports pending keyboard input.
 */
bool redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(
    const RedmcsbF0712InputStatePc34Compat *state);

const char *redmcsb_f0712_any_keyboard_or_mouse_input_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0712_ANY_KEYBOARD_OR_MOUSE_INPUT_PC34_COMPAT_H */
