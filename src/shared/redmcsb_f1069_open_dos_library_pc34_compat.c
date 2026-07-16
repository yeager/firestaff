#include "redmcsb_f1069_open_dos_library_pc34_compat.h"

void redmcsb_f1069_open_dos_library_pc34_compat(void)
{
}

const char *redmcsb_f1069_open_dos_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_"
           "AU3E Amiga-only source guard. AMIGINIT.C:67-77 defines "
           "F1069_OpenDosLibrary: it opens dos.library at version 31 into "
           "DOSBase and, when that fails, calls "
           "F1050_AlertCSBSystemError(0x80FF0001) for C03_GAME or "
           "F9073_DisplayError(0x80F10001) for other executable types. "
           "AMIGINIT.C:343 calls F1069_OpenDosLibrary during Amiga "
           "initialization. No PC 3.4 branch or portable host behavior is "
           "supplied by the source.";
}
