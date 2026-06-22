/*
 * test_csb_hint_oracle_htc.c
 *
 * Data-free contract tests for the CSB Utility Disk HCSB.HTC parser.
 *
 * Scope:
 *   - dmweb/ReDMCSB big-endian HTC table layout
 *   - location-to-hint matching including the 255/255 "any XY" rule
 *   - compressed content slicing by first-page length
 *   - bounded HTC LZW decompression including ReDMCSB's 0x90 repeat marker
 *
 * Non-claims:
 *   - no real Utility Disk asset is loaded
 *   - no Hint Oracle UI/runtime path is wired
 */

#include "csb_hint_oracle_htc.h"

#include <stdio.h>
#include <string.h>

#define ASSERT_TRUE(cond) do {                                             \
    if (!(cond)) {                                                         \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",                \
                __FILE__, __LINE__, #cond);                                \
        return 0;                                                          \
    }                                                                      \
} while (0)

static void put_be16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8u);
    p[1] = (uint8_t)(v & 0xffu);
}

static size_t pack_codes_9bit(const uint16_t *codes,
                              size_t code_count,
                              uint8_t *out,
                              size_t out_cap)
{
    size_t bit_pos = 0;
    size_t i;

    memset(out, 0, out_cap);
    for (i = 0; i < code_count; ++i) {
        unsigned int bit;
        for (bit = 0; bit < 9u; ++bit) {
            size_t pos = bit_pos + bit;
            if ((codes[i] & (uint16_t)(1u << bit)) != 0u) {
                out[pos / 8u] |= (uint8_t)(1u << (pos % 8u));
            }
        }
        bit_pos += 9u;
    }
    return (bit_pos + 7u) / 8u;
}

static size_t pack_literal_string(const char *text,
                                  uint8_t *out,
                                  size_t out_cap)
{
    uint16_t codes[96];
    size_t i;
    size_t len = strlen(text) + 1u;

    ASSERT_TRUE(len <= sizeof(codes) / sizeof(codes[0]));
    for (i = 0; i < len; ++i) {
        codes[i] = (uint16_t)(uint8_t)text[i];
    }
    return pack_codes_9bit(codes, len, out, out_cap);
}

static void write_hint_record(uint8_t *dst,
                              const char *name,
                              uint16_t first_page,
                              uint16_t page_count)
{
    size_t len = strlen(name);

    memset(dst, 0, CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE);
    if (len > CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES) {
        len = CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES;
    }
    memcpy(dst, name, len);
    put_be16(dst + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES, first_page);
    put_be16(dst + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 2u, page_count);
}

static size_t build_fixture(uint8_t *buf, size_t cap)
{
    uint8_t content0[96];
    uint8_t content1[32];
    size_t content0_len;
    size_t content1_len;
    size_t off = 0;

    content0_len = pack_literal_string("Cast/ZOKATHRA", content0,
                                       sizeof(content0));
    content1_len = pack_literal_string("Anywhere", content1,
                                       sizeof(content1));

    ASSERT_TRUE(cap >= 10u + 12u + 4u + 52u + 2u + 8u +
                content0_len + content1_len);

    put_be16(buf + off, CSB_HINT_ORACLE_HTC_FORMAT_WORD); off += 2u;
    put_be16(buf + off, CSB_HINT_ORACLE_HTC_DUNGEON_ID); off += 2u;
    put_be16(buf + off, 3u); off += 2u;

    put_be16(buf + off, 2u); off += 2u;
    put_be16(buf + off, CSB_HINT_ORACLE_HTC_LOCATION_RECORD_SIZE); off += 2u;
    buf[off++] = 25u; buf[off++] = 7u; buf[off++] = 5u; buf[off++] = 0u;
    put_be16(buf + off, 0u); off += 2u;
    buf[off++] = CSB_HINT_ORACLE_HTC_ANY_XY;
    buf[off++] = CSB_HINT_ORACLE_HTC_ANY_XY;
    buf[off++] = 5u; buf[off++] = 0u;
    put_be16(buf + off, 1u); off += 2u;

    put_be16(buf + off, 2u); off += 2u;
    put_be16(buf + off, CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE); off += 2u;
    write_hint_record(buf + off, "PROVE YOU ARE WIZARD", 0u, 3u);
    off += CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE;
    write_hint_record(buf + off, "ANYWHERE", 3u, 1u);
    off += CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE;

    put_be16(buf + off, 4u); off += 2u;
    put_be16(buf + off, (uint16_t)content0_len); off += 2u;
    put_be16(buf + off, 0u); off += 2u;
    put_be16(buf + off, 0u); off += 2u;
    put_be16(buf + off, (uint16_t)content1_len); off += 2u;

    memcpy(buf + off, content0, content0_len); off += content0_len;
    memcpy(buf + off, content1, content1_len); off += content1_len;
    return off;
}

