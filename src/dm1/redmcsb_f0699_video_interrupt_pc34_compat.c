#include "redmcsb_f0699_video_interrupt_pc34_compat.h"

#include <stddef.h>

bool F0699_InitVideoInterrupt_PC34(
    ReDMCSBF0699VideoInterruptPc34Compat *state,
    ReDMCSBF0699GetVectorPc34Compat get_vector,
    void *context)
{
    const ReDMCSBF0699VideoDriverPc34Compat *video_driver;

    if (state == NULL || get_vector == NULL) {
        return false;
    }

    video_driver = get_vector(REDMCSB_F0699_DM_VIDEO_INTERRUPT_PC34, context);
    if (video_driver == NULL || video_driver->initialize_unused_globals == NULL) {
        return false;
    }

    /* ReDMCSB IMAGE.C:396-398: install table before calling slot 13. */
    state->video_driver = video_driver;
    video_driver->initialize_unused_globals(&state->first_unused,
                                            &state->second_unused);
    return true;
}
