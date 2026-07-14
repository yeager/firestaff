#include "redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat.h"

void redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat(void)
{
}

const char *redmcsb_f1053_pre_f0380_command_process_queue_source_evidence_pc34(
    void)
{
    return "ReDMCSB Toolchains/Common/Source/COMMAND.C:2028-2043 encloses "
           "F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC in "
           "MEDIA626_A31E_A31M_A33M_A35E_A35M. COMMAND.C:2033-2040 "
           "includes FAKE3.C for MEDIA641_A31E_A33M_A35E, FAKE1.C for "
           "MEDIA655_A31M, or FAKE2.C for MEDIA742_A35M. FAKE1.C:1-17, "
           "FAKE2.C:1-17, and FAKE3.C:1-19 each guard a 32 byte long "
           "68k asm block explicitly marked never executed. AMIGA.H:400 "
           "declares F1053. No PC 3.4 branch or portable queue hook is "
           "supplied.";
}
