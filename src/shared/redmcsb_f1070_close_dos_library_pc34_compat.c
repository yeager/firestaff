#include "redmcsb_f1070_close_dos_library_pc34_compat.h"

void redmcsb_f1070_close_dos_library_pc34_compat(void)
{
}

const char *redmcsb_f1070_close_dos_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_"
           "AU3E Amiga-only source guard. AMIGINIT.C:79-86 defines "
           "F1070_CloseDosLibrary: when DOSBase is non-null, it calls "
           "CloseLibrary(DOSBase) and then clears DOSBase. AMIGINIT.C:363-"
           "373 calls F1070_CloseDosLibrary from F1089_CloseAmigaStuff. "
           "No PC 3.4 branch or portable host behavior is supplied by the "
           "source.";
}
