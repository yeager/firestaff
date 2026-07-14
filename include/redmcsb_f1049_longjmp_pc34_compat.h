#ifndef FIRESTAFF_REDMCSB_F1049_LONGJMP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1049_LONGJMP_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFS.H retains F1049_longjmp only as a disabled alias for non-PC media
 * routes. PC 3.4 has no F1049 entry point or portable jump-buffer adapter.
 */
bool redmcsb_f1049_longjmp_pc34_compat(void);

const char *redmcsb_f1049_longjmp_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1049_LONGJMP_PC34_COMPAT_H */
