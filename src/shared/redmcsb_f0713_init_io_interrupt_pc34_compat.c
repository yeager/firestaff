#include "redmcsb_f0713_init_io_interrupt_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0713_init_io_interrupt_pc34_compat(
    redmcsb_f0713_state_pc34_compat *state)
{
    void *io_driver;

    if (state == NULL || state->get_vector == NULL ||
        state->set_vertical_blank == NULL || state->get_data_segment == NULL ||
        state->vertical_blank_routine == NULL) {
        return false;
    }

    /* IO.C:3893-3895: both globals receive getvect(C254), where C254 is 254. */
    io_driver = state->get_vector(state->context, 254U);
    if (io_driver == NULL) {
        return false;
    }
    state->io_driver_primary = io_driver;
    state->io_driver_secondary = io_driver;

    /* IO.C:3897: retain the driver-owned prior vertical-blank callback. */
    state->previous_vertical_blank_routine = state->set_vertical_blank(
        state->context, io_driver, state->vertical_blank_routine,
        state->vertical_blank_context);

    /* IO.C:3899-3900: preserve DS for the later interrupt routine. */
    state->data_segment_backup = state->get_data_segment(state->context);
    return true;
}

const char *redmcsb_f0713_init_io_interrupt_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3883-3903, MEDIA707_I34E_I34M and "
           "MEDIA709_I34E_I34M_P31J: getvect(C254_DM_IO_INTERRUPT), assign "
           "G2161/G2162, install F0782 through IODRV_14_SetCustomVerticalBlankRoutine, "
           "then save DS into V0713000_DSRegisterBackup.";
}
