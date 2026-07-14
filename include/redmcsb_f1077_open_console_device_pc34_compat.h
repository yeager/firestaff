#ifndef FIRESTAFF_REDMCSB_F1077_OPEN_CONSOLE_DEVICE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1077_OPEN_CONSOLE_DEVICE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1077_OpenConsoleDevice opens Amiga console.device for source media
 * variants. The source supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1077_open_console_device_pc34_compat(void);

const char *redmcsb_f1077_open_console_device_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1077_OPEN_CONSOLE_DEVICE_PC34_COMPAT_H */
