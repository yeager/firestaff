#include "redmcsb_f0714_get_read_first_sector_result_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0714_get_read_first_sector_result_pc34_compat(
    const redmcsb_f0714_io_driver_pc34_compat *io_driver,
    int16_t drive_ordinal,
    int16_t *out_result)
{
    if (io_driver == NULL || io_driver->read_first_sector == NULL ||
        out_result == NULL) {
        return false;
    }

    *out_result = io_driver->read_first_sector(io_driver->context, drive_ordinal);
    return true;
}

const char *redmcsb_f0714_get_read_first_sector_result_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3907-3920, MEDIA709_I34E_I34M_P31J: F0714 calls "
           "G2162_IODriver->IODRV_18 with P2398_i_DriveOrdinal and returns "
           "the resulting int16_t unchanged.";
}
