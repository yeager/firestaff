#include "redmcsb_f0713_init_io_interrupt_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0713_init_io_interrupt_pc34_compat(
    const redmcsb_f0713_io_driver_pc34_compat *io_driver)
{
    /* ReDMCSB ANIM.C:1632, PC 3.4 I34E/I34M path. */
    if (io_driver == NULL || io_driver->init_io_interrupt == NULL) {
        return false;
    }

    io_driver->init_io_interrupt(io_driver->context);
    return true;
}

const char *redmcsb_f0713_init_io_interrupt_source_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C:1632 calls F0713_InitIOInterrupt during "
           "animation startup. The PC 3.4 I34E/I34M route is the "
           "I/O-driver-owned interrupt initialization vector; F0713 "
           "forwards no arguments and adds no timing or interrupt policy.";
}
