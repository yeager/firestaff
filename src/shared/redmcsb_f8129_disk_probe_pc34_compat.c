#include "redmcsb_f8129_disk_probe_pc34_compat.h"

bool redmcsb_f8129_get_converted_device_type_pc34_compat(
    redmcsb_f8129_get_device_type_pc34_compat get_device_type,
    void *context,
    int16_t drive_number,
    int16_t *out_converted_type)
{
    static const int16_t converted_device_types[8] = {
        16, 17, 18, 0, 0, 1, 0, 19
    };
    uint8_t device_type;

    if (get_device_type == 0 || out_converted_type == 0 ||
        !get_device_type(context, drive_number, &device_type) ||
        device_type >= 8U) {
        return false;
    }

    *out_converted_type = converted_device_types[device_type];
    return true;
}

int16_t redmcsb_f8130_get_read_first_sector_result_pc34_compat(
    redmcsb_f8129_read_first_sector_pc34_compat read_first_sector,
    redmcsb_f8129_reset_disk_pc34_compat reset_disk,
    void *context,
    int16_t drive_ordinal)
{
    uint8_t sector[REDMCSB_F8129_FIRST_SECTOR_BYTES_PC34];
    int16_t bios_drive_number;
    int attempt;

    if (read_first_sector == 0) {
        return 0;
    }

    bios_drive_number = (int16_t)(drive_ordinal - 1);
    for (attempt = 0; attempt < 3; ++attempt) {
        if (read_first_sector(context, bios_drive_number, sector)) {
            return 1;
        }
        if (reset_disk != 0) {
            reset_disk(context, bios_drive_number);
        }
    }
    return 0;
}

const char *redmcsb_f8129_disk_probe_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:2167-2206,2209-2240; MEDIA701_I34E PC route";
}
