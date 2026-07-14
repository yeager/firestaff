#include "redmcsb_f1072_close_graphics_library_pc34_compat.h"

void redmcsb_f1072_close_graphics_library_pc34_compat(void)
{
}

const char *redmcsb_f1072_close_graphics_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_"
           "AU3E Amiga-only source guard. AMIGINIT.C:101-108 defines "
           "F1072_CloseGraphicsLibrary: when GfxBase is non-null, it calls "
           "CloseLibrary(GfxBase) and then clears GfxBase. AMIGINIT.C:363-"
           "375 calls F1072_CloseGraphicsLibrary from F1089_CloseAmigaStuff "
           "when EXETYPE is not C12_FIO1. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
