#ifndef FIRESTAFF_REDMCSB_F1070_CLOSE_DOS_LIBRARY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1070_CLOSE_DOS_LIBRARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1070_CloseDosLibrary releases an Amiga DOS library handle. The source
 * supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1070_close_dos_library_pc34_compat(void);

const char *redmcsb_f1070_close_dos_library_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1070_CLOSE_DOS_LIBRARY_PC34_COMPAT_H */
