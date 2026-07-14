#ifndef FIRESTAFF_REDMCSB_F0944_EXEC_VECTOR_PATCH_CHECK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0944_EXEC_VECTOR_PATCH_CHECK_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0944 is an Amiga-only 68000 assembly routine. It reads Exec's
 * DoIO, WaitIO, and OpenDevice vectors through the library at address 4,
 * then records whether any entry lies below address 0x80000. No PC 3.4
 * route or portable host adapter is supplied, so it is not applicable here.
 */
bool redmcsb_f0944_exec_vector_patch_check_pc34_compat(void);

const char *redmcsb_f0944_exec_vector_patch_check_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0944_EXEC_VECTOR_PATCH_CHECK_PC34_COMPAT_H */
