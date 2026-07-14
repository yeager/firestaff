/*
 * ReDMCSB IO.C F0812_CheckCopyProtectionSide0Track1_CPSX, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0812_CHECK_COPY_PROTECTION_SIDE0_TRACK1_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0812_CHECK_COPY_PROTECTION_SIDE0_TRACK1_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO.C asks the PC-98 disk BIOS to read sector 240 on side 0, cylinder 1.
 * The original disk has an ID for that deliberately unreadable sector, so
 * only BIOS AH=0xA0 (CRC error) proves the expected media layout.  The host
 * boundary records this request and accepts the BIOS status supplied by its
 * caller; it neither reads media nor synthesizes copy-protection results.
 */
typedef uint8_t (*redmcsb_f0812_pc98_read_data_callback_pc34_compat)(
    uint8_t physical_device_address,
    uint16_t data_length,
    uint8_t sector_length,
    uint8_t cylinder,
    uint8_t head,
    uint8_t sector,
    uint8_t *data_buffer,
    void *context);

bool redmcsb_f0812_check_copy_protection_side0_track1_pc34_compat(
    uint8_t disk_boot_physical_device_address,
    uint8_t *data_buffer,
    redmcsb_f0812_pc98_read_data_callback_pc34_compat read_data,
    void *context);

const char *redmcsb_f0812_check_copy_protection_side0_track1_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
