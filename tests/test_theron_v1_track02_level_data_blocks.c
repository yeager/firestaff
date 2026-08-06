#include "theron_v1_track02_level_data_blocks.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Re-check the descriptor table against an authentic MODE1/2352 Track 02
 * dump when a data host supplies FIRESTAFF_THERON_TRACK02_RAW.  The source
 * loader's raw-sector contract is 2048 user bytes at raw-sector offset 16;
 * this test deliberately validates bytes only and does not infer tile or
 * dungeon semantics from them.
 */
static int read_raw_user_byte(FILE *file, size_t user_offset, uint8_t *out) {
    const size_t raw_offset = (user_offset / 2048u) * 2352u + 16u +
                              (user_offset % 2048u);
    if (fseek(file, (long)raw_offset, SEEK_SET) != 0 ||
        fread(out, 1u, 1u, file) != 1u) {
        return 0;
    }
    return 1;
}

static const char *resolve_track02_path(const char *env_name,
                                        const char *file_name,
                                        char *fallback,
                                        size_t fallback_size) {
    const char *value = getenv(env_name);
    const char *home = getenv("HOME");
    if (value && value[0]) return value;
    if (!home || !home[0] || !fallback || fallback_size == 0u) return NULL;
    if (snprintf(fallback, fallback_size,
                 "%s/.firestaff/data/theron/%s", home, file_name) < 0) {
        return NULL;
    }
    return fallback;
}

static void verify_real_track02_level_blocks(const char *env_name,
                                             const char *file_name,
                                             Theron_Track02Variant variant,
                                             const char *label) {
    char fallback[512];
    const char *path = resolve_track02_path(env_name, file_name,
                                            fallback, sizeof(fallback));
    FILE *file;
    long file_size;
    uint8_t shared[THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE];
    uint8_t *user_data;
    size_t user_data_size;

    if (!path || !path[0]) {
        puts("SKIP: no standard or FIRESTAFF_THERON_TRACK02_* real-data path");
        return;
    }
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    file_size = ftell(file);
    assert(file_size > 0L);
    assert((file_size % 2352L) == 0L);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    user_data_size = (size_t)(file_size / 2352L) * 2048u;
    user_data = calloc(1u, user_data_size);
    assert(user_data != NULL);
    for (size_t sector = 0u; sector < (size_t)(file_size / 2352L); ++sector) {
        assert(fseek(file, (long)(sector * 2352u + 16u), SEEK_SET) == 0);
        assert(fread(user_data + sector * 2048u, 1u, 2048u, file) == 2048u);
    }

    for (size_t i = 0; i < THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE; ++i) {
        assert(read_raw_user_byte(file,
                                  theron_v1_track02_level_data_block_for_variant(
                                      variant, 0)->ud_offset + i,
                                  &shared[i]));
    }
    for (unsigned int level = 0; level < THERON_TRACK02_LEVEL_COUNT; ++level) {
        const Theron_LevelDataBlockDesc *block =
            theron_v1_track02_level_data_block_for_variant(variant, level);
        for (size_t i = 0; i < THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE; ++i) {
            uint8_t byte;
            assert(read_raw_user_byte(file, block->ud_offset + i, &byte));
            assert(byte == shared[i]);
        }
        for (size_t i = 0; i < sizeof(block->per_level_meta); ++i) {
            uint8_t byte;
            assert(read_raw_user_byte(file,
                                      block->ud_offset +
                                          THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE + i,
                                      &byte));
            assert(byte == block->per_level_meta[i]);
        }
        {
            Theron_LevelDataBlockReceipt receipt;
            assert(theron_v1_track02_level_data_block_read(
                user_data, user_data_size, variant, level, &receipt));
            assert(receipt.valid && receipt.level == level &&
                   receipt.compressed_ud_offset == block->ud_offset + 0xF0u &&
                   receipt.resource_end_ud_offset > receipt.compressed_ud_offset &&
                   receipt.resource_end_ud_offset <=
                       receipt.compressed_ud_offset + receipt.compressed_bytes &&
                   receipt.resource_length >= 5u &&
                   receipt.compressed_bytes > 0u &&
                   receipt.resource_header_verified &&
                   receipt.resource_bitstream != NULL &&
                   receipt.resource_bitstream_bytes > 0u &&
                   receipt.resource_bitstream_bytes <=
                       receipt.compressed_bytes -
                           THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE &&
                   receipt.compressed_fnv1a != 0u &&
                   receipt.shared_prologue_fnv1a != 0u &&
                   memcmp(receipt.per_level_meta, block->per_level_meta,
                          sizeof(receipt.per_level_meta)) == 0);
        }
    }
    {
        Theron_LevelDataBlockReceipt rejected;
        const Theron_LevelDataBlockDesc *first =
            theron_v1_track02_level_data_block_for_variant(variant, 0u);
        user_data[first->ud_offset] ^= 1u;
        assert(!theron_v1_track02_level_data_block_read(
            user_data, user_data_size, variant, 0u, &rejected));
        user_data[first->ud_offset] ^= 1u;
        user_data[first->ud_offset + THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE] ^= 1u;
        assert(!theron_v1_track02_level_data_block_read(
            user_data, user_data_size, variant, 0u, &rejected));
        user_data[first->ud_offset + THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE] ^= 1u;
        user_data[first->ud_offset + THERON_TRACK02_LEVEL_PROLOGUE_SIZE] ^= 1u;
        assert(!theron_v1_track02_level_data_block_read(
            user_data, user_data_size, variant, 0u, &rejected));
        user_data[first->ud_offset + THERON_TRACK02_LEVEL_PROLOGUE_SIZE] ^= 1u;
    }
    free(user_data);
    fclose(file);
    printf("PASS: authentic %s Track 02 level-block prologues and metadata\n", label);
}

