/*
 * ReDMCSB FILE.C F0774_FILE_Seek, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0774_FILE_SEEK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0774_FILE_SEEK_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host bridge for DOS INT 21h/AH=42h with AL=0 (seek from file beginning).
 * true represents carry clear, exactly as F0774 maps it to AX=1.
 */
typedef bool (*redmcsb_f0774_dos_seek_from_beginning_pc34_compat)(
    void *context,
    int16_t file_handle,
    int32_t offset);

bool redmcsb_f0774_file_seek_pc34_compat(
    redmcsb_f0774_dos_seek_from_beginning_pc34_compat seek_from_beginning,
    void *context,
    int16_t file_handle,
    int32_t offset);

const char *redmcsb_f0774_file_seek_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
