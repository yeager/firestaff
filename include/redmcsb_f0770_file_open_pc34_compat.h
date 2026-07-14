/*
 * ReDMCSB FILE.C F0770_FILE_Open, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0770_FILE_OPEN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0770_FILE_OPEN_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { REDMCSB_F0770_DOS_OPEN_READ_WRITE_PC34 = 2 };

typedef bool (*redmcsb_f0770_dos_open_pc34_compat)(
    void *context,
    const char *file_name,
    uint8_t access_mode,
    int16_t *file_handle);

/*
 * Calls the source's DOS INT 21h/AH=3Dh file-open route using AL=2.
 * It returns the DOS file handle on success and -1 if DOS reports carry.
 */
int16_t redmcsb_f0770_file_open_pc34_compat(
    redmcsb_f0770_dos_open_pc34_compat open,
    void *context,
    const char *file_name);

const char *redmcsb_f0770_file_open_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
