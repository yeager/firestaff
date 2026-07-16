#include "dm2_v1_pict_picst_helpers.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int category;
    int index;
    int field;
    DM2_V1_PictBits bits;
    const uint8_t *pixels;
    size_t pixel_count;
    int present;
} PictFixture;

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static int pict_bits_provider(int category,
                              int index,
                              int field,
                              DM2_V1_PictBits *out_bits,
                              void *userdata)
{
    PictFixture *fixture = (PictFixture *)userdata;

    if (!fixture || !fixture->present || category != fixture->category ||
        index != fixture->index || field != fixture->field || !out_bits) {
        return 0;
    }
    *out_bits = fixture->bits;
    return 1;
}

static int picst_image_provider(int category,
                                int index,
                                int field,
                                const uint8_t **out_pixels,
                                size_t *out_pixel_count,
                                DM2_V1_PictBits *out_bits,
                                void *userdata)
{
    PictFixture *fixture = (PictFixture *)userdata;

    if (!fixture || !fixture->present || category != fixture->category ||
        index != fixture->index || field != fixture->field || !out_pixels ||
        !out_pixel_count || !out_bits) {
        return 0;
    }
    *out_pixels = fixture->pixels;
    *out_pixel_count = fixture->pixel_count;
    *out_bits = fixture->bits;
    return 1;
}

static void test_query_pict_bits(void)
{
    PictFixture fixture = {
        9, 3, 4, {16, 8, 4, 3, 96u, 0x12345678u}, 0, 0u, 1
    };
    DM2_V1_PictBits bits;
    DM2_V1_PictPicstReceipt receipt;

    expect_true(dm2_v1_QUERY_PICT_BITS(pict_bits_provider, &fixture, 9, 3,
                                       4, &bits, &receipt) == 1,
                "QUERY_PICT_BITS accepts bounded source metadata");
    expect_true(bits.width == 16u && bits.height == 8u &&
                    receipt.valid && receipt.pixel_count == 128u &&
                    receipt.source_hash == 0x12345678u &&
                    strcmp(receipt.symbol, "QUERY_PICT_BITS") == 0,
                "QUERY_PICT_BITS records dimensions and source hash");

    fixture.bits.width = 0u;
    expect_true(dm2_v1_QUERY_PICT_BITS(pict_bits_provider, &fixture, 9, 3,
                                       4, &bits, &receipt) == 0,
                "QUERY_PICT_BITS blocks zero width");
    expect_true(receipt.handled && receipt.blocked && !receipt.valid,
                "invalid PICT metadata is fail-closed");

    expect_true(dm2_v1_QUERY_PICT_BITS(0, &fixture, 9, 3, 4, &bits,
                                       &receipt) == 0,
                "QUERY_PICT_BITS rejects missing provider");
}

static void test_query_picst_image(void)
{
    static const uint8_t pixels[6] = {1, 2, 3, 4, 5, 6};
    PictFixture fixture = {
        1, 4, 2, {3, 2, 8, 8, 14u, 0x00abcddcu}, pixels, sizeof(pixels), 1
    };
    const uint8_t *out_pixels;
    size_t pixel_count;
    DM2_V1_PictPicstReceipt receipt;

    expect_true(dm2_v1_QUERY_PICST_IMAGE(picst_image_provider, &fixture, 1,
                                         4, 2, &out_pixels, &pixel_count,
                                         &receipt) == 1,
                "QUERY_PICST_IMAGE accepts matching source pixel count");
    expect_true(out_pixels == pixels && pixel_count == sizeof(pixels) &&
                    receipt.valid && receipt.pixel_count == 6u &&
                    strcmp(receipt.symbol, "QUERY_PICST_IMAGE") == 0,
                "QUERY_PICST_IMAGE returns caller-owned source pixels");

    fixture.pixel_count = 5u;
    expect_true(dm2_v1_QUERY_PICST_IMAGE(picst_image_provider, &fixture, 1,
                                         4, 2, &out_pixels, &pixel_count,
                                         &receipt) == 0,
                "QUERY_PICST_IMAGE blocks pixel count mismatch");
    expect_true(receipt.blocked && !receipt.valid,
                "mismatched PICST image is fail-closed");
}

static void test_query_picst_it(void)
{
    DM2_V1_PicstItEntry entries[2] = {
        {8, 1, 0, {4, 4, 4, 3, 24u, 0x01020304u}},
        {8, 2, 1, {5, 3, 4, 3, 24u, 0x05060708u}}
    };
    DM2_V1_PicstItEntry selected;
    DM2_V1_PictPicstReceipt receipt;

    expect_true(dm2_v1_QUERY_PICST_IT(entries, 2u, 1u, &selected,
                                      &receipt) == 1,
                "QUERY_PICST_IT selects bounded table entry");
    expect_true(selected.index == 2 && selected.field == 1 &&
                    receipt.category == 8 && receipt.pixel_count == 15u &&
                    strcmp(receipt.symbol, "QUERY_PICST_IT") == 0,
                "QUERY_PICST_IT records selected GDAT address");

    entries[1].bits.source_hash = 0u;
    expect_true(dm2_v1_QUERY_PICST_IT(entries, 2u, 1u, &selected,
                                      &receipt) == 0,
                "QUERY_PICST_IT blocks unproven source hash");
    expect_true(receipt.blocked && !receipt.valid,
                "unproven PICST table entry is fail-closed");

    expect_true(dm2_v1_QUERY_PICST_IT(entries, 2u, 7u, &selected,
                                      &receipt) == 0,
                "QUERY_PICST_IT blocks out-of-range selector");
}

int main(void)
{
    test_query_pict_bits();
    test_query_picst_image();
    test_query_picst_it();
    expect_true(strstr(dm2_v1_pict_picst_helpers_source_evidence(),
                       "QUERY_PICT_BITS:6006") != 0,
                "source evidence includes PICT_BITS symbol");
    expect_true(strstr(dm2_v1_pict_picst_helpers_source_evidence(),
                       "QUERY_PICST_IMAGE:6113") != 0,
                "source evidence includes PICST_IMAGE symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 PICT/PICST helpers: ok");
    return 0;
}
