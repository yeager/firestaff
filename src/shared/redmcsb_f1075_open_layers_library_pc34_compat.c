#include "redmcsb_f1075_open_layers_library_pc34_compat.h"

void redmcsb_f1075_open_layers_library_pc34_compat(void)
{
}

const char *redmcsb_f1075_open_layers_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:132-143 defines "
           "F1075_OpenLayersLibrary: it opens layers.library at version 31 "
           "into LayersBase and reports 0x80FF0005 for game media or "
           "0x80F10005 for other media when OpenLibrary fails. "
           "AMIGINIT.C:333-361 calls F1075_OpenLayersLibrary while opening "
           "Amiga resources. No PC 3.4 branch or portable host behavior is "
           "supplied by the source.";
}
