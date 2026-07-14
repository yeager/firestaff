/*
 * ReDMCSB FILE.C F0776_FILE_Create, PC 3.4 (C03_GAME/C06_CEDT) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0776_FILE_CREATE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0776_FILE_CREATE_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { REDMCSB_F0776_DOS_CREATE_ATTRIBUTES_PC34 = 0 };

typedef bool (*redmcsb_f0776_dos_create_pc34_compat)(
    void *context,
    const char *file_name,
    uint16_t attributes,
    int16_t *file_handle);

/*
 * Calls the source's DOS INT 21h/AH=3Ch create route with CX=0.
 * It returns the DOS file handle on success and -1 if DOS reports carry.
 */
int16_t redmcsb_f0776_file_create_pc34_compat(
    redmcsb_f0776_dos_create_pc34_compat create,
    void *context,
    const char *file_name);

const char *redmcsb_f0776_file_create_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
