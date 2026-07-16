#include "csb_v1_f1092_f1093_f1094_alert_helpers_pc34_compat.h"
#include "redmcsb_f1090_get_csb_internal_error_message_pc34_compat.h"
#include "redmcsb_f1091_get_csb_system_error_message_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void test_f1092_hex_digits(void)
{
    const char *digits = csb_v1_f1092_get_hexadecimal_digits_pc34_compat();
    const char *named = F1092_GetHexadecimalDigits();
    const char *expected = "0123456789abcdef";
    size_t i;

    CHECK(digits != NULL);
    CHECK(named == digits);
    CHECK(strlen(digits) == 16U);
    CHECK(strcmp(digits, expected) == 0);

    for (i = 0; i < 10U; ++i) {
        CHECK(digits[i] == (char)('0' + i));
    }
    for (i = 10U; i < 16U; ++i) {
        CHECK(digits[i] == (char)('a' + (i - 10U)));
    }
}

static void test_f1090_f1091_templates_and_aliases(void)
{
    uint8_t *internal_msg =
        csb_v1_f1090_get_csb_internal_error_message_pc34_compat();
    uint8_t *system_msg =
        csb_v1_f1091_get_csb_system_error_message_pc34_compat();

    CHECK(internal_msg == (uint8_t *)F1090_GetCSBInternalErrorMessage());
    CHECK(system_msg == (uint8_t *)F1091_GetCSBSystemErrorMessage());
    CHECK(internal_msg ==
          redmcsb_f1090_get_csb_internal_error_message_pc34_compat());
    CHECK(system_msg ==
          redmcsb_f1091_get_csb_system_error_message_pc34_compat());

    CHECK(internal_msg[0] == 0x00U);
    CHECK(internal_msg[1] == 0xB8U);
    CHECK(internal_msg[2] == 0x10U);
    CHECK(memcmp(internal_msg + 3U, "Chaos Strikes Back System Error 00",
                 34U) == 0);
    CHECK(internal_msg[37] == 0x00U);
    CHECK(internal_msg[38] == 0xFFU);
    CHECK(internal_msg[39] == 0x00U);
    CHECK(internal_msg[40] == 0xB8U);
    CHECK(internal_msg[41] == 0x1CU);
    CHECK(strncmp((const char *)(internal_msg + 42U),
                  "Press Left Mouse Button to Restart", 34U) == 0);

    CHECK(system_msg[0] == 0x00U);
    CHECK(system_msg[1] == 0x94U);
    CHECK(system_msg[2] == 0x10U);
    CHECK(memcmp(system_msg + 3U,
                 "Chaos Strikes Back Internal Error: 00000000", 43U) == 0);
    CHECK(system_msg[46] == 0x00U);
    CHECK(system_msg[47] == 0xFFU);
}

static void test_f1093_f1094_mutate_templates_without_host_alert(void)
{
    uint8_t *internal_msg =
        csb_v1_f1090_get_csb_internal_error_message_pc34_compat();
    uint8_t *system_msg =
        csb_v1_f1091_get_csb_system_error_message_pc34_compat();

    csb_v1_f1093_display_alert_csb_internal_error_pc34_compat(42);
    CHECK(internal_msg[35] == '4');
    CHECK(internal_msg[36] == '2');
    F1093_DisplayAlertCSBInternalError(7);
    CHECK(internal_msg[35] == '0');
    CHECK(internal_msg[36] == '7');

    csb_v1_f1094_display_alert_csb_system_error_pc34_compat(0xDEADBEEFU);
    CHECK(memcmp(system_msg + 38U, "deadbeef", 8U) == 0);
    csb_v1_f1094_display_alert_csb_system_error_pc34_compat(0xDEADBEEFU);
    CHECK(memcmp(system_msg + 38U, "deadbeef", 8U) == 0);
    F1094_DisplayAlertCSBSystemError(0U);
    CHECK(memcmp(system_msg + 38U, "00000000", 8U) == 0);
}

static void test_evidence_strings(void)
{
    const char *f1092 =
        csb_v1_f1092_get_hexadecimal_digits_source_evidence_pc34();
    const char *f1090 =
        csb_v1_f1090_get_csb_internal_error_message_source_evidence_pc34();
    const char *f1091 =
        csb_v1_f1091_get_csb_system_error_message_source_evidence_pc34();
    const char *f1093 =
        csb_v1_f1093_display_alert_csb_internal_error_source_evidence_pc34();
    const char *f1094 =
        csb_v1_f1094_display_alert_csb_system_error_source_evidence_pc34();

    check_contains(f1092, "AMIGINIT.C:430");
    check_contains(f1092, "F1092_GetHexadecimalDigits");
    check_contains(f1092, "hexadecimal digit table");

    check_contains(f1090, "AMIGINIT.C:392");
    check_contains(f1090, "F1090_GetCSBInternalErrorMessage");
    check_contains(f1090, "mutable DisplayAlert byte template");

    check_contains(f1091, "AMIGINIT.C:411");
    check_contains(f1091, "F1091_GetCSBSystemErrorMessage");
    check_contains(f1091, "mutable DisplayAlert byte template");

    check_contains(f1093, "AMIGINIT.C:442");
    check_contains(f1093, "F1093_DisplayAlertCSBInternalError");
    check_contains(f1093, "DisplayAlert/ResetAmiga");
    check_contains(f1093, "no PC34 host alert");

    check_contains(f1094, "AMIGINIT.C:459");
    check_contains(f1094, "F1094_DisplayAlertCSBSystemError");
    check_contains(f1094, "DisplayAlert/ResetAmiga");
    check_contains(f1094, "no PC34 host alert");
}

int main(void)
{
    test_f1092_hex_digits();
    test_f1090_f1091_templates_and_aliases();
    test_f1093_f1094_mutate_templates_without_host_alert();
    test_evidence_strings();
    return 0;
}
