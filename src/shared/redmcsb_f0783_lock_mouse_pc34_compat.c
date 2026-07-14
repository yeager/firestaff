#include "redmcsb_f0783_lock_mouse_pc34_compat.h"

void redmcsb_f0783_lock_mouse_pc34_compat(
    const redmcsb_f0783_io_driver_pc34_compat *io_driver)
{
    io_driver->lock_mouse();
}

const char *redmcsb_f0783_lock_mouse_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:1675-1683; IODRV_05_LockMouse dispatch";
}
