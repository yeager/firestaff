#include "redmcsb_f1089_close_amiga_stuff_pc34_compat.h"

void redmcsb_f1089_close_amiga_stuff_pc34_compat(void)
{
}

const char *redmcsb_f1089_close_amiga_stuff_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:363-389 defines F1089_CloseAmigaStuff under the "
           "AMIGINIT.C:4 Amiga media guard. It reverses the EXETYPE-selected "
           "NIL, console, input, library, vector, and AMISTRUCT setup. No PC "
           "3.4 branch or portable host teardown behavior is supplied by the "
           "source.";
}
