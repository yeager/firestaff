
#include "csb_v2_minimap.h"

uint32_t csb_v2_minimap_square_color(int square_type, int has_dsa, int explored) {
    /* ReDMCSB supplies map semantics, not this host RGB legend or a DSA
     * marker palette. Leave the overlay transparent until a source-owned
     * minimap surface and colour transaction are available. */
    (void)square_type;
    (void)has_dsa;
    (void)explored;
    return 0u;
}

const char *csb_v2_minimap_source_evidence(void) {
    return "ReDMCSB DUNGEON.C map semantics; no source receipt admits a "
           "host-coloured CSB V2 minimap.\n";
}
