#include "redmcsb_f0915_graphic538_pc34_compat.h"

bool redmcsb_f0915_graphic538_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0915_graphic538_source_evidence_pc34(void)
{
    return "ReDMCSB GRAPH538.C:1-56 directly programs Atari ST PSG, DMA, "
           "and FDC registers for the sector-7 read; GRAPH538.C:58-76 and "
           "GRAPH538.C:78-110 acquire Amiga disk.resource and program "
           "CIA/custom DMA. "
           "No PC 3.4 branch or portable host adapter is supplied.";
}
