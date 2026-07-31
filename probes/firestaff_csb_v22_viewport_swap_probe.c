/*
 * firestaff_csb_v22_viewport_swap_probe.c
 *
 * CSB V2.2 legacy per-cell swap boundary. The former raw-byte classifier and
 * generic 3x3 painter are intentionally retired: F0128 owns each real draw
 * command, including raster, palette, projection, clip and order.
 *
 * Source-lock:
 *   CSBWin/Viewport.cpp:7290  (9-square viewport layout)
 *   ReDMCSB DUNVIEW.C F0128   (CSB viewport routing)
 */

#include "csb_v22_viewport_swap_pc34.h"

#include <stdio.h>
#include <string.h>

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats, const char* id, int ok, const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

int main(void) {
    static const unsigned char raw_values[] = { 0x00, 0x03, 0x10, 0x20, 0x40, 0x80, 0xff };
    static const CSB_V22_SwapShapeType legacy_shapes[] = {
        CSB_V22_SWAP_SHAPE_WALL_STRAIGHT,
        CSB_V22_SWAP_SHAPE_FLOOR_PLAIN,
        CSB_V22_SWAP_SHAPE_CREATURE,
        CSB_V22_SWAP_SHAPE_PRISON_DOOR,
        CSB_V22_SWAP_SHAPE_NONE
    };
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x04, 0x80 },
        { 0x05, 0x40, 0x06 },
        { 0x10, 0x01, 0x20 }
    };
    unsigned char fb[320 * 200];
    ProbeStats stats;
    int i, direction;

    memset(&stats, 0, sizeof(stats));
    for (i = 0; i < (int)(sizeof(raw_values) / sizeof(raw_values[0])); ++i) {
        for (direction = 0; direction < 4; ++direction) {
            probe_record(&stats, "CSB_V22_RAW_CELL_REJECTED",
                         csb_v22_swap_shape_for_cell(raw_values[i], (uint8_t)direction) ==
                             CSB_V22_SWAP_SHAPE_NONE,
                         "raw cell has no authenticated F0128 material receipt");
        }
    }
    for (i = 0; i < (int)(sizeof(legacy_shapes) / sizeof(legacy_shapes[0])); ++i) {
        probe_record(&stats, "CSB_V22_LEGACY_ASSET_REJECTED",
                     csb_v22_swap_asset_id_for_shape(legacy_shapes[i]) == NULL &&
                     csb_v22_swap_category_for_shape(legacy_shapes[i]) == NULL,
                     "legacy shape cannot select an artpack bitmap");
    }

    csb_v22_viewport_swap_update(0, (const unsigned char (*)[3])raw_cells);
    probe_record(&stats, "CSB_V22_OBSERVATION_ONLY",
                 csb_v22_viewport_swap_populated() && !csb_v22_viewport_swap_active(),
                 "cell observation is not a live render admission");

    memset(fb, 0xa5, sizeof(fb));
    probe_record(&stats, "CSB_V22_RENDER_UNBOUND_NO_PAINT",
                 csb_v22_viewport_swap_render(fb, 320, 200) == 0 &&
                 csb_v22_viewport_swap_cells_painted() == 0,
                 "legacy route cannot replace source viewport pixels");
    for (i = 0; i < (int)sizeof(fb); ++i) {
        if (fb[i] != 0xa5) break;
    }
    probe_record(&stats, "CSB_V22_FRAMEBUFFER_PRESERVED", i == (int)sizeof(fb),
                 "no generic viewport rectangle is drawn");

    probe_record(&stats, "CSB_V22_SOURCE_EVIDENCE",
                 strstr(csb_v22_viewport_swap_source_evidence(), "CSBWin") != NULL &&
                 strstr(csb_v22_viewport_swap_source_evidence(), "ReDMCSB") != NULL &&
                 strstr(csb_v22_viewport_swap_source_evidence(), "F0128") != NULL,
                 "source evidence identifies the command owner");
    printf("# summary: %d/%d invariants passed\n", stats.passed, stats.total);
    return stats.failed == 0 ? 0 : 1;
}
