#include "redmcsb_f0785_set_mouse_pointer_coordinates_pc34_compat.h"

void redmcsb_f0785_set_mouse_pointer_coordinates_pc34_compat(
    const redmcsb_f0785_io_driver_pc34_compat *io_driver,
    int16_t x,
    int16_t y)
{
    io_driver->set_mouse_pointer_coordinates(x, y);
}

const char *redmcsb_f0785_set_mouse_pointer_coordinates_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3739-3744; CEDT026.C:201-204; IODRV_12 dispatch";
}
