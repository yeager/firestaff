/*
 * test_dm2_v1_gdat_word_values.c
 *
 * Focused DM2 GDAT typed-data gate. This covers skproject dtWordValue
 * access for item metadata used by HUD/dungeon item rendering.
 */

#include "dm2_v1_asset_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
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

static int load_graphics(uint8_t **out_data, size_t *out_size, char *path, size_t path_size)
{
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;

    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static void test_item_word_values_real_data(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_AssetLoader loader;
    int category_count = 0;
    int weight_count = 0;
    int money_count = 0;
    int flags_count = 0;
    int equip_count = 0;
    uint32_t receipt_hash = 0x32495756u;

    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return;
    }

    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real DM2 GRAPHICS.DAT initializes typed GDAT loader");
    if (!loader.loaded) {
        free(graphics);
        return;
    }

    for (int category = DM2_GDAT_CATEGORY_WEAPONS;
         category <= DM2_GDAT_CATEGORY_MISCELLANEOUS;
         ++category) {
        int category_hit = 0;
        for (int index = 0; index < 0x100; ++index) {
            uint16_t value;
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x00, &value)) {
                ++flags_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x01, &value)) {
                ++weight_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x02, &value)) {
                ++money_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x04, &value)) {
                ++equip_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
        }
        if (category_hit) ++category_count;
    }

    CHECK(category_count >= 4,
          "skproject item categories expose dtWordValue metadata");
    CHECK(flags_count > 0 && weight_count > 0 && money_count > 0,
          "item flags, weight and money word-values are available");
    CHECK(equip_count > 0,
          "item equipment word-values are available where present");
    CHECK(receipt_hash != 0u,
          "item word-value receipt hash is nonzero");

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
}

int main(void)
{
    printf("=== DM2 V1 GDAT Word-Value Test ===\n");
    test_item_word_values_real_data();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
