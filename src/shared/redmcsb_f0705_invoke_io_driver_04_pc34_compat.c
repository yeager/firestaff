#include "redmcsb_f0705_invoke_io_driver_04_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0705_invoke_io_driver_04_pc34_compat(
    const redmcsb_f0705_io_driver_pc34_compat *io_driver)
{
    /* ReDMCSB IO.C:3732-3737, MEDIA463 ... I34E_I34M. */
    if (io_driver == NULL || io_driver->io_driver_04 == NULL) {
        return false;
    }

    io_driver->io_driver_04(io_driver->context);
    return true;
}

const char *redmcsb_f0705_invoke_io_driver_04_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3732-3737 F0705_, "
           "MEDIA463_P20JA_P20JB_I34E_I34M_P31J: calls "
           "G2161_IODriver->IODRV_04_ with no arguments; DEFS.H:4297 "
           "declares the no-argument IODRV_04_ slot.";
}
