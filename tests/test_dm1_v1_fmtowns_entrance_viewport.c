#include "dm1_v1_fmtowns_entrance_viewport.h"

#include <stdio.h>
#include <string.h>

static uint8_t assets[30][224 * 136];
static int failures;

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expression); \
        ++failures; \
    } \
} while (0)

static int source_graphic(void *user_data, unsigned int record,
                         const uint8_t **pixels, int *width, int *height)
{
    unsigned int x;
    unsigned int y;
    int *calls = (int *)user_data;

    if (record < 78u || record > 107u || !pixels || !width || !height) {
        return 0;
    }
    if (calls) ++*calls;
    for (y = 0; y < 136u; ++y) {
        for (x = 0; x < 224u; ++x) {
            assets[record - 78u][y * 224u + x] =
                (uint8_t)(record + x * 3u + y * 7u);
        }
    }
    *pixels = assets[record - 78u];
    *width = 224;
    *height = 136;
    return 1;
}

static int missing_wall_graphic(void *user_data, unsigned int record,
                               const uint8_t **pixels, int *width, int *height)
{
    if (record >= 86u && record <= 107u) return 0;
    return source_graphic(user_data, record, pixels, width, height);
}

static int fmtowns_native_size(unsigned int record, int *width, int *height)
{
    static const struct { unsigned short record, width, height; } native[] = {
        { 78, 25, 94 }, { 79, 21, 65 }, { 95, 158, 100 },
        { 96, 32, 44 }, { 101, 32, 91 }, { 105, 20, 43 },
        { 106, 20, 39 }
    };
    size_t i;

    for (i = 0; i < sizeof(native) / sizeof(native[0]); ++i) {
        if (native[i].record == record) {
            *width = native[i].width;
            *height = native[i].height;
            return 1;
        }
    }
    return 0;
}

static int native_towns_graphic(void *user_data, unsigned int record,
                                const uint8_t **pixels, int *width, int *height)
{
    if (!source_graphic(user_data, record, pixels, width, height)) return 0;
    (void)fmtowns_native_size(record, width, height);
    return 1;
}

int main(void)
{
    uint8_t viewport[224 * 136];
    uint8_t repeat[224 * 136];
    DM1_V1_FmtownsEntranceViewportReceipt receipt;
    DM1_V1_FmtownsEntranceViewportReceipt repeat_receipt;
    int provider_calls = 0;

    memset(viewport, 0xee, sizeof(viewport));
    {
    int rendered = dm1_v1_fmtowns_entrance_render_c255(
        viewport, sizeof(viewport), 224, native_towns_graphic, &provider_calls,
        DM1_FMTOWNS_LANG_EN, &receipt);
    CHECK(rendered);
    }
    CHECK(receipt.valid);
    CHECK(receipt.aperture_x == 0 && receipt.aperture_y == 33);
    CHECK(receipt.missing_source_graphics == 0u);
    CHECK((receipt.loaded_source_graphics & 3u) == 3u);
    CHECK((receipt.requested_source_graphics & UINT32_C(0x00fffffc)) != 0u);
    CHECK(provider_calls > 2);
    CHECK(receipt.pixel_hash_fnv1a != 0u);
    memset(repeat, 0x55, sizeof(repeat));
    CHECK(dm1_v1_fmtowns_entrance_render_c255(
        repeat, sizeof(repeat), 224, native_towns_graphic, NULL,
        DM1_FMTOWNS_LANG_EN, &repeat_receipt));
    CHECK(repeat_receipt.pixel_hash_fnv1a == receipt.pixel_hash_fnv1a);
    CHECK(memcmp(viewport, repeat, sizeof(viewport)) == 0);
    CHECK(dm1_v1_fmtowns_entrance_render_c255(
        repeat, sizeof(repeat), 224, native_towns_graphic, NULL,
        DM1_FMTOWNS_LANG_JP, &repeat_receipt));
    CHECK(repeat_receipt.aperture_x == 0 && repeat_receipt.aperture_y == 31);
    CHECK(repeat_receipt.pixel_hash_fnv1a == receipt.pixel_hash_fnv1a);

    memset(viewport, 0xee, sizeof(viewport));
    CHECK(!dm1_v1_fmtowns_entrance_render_c255(
        viewport, sizeof(viewport), 224, missing_wall_graphic, NULL,
        DM1_FMTOWNS_LANG_JP, &receipt));
    CHECK(!receipt.valid);
    CHECK(receipt.aperture_y == 31);
    CHECK(receipt.missing_source_graphics != 0u);
    CHECK(viewport[0] == 0xee && viewport[sizeof(viewport) - 1] == 0xee);

    if (failures) return 1;
    puts("PASS: DM1 FM Towns C255 entrance viewport");
    return 0;
}
