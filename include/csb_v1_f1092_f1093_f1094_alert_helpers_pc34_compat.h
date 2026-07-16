#ifndef FIRESTAFF_CSB_V1_F1092_F1093_F1094_ALERT_HELPERS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1092_F1093_F1094_ALERT_HELPERS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *F1092_GetHexadecimalDigits(void);
void F1093_DisplayAlertCSBInternalError(int16_t error_code);
void F1094_DisplayAlertCSBSystemError(uint32_t error_code);

char *F1090_GetCSBInternalErrorMessage(void);
char *F1091_GetCSBSystemErrorMessage(void);
const char *csb_v1_f1092_get_hexadecimal_digits_pc34_compat(void);
uint8_t *csb_v1_f1090_get_csb_internal_error_message_pc34_compat(void);
uint8_t *csb_v1_f1091_get_csb_system_error_message_pc34_compat(void);
void csb_v1_f1093_display_alert_csb_internal_error_pc34_compat(
    int16_t error_code);
void csb_v1_f1094_display_alert_csb_system_error_pc34_compat(
    uint32_t error_code);

const char *csb_v1_f1090_get_csb_internal_error_message_source_evidence_pc34(
    void);
const char *csb_v1_f1091_get_csb_system_error_message_source_evidence_pc34(
    void);
const char *csb_v1_f1092_get_hexadecimal_digits_source_evidence_pc34(void);
const char *csb_v1_f1093_display_alert_csb_internal_error_source_evidence_pc34(
    void);
const char *csb_v1_f1094_display_alert_csb_system_error_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1092_F1093_F1094_ALERT_HELPERS_PC34_COMPAT_H */
