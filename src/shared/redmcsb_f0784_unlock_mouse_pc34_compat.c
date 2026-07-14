#include "redmcsb_f0784_unlock_mouse_pc34_compat.h"

void redmcsb_f0784_unlock_mouse_pc34_compat(
    const redmcsb_f0784_io_driver_pc34_compat *io_driver)
{
    io_driver->unlock_mouse(io_driver->context);
}

const char *redmcsb_f0784_unlock_mouse_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:1685-1692; CEDT026.C:171-175; "
           "G2161_IODriver->IODRV_06_UnlockMouse";
}
