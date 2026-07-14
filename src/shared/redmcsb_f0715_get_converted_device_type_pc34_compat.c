#include "redmcsb_f0715_get_converted_device_type_pc34_compat.h"

int16_t redmcsb_f0715_get_converted_device_type_pc34_compat(
    const redmcsb_f0715_io_driver_pc34_compat *io_driver,
    int16_t drive_ordinal)
{
    /* ReDMCSB IO.C:3922-3926, PC 3.4 MEDIA709_I34E_I34M_P31J. */
    return io_driver->get_converted_device_type(io_driver->context,
                                                drive_ordinal);
}

const char *redmcsb_f0715_get_converted_device_type_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:4322 declares IODRV_20_ as int16_t "
           "(*)(int16_t); IO.C:3922-3926 defines F0715 as exactly "
           "G2162_IODriver->IODRV_20_(P2399_i_).";
}
