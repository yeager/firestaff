#ifndef FIRESTAFF_REDMCSB_F1046_OPEN_LIBRARIES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1046_OPEN_LIBRARIES_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MAINLIB.C F1046_OpenLibraries belongs only to the MEDIA749 Amiga/X68000
 * route. PC 3.4 has no call site, FTL library ABI, or portable host adapter,
 * so this boundary does not attempt to open its PRIM/GRF1/VDEO/INT1/MUSC
 * libraries.
 */
bool redmcsb_f1046_open_libraries_pc34_compat(void);

const char *redmcsb_f1046_open_libraries_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1046_OPEN_LIBRARIES_PC34_COMPAT_H */
