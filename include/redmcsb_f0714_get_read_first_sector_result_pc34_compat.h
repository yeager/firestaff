#ifndef FIRESTAFF_REDMCSB_F0714_GET_READ_FIRST_SECTOR_RESULT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0714_GET_READ_FIRST_SECTOR_RESULT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t (*redmcsb_f0714_read_first_sector_pc34_compat)(
    void *context, int16_t drive_ordinal);

typedef struct {
    redmcsb_f0714_read_first_sector_pc34_compat read_first_sector;
    void *context;
} redmcsb_f0714_io_driver_pc34_compat;

/* ReDMCSB IO.C:3907-3920: IODRV_18(drive ordinal), no local policy. */
bool redmcsb_f0714_get_read_first_sector_result_pc34_compat(
    const redmcsb_f0714_io_driver_pc34_compat *io_driver,
    int16_t drive_ordinal,
    int16_t *out_result);

const char *redmcsb_f0714_get_read_first_sector_result_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
