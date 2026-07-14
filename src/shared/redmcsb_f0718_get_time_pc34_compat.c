#include "redmcsb_f0718_get_time_pc34_compat.h"

int16_t redmcsb_f0718_get_time_pc34_compat(
    const redmcsb_f0718_io_driver_pc34_compat *io_driver)
{
    /* ReDMCSB IO.C:3941-3945, PC 3.4 MEDIA709_I34E_I34M_P31J. */
    return io_driver->get_time(io_driver->context);
}

const char *redmcsb_f0718_get_time_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:4323 declares IODRV_21_GetTime as int16_t "
           "(*)(void); IO.C:3941-3945 defines F0718 as exactly "
           "G2161_IODriver->IODRV_21_GetTime().";
}
