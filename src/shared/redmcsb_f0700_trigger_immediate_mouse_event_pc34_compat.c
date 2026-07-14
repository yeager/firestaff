#include "redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat(
    const redmcsb_f0700_io_driver_pc34_compat *io_driver)
{
    /* ReDMCSB IO.C:497,680-685, MEDIA463_P20JA_P20JB_I34E_I34M_P31J. */
    static const int16_t impossible_box[4] = { 1, 0, 1, 0 };

    if (io_driver == NULL || io_driver->set_mouse_bound_event == NULL) {
        return false;
    }

    io_driver->set_mouse_bound_event(
        io_driver->context, impossible_box,
        REDMCSB_F0700_MOUSE_EVENT_CHANGE_SCREEN_REGION_PC34_COMPAT);
    return true;
}

const char *redmcsb_f0700_trigger_immediate_mouse_event_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:497 defines G2050_ai_Box_Impossible as {1,0,1,0}; "
           "IO.C:680-685 (MEDIA463_P20JA_P20JB_I34E_I34M_P31J) calls "
           "IODRV_11_SetMouseBoundEvent with that box and "
           "C32_MOUSE_EVENT_CHANGE_SCREEN_REGION.";
}
