#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static int failures;

#define CHECK(condition, message) do { \
    if (condition) printf("PASS: %s\n", message); \
    else { printf("FAIL: %s\n", message); ++failures; } \
} while (0)

int main(void)
{
    const char *real_path = getenv("FIRESTAFF_CSB_ANIMATE_DAT");

    CHECK(csb_atari_st_graphics_loader_self_test() == 0,
          "DMCSB1 uses separate size tables and preserves raw items");

    if (real_path && real_path[0] != '\0') {
        CSB_AtariStLoader loader;
        csb_atari_st_graphics_loader_init(&loader);
        CHECK(csb_atari_st_graphics_loader_open(&loader, real_path),
              "real Atari ANIMATE.DAT opens");
        if (loader.loaded) {
            CHECK(loader.item_count == 87u &&
                      loader.data_section_offset == 350u &&
                      loader.items[0].compressed_size == 32u &&
                      loader.items[0].decompressed_size == 32u &&
                      loader.items[30].compressed_size == 4613u &&
                      loader.items[30].decompressed_size == 4613u &&
                      loader.items[75].compressed_size == 7801u &&
                      loader.items[75].decompressed_size == 7801u &&
                      loader.items[86].compressed_size == 712u &&
                      loader.items[86].decompressed_size == 712u,
                  "real ANIMATE.DAT has the documented 87-item DMCSB1 layout");
            csb_atari_st_graphics_loader_close(&loader);
        }
    }

    return failures == 0 ? 0 : 1;
}
