
#include "csb_v2_minimap.h"

uint32_t csb_v2_minimap_square_color(int square_type, int has_dsa, int explored) {
    if (!explored) return 0xFF000000;
    if (has_dsa) return 0xFFFF00FF; /* magenta = DSA scripted room */
    switch (square_type) {
        case 0: return 0xFF404040;
        case 1: return 0xFFA0A0A0;
        case 2: return 0xFF0000FF;
        case 3: return 0xFF0000CC;
        case 4: return 0xFFCC0000;
        case 16: return 0xFF806030;
        default: return 0xFF606060;
    }
}

const char *csb_v2_minimap_source_evidence(void) {
    return "CSB V2.2: minimap with DSA room markers (magenta)\n";
}

