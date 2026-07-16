#include "redmcsb_f1073_open_intuition_library_pc34_compat.h"

void redmcsb_f1073_open_intuition_library_pc34_compat(void)
{
}

const char *redmcsb_f1073_open_intuition_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:88-130 "
           "encloses F1073_OpenIntuitionLibrary for executable types other "
           "than C12_FIO1. AMIGINIT.C:110-120 defines it: it opens "
           "intuition.library at version 31 into IntuitionBase and, when "
           "that fails, calls F1050_AlertCSBSystemError(0x80FF0004) for "
           "C03_GAME or F9073_DisplayError(0x80F10004) for other executable "
           "types. AMIGINIT.C:333-361 calls "
           "F1073_OpenIntuitionLibrary during Amiga initialization. No PC "
           "3.4 branch or portable host behavior is supplied by the source.";
}
