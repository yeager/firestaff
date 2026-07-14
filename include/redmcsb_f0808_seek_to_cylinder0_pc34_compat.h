/*
 * ReDMCSB IO.C F0808_SeekToCylinder0_Unreferenced, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0808_SEEK_TO_CYLINDER0_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0808_SEEK_TO_CYLINDER0_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The original reads DISK_BOOT from real-mode address 0000:0584, then asks
 * the PC-98 disk BIOS to seek cylinder zero and recalibrate that same drive.
 * The host boundary deliberately records only those BIOS requests; it does
 * not invent a floppy controller or disk state machine.
 */
typedef void (*redmcsb_f0808_pc98_disk_bios_callback_pc34_compat)(
    uint8_t ah,
    uint8_t al,
    uint8_t cl,
    void *context);

void redmcsb_f0808_seek_to_cylinder0_pc34_compat(
    uint8_t disk_boot_physical_device_address,
    redmcsb_f0808_pc98_disk_bios_callback_pc34_compat disk_bios,
    void *context);

const char *redmcsb_f0808_seek_to_cylinder0_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
