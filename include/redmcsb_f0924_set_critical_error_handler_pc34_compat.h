#ifndef FIRESTAFF_REDMCSB_F0924_SET_CRITICAL_ERROR_HANDLER_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0924_SET_CRITICAL_ERROR_HANDLER_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F0924 installs a 68000 GEMDOS critical-error vector while running in Atari
 * supervisor mode. The PC 3.4 host supplies neither supervisor mode nor the
 * Atari ST low-memory vector table, so it requires a platform adapter.
 */
bool redmcsb_f0924_set_critical_error_handler_pc34_compat(void);

const char *redmcsb_f0924_set_critical_error_handler_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0924_SET_CRITICAL_ERROR_HANDLER_PC34_COMPAT_H */
