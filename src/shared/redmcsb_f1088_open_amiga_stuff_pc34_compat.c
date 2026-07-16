#include "redmcsb_f1088_open_amiga_stuff_pc34_compat.h"

void redmcsb_f1088_open_amiga_stuff_pc34_compat(void)
{
}

const char *redmcsb_f1088_open_amiga_stuff_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:333-361 defines F1088_OpenAmigaStuff under the "
           "AMIGINIT.C:4 Amiga media guard. It sequences ExecBase/A5 setup, "
           "Amiga libraries, C03 task/vector/input setup, console device, and "
           "NIL: according to EXETYPE. No PC 3.4 branch or portable host "
           "startup behavior is supplied by the source.";
}
