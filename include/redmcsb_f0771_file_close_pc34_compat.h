/*
 * ReDMCSB FILE.C F0771_FILE_Close, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0771_FILE_CLOSE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0771_FILE_CLOSE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Invokes the host's DOS close bridge for INT 21h/AH=3Eh.  The original
 * routine neither observes nor returns the DOS carry/error status.
 */
typedef void (*redmcsb_f0771_dos_close_pc34_compat)(
    void *context,
    int16_t file_handle);

void redmcsb_f0771_file_close_pc34_compat(
    redmcsb_f0771_dos_close_pc34_compat close_file,
    void *context,
    int16_t file_handle);

const char *redmcsb_f0771_file_close_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
