#include "redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(
    const RedmcsbF0712InputStatePc34Compat *state)
{
    int16_t mouse_x = 0;
    int16_t mouse_y = 0;
    int16_t mouse_buttons = 0;

    if (state == NULL) {
        return false;
    }

    /* IO2.C:272 obtains all three words before testing buttons and keys. */
    ReDMCSB_F0706_GetMouseStatePc34Compat(
        state->io_driver, &mouse_x, &mouse_y, &mouse_buttons);

    return mouse_buttons != 0 || redmcsb_f0539_input_cconis_pc34_compat(
        state->keyboard_input_present, state->keyboard_context);
}

const char *redmcsb_f0712_any_keyboard_or_mouse_input_source_evidence_pc34(void)
{
    return "ReDMCSB IO2.C:262-274 F0712_AnyKeyboardOrMouseInput, "
           "MEDIA463_P20JA_P20JB_I34E_I34M_P31J: "
           "F0706_GetMouseState(&A, &B, &C); return (C != 0) || "
           "F0539_INPUT_Cconis(). IO.C:3745-3751 defines F0706 as the "
           "direct IODRV_13_GetMouseState dispatch, and IO2.C:179-185 "
           "defines F0539 as IODRV_01_IsKeyboardInputPresent.";
}
