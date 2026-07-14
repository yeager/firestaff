#include "redmcsb_f0710_wait_sound_play_complete_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0710_wait_sound_play_complete_pc34_compat(
    const redmcsb_f0710_io_driver_pc34_compat *io_driver)
{
    /* ReDMCSB IO.C:3874-3879, MEDIA463 / I34E-I34M. */
    if (io_driver == NULL || io_driver->wait_sound_play_complete == NULL) {
        return false;
    }

    io_driver->wait_sound_play_complete(io_driver->context);
    return true;
}

const char *redmcsb_f0710_wait_sound_play_complete_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:4315-4319 declares IODRV_16_"
           "WaitSoundPlayComplete as a void callback; IO.C:3874-3879 "
           "(MEDIA463_P20JA_P20JB_I34E_I34M_P31J) defines F0710 as exactly "
           "G2161_IODriver->IODRV_16_WaitSoundPlayComplete().";
}
