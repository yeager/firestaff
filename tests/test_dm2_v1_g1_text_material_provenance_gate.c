/*
 * PC G1 DB2 map-5 receipt provenance gate.
 *
 * Source-lock:
 *   dm2_v1_dungeon_materialize_g1_map5_text_runtime follows
 *   skproject/SKWIN/DME.h Text::w2 only and commits with zero generic and
 *   blocked record reads. QUERY_CLS2_OF_TEXT_RECORD then maps TextMode one
 *   to WALL_GFX before DRAW_WALL_ORNATE consumes scalar GDAT fields.
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int calls;
} Calls;

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { \
        ++passed; \
        printf("  PASS: %s\n", message); \
    } else { \
        ++failed; \
        printf("  FAIL: %s\n", message); \
    } \
} while (0)

static int read_scalar(void *userdata, int data_type, int category,
                       int entry, int field, uint16_t *out_value)
{
    Calls *calls = (Calls *)userdata;

    ++calls->calls;
    if (category != 9 || entry != 0x2a)
        return 0;
    if (data_type == 0x0b && field == 4) *out_value = 9;
    else if (data_type == 0x0b && field == 5) *out_value = 3;
    else if (data_type == 0x0b && field == 7) *out_value = 1;
    else if (data_type == 0x0b && field == 0x0a) *out_value = 2;
    else if (data_type == 0x0c && field == 0xfd) *out_value = 0xfe02;
    else return 0;
    return 1;
}

int main(void)
{
    DM2_V1_G1Map5TextRuntimeReceipt texts;
    DM2_V1_G1TextWallGfxRuntimeReceipt receipt;
    Calls calls;

    memset(&texts, 0, sizeof(texts));
    texts.committed = 1;
    texts.incomplete_world = 1;
    texts.map = 5;
    texts.text_root_count = 1;
    texts.text_record_reads = 1;
    texts.texts[0].object_id = 0x8800u;
    texts.texts[0].mode = 1;
    texts.texts[0].text_index = 0x012au;

    memset(&calls, 0, sizeof(calls));
    memset(&receipt, 0xa5, sizeof(receipt));
    texts.generic_record_reads = 1;
    CHECK(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
              &texts, read_scalar, &calls, &receipt) == 0,
          "generic-record provenance rejects a DB2-to-WALL_GFX request");
    CHECK(calls.calls == 0,
          "rejected DB2 provenance performs no GDAT scalar reads");
    CHECK(receipt.valid == 0 && receipt.material_count == 0,
          "rejected DB2 provenance leaves an invalid material receipt");

    texts.generic_record_reads = 0;
    texts.blocked_record_reads = 0;
    CHECK(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
              &texts, read_scalar, &calls, &receipt) == 1 &&
              receipt.valid && receipt.material_count == 1,
          "direct DB2-only provenance materializes the WALL_GFX receipt");
    CHECK(calls.calls == 5 &&
              receipt.materials[0].wall_gfx_index == 0x2a &&
              receipt.materials[0].image_offset == 0xfe02,
          "accepted DB2 provenance retains all five source GDAT scalars");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
