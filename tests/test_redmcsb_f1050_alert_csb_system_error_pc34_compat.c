#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1050_alert_csb_system_error_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1050_alert_csb_system_error_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1050_alert_csb_system_error_pc34_compat(0L));
    assert(!redmcsb_f1050_alert_csb_system_error_pc34_compat(0x80FF0001L));
    assert(!redmcsb_f1050_alert_csb_system_error_pc34_compat(0xFFFFFFFFL));

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:480-495") != NULL);
    assert(strstr(evidence, "F1050_AlertCSBSystemError") != NULL);
    assert(strstr(evidence, "IntuitionBase") != NULL);
    assert(strstr(evidence, "0xDEADBEEF") != NULL);
    assert(strstr(evidence, "0xFEEDBEEF") != NULL);
    assert(strstr(evidence, "Alert(AT_DeadEnd | error, &parameter)") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:459-478") != NULL);
    assert(strstr(evidence, "DisplayAlert") != NULL);
    assert(strstr(evidence, "ResetAmiga") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1050 Amiga terminal-alert boundary");
    return 0;
}
