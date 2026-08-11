/*
 * test_dm2_v1_gdat_image_helper_receipts.c
 *
 * Focused real-GDAT coverage for the shared DM2 image metadata/local-palette
 * helpers consumed by weather, wall ornaments, and HUD hand-action materials.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_hand_action_gdat.h"
#include "dm2_v1_weather_gdat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_graphics(uint8_t **out_data, size_t *out_size,
                         char *path, size_t path_size)
{
    const char *fmtowns = getenv("FIRESTAFF_DM2_FMTOWNS_GRAPHICS_DAT");
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;

    if (fmtowns && fmtowns[0] && snprintf(path, path_size, "%s", fmtowns) > 0 &&
        read_file(path, out_data, out_size)) {
        return 1;
    }

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static uint32_t hash_pixels(const uint8_t *pixels, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < count; ++i) {
        hash ^= pixels[i];
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_AssetLoader loader;
    int weather_text_count = 0;

    memset(&loader, 0, sizeof(loader));
    printf("=== DM2 V1 GDAT Image Helper Receipt Test ===\n");
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return 0;
    }

    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real DM2 GRAPHICS.DAT initializes GDAT helper loader");
    if (!loader.loaded) {
        free(graphics);
        printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
        return failed == 0 ? 0 : 1;
    }

    {
        DM2_V1_GdatImageMetadata rejected;
        CHECK(dm2_v1_asset_load_image_metadata(
                  &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0xff, 0xff,
                  &rejected) == 0 && rejected.metadata_hash == 0u,
              "missing typed image has no metadata fallback");
    }

    for (int entry = 2; entry <= 5; ++entry) {
        DM2_V1_GdatImageMetadata metadata;
        uint8_t palette16[16];
        uint32_t palette_hash = 0u;
        uint8_t *pixels;
        int width = 0;
        int height = 0;
        DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
        size_t pixel_count;

        CHECK(dm2_v1_asset_load_image_metadata(
                  &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 4, entry,
                  &metadata) == 1 && metadata.width > 0u &&
                  metadata.height > 0u && metadata.bits_per_pixel == 4u &&
                  metadata.metadata_hash != 0u,
              "real hand-action FM Towns IMG2 metadata is source-typed and bounded");
        CHECK(dm2_v1_asset_load_image_local_palette(
                  &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 4, entry,
                  palette16, &palette_hash) == 1 && palette_hash != 0u,
              "real hand-action FM Towns local palette is source-typed");
        pixels = dm2_v1_asset_load_image_field(
            &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 4, entry,
            &width, &height, &format);
        pixel_count = (size_t)width * (size_t)height;
        CHECK(pixels != NULL && width == (int)metadata.width &&
                  height == (int)metadata.height &&
                  format == DM2_IMG_FMT_U4 && pixel_count > 0u &&
                  hash_pixels(pixels, pixel_count) != 0u,
              "real hand-action FM Towns IMG2 pixels decode with matching metadata");
        dm2_v1_asset_free_pixels(pixels);
    }

    {
        int route_count = 0;
        int route_failures = 0;
        int image_failures = 0;

        /* DRAW_HAND_ACTION_ICONS has a 2 x 2 x 4 x 4 source domain:
         * possession side, left/right icon, party position and facing.
         * Exercise the production route for every tuple against the mounted
         * Towns v4 file.  The image bytes are deliberately decoded again
         * through dm2_v1_hand_action_gdat_load_image; this is not a table-only
         * test and cannot pass by accepting a generated surface. */
        for (int possession = 0; possession < 2; ++possession) {
            for (int side = 0; side < 2; ++side) {
                for (int player_position = 0; player_position < 4;
                     ++player_position) {
                    for (int party_direction = 0; party_direction < 4;
                         ++party_direction) {
                        DM2_V1_HandActionInput input;
                        DM2_V1_HandActionGdatRoute route;
                        uint8_t *pixels;
                        int width = 0;
                        int height = 0;
                        DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

                        memset(&input, 0, sizeof(input));
                        input.possession_index = possession;
                        input.left_or_right = side;
                        input.player_position = player_position;
                        input.party_direction = party_direction;
                        ++route_count;
                        if (!dm2_v1_hand_action_gdat_route(&input, &route) ||
                            route.category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                            route.subcategory != 4u ||
                            route.entry != (uint8_t)(possession * 2 + side + 2) ||
                            route.rectno != (uint8_t)((possession == 1 ? 0x46 : 0x4a) +
                                ((player_position + 4 - party_direction) & 3))) {
                            ++route_failures;
                            continue;
                        }
                        pixels = dm2_v1_hand_action_gdat_load_image(
                            &loader, &input, NULL, &width, &height, &format);
                        if (!pixels || width <= 0 || height <= 0 ||
                            format != DM2_IMG_FMT_U4 ||
                            hash_pixels(pixels, (size_t)width * (size_t)height) == 0u) {
                            ++image_failures;
                        }
                        dm2_v1_asset_free_pixels(pixels);
                    }
                }
            }
        }
        CHECK(route_count == 64 && route_failures == 0,
              "all 64 authentic FM Towns hand-action route tuples are source-valid");
        CHECK(image_failures == 0,
              "all 64 authentic FM Towns hand-action tuples decode real U4 pixels");
    }

    for (int graphicsset = 1; graphicsset <= 5; ++graphicsset) {
        for (unsigned int command = DM2_V1_WEATHER_CLOUD_LIGHT_CMD;
             command <= DM2_V1_WEATHER_RAIN_STORM_CMD;
             ++command) {
            size_t size = 0u;
            const uint8_t *text = dm2_v1_asset_load_text_sized(
                &loader, DM2_GDAT_CATEGORY_ENVIRONMENT, graphicsset, command,
                &size);
            if (text && size > 0u) {
                ++weather_text_count;
                CHECK(size <= UINT32_MAX && hash_pixels(text, size) != 0u,
                      "real ENVIRONMENT command text has typed source bytes");
            }
        }
    }
    CHECK(weather_text_count > 0,
          "real ENVIRONMENT command text resolves through typed GDAT helper");

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
