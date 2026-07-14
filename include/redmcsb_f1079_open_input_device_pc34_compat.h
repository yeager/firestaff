#ifndef FIRESTAFF_REDMCSB_F1079_OPEN_INPUT_DEVICE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1079_OPEN_INPUT_DEVICE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1079_OpenInputDevice opens Amiga input.device for the C03_GAME build.
 * The source supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1079_open_input_device_pc34_compat(void);

const char *redmcsb_f1079_open_input_device_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1079_OPEN_INPUT_DEVICE_PC34_COMPAT_H */
