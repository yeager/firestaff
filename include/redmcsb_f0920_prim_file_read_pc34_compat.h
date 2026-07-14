/*
 * ReDMCSB PRIM2C.C F0920_PRIM_20_File_Read, PC 3.4 (I34E/I34M) route.
 *
 * The PC 3.4 PRIM table exposes this routine as PrimRead in slot 20.
 * It performs one READ boundary call and treats any non-exact byte count as
 * failure.
 */
#ifndef FIRESTAFF_REDMCSB_F0920_PRIM_FILE_READ_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0920_PRIM_FILE_READ_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Explicit host boundary for the source READ(file_handle, buffer, length). */
typedef int32_t (*redmcsb_f0920_prim_file_read_backend_pc34_compat)(
    void *context,
    int32_t file_handle,
    void *buffer,
    int32_t length);

/*
 * Executes the sole F0920 PC 3.4 action. The backend, buffer, and context
 * must be valid whenever the host READ boundary requires them.
 *
 * Returns 0 when the backend reports exactly length bytes; returns 1 for any
 * other result, matching the source char status contract.
 */
int redmcsb_f0920_prim_file_read_pc34_compat(
    int32_t file_handle,
    int32_t length,
    void *buffer,
    redmcsb_f0920_prim_file_read_backend_pc34_compat backend,
    void *context);

const char *redmcsb_f0920_prim_file_read_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0920_PRIM_FILE_READ_PC34_COMPAT_H */
