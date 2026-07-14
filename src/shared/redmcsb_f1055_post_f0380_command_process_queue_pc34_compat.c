#include "redmcsb_f1055_post_f0380_command_process_queue_pc34_compat.h"

void redmcsb_f1055_post_f0380_command_process_queue_pc34_compat(void)
{
}

const char *redmcsb_f1055_post_f0380_command_process_queue_source_evidence_pc34(
    void)
{
    return "ReDMCSB Toolchains/Common/Source/COMMAND.C:2480-2498 defines "
           "F1055_Post_F0380_COMMAND_ProcessQueue_CPSC only inside "
           "MEDIA626_A31E_A31M_A33M_A35E_A35M. Its body includes FAKE4.C "
           "for MEDIA645_A31E_A35E, FAKE3.C for MEDIA655_A31M, FAKE2.C "
           "for MEDIA665_A33M, or FAKE1.C for MEDIA742_A35M; no portable "
           "command-queue behavior or PC 3.4 branch is supplied.";
}
