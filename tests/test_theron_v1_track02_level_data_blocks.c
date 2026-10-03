#include "theron_v1_track02_level_data_blocks.h"
#include "theron_v1_track02.h"
#include "asset_status_m12.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/*
 * Re-check the descriptor table against an authentic MODE1/2352 Track 02
 * dump when a data host supplies FIRESTAFF_THERON_TRACK02_RAW.  The source
 * loader's raw-sector contract is 2048 user bytes at raw-sector offset 16;
 * this test deliberately validates bytes only and does not infer tile or
 * dungeon semantics from them.
 */
static int resolve_media_path(const char *env_name, const char *file_name,
                              const char **path_out, char *fallback,
                              size_t fallback_size) {
    const char *override = getenv(env_name);
    const char *home = getenv("HOME");
    FILE *file;
    int written;

    if (override && override[0]) {
        *path_out = override;
        return 0;
    }
    if (!file_name || !home || !home[0]) return 77;
    written = snprintf(fallback, fallback_size,
                       "%s/.firestaff/data/theron/%s", home, file_name);
    if (written < 0 || (size_t)written >= fallback_size) {
        fputs("FAIL: HOME-derived Theron media path is too long\n", stderr);
        return 1;
    }
    errno = 0;
    file = fopen(fallback, "rb");
    if (!file) {
        if (errno == ENOENT || errno == ENOTDIR) return 77;
        fprintf(stderr, "FAIL: cannot open default Theron media: %s\n",
                strerror(errno));
        return 1;
    }
    if (fclose(file) != 0) {
        fputs("FAIL: could not close default Theron media\n", stderr);
        return 1;
    }
    *path_out = fallback;
    return 0;
}

