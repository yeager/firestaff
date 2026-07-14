#include "redmcsb_f1076_close_layers_library_pc34_compat.h"

void redmcsb_f1076_close_layers_library_pc34_compat(void)
{
}

const char *redmcsb_f1076_close_layers_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_"
           "AU3E Amiga-only source guard. AMIGINIT.C:145-152 defines "
           "F1076_CloseLayersLibrary: when LayersBase is non-null, it calls "
           "CloseLibrary(LayersBase) and then clears LayersBase. "
           "AMIGINIT.C:363-378 calls F1076_CloseLayersLibrary from "
           "F1089_CloseAmigaStuff when EXETYPE is C03_GAME, C07_HINT, "
           "C01_SWOOSH, or defined(MEDIA762_AU1E) with C06_CEDT. No PC "
           "3.4 branch or portable host behavior is supplied by the source.";
}
