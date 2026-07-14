#ifndef FIRESTAFF_REDMCSB_F1027_SUPERVISOR_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1027_SUPERVISOR_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IMAGE.C/CEDT027.C F1027_Supervisor calls the X68000 DOS SUPER service.
 * PC 3.4 provides neither that privilege transition nor a host adapter for
 * its supervisor-stack token, so the operation is unavailable.
 */
bool redmcsb_f1027_supervisor_pc34_compat(long supervisor_stack);

const char *redmcsb_f1027_supervisor_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1027_SUPERVISOR_PC34_COMPAT_H */
