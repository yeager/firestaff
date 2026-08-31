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
    static const uint8_t k_two_raw_items[] = {
        0x00, 0x02,             /* count */
        0x00, 0x03, 0x00, 0x03, /* compressed-size table */
        0x00, 0x03, 0x00, 0x03, /* decompressed-size table */
        'A', 'B', 'C', 'R', 'A', 'W'
    };
    CSB_AtariStLoader fixture_loader;
    uint8_t fixture_item[3] = {0};

    csb_atari_st_graphics_loader_init(&fixture_loader);
    CHECK(csb_atari_st_graphics_loader_open_bytes(&fixture_loader,
                                                   k_two_raw_items,
                                                   sizeof(k_two_raw_items)),
          "in-memory DMCSB1 fixture parses without creating a disk file");
    CHECK(fixture_loader.item_count == 2u &&
              fixture_loader.items[0].compressed_size == 3u &&
              fixture_loader.items[0].decompressed_size == 3u &&
              fixture_loader.items[1].data_offset ==
                  fixture_loader.data_section_offset + 3u,
          "DMCSB1 uses separate size tables for raw items");
    CHECK(csb_atari_st_graphics_loader_read_item(&fixture_loader, 1,
                                                 fixture_item,
                                                 sizeof(fixture_item)) == 3 &&
              fixture_item[0] == 'R' && fixture_item[1] == 'A' &&
              fixture_item[2] == 'W',
          "in-memory raw item is returned byte-for-byte");
    csb_atari_st_graphics_loader_close(&fixture_loader);

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
