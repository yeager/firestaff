/*
 * ReDMCSB FILE.C F0780_FILE_IsHandleInvalid, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0780_FILE_IS_HANDLE_INVALID_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0780_FILE_IS_HANDLE_INVALID_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mirrors the DOS-family source route: a file handle is invalid exactly when
 * its signed 16-bit value is negative. This has no host file-I/O side effect.
 */
bool redmcsb_f0780_file_is_handle_invalid_pc34_compat(int16_t file_handle);

const char *redmcsb_f0780_file_is_handle_invalid_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