static uint8_t *load_raw_track02_user_data(const char *path,
                                          size_t *out_size) {
    FILE *file;
    long file_size;
    size_t raw_size;
    size_t sectors;
    size_t user_data_size;
    size_t bytes_read;
    uint8_t *raw = NULL;
    uint8_t *user_data = NULL;
    int read_error;
    int close_result;

    if (out_size) *out_size = 0u;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    file_size = ftell(file);
    if (file_size <= 0 || (uintmax_t)file_size > (uintmax_t)SIZE_MAX ||
        file_size % 2352L != 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    raw_size = (size_t)file_size;
    sectors = raw_size / 2352u;
    if (sectors > SIZE_MAX / 2048u) {
        fclose(file);
        return NULL;
    }
    user_data_size = sectors * 2048u;
    raw = (uint8_t *)malloc(raw_size);
    user_data = (uint8_t *)malloc(user_data_size);
    if (!raw || !user_data) {
        free(raw);
        free(user_data);
        fclose(file);
        return NULL;
    }
    bytes_read = fread(raw, 1u, raw_size, file);
    read_error = ferror(file);
    close_result = fclose(file);
    if (bytes_read != raw_size || read_error || close_result != 0) {
        free(raw);
        free(user_data);
        return NULL;
    }
    for (size_t sector = 0u; sector < sectors; ++sector) {
        memcpy(user_data + sector * 2048u,
               raw + sector * 2352u + 16u, 2048u);
    }
    free(raw);
    if (out_size) *out_size = user_data_size;
    return user_data;
}

static uint8_t *read_entire_file(const char *path, size_t expected_size) {
    FILE *file = fopen(path, "rb");
    uint8_t *image;
    long file_size;
    size_t bytes_read;
    int read_error;
    int close_result;

    if (!file) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    file_size = ftell(file);
    if (file_size < 0 || (uintmax_t)file_size != (uintmax_t)expected_size ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    image = (uint8_t *)malloc(expected_size);
    if (!image) {
        fclose(file);
        return NULL;
    }
    bytes_read = fread(image, 1u, expected_size, file);
    read_error = ferror(file);
    close_result = fclose(file);
    if (bytes_read != expected_size || read_error || close_result != 0) {
        free(image);
        return NULL;
    }
    return image;
}

static int verify_real_track02_level_blocks(const char *env_name,
                                            const char *file_name,
                                            Theron_Track02Variant variant,
                                            const char *expected_md5,
                                            const char *label) {
    char fallback[4096];
    const char *path = NULL;
    uint8_t shared[THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE];
    uint8_t *user_data;
    size_t user_data_size = 0u;
    char actual_md5[33];
    int resolve_result = resolve_media_path(env_name, file_name, &path,
                                            fallback, sizeof(fallback));

    if (resolve_result != 0) {
        if (resolve_result == 77)
            printf("SKIP: default %s Track 02 media unavailable\n", label);
        return resolve_result;
    }
    if (!m12_file_md5_hex(path, actual_md5)) {
        fprintf(stderr, "FAIL: cannot hash selected %s Track 02 media\n", label);
        return 1;
    }
    if (strcmp(actual_md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: selected %s Track 02 media has unexpected MD5\n",
                label);
        return 1;
    }
    user_data = load_raw_track02_user_data(path, &user_data_size);
    if (!user_data) {
        fprintf(stderr, "FAIL: could not read selected %s Track 02 media\n",
                label);
        return 1;
    }

    {
        const Theron_LevelDataBlockDesc *first =
            theron_v1_track02_level_data_block_for_variant(variant, 0u);
        assert(first != NULL);
        assert(first->ud_offset <= user_data_size);
        assert(THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE <=
               user_data_size - first->ud_offset);
        memcpy(shared, user_data + first->ud_offset, sizeof(shared));
    }
    for (unsigned int level = 0; level < THERON_TRACK02_LEVEL_COUNT; ++level) {
        const Theron_LevelDataBlockDesc *block =
            theron_v1_track02_level_data_block_for_variant(variant, level);
        assert(block != NULL);
        assert(block->ud_offset <= user_data_size);
        assert(THERON_TRACK02_LEVEL_PROLOGUE_SIZE <=
               user_data_size - block->ud_offset);
        for (size_t i = 0; i < THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE; ++i)
            assert(user_data[block->ud_offset + i] == shared[i]);
        for (size_t i = 0; i < sizeof(block->per_level_meta); ++i)
            assert(user_data[block->ud_offset +
                             THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE + i] ==
                   block->per_level_meta[i]);
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
    printf("PASS: authentic %s Track 02 level-block prologues and metadata\n", label);
    return 0;
}

static int verify_real_iso_level_blocks(const char *env_name,
                                        const char *file_name,
                                        Theron_Track02Variant variant,
                                        size_t expected_size,
                                        const char *expected_md5,
                                        const char *label) {
    char fallback[4096];
    const char *path = NULL;
    uint8_t *image;
    char actual_md5[33];
    int resolve_result = resolve_media_path(env_name, file_name, &path,
                                            fallback, sizeof(fallback));

    if (resolve_result != 0) {
        if (resolve_result == 77)
            printf("SKIP: default %s ISO media unavailable\n", label);
        return resolve_result;
    }
    if (!m12_file_md5_hex(path, actual_md5)) {
        fprintf(stderr, "FAIL: cannot hash selected %s ISO media\n", label);
        return 1;
    }
    if (strcmp(actual_md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: selected %s ISO media has unexpected MD5\n",
                label);
        return 1;
    }
    image = read_entire_file(path, expected_size);
    if (!image) {
        fprintf(stderr, "FAIL: could not read selected %s ISO media\n", label);
        return 1;
    }

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
    return 0;
}

static void verify_huc6280_decoder_lift(void) {
    /* One authentic $23A4 framing shape with a bounded literal token.  This
     * is an algorithm-boundary fixture only; no game asset is produced from
     * it.  The real Track 02 resource receipts are still the only accepted
     * source for runtime data. */
    const uint8_t resource[8] = {
        0x20u, 0x00u, 0x07u, 0x00u, 0x00u, 0x00u, 0x20u, 0x80u
    };
    uint8_t destination[16] = {0};
    uint16_t pointer_table[8] = {0};
    Theron_Huc6280DecodeReceipt receipt;

    assert(theron_v1_huc6280_decode_resource(
        resource, sizeof(resource), destination, sizeof(destination),
        0x6000u, pointer_table, 8u, 0u, &receipt));
    assert(receipt.status == THERON_HUC6280_DECODE_READY);
    assert(receipt.resource_length == 7u);
    assert(receipt.resource_bitstream_bytes == 2u);
    assert(receipt.output_bytes == 1u);
    assert(receipt.literal_tokens == 1u);
    assert(receipt.backreference_tokens == 0u);
    assert(receipt.pointer_entries == 1u);
    assert(receipt.tokens == 1u);
    assert(destination[0] == 0x41u);

    {
        Theron_Huc6280DecodeReceipt rejected;
        const uint8_t truncated[6] = {0x20u, 0x00u, 0x08u, 0x00u,
                                      0x00u, 0x00u};
        assert(!theron_v1_huc6280_decode_resource(
            truncated, sizeof(truncated), destination, sizeof(destination),
            0x6000u, pointer_table, 8u, 0u, &rejected));
        assert(rejected.status == THERON_HUC6280_DECODE_TRUNCATED);
    }
    puts("PASS: HuC6280 $23AD variable-bit decoder boundary");
}

int main(int argc, char **argv) {
    if (argc != 2 || (strcmp(argv[1], "static") != 0 &&
                      strcmp(argv[1], "us") != 0 &&
                      strcmp(argv[1], "jp") != 0 &&
                      strcmp(argv[1], "clonecd") != 0 &&
                      strcmp(argv[1], "us_iso") != 0 &&
                      strcmp(argv[1], "jp_iso") != 0)) {
        fputs("usage: test_theron_v1_track02_level_data_blocks "
              "static|us|jp|clonecd|us_iso|jp_iso\n", stderr);
        return 2;
    }
    verify_huc6280_decoder_lift();
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

    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_US_ISO, 0)->ud_offset == 0x02E800);
    assert(theron_v1_track02_level_data_block_for_variant(
               THERON_TRACK02_VARIANT_JP_REV1_ISO, 0)->ud_offset == 0x02E82F);
    if (strcmp(argv[1], "static") == 0) {
        puts("PASS: static Theron level-data-block checks");
        return 0;
    }
    if (strcmp(argv[1], "us") == 0)
        return verify_real_track02_level_blocks(
            "FIRESTAFF_THERON_TRACK02_RAW", "TQUS02.bin",
            THERON_TRACK02_VARIANT_US_BIN, THERON_TRACK02_MD5_US_BIN, "US");
    if (strcmp(argv[1], "jp") == 0)
        return verify_real_track02_level_blocks(
            "FIRESTAFF_THERON_TRACK02_JP_RAW", "TQJP02.bin",
            THERON_TRACK02_VARIANT_JP_BIN, THERON_TRACK02_MD5_JP_BIN, "JP");
    if (strcmp(argv[1], "clonecd") == 0)
        return verify_real_track02_level_blocks(
            "FIRESTAFF_THERON_TRACK02_CLONECD_RAW",
            "raw-us-clonecd/TQUS02.bin",
            THERON_TRACK02_VARIANT_US_CLONECD_RAW,
            THERON_TRACK02_MD5_US_CLONECD_BIN, "US CloneCD");
    if (strcmp(argv[1], "us_iso") == 0)
        return verify_real_iso_level_blocks(
            "FIRESTAFF_THERON_US_ISO", "TQUS19.iso",
            THERON_TRACK02_VARIANT_US_ISO, 5984256u,
            "51b40a17b92a30339957ba564aa0015c", "US");
    return verify_real_iso_level_blocks(
        "FIRESTAFF_THERON_JP_ISO", "TQJP19.iso",
        THERON_TRACK02_VARIANT_JP_REV1_ISO, 6291456u,
        "f9f069a5e489b91207f3156059b756f1", "JP");
}
