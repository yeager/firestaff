#include "redmcsb_f1020_initialize_x68000_pc34_compat.h"

bool redmcsb_f1020_initialize_x68000_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1020_initialize_x68000_source_evidence_pc34(void)
{
    return "ReDMCSB STARTUP2.C:1361-1374 calls "
           "F1020_InitializeX68000 only in MEDIA607_X30J_X31J, before "
           "F1024_SetTrap14VectorErrorProcessing, graphics-header reading, "
           "layout initialization, graphic-data initialization, and mouse "
           "pointer update. STARTUP2.C:1625-1671 seeds the random number "
           "with IOCS TIMEGET (trap #15), then initializes X30J "
           "MEDIA577_X30J memory from F1017_Malloc (800000 bytes, with a "
           "low-24-bit fallback), or X31J MEDIA692_X31J memory from "
           "F1017_Malloc (1000000 bytes, with the same fallback). Both "
           "routes allocate interface-and-scroll font colors; X30J also "
           "allocates conversion tables and calls F1019_, while X31J calls "
           "VDEO_19_Operation, inverts G3076_B_, and derives its viewport "
           "address from the physical screen. No PC 3.4 branch or portable "
           "host adapter is supplied.";
}
