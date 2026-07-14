#include "redmcsb_f1068_free_amiga_stuff_pc34_compat.h"

void redmcsb_f1068_free_amiga_stuff_pc34_compat(void)
{
}

const char *redmcsb_f1068_free_amiga_stuff_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_"
           "AU3E Amiga-only source guard. AMIGINIT.C:630-671 defines "
           "F1068_FreeAmigaStuff for game media variants: it waits for the "
           "blitter, frees Amiga chip/fast memory and the fuzzy-sector "
           "buffer where applicable, releases audio and CPSX resources, "
           "deinitializes floppy, copper-interrupt, and vertical-blank "
           "services, then closes Amiga resources. HINT001.C:4,65-72 "
           "defines the MEDIA763_AU1E_AU2E hint variant: it deinitializes "
           "floppy and vertical blank, uninitializes the screen, then "
           "closes Amiga resources. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
