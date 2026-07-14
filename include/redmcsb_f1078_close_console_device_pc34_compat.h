#ifndef FIRESTAFF_REDMCSB_F1078_CLOSE_CONSOLE_DEVICE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1078_CLOSE_CONSOLE_DEVICE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1078_CloseConsoleDevice releases Amiga console.device resources. The
 * source supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1078_close_console_device_pc34_compat(void);

const char *redmcsb_f1078_close_console_device_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1078_CLOSE_CONSOLE_DEVICE_PC34_COMPAT_H */
