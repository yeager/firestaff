#include "csb_v1_pc34_wallset_graphics_map.h"

#include <stdio.h>

static int failed;

static void expect_entry(const char *name,
                         int wall_set,
                         CSB_V1_PC34WallSetSurface surface,
                         uint32_t count,
                         int expected_result,
                         uint32_t expected_entry)
{
    uint32_t entry = 0xdeadbeefu;
    int result = csb_v1_pc34_wallset_graphics_entry_index(
        wall_set, surface, count, &entry);
    if (result != expected_result || entry != expected_entry) {
        printf("FAIL %s: got result=%d entry=%u\n", name, result, entry);
        ++failed;
    } else {
        printf("PASS %s\n", name);
    }
}

int main(void)
{
    /* ReDMCSB DEFS.H PC/I34 M646=86, M647=40; DUNVIEW.C F0095 order. */
    expect_entry("set0_door_frame", 0,
                 CSB_V1_PC34_WALLSET_DOOR_FRAME_FRONT_D0C,
                 126u, 1, 86u);
    expect_entry("set0_d1c", 0, CSB_V1_PC34_WALLSET_WALL_D1C,
                 126u, 1, 97u);
    expect_entry("set0_d3c", 0, CSB_V1_PC34_WALLSET_WALL_D3C,
                 126u, 1, 107u);
    expect_entry("set1_d0r", 1, CSB_V1_PC34_WALLSET_WALL_D0R,
                 166u, 1, 133u);
    expect_entry("catalog_boundary_rejects_missing_set", 1,
                 CSB_V1_PC34_WALLSET_WALL_D3C, 126u, 0, 0u);
    expect_entry("negative_set_rejected", -1,
                 CSB_V1_PC34_WALLSET_WALL_D1C, 126u, 0, 0u);
    expect_entry("invalid_surface_rejected", 0,
                 (CSB_V1_PC34WallSetSurface)22, 126u, 0, 0u);
    expect_entry("empty_catalog_rejected", 0,
                 CSB_V1_PC34_WALLSET_WALL_D1C, 0u, 0, 0u);

    printf("%s csb_v1_pc34_wallset_graphics_map\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
