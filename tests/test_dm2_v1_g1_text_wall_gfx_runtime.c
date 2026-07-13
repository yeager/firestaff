#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int failures;

static void expect_true(int condition, const char *label)
{
    ++checks;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

typedef struct {
    int omit_image_offset;
} GdatFixture;

static int read_scalar(void *userdata, int data_type, int category,
                       int entry, int field, uint16_t *out_value)
{
    const GdatFixture *fixture = (const GdatFixture *)userdata;

    if (!fixture || !out_value || category != 0x09 || entry != 0x2a)
        return 0;
    if (data_type == 0x0b && field == 0x04) *out_value = 9;
    else if (data_type == 0x0b && field == 0x05) *out_value = 12;
    else if (data_type == 0x0b && field == 0x07) *out_value = 1;
    else if (data_type == 0x0b && field == 0x0a) *out_value = 3;
    else if (data_type == 0x0c && field == 0xfd && !fixture->omit_image_offset)
        *out_value = 0xfe02;
    else return 0;
    return 1;
}

int main(void)
{
    DM2_V1_G1Map5TextRuntimeReceipt texts;
    DM2_V1_G1TextWallGfxRuntimeReceipt receipt;
    GdatFixture fixture;

    memset(&texts, 0, sizeof(texts));
    memset(&fixture, 0, sizeof(fixture));
    texts.committed = 1;
    texts.incomplete_world = 1;
    texts.map = 5;
    texts.text_root_count = 2;
    texts.texts[0].object_id = 0x8800;
    texts.texts[0].mode = 1;
    texts.texts[0].text_index = 0x022a;
    texts.texts[1].object_id = 0x0801;
    texts.texts[1].mode = 0;
    texts.texts[1].text_index = 0x0033;
    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, read_scalar, &fixture, &receipt) == 1 && receipt.valid,
                "source DB2 text receipt reaches GDAT material consumer");
    expect_true(receipt.material_count == 1 &&
                    receipt.materials[0].object_id == 0x8800 &&
                    receipt.materials[0].wall_gfx_index == 0x2a &&
                    receipt.materials[0].position == 12 &&
                    receipt.materials[0].image_offset == 0xfe02,
                "TextMode one uses TextIndex low byte and original ornament fields");

    {
        static const uint16_t allowed_indices[] = {
            0x002au, 0x022au, 0x032au, 0x052au, 0x0d2au
        };
        for (size_t i = 0; i < sizeof(allowed_indices) / sizeof(allowed_indices[0]); ++i) {
            texts.texts[0].text_index = allowed_indices[i];
            expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                            &texts, read_scalar, &fixture, &receipt) == 1 &&
                            receipt.valid && receipt.material_count == 1 &&
                            receipt.materials[0].wall_gfx_index == 0x2a,
                        "source-authorized SimpleTextExtUsage selects WALL_GFX");
        }
    }

    texts.texts[0].text_index = 0x012a;
    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, read_scalar, &fixture, &receipt) == 1 && receipt.valid &&
                    receipt.material_count == 0,
                "unsupported SimpleTextExtUsage cannot invent a WALL_GFX material");
    texts.texts[0].text_index = 0x022a;

    fixture.omit_image_offset = 1;
    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, read_scalar, &fixture, &receipt) == 0 && !receipt.valid,
                "missing original ornament field blocks the full consumer");
    fixture.omit_image_offset = 0;
    texts.texts[0].mode = 2;
    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, read_scalar, &fixture, &receipt) == 1 && receipt.valid &&
                    receipt.material_count == 0,
                "non-ornamental DB2 text roots do not invent a GDAT material");

    printf("DM2 G1 DB2 wall-gfx runtime: %d/%d checks passed\n",
           checks - failures, checks);
    return failures ? 1 : 0;
}
