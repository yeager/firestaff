#include "redmcsb_f0812_check_copy_protection_side0_track1_pc34_compat.h"

bool redmcsb_f0812_check_copy_protection_side0_track1_pc34_compat(
    uint8_t disk_boot_physical_device_address,
    uint8_t *data_buffer,
    redmcsb_f0812_pc98_read_data_callback_pc34_compat read_data,
    void *context)
{
    uint8_t bios_ah;

    /* IO.C:4161-4171, PC-98 INT 1Bh AH=76h (MFM, no retry, seek). */
    bios_ah = read_data(disk_boot_physical_device_address,
                        256u,
                        7u,
                        1u,
                        0u,
                        240u,
                        data_buffer,
                        context);
    return bios_ah == 0xA0u;
}

const char *redmcsb_f0812_check_copy_protection_side0_track1_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:4153-4172; PC-98 INT 1Bh sector 240 CRC-status gate";
}
