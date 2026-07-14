#include "redmcsb_f0924_set_critical_error_handler_pc34_compat.h"

bool redmcsb_f0924_set_critical_error_handler_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0924_set_critical_error_handler_source_evidence_pc34(
    void)
{
    return "ReDMCSB CEDTINCI.C:306 calls F0924_SetCriticalErrorHandler; "
           "PRIM1.C:253-269 enters supervisor mode with Super(0L), installs "
           "a 68000 handler returning -1 at GEMDOS etv_critic (0x0404), then "
           "restores the saved supervisor stack. No PC 3.4 host adapter for "
           "Atari ST supervisor mode or the low-memory vector table is "
           "supplied.";
}
