#ifndef FIRESTAFF_REDMCSB_F1047_CLOSE_LIBRARIES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1047_CLOSE_LIBRARIES_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MAINLIB.C F1047_CloseLibraries belongs only to the non-PC MEDIA749 route.
 * It closes that route's PRIM and, for X68000, MUSC/INT1/VDEO/GRF1 libraries.
 * PC 3.4 supplies neither this call nor a portable host-library adapter.
 */
bool redmcsb_f1047_close_libraries_pc34_compat(void);

const char *redmcsb_f1047_close_libraries_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1047_CLOSE_LIBRARIES_PC34_COMPAT_H */
