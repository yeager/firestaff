#include "redmcsb_f1074_close_intuition_library_pc34_compat.h"

void redmcsb_f1074_close_intuition_library_pc34_compat(void)
{
}

const char *redmcsb_f1074_close_intuition_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:122-129 defines "
           "F1074_CloseIntuitionLibrary: when IntuitionBase is non-null, "
           "it calls CloseLibrary(IntuitionBase) and then clears "
           "IntuitionBase. AMIGINIT.C:363-385 calls "
           "F1074_CloseIntuitionLibrary from F1089_CloseAmigaStuff when "
           "EXETYPE is not C12_FIO1. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
