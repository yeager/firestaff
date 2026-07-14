#include "redmcsb_f1042_uninstall_interrupt_handler_pc34_compat.h"

bool redmcsb_f1042_uninstall_interrupt_handler_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1042_uninstall_interrupt_handler_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:1076-1085 encloses F1042_ in "
           "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J. IO.C:1081-1083 "
           "calls INT1_05_UninstallInterruptHandler(&G3146_s_InterruptHandler) "
           "only in MEDIA692_X31J. STARTUP2.C:1674-1680 calls F1042_ only "
           "from the MEDIA692_X31J F0750_CPSX cleanup route before "
           "F1018_Mfree. No PC 3.4 branch or portable host adapter is "
           "supplied.";
}
