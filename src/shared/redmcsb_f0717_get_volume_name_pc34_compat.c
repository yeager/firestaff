#include "redmcsb_f0717_get_volume_name_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0717_get_volume_name_pc34_compat(
    const redmcsb_f0717_io_driver_pc34_compat *io_driver,
    int16_t drive_ordinal,
    char *volume_name)
{
    /* ReDMCSB IO.C:3934-3939, MEDIA709_I34E_I34M_P31J. */
    if (io_driver == NULL || io_driver->get_volume_name == NULL) {
        return;
    }

    io_driver->get_volume_name(io_driver->context, drive_ordinal, volume_name);
}

const char *redmcsb_f0717_get_volume_name_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3934-3939, MEDIA709_I34E_I34M_P31J: F0717 calls "
           "G2162_IODriver->IODRV_19_GetVolumeName with P2405_i_DriveOrdinal "
           "and P2407_p_VolumeName unchanged.";
}
