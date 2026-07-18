#ifndef FIRESTAFF_REDMCSB_F1020_INITIALIZE_X68000_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1020_INITIALIZE_X68000_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * STARTUP2.C F1020_InitializeX68000 is enclosed by
 * MEDIA607_X30J_X31J. Its X30J and X31J routes use X68000 IOCS,
 * allocator, video-driver, and physical-screen state.
 * No PC 3.4 branch or portable host adapter exists.
 */
bool redmcsb_f1020_initialize_x68000_pc34_compat(void);

const char *redmcsb_f1020_initialize_x68000_source_evidence_pc34(void);

/* ReDMCSB source-named alias for
 * redmcsb_f1020_initialize_x68000_pc34_compat. */
bool F1020_InitializeX68000(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1020_INITIALIZE_X68000_PC34_COMPAT_H */
