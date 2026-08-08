#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_Engine engine;
    Nexus_V1_SmapRuntimeReceipt receipt;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 77;
    }
    /* LEV00 is intentionally not loadable into runtime state until the
     * authentic Saturn start-selector/pose is captured.  SMAP ownership is
     * independent of that startup gate, so use the next hash-authenticated
     * retail level to test the real automap decode path. */
    if (nexus_v1_init(&engine, data_dir) != 0 ||
        nexus_v1_load_level(&engine, 1) != 0) {
        fprintf(stderr, "FAIL: could not load retail Nexus level 1\n");
        return 1;
    }
    if (nexus_v1_current_level_smap_runtime_receipt(&engine, &receipt) != 0 ||
        !receipt.valid || receipt.level_index != 1 ||
        !receipt.source.canonical_hash_verified ||
        !receipt.header.valid || !receipt.decode.valid ||
        receipt.decode.rendered_width != NEXUS_SMAP_PIXEL_WIDTH ||
        receipt.decode.rendered_height != NEXUS_SMAP_PIXEL_HEIGHT ||
        !receipt.decoded_pixels_retained || !receipt.no_draw_only ||
        receipt.vdp2_placement_proven ||
        receipt.explored_state_write_proven ||
        receipt.fallback_visuals_permitted) {
        fprintf(stderr, "FAIL: SMAP runtime binding is not source-gated\n");
        return 1;
    }
    puts("Nexus SMAP runtime binding: PASS (retail decode, VDP2 no-draw gate)");
    return 0;
}
