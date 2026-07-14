/*
 * ReDMCSB FILE.C F0779_FILE_Tell, PC 3.4 (P20JB) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0779_FILE_TELL_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0779_FILE_TELL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host bridge for DOS INT 21h/AH=42h with AL=1, CX:DX=0.  The callback
 * supplies the resulting signed 32-bit current mark; it performs no host
 * file I/O itself.
 */
typedef int32_t (*redmcsb_f0779_dos_tell_pc34_compat)(
    void *context,
    int16_t file_handle);

int32_t redmcsb_f0779_file_tell_pc34_compat(
    redmcsb_f0779_dos_tell_pc34_compat tell,
    void *context,
    int16_t file_handle);

const char *redmcsb_f0779_file_tell_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
