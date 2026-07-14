/* ReDMCSB IBMIO.C F8129/F8130 PC 3.4 disk-probe routes. */
#ifndef FIRESTAFF_REDMCSB_F8129_DISK_PROBE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8129_DISK_PROBE_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F8129_FIRST_SECTOR_BYTES_PC34 = 512
};

typedef bool (*redmcsb_f8129_get_device_type_pc34_compat)(
    void *context,
    int16_t drive_number,
    uint8_t *out_device_type);
typedef bool (*redmcsb_f8129_read_first_sector_pc34_compat)(
    void *context,
    int16_t bios_drive_number,
    uint8_t out_sector[REDMCSB_F8129_FIRST_SECTOR_BYTES_PC34]);
typedef void (*redmcsb_f8129_reset_disk_pc34_compat)(
    void *context,
    int16_t bios_drive_number);

bool redmcsb_f8129_get_converted_device_type_pc34_compat(
    redmcsb_f8129_get_device_type_pc34_compat get_device_type,
    void *context,
    int16_t drive_number,
    int16_t *out_converted_type);

int16_t redmcsb_f8130_get_read_first_sector_result_pc34_compat(
    redmcsb_f8129_read_first_sector_pc34_compat read_first_sector,
    redmcsb_f8129_reset_disk_pc34_compat reset_disk,
    void *context,
    int16_t drive_ordinal);

const char *redmcsb_f8129_disk_probe_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
