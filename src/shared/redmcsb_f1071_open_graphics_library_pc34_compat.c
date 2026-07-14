#include "redmcsb_f1071_open_graphics_library_pc34_compat.h"

void redmcsb_f1071_open_graphics_library_pc34_compat(void)
{
}

const char *redmcsb_f1071_open_graphics_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:88-99 defines "
           "F1071_OpenGraphicsLibrary: it opens graphics.library at "
           "version 31 into GfxBase and reports 0x80FF0003 for game media "
           "or 0x80F10003 for other media when OpenLibrary fails. "
           "AMIGINIT.C:333-361 calls F1071_OpenGraphicsLibrary while "
           "opening Amiga resources. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
