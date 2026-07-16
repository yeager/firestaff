#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                          const uint8_t *data,
                                          int data_size,
                                          int dungeon_id,
                                          int sub_level_index) {
    (void)level;
    (void)data;
    (void)data_size;
    (void)dungeon_id;
    (void)sub_level_index;
    return THERON_MAP_ERR_NULL;
}

void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world) {
    (void)world;
}

static void make_sample(Theron_Track02StartupBitmapSample *sample,
                        unsigned int route_bit,
                        size_t raw_offset,
                        size_t user_data_offset,
                        uint8_t pixel,
                        size_t nonzero_pixel_count,
                        uint32_t checksum) {
    memset(sample, 0, sizeof(*sample));
    sample->route_bit = route_bit;
    sample->raw_offset = raw_offset;
    sample->user_data_offset = user_data_offset;
    sample->byte_count = THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES;
    sample->width = 8u;
    sample->height = 8u;
    sample->bpp = 4u;
    sample->nonzero_pixel_count = nonzero_pixel_count;
    sample->checksum = checksum;
    memset(sample->pixels, pixel, sizeof(sample->pixels));
}

int main(void) {
    Theron_Track02StartupBitmapCatalog catalog;
    Theron_Track02StartupBitmapAtlas atlas;

    memset(&catalog, 0, sizeof(catalog));
    catalog.variant = THERON_TRACK02_VARIANT_US_ISO;
    catalog.sample_count = 4u;
    make_sample(&catalog.samples[0],
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
                0x3000u, 0x3000u, 1u, 64u, 0x1101u);
    make_sample(&catalog.samples[1],
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
                0x3004u, 0x3004u, 2u, 64u, 0x1102u);
    make_sample(&catalog.samples[2],
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
                0x3040u, 0x3040u, 3u, 64u, 0x2201u);
    make_sample(&catalog.samples[3],
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
                0x3044u, 0x3044u, 4u, 64u, 0x2202u);
    catalog.route_mask = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;

    CHECK(theron_v1_track02_build_startup_bitmap_atlas(&catalog, &atlas) ==
          THERON_TRACK02_SIGNAL_OK);
    CHECK(atlas.variant == THERON_TRACK02_VARIANT_US_ISO);
    CHECK(atlas.route_count == 2u);
    CHECK(atlas.route_mask == catalog.route_mask);
    CHECK(atlas.total_tile_count == 4u);
    CHECK(atlas.total_nonzero_pixel_count == 256u);
    CHECK(atlas.routes[0].route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE);
    CHECK(atlas.routes[0].tile_count == 2u);
    CHECK(atlas.routes[0].width == 16u);
    CHECK(atlas.routes[0].height == 8u);
    CHECK(atlas.routes[0].first_raw_offset == 0x3000u);
    CHECK(atlas.routes[0].last_raw_offset == 0x3004u);
    CHECK(atlas.routes[0].raw_offsets[0] == 0x3000u);
    CHECK(atlas.routes[0].raw_offsets[1] == 0x3004u);
    CHECK(atlas.routes[0].user_data_offsets[1] == 0x3004u);
    CHECK(atlas.routes[1].route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
    CHECK(atlas.routes[1].tile_count == 2u);
    CHECK(atlas.routes[1].first_raw_offset == 0x3040u);
    CHECK(atlas.routes[1].last_raw_offset == 0x3044u);
    CHECK(atlas.routes[1].raw_offsets[0] == 0x3040u);
    CHECK(atlas.routes[1].raw_offsets[1] == 0x3044u);
    CHECK(atlas.checksum != 0u);

    catalog.samples[0].nonzero_pixel_count = 0u;
    catalog.samples[1].nonzero_pixel_count = 0u;
    catalog.samples[2].nonzero_pixel_count = 0u;
    catalog.samples[3].nonzero_pixel_count = 0u;
    CHECK(theron_v1_track02_build_startup_bitmap_atlas(&catalog, &atlas) ==
          THERON_TRACK02_SIGNAL_NOT_FOUND);
    CHECK(atlas.route_count == 0u);
    CHECK(atlas.total_tile_count == 0u);
    CHECK(atlas.total_nonzero_pixel_count == 0u);
    CHECK(atlas.checksum == 0u);

    printf("test_theron_v1_track02_bitmap_atlas_layout: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
