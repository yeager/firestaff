#ifndef FIRESTAFF_CSB_V1_F1075_F1076_F1077_F1078_DEVICE_HELPERS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1075_F1076_F1077_F1078_DEVICE_HELPERS_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

void F1075_OpenLayersLibrary(void);

void F1076_CloseLayersLibrary(void);

void F1077_OpenConsoleDevice(void);

void F1078_CloseConsoleDevice(void);

void csb_v1_f1075_open_layers_library_pc34_compat(void);
void csb_v1_f1076_close_layers_library_pc34_compat(void);
void csb_v1_f1077_open_console_device_pc34_compat(void);
void csb_v1_f1078_close_console_device_pc34_compat(void);

const char *csb_v1_f1075_open_layers_library_source_evidence_pc34(void);
const char *csb_v1_f1076_close_layers_library_source_evidence_pc34(void);
const char *csb_v1_f1077_open_console_device_source_evidence_pc34(void);
const char *csb_v1_f1078_close_console_device_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1075_F1076_F1077_F1078_DEVICE_HELPERS_PC34_COMPAT_H */
