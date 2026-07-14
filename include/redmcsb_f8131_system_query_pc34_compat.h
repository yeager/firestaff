/* ReDMCSB IBMIO.C F8131/F8132/F8133 PC 3.4 system-query routes. */
#ifndef FIRESTAFF_REDMCSB_F8131_SYSTEM_QUERY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8131_SYSTEM_QUERY_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F8131_VOLUME_LABEL_BYTES_PC34 = 11,
    REDMCSB_F8131_VOLUME_NAME_CAPACITY_PC34 = 12
};

typedef bool (*redmcsb_f8131_find_volume_label_pc34_compat)(
    void *context,
    uint8_t drive,
    uint8_t out_label[REDMCSB_F8131_VOLUME_LABEL_BYTES_PC34]);
typedef void (*redmcsb_f8131_get_dos_time_pc34_compat)(
    void *context,
    uint8_t *out_seconds,
    uint8_t *out_hundredths);

void redmcsb_f8131_get_volume_name_pc34_compat(
    redmcsb_f8131_find_volume_label_pc34_compat find_volume_label,
    void *context,
    uint8_t drive,
    char out_volume_name[REDMCSB_F8131_VOLUME_NAME_CAPACITY_PC34]);

uint16_t redmcsb_f8132_get_random_seed_pc34_compat(
    redmcsb_f8131_get_dos_time_pc34_compat get_dos_time,
    void *context);

/* IBMIO.C:2313-2316 has an intentionally empty PC implementation. */
void redmcsb_f8133_read_floppy_sector_pc34_compat(void);

const char *redmcsb_f8131_system_query_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
