#include "redmcsb_f1050_alert_csb_system_error_pc34_compat.h"

bool redmcsb_f1050_alert_csb_system_error_pc34_compat(long error_code)
{
    (void)error_code;
    return false;
}

const char *redmcsb_f1050_alert_csb_system_error_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_"
           "AU3E Amiga-only source guard. AMIGINIT.C:480-495 defines "
           "F1050_AlertCSBSystemError: when IntuitionBase is present it "
           "calls F1094_DisplayAlertCSBSystemError, then selects "
           "0xDEADBEEF or 0xFEEDBEEF and calls Alert(AT_DeadEnd | error, "
           "&parameter). AMIGINIT.C:459-478 shows F1094 disabling the "
           "system, calling DisplayAlert, and ResetAmiga. No PC 3.4 branch "
           "or portable terminal-error adapter is supplied.";
}
