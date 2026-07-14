#include "redmcsb_f0704_io_driver_slot03_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0704_call_io_driver_slot03_pc34_compat(
    const redmcsb_f0704_io_driver_pc34_compat *io_driver)
{
    /* ReDMCSB IO.C:3725-3731, MEDIA463 / I34E-I34M. */
    if (io_driver == NULL || io_driver->slot03 == NULL) {
        return false;
    }

    io_driver->slot03(io_driver->context);
    return true;
}

const char *redmcsb_f0704_io_driver_slot03_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:4293-4298 declares IODRV_03_ as a void callback; "
           "IO.C:3725-3731 (MEDIA463_P20JA_P20JB_I34E_I34M_P31J) defines "
           "F0704_ as exactly G2161_IODriver->IODRV_03_().";
}