static int test_parse_fixture_shape(void)
{
    uint8_t buf[256];
    CSB_HintOracleHTC htc;
    CSB_HintOracleHTC_Location loc;
    CSB_HintOracleHTC_Hint hint;
    size_t size = build_fixture(buf, sizeof(buf));

    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(htc.format_word == CSB_HINT_ORACLE_HTC_FORMAT_WORD);
    ASSERT_TRUE(htc.dungeon_id == CSB_HINT_ORACLE_HTC_DUNGEON_ID);
    ASSERT_TRUE(htc.location_count == 2u);
    ASSERT_TRUE(htc.hint_count == 2u);
    ASSERT_TRUE(htc.page_count == 4u);
    ASSERT_TRUE(htc.content_size > 0u);

    ASSERT_TRUE(csb_hint_oracle_htc_get_location(&htc, 0u, &loc) ==
                CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(loc.x == 25u && loc.y == 7u && loc.level == 5u);
    ASSERT_TRUE(loc.hint_index == 0u);

    ASSERT_TRUE(csb_hint_oracle_htc_get_hint(&htc, 0u, &hint) ==
                CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(strcmp(hint.name, "PROVE YOU ARE WIZARD") == 0);
    ASSERT_TRUE(hint.first_page_index == 0u);
    ASSERT_TRUE(hint.page_count == 3u);
    return 1;
}

static int test_location_matching(void)
{
    uint8_t buf[256];
    CSB_HintOracleHTC htc;
    uint16_t hints[4];
    size_t count = 0;
    size_t size = build_fixture(buf, sizeof(buf));

    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(csb_hint_oracle_htc_find_hints_for_location(
        &htc, 5u, 25u, 7u, hints, 4u, &count) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(count == 2u);
    ASSERT_TRUE(hints[0] == 0u && hints[1] == 1u);

    ASSERT_TRUE(csb_hint_oracle_htc_find_hints_for_location(
        &htc, 5u, 24u, 7u, hints, 4u, &count) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(count == 1u && hints[0] == 1u);

    ASSERT_TRUE(csb_hint_oracle_htc_find_hints_for_location(
        &htc, 6u, 25u, 7u, hints, 4u, &count) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(count == 0u);

    ASSERT_TRUE(csb_hint_oracle_htc_find_hints_for_location(
        &htc, 5u, 25u, 7u, hints, 1u, &count) ==
        CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL);
    ASSERT_TRUE(count == 2u);
    return 1;
}

static int test_content_slices_and_lzw(void)
{
    uint8_t buf[256];
    uint8_t decoded[64];
    CSB_HintOracleHTC htc;
    const uint8_t *compressed = NULL;
    size_t compressed_size = 0;
    size_t decoded_size = 0;
    size_t size = build_fixture(buf, sizeof(buf));

    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(csb_hint_oracle_htc_get_hint_content_slice(
        &htc, 0u, &compressed, &compressed_size) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(compressed == htc.contents);
    ASSERT_TRUE(compressed_size ==
                csb_hint_oracle_htc_page_compressed_length(&htc, 0u));
    ASSERT_TRUE(csb_hint_oracle_htc_lzw_decompress(
        compressed, compressed_size, decoded, sizeof(decoded),
        &decoded_size) == CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(strcmp((const char *)decoded, "Cast/ZOKATHRA") == 0);

    ASSERT_TRUE(csb_hint_oracle_htc_get_hint_content_slice(
        &htc, 1u, &compressed, &compressed_size) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(csb_hint_oracle_htc_lzw_decompress(
        compressed, compressed_size, decoded, sizeof(decoded),
        &decoded_size) == CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(strcmp((const char *)decoded, "Anywhere") == 0);
    return 1;
}

static int test_lzw_dictionary_and_repeat_marker(void)
{
    uint8_t packed[32];
    uint8_t decoded[32];
    size_t packed_size;
    size_t decoded_size = 0;
    const uint16_t dict_codes[] = { 65u, 66u, 65u, 257u, 0u };
    const uint16_t repeat_codes[] = { 65u, 0x90u, 4u, 0u };

    packed_size = pack_codes_9bit(dict_codes,
                                  sizeof(dict_codes) / sizeof(dict_codes[0]),
                                  packed, sizeof(packed));
    ASSERT_TRUE(csb_hint_oracle_htc_lzw_decompress(
        packed, packed_size, decoded, sizeof(decoded), &decoded_size) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(strcmp((const char *)decoded, "ABAAB") == 0);

    packed_size = pack_codes_9bit(repeat_codes,
                                  sizeof(repeat_codes) /
                                      sizeof(repeat_codes[0]),
                                  packed, sizeof(packed));
    ASSERT_TRUE(csb_hint_oracle_htc_lzw_decompress(
        packed, packed_size, decoded, sizeof(decoded), &decoded_size) ==
        CSB_HINT_ORACLE_HTC_OK);
    ASSERT_TRUE(strcmp((const char *)decoded, "AAAA") == 0);

    ASSERT_TRUE(csb_hint_oracle_htc_lzw_decompress(
        packed, packed_size, decoded, 2u, &decoded_size) ==
        CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL);
    return 1;
}

static int test_malformed_inputs(void)
{
    uint8_t buf[256];
    CSB_HintOracleHTC htc;
    size_t size = build_fixture(buf, sizeof(buf));

    ASSERT_TRUE(csb_hint_oracle_htc_parse(NULL, size, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_ARGUMENT);
    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, 9u, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_TRUNCATED);

    buf[1] = 3u;
    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_BAD_FORMAT);
    size = build_fixture(buf, sizeof(buf));

    buf[3] = 12u;
    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_BAD_DUNGEON);
    size = build_fixture(buf, sizeof(buf));

    put_be16(buf + 8u, 5u);
    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_BAD_RECORD_SIZE);
    size = build_fixture(buf, sizeof(buf));

    buf[10u + 4u] = 99u;
    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_BAD_HINT_RANGE);
    size = build_fixture(buf, sizeof(buf));

    put_be16(buf + size - 1u, 0u);
    ASSERT_TRUE(csb_hint_oracle_htc_parse(buf, size - 1u, &htc) ==
                CSB_HINT_ORACLE_HTC_ERR_BAD_CONTENT_SIZE);
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 0;

#define RUN_TEST(fn) do {                                                   \
    ++total;                                                                \
    if (fn()) {                                                             \
        ++passed;                                                           \
        printf("PASS: %s\n", #fn);                                          \
    } else {                                                                \
        printf("FAIL: %s\n", #fn);                                          \
    }                                                                       \
} while (0)

    RUN_TEST(test_parse_fixture_shape);
    RUN_TEST(test_location_matching);
    RUN_TEST(test_content_slices_and_lzw);
    RUN_TEST(test_lzw_dictionary_and_repeat_marker);
    RUN_TEST(test_malformed_inputs);

#undef RUN_TEST

    printf("csb_hint_oracle_htc: %d/%d tests passed\n", passed, total);
    return passed == total ? 0 : 1;
}
