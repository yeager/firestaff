#include "redmcsb_f0539_input_cconis_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0539_input_cconis_pc34_compat(
    RedmcsbF0539KeyboardInputPresentPc34Compat input_present,
    void *context)
{
    if (input_present == NULL) {
        return false;
    }

    return input_present(context);
}

const char *redmcsb_f0539_input_cconis_source_evidence_pc34(void)
{
    return "ReDMCSB IO2.C:179-185 F0539_INPUT_Cconis: under "
           "MEDIA463_P20JA_P20JB_I34E_I34M_P31J it returns exactly "
           "G2162_IODriver->IODRV_01_IsKeyboardInputPresent(). "
           "DEFS.H:4292-4332 defines IODRV_01 as the keyboard-input "
           "presence Boolean callback; ENTRANCE.C:863-874 consumes the "
           "status before calling F0540_INPUT_Crawcin on the PC path.";
}
