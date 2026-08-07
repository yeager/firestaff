/* Real DM2 GRAPHICS.DAT proof of the IMG9 global-palette identity for the
 * weather slot commands 0x64..0x6c.
 *
 * skproject provenance:
 * - SkWinCore::QUERY_GDAT_IMAGE_LOCALPAL (SkWinCore.cpp 3e74:521A,
 *   DM2_EXTENDED_MODE == 1) returns NULL whenever the realized image is not
 *   4bpp, so the real 8bpp IMG9 ENVIRONMENT command images carry no
 *   16-color local palette.
 * - SkWinCore::QUERY_GDAT_SUMMARY_IMAGE (SkWinCore.cpp 0B36:0520) then
 *   installs the 256-entry identity translation (ref->b58[i] = i,
 *   ref->w56 = 256): each decoded pixel byte indexes the global screen
 *   palette directly.
 * This binds that palette receipt against the actual GDAT data so the
 * weather chain reaches a full material receipt with no synthetic
 * substitute. */

#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int load_graphics(uint8_t **out, size_t *out_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1100];

    if (!root || !root[0]) return 0;
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    return read_file(path, out, out_size);
}

static int check(int condition, const char *label)
{
    if (condition) {
        printf("PASS: %s\n", label);
        return 1;
    }
    printf("FAIL: %s\n", label);
    return 0;
}

/* The exact FNV-1a step of dm2_weather_hash_step over the SUMMARY_IMAGE
 * identity table b58[0..255] = 0..255. */
static uint32_t summary_image_identity_hash(void)
{
    uint32_t hash = 2166136261u;
    unsigned int entry;

    for (entry = 0u; entry < 256u; ++entry) {
        hash ^= entry;
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    /* Real set-5 extents verified against the canonical DM2 GRAPHICS.DAT:
     * bolts 16x36/23x33/28x38, clouds 224x39, rain 224x62, all 8bpp. */
    static const uint16_t expected_width[9] = {
        16u, 23u, 28u, 224u, 224u, 224u, 224u, 224u, 224u
    };
    static const uint16_t expected_height[9] = {
        36u, 33u, 38u, 39u, 39u, 39u, 62u, 62u, 62u
    };
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_WeatherGdatReceipt weather;
    DM2_V1_WeatherOverlayPlan plan;
    uint32_t identity_hash;
    unsigned int weather_command_text_count = 0u;
    int failures = 0;
    int i;

    if (!getenv("FIRESTAFF_DM2_DATA_DIR") ||
        !getenv("FIRESTAFF_DM2_DATA_DIR")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not set");
        return 0;
    }
    if (!load_graphics(&graphics, &graphics_size)) {
        fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        puts("FAIL: canonical DM2 GRAPHICS.DAT did not load");
        free(graphics);
        return 1;
    }

    for (i = 0; i < (int)loader.entry_count; ++i) {
        if (loader.entries[i].cls1 == DM2_GDAT_CATEGORY_ENVIRONMENT &&
            loader.entries[i].cls2 == 5u &&
            loader.entries[i].cls3 == DM2_GDAT_ENTRY_TYPE_TEXT &&
            loader.entries[i].cls4 >= 0x64u &&
            loader.entries[i].cls4 <= 0x6cu) {
            ++weather_command_text_count;
        }
    }
    failures += !check(
        weather_command_text_count == 9u,
        "real set 5 text rows are the nine source weather command payloads");

    identity_hash = summary_image_identity_hash();
    failures += !check(identity_hash != 0u,
                       "SUMMARY_IMAGE identity table hashes nonzero");

    memset(&weather, 0, sizeof(weather));
    failures += !check(
        dm2_v1_weather_gdat_receipt(&loader, 5u, &weather) &&
            weather.valid && weather.graphicsset == 5u &&
            weather.command_mask == 0x1ffu &&
            weather.material_mask == 0x1ffu &&
            weather.receipt_hash != 0u,
        "real set 5 binds all nine 0x64..0x6c commands to full material");

    for (i = 0; i < 9; ++i) {
        const DM2_V1_WeatherCommandReceipt *command = &weather.commands[i];
        char label[160];

        snprintf(label, sizeof(label),
                 "command 0x%02x decodes real 8bpp IMG9 with proven extent",
                 0x64 + i);
        failures += !check(
            command->material_valid && command->decoded_pixels_valid &&
                command->query_metadata_valid &&
                command->query_metadata.bits_per_pixel == 8u &&
                command->decoded_format == DM2_IMG_FMT_IMG9 &&
                command->decoded_width == expected_width[i] &&
                command->decoded_height == expected_height[i] &&
                command->decoded_pixel_count ==
                    (uint32_t)expected_width[i] * expected_height[i] &&
                command->decoded_pixels_hash != 0u,
            label);

        snprintf(label, sizeof(label),
                 "command 0x%02x binds the SUMMARY_IMAGE 256-entry identity",
                 0x64 + i);
        failures += !check(
            !command->local_palette_valid &&
                command->local_palette_hash == 0u &&
                command->global_palette_identity_valid &&
                command->global_palette_identity_hash == identity_hash &&
                command->palette_translation_count == 256u &&
                command->palette_translation_hash == identity_hash,
            label);
    }

    /* The full material chain now authorizes the source overlay plan:
     * storm cloud (0x80) + light rain (0x40) select 0x69 and 0x6a, both
     * material-valid only through the IMG9 identity receipt. */
    memset(&plan, 0, sizeof(plan));
    failures += !check(
        dm2_v1_weather_gdat_overlay_plan(&weather, 0x80u, 0x40u, &plan) &&
            plan.valid && plan.command_count == 2u &&
            plan.commands[0].command == 0x69u &&
            plan.commands[1].command == 0x6au &&
            plan.commands[0].material_hash != 0u &&
            plan.commands[1].material_hash != 0u &&
            plan.material_mask ==
                (DM2_V1_WEATHER_COMMAND_MASK(0x69u) |
                 DM2_V1_WEATHER_COMMAND_MASK(0x6au)),
        "storm cloud and light rain reach the overlay plan as full material");

    free(graphics);
    if (failures) {
        printf("DM2 weather IMG9 global-palette identity: %d failure(s)\n",
               failures);
        return 1;
    }
    puts("DM2 weather IMG9 global-palette identity: all checks passed");
    return 0;
}
