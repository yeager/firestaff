#ifndef FIRESTAFF_REDMCSB_F1037_MOUSE2_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1037_MOUSE2_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO.C F1037_Mouse2 exists only for X68000 and Amiga media routes. PC 3.4
 * supplies neither a branch nor a portable host adapter for this compositor.
 */
bool redmcsb_f1037_mouse2_pc34_compat(void);

const char *redmcsb_f1037_mouse2_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1037_MOUSE2_PC34_COMPAT_H */
