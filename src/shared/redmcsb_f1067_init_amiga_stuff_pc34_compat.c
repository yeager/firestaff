#include "redmcsb_f1067_init_amiga_stuff_pc34_compat.h"

void redmcsb_f1067_init_amiga_stuff_pc34_compat(void)
{
}

const char *redmcsb_f1067_init_amiga_stuff_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:539-628 defines "
           "F1067_InitAmigaStuff for Amiga media variants: it opens Amiga "
           "resources, initializes vertical blank and a copper interrupt, "
           "initializes CPSX and audio, and allocates Amiga chip/fast memory. "
           "HINT001.C:4,56-63 defines the MEDIA763_AU1E_AU2E variant: it "
           "opens Amiga resources, initializes the screen and vertical blank, "
           "then invokes CPSX. No PC 3.4 branch or portable host behavior is "
           "supplied by the source.";
}