static void verify_real_iso_level_blocks(const char *env_name,
                                         const char *file_name,
                                         Theron_Track02Variant variant,
                                         size_t expected_size,
                                         const char *label) {
    char fallback[512];
    const char *path = resolve_track02_path(env_name, file_name,
                                            fallback, sizeof(fallback));
    FILE *file;
    long file_size;
    uint8_t *image;

    if (!path || !path[0]) {
        puts("SKIP: no standard or FIRESTAFF_THERON_*_ISO real-data path");
        return;
    }
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    file_size = ftell(file);
    assert(file_size == (long)expected_size);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    image = malloc(expected_size);
    assert(image != NULL);
    assert(fread(image, 1u, expected_size, file) == expected_size);
    fclose(file);

    for (unsigned int level = 0; level < THERON_TRACK02_LEVEL_COUNT; ++level) {
        const Theron_LevelDataBlockDesc *block =
            theron_v1_track02_level_data_block_for_variant(variant, level);
        Theron_LevelDataBlockReceipt receipt;
        assert(block != NULL);
        assert(theron_v1_track02_level_data_block_read(
            image, expected_size, variant, level, &receipt));
        assert(receipt.valid && receipt.variant == variant &&
               receipt.level == level &&
               receipt.block_ud_offset == block->ud_offset &&
               receipt.resource_end_ud_offset > receipt.compressed_ud_offset &&
               receipt.resource_end_ud_offset <=
                   receipt.compressed_ud_offset + receipt.compressed_bytes &&
               receipt.resource_length >= 5u &&
               receipt.compressed_bytes > 0u &&
               receipt.resource_header_verified &&
               receipt.resource_bitstream != NULL &&
               receipt.resource_bitstream_bytes > 0u &&
               receipt.resource_bitstream_bytes <=
                   receipt.compressed_bytes -
                       THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE &&
               receipt.compressed_fnv1a != 0u &&
               receipt.shared_prologue_fnv1a != 0u &&
               memcmp(receipt.per_level_meta, block->per_level_meta,
                      sizeof(receipt.per_level_meta)) == 0);
    }

    {
        const Theron_LevelDataBlockDesc *first =
            theron_v1_track02_level_data_block_for_variant(variant, 0u);
        Theron_LevelDataBlockReceipt rejected;
        image[first->ud_offset] ^= 1u;
        assert(!theron_v1_track02_level_data_block_read(
            image, expected_size, variant, 0u, &rejected));
    }
    free(image);
    printf("PASS: authentic %s Track 19 ISO level-block offsets and hashes\n",
           label);
}

int main(void) {
    /* 7 levels */
    assert(THERON_TRACK02_LEVEL_COUNT == 7);

    /* Level 1 */
    const Theron_LevelDataBlockDesc *b0 = theron_v1_track02_level_data_block(0);
    assert(b0 != NULL);
    assert(b0->ud_offset == 0x09F000);
    assert(b0->per_level_meta[0] == 0x07);
    assert(b0->per_level_meta[1] == 0x87);

    /* Level 2 — non-aligned UD offset */
    const Theron_LevelDataBlockDesc *b1 = theron_v1_track02_level_data_block(1);
    assert(b1->ud_offset == 0x0DF342);

    /* Level 5 — has 0xFF in metadata */
    const Theron_LevelDataBlockDesc *b4 = theron_v1_track02_level_data_block(4);
    assert(b4->per_level_meta[5] == 0xFF);

    /* Level 7 */
    const Theron_LevelDataBlockDesc *b6 = theron_v1_track02_level_data_block(6);
    assert(b6->ud_offset == 0x21F000);
    assert(b6->per_level_meta[1] == 0x86);

    /* Out of bounds */
    assert(theron_v1_track02_level_data_block(7) == NULL);
    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_JP_BIN, 0)->ud_offset == 0x09E82F);
    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_JP_BIN, 1)->per_level_meta[2] == 0x04);
    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_US_ISO, 0)->ud_offset == 0x02E800);

    /* All blocks have non-zero UD offsets */
    for (unsigned i = 0; i < THERON_TRACK02_LEVEL_COUNT; i++) {
        const Theron_LevelDataBlockDesc *b = theron_v1_track02_level_data_block(i);
        assert(b->ud_offset > 0);
    }

    verify_real_track02_level_blocks("FIRESTAFF_THERON_TRACK02_RAW", "TQUS02.bin",
                                     THERON_TRACK02_VARIANT_US_BIN, "US");
    verify_real_track02_level_blocks("FIRESTAFF_THERON_TRACK02_JP_RAW", "TQJP02.bin",
                                     THERON_TRACK02_VARIANT_JP_BIN, "JP");
    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_US_ISO, 0)->ud_offset == 0x02E800);
    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_JP_REV1_ISO, 0)->ud_offset == 0x02E82F);
    verify_real_iso_level_blocks("FIRESTAFF_THERON_US_ISO", "TQUS19.iso",
                                 THERON_TRACK02_VARIANT_US_ISO, 5984256u, "US");
    verify_real_iso_level_blocks("FIRESTAFF_THERON_JP_ISO", "TQJP19.iso",
                                 THERON_TRACK02_VARIANT_JP_REV1_ISO, 6291456u, "JP");

    printf("PASS: theron_v1_track02_level_data_blocks\n");
    return 0;
}
