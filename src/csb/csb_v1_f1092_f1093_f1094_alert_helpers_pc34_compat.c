#include "csb_v1_f1092_f1093_f1094_alert_helpers_pc34_compat.h"
#include "redmcsb_f1090_get_csb_internal_error_message_pc34_compat.h"
#include "redmcsb_f1091_get_csb_system_error_message_pc34_compat.h"

static uint8_t g_f1090_csb_internal_error_message[] = {
    0x00U, 0xB8U, 0x10U,
    'C', 'h', 'a', 'o', 's', ' ', 'S', 't', 'r', 'i', 'k', 'e', 's', ' ',
    'B', 'a', 'c', 'k', ' ', 'S', 'y', 's', 't', 'e', 'm', ' ', 'E', 'r',
    'r', 'o', 'r', ' ', '0', '0', 0x00U,
    0xFFU,
    0x00U, 0xB8U, 0x1CU,
    'P', 'r', 'e', 's', 's', ' ', 'L', 'e', 'f', 't', ' ', 'M', 'o', 'u',
    's', 'e', ' ', 'B', 'u', 't', 't', 'o', 'n', ' ', 't', 'o', ' ', 'R',
    'e', 's', 't', 'a', 'r', 't', 0x00U,
    0x00U
};

static uint8_t g_f1091_csb_system_error_message[] = {
    0x00U, 0x94U, 0x10U,
    'C', 'h', 'a', 'o', 's', ' ', 'S', 't', 'r', 'i', 'k', 'e', 's', ' ',
    'B', 'a', 'c', 'k', ' ', 'I', 'n', 't', 'e', 'r', 'n', 'a', 'l', ' ',
    'E', 'r', 'r', 'o', 'r', ':', ' ', '0', '0', '0', '0', '0', '0', '0',
    '0', 0x00U,
    0xFFU,
    0x00U, 0xB8U, 0x1CU,
    'P', 'r', 'e', 's', 's', ' ', 'L', 'e', 'f', 't', ' ', 'M', 'o', 'u',
    's', 'e', ' ', 'B', 'u', 't', 't', 'o', 'n', ' ', 't', 'o', ' ', 'R',
    'e', 's', 't', 'a', 'r', 't', 0x00U,
    0x00U
};

uint8_t *csb_v1_f1090_get_csb_internal_error_message_pc34_compat(void)
{
    return g_f1090_csb_internal_error_message;
}

uint8_t *redmcsb_f1090_get_csb_internal_error_message_pc34_compat(void)
{
    return csb_v1_f1090_get_csb_internal_error_message_pc34_compat();
}

char *F1090_GetCSBInternalErrorMessage(void)
{
    return (char *)csb_v1_f1090_get_csb_internal_error_message_pc34_compat();
}

const char *csb_v1_f1090_get_csb_internal_error_message_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGINIT.C:392 F1090_GetCSBInternalErrorMessage; "
           "exact mutable DisplayAlert byte template for the two-digit CSB "
           "internal-error route";
}

const char *redmcsb_f1090_get_csb_internal_error_message_source_evidence_pc34(
    void)
{
    return csb_v1_f1090_get_csb_internal_error_message_source_evidence_pc34();
}

uint8_t *csb_v1_f1091_get_csb_system_error_message_pc34_compat(void)
{
    return g_f1091_csb_system_error_message;
}

uint8_t *redmcsb_f1091_get_csb_system_error_message_pc34_compat(void)
{
    return csb_v1_f1091_get_csb_system_error_message_pc34_compat();
}

char *F1091_GetCSBSystemErrorMessage(void)
{
    return (char *)csb_v1_f1091_get_csb_system_error_message_pc34_compat();
}

const char *csb_v1_f1091_get_csb_system_error_message_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGINIT.C:411 F1091_GetCSBSystemErrorMessage; "
           "exact mutable DisplayAlert byte template for the eight-digit CSB "
           "system-error route";
}

const char *redmcsb_f1091_get_csb_system_error_message_source_evidence_pc34(
    void)
{
    return csb_v1_f1091_get_csb_system_error_message_source_evidence_pc34();
}

const char *csb_v1_f1092_get_hexadecimal_digits_pc34_compat(void)
{
    return "0123456789abcdef";
}

const char *F1092_GetHexadecimalDigits(void)
{
    return csb_v1_f1092_get_hexadecimal_digits_pc34_compat();
}

const char *csb_v1_f1092_get_hexadecimal_digits_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:430 F1092_GetHexadecimalDigits; "
           "source hexadecimal digit table used by the CSB DisplayAlert "
           "message builders";
}

void csb_v1_f1093_display_alert_csb_internal_error_pc34_compat(
    int16_t error_code)
{
    uint8_t *message = csb_v1_f1090_get_csb_internal_error_message_pc34_compat();
    const char *digits = csb_v1_f1092_get_hexadecimal_digits_pc34_compat();
    int16_t value = error_code;
    uint8_t *cursor = message + 37U;

    if (value < 0) {
        value = (int16_t)-value;
    }
    *--cursor = (uint8_t)digits[value % 10];
    value = (int16_t)(value / 10);
    *--cursor = (uint8_t)digits[value % 10];
}

void F1093_DisplayAlertCSBInternalError(int16_t error_code)
{
    csb_v1_f1093_display_alert_csb_internal_error_pc34_compat(error_code);
}

const char *csb_v1_f1093_display_alert_csb_internal_error_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGINIT.C:442 F1093_DisplayAlertCSBInternalError; "
           "terminal Amiga Intuition DisplayAlert/ResetAmiga boundary for "
           "internal errors, no PC34 host alert or restart route";
}

void csb_v1_f1094_display_alert_csb_system_error_pc34_compat(
    uint32_t error_code)
{
    uint8_t *message = csb_v1_f1091_get_csb_system_error_message_pc34_compat();
    const char *digits = csb_v1_f1092_get_hexadecimal_digits_pc34_compat();
    uint8_t *cursor = message + 46U;
    int i;

    for (i = 0; i < 8; ++i) {
        *--cursor = (uint8_t)digits[error_code & 0x0FU];
        error_code >>= 4U;
    }
}

void F1094_DisplayAlertCSBSystemError(uint32_t error_code)
{
    csb_v1_f1094_display_alert_csb_system_error_pc34_compat(error_code);
}

const char *csb_v1_f1094_display_alert_csb_system_error_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGINIT.C:459 F1094_DisplayAlertCSBSystemError; "
           "terminal Amiga Intuition DisplayAlert/ResetAmiga boundary for "
           "system errors, no PC34 host alert or restart route";
}
