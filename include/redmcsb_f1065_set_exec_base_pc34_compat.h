#ifndef FIRESTAFF_REDMCSB_F1065_SET_EXEC_BASE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1065_SET_EXEC_BASE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AMIGALIB.C F1065_SetExecBase loads the Amiga Exec pointer from address 4
 * into the 68k global-base register. No PC 3.4 branch or portable host
 * behavior is supplied by the source.
 */
void redmcsb_f1065_set_exec_base_pc34_compat(void);

const char *redmcsb_f1065_set_exec_base_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1065_SET_EXEC_BASE_PC34_COMPAT_H */
