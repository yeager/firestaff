#ifndef FIRESTAFF_REDMCSB_F1050_ALERT_CSB_SYSTEM_ERROR_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1050_ALERT_CSB_SYSTEM_ERROR_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AMIGINIT.C F1050_AlertCSBSystemError terminates through Amiga Intuition
 * and Exec alert services. PC 3.4 supplies neither those services nor a
 * portable terminal-error adapter.
 */
bool redmcsb_f1050_alert_csb_system_error_pc34_compat(long error_code);

const char *redmcsb_f1050_alert_csb_system_error_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1050_ALERT_CSB_SYSTEM_ERROR_PC34_COMPAT_H */
