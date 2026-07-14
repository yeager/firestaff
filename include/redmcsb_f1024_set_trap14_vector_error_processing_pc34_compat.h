#ifndef FIRESTAFF_REDMCSB_F1024_SET_TRAP14_VECTOR_ERROR_PROCESSING_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1024_SET_TRAP14_VECTOR_ERROR_PROCESSING_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FILE.C F1024 installs an X68000 vector-46 (TRAP 14) exception handler and
 * captures 68000 register state. No PC 3.4 branch or portable host adapter
 * exists, so this boundary does not simulate installation or trap delivery.
 */
bool redmcsb_f1024_set_trap14_vector_error_processing_pc34_compat(void);

const char *redmcsb_f1024_set_trap14_vector_error_processing_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1024_SET_TRAP14_VECTOR_ERROR_PROCESSING_PC34_COMPAT_H */
