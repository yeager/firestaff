#ifndef FIRESTAFF_REDMCSB_F1069_OPEN_DOS_LIBRARY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1069_OPEN_DOS_LIBRARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1069_OpenDosLibrary opens the Amiga DOS library and reports through the
 * Amiga alert path when it is unavailable. The source supplies no PC 3.4
 * branch or portable host behavior.
 */
void redmcsb_f1069_open_dos_library_pc34_compat(void);

const char *redmcsb_f1069_open_dos_library_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1069_OPEN_DOS_LIBRARY_PC34_COMPAT_H */
