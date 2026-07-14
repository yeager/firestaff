#include "redmcsb_f1059_post_f0433_startend_process_command140_save_game_pc34_compat.h"

void redmcsb_f1059_post_f0433_startend_process_command140_save_game_pc34_compat(
    void)
{
}

const char *
redmcsb_f1059_post_f0433_startend_process_command140_save_game_source_evidence_pc34(
    void)
{
    return "ReDMCSB Toolchains/Common/Source/LOADSAVE.C:1783-1801 defines "
           "F1059_Post_F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF "
           "only inside MEDIA626_A31E_A31M_A33M_A35E_A35M. LOADSAVE.C:1788-"
           "1799 includes FAKE2.C for MEDIA618_A31E, FAKE1.C for "
           "MEDIA739_A35E, FAKE4.C for MEDIA662_A31M_A35M, or FAKE3.C for "
           "MEDIA665_A33M. FAKE1.C:1-17, FAKE2.C:1-17, FAKE3.C:1-19, and "
           "FAKE4.C:1-17 each guard a 32 byte long 68k asm block explicitly "
           "marked never executed. AMIGA.H:403 declares F1059. No PC 3.4 "
           "branch or portable save-game hook is supplied.";
}
