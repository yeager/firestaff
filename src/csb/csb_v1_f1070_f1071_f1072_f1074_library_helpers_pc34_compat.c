#include "csb_v1_f1070_f1071_f1072_f1074_library_helpers_pc34_compat.h"
#include "redmcsb_f1070_close_dos_library_pc34_compat.h"
#include "redmcsb_f1071_open_graphics_library_pc34_compat.h"
#include "redmcsb_f1072_close_graphics_library_pc34_compat.h"
#include "redmcsb_f1074_close_intuition_library_pc34_compat.h"

void redmcsb_f1070_close_dos_library_pc34_compat(void)
{
}

void F1070_CloseDosLibrary(void)
{
    redmcsb_f1070_close_dos_library_pc34_compat();
}

const char *redmcsb_f1070_close_dos_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:79-87 defines "
           "F1070_CloseDosLibrary: it closes DOSBase when present and clears "
           "the Amiga DOS library handle. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}

void redmcsb_f1071_open_graphics_library_pc34_compat(void)
{
}

void F1071_OpenGraphicsLibrary(void)
{
    redmcsb_f1071_open_graphics_library_pc34_compat();
}

const char *redmcsb_f1071_open_graphics_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:89-99 defines "
           "F1071_OpenGraphicsLibrary: it opens graphics.library at version "
           "31 into GfxBase and reports failure through the Amiga error "
           "route. No PC 3.4 branch or portable host behavior is supplied by "
           "the source.";
}

void redmcsb_f1072_close_graphics_library_pc34_compat(void)
{
}

void F1072_CloseGraphicsLibrary(void)
{
    redmcsb_f1072_close_graphics_library_pc34_compat();
}

const char *redmcsb_f1072_close_graphics_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:101-108 defines "
           "F1072_CloseGraphicsLibrary: it closes GfxBase when present and "
           "clears the Amiga graphics library handle. No PC 3.4 branch or "
           "portable host behavior is supplied by the source.";
}

void redmcsb_f1074_close_intuition_library_pc34_compat(void)
{
}

void F1074_CloseIntuitionLibrary(void)
{
    redmcsb_f1074_close_intuition_library_pc34_compat();
}

const char *redmcsb_f1074_close_intuition_library_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:122-130 defines "
           "F1074_CloseIntuitionLibrary: it closes IntuitionBase when present "
           "and clears the Amiga Intuition library handle. No PC 3.4 branch "
           "or portable host behavior is supplied by the source.";
}
