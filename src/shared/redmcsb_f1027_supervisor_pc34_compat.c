#include "redmcsb_f1027_supervisor_pc34_compat.h"

bool redmcsb_f1027_supervisor_pc34_compat(long supervisor_stack)
{
    (void)supervisor_stack;
    return false;
}

const char *redmcsb_f1027_supervisor_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE.C:30-38 and CEDT027.C:540-547 define "
           "F1027_Supervisor only for MEDIA577_X30J and MEDIA692_X31J. "
           "Each pushes its long argument, invokes X68000 DOS CALL SUPER "
           "(0xFF20), then restores the caller stack by four bytes. DEFS.H:"
           "9636-9639 declares it only in MEDIA607_X30J_X31J. IO.C:1887-"
           "1918 saves a supervisor-stack token with F1027_Supervisor(0L) "
           "before direct video-memory access and passes that token back "
           "afterward. No PC 3.4 branch or portable host adapter is "
           "supplied.";
}
