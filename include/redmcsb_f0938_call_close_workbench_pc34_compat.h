#ifndef FIRESTAFF_REDMCSB_F0938_CALL_CLOSE_WORKBENCH_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0938_CALL_CLOSE_WORKBENCH_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0938 only exists in the Amiga A20E/A21E build. Intuition's
 * Workbench ownership has no PC 3.4 equivalent, so these callbacks are the
 * explicit host boundary; this adapter does not synthesize desktop UI state.
 */
typedef void *(*redmcsb_f0938_open_library_pc34_compat)(
    void *context,
    const char *name,
    long version);
typedef void (*redmcsb_f0938_alert_pc34_compat)(void *context,
                                                 unsigned long alert_code,
                                                 long parameter);
typedef void (*redmcsb_f0938_delay_pc34_compat)(void *context, long ticks);
typedef bool (*redmcsb_f0938_close_workbench_pc34_compat)(void *context);
typedef void (*redmcsb_f0938_close_library_pc34_compat)(void *context,
                                                         void *library);

typedef struct {
    redmcsb_f0938_open_library_pc34_compat open_library;
    redmcsb_f0938_alert_pc34_compat alert;
    redmcsb_f0938_delay_pc34_compat delay;
    redmcsb_f0938_close_workbench_pc34_compat close_workbench;
    redmcsb_f0938_close_library_pc34_compat close_library;
} redmcsb_f0938_call_close_workbench_ops_pc34_compat;

void redmcsb_f0938_call_close_workbench_pc34_compat(
    const redmcsb_f0938_call_close_workbench_ops_pc34_compat *ops,
    void *context);

const char *redmcsb_f0938_call_close_workbench_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0938_CALL_CLOSE_WORKBENCH_PC34_COMPAT_H */
