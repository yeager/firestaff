#include "redmcsb_f1048_setjmp.h"
#include "redmcsb_f1049_longjmp_pc34_compat.h"
#include "redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat.h"
#include "redmcsb_f1055_post_f0380_command_process_queue_pc34_compat.h"
#include "redmcsb_f1061_pre_unreferenced_pc34_compat.h"

const char *redmcsb_f1048_setjmp_source_evidence(void)
{
    return "ReDMCSB DEFS.H:3209 F1048_setjmp native alias";
}

bool redmcsb_f1049_longjmp_pc34_compat(void)
{
    return false;
}

bool F1049_longjmp(void)
{
    return redmcsb_f1049_longjmp_pc34_compat();
}

const char *redmcsb_f1049_longjmp_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:3210 F1049_longjmp disabled non-PC alias; "
           "no PC34 portable longjmp adapter";
}

void redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat(void)
{
}

void F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC(void)
{
    redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat();
}

const char *redmcsb_f1053_pre_f0380_command_process_queue_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGA.H:400 F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC; "
           "Amiga-only FAKE code container, no portable behavior";
}

void redmcsb_f1055_post_f0380_command_process_queue_pc34_compat(void)
{
}

void F1055_Post_F0380_COMMAND_ProcessQueue_CPSC(void)
{
    redmcsb_f1055_post_f0380_command_process_queue_pc34_compat();
}

const char *redmcsb_f1055_post_f0380_command_process_queue_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGA.H:401 F1055_Post_F0380_COMMAND_ProcessQueue_CPSC; "
           "Amiga-only copy-protection padding, no portable behavior";
}

void redmcsb_f1061_pre_unreferenced_pc34_compat(void)
{
}

void F1061_Pre_Unreferenced(void)
{
    redmcsb_f1061_pre_unreferenced_pc34_compat();
}

const char *redmcsb_f1061_pre_unreferenced_source_evidence_pc34(void)
{
    return "ReDMCSB READWRIT.C:75 F1061_Pre_Unreferenced; "
           "Amiga-only copy-protection padding, no portable behavior";
}
