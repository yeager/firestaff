/*
 * test_dm2_v1_graphics_data_open_receipt.c
 *
 * Real-data GRAPHICS_DATA_OPEN admission for DM2 GDAT HUD/dungeon material.
 */

#include "dm2_v1_graphics_data_open.h"

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
    const char *dm2_data = getenv("FIRESTAFF_DM2_DATA_DIR");
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;

    /* The mounted DM2 corpus is a game directory, unlike FIRESTAFF_DATA
     * which is the parent data root.  Prefer it when provided so this is a
     * real-data regression on both layouts without making a copy or fixture. */
    if (dm2_data && dm2_data[0]) {
        static const char *names[] = {
            "GRAPHICS.DAT", "graphics.dat", "DM2GRAPHICS.DAT", "DM2GRA.DAT"
        };
        for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i) {
            snprintf(path, path_size, "%s/%s", dm2_data, names[i]);
            if (read_file(path, out_data, out_size)) return 1;
        }
    }

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_GraphicsDataOpenReceipt receipt;

    printf("=== DM2 V1 GRAPHICS_DATA_OPEN Receipt Test ===\n");
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return 0;
    }

    CHECK(dm2_v1_GRAPHICS_DATA_OPEN_receipt(
              graphics, graphics_size, &receipt) == 1 && receipt.valid,
          "real GRAPHICS.DAT opens as source-admitted GDAT handle");
    CHECK(receipt.container_byte_count == graphics_size &&
              receipt.raw_data_count > 0u && receipt.entry_count > 0u &&
              receipt.typed_graph_hash != 0u,
          "receipt records bounded container and typed ENT1 graph identity");
    CHECK(receipt.interface_palette_hash != 0u &&
              receipt.title_menu_pixel_count > 0u &&
              receipt.title_menu_hash != 0u,
          "interface palette and TITLE/0 field-4 source image are present");
    CHECK(receipt.hud_hand_action_image_mask == 0x3cu &&
              receipt.hud_hand_action_palette_hash != 0u &&
              receipt.hud_hand_action_pixel_hash != 0u,
          "INTERFACE_GENERAL/4 hand-action IMG3 localpal and pixels are admitted");
    CHECK(receipt.environment_text_count > 0u &&
              receipt.environment_text_hash != 0u &&
              receipt.admission_hash != 0u,
          "ENVIRONMENT dtText bytes participate in admission hash");
    CHECK(dm2_v1_GRAPHICS_DATA_OPEN_receipt(NULL, 0u, &receipt) == 0 &&
              !receipt.valid,
          "missing GRAPHICS.DAT bytes reject without fallback material");
    CHECK(dm2_v1_GRAPHICS_DATA_OPEN_source_evidence() &&
              strstr(dm2_v1_GRAPHICS_DATA_OPEN_source_evidence(),
                     "GRAPHICS_DATA_OPEN") != NULL,
          "source evidence names GRAPHICS_DATA_OPEN boundary");

    free(graphics);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
