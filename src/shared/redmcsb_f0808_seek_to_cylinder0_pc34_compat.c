#include "redmcsb_f0808_seek_to_cylinder0_pc34_compat.h"

void redmcsb_f0808_seek_to_cylinder0_pc34_compat(
    uint8_t disk_boot_physical_device_address,
    redmcsb_f0808_pc98_disk_bios_callback_pc34_compat disk_bios,
    void *context)
{
    /* IO.C issues BIOS INT 1Bh AH=10h with CL=0, then AH=07h. */
    disk_bios(0x10u, disk_boot_physical_device_address, 0u, context);
    disk_bios(0x07u, disk_boot_physical_device_address, 0u, context);
}

const char *redmcsb_f0808_seek_to_cylinder0_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3980-3995; DISK_BOOT, PC-98 INT 1Bh seek and recalibrate";
}
