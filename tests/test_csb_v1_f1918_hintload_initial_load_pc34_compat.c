#include "csb_v1_f1918_hintload_initial_load_pc34_compat.h"
#include "redmcsb_f7055_saveutil_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const uint8_t *bytes;
    size_t byte_count;
    size_t offset;
    unsigned int calls;
} Stream;

static int g_failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        ++g_failures; \
        printf("FAIL: %s\n", msg); \
    } else { \
        printf("PASS: %s\n", msg); \
    } \
} while (0)

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void write_le16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)(value >> 8);
}

static int read_exact(void *context, uint8_t *destination, size_t byte_count)
{
    Stream *stream = (Stream *)context;

    if (stream == 0 || destination == 0 ||
        stream->offset + byte_count > stream->byte_count) {
        return 0;
    }
    memcpy(destination, stream->bytes + stream->offset, byte_count);
    stream->offset += byte_count;
    ++stream->calls;
    return 1;
}

static uint16_t header_expected_checksum(const uint8_t header[512])
{
    uint16_t expected = 0U;
    size_t word_index;

    for (word_index = 0U; word_index < 128U; word_index += 4U) {
        expected = (uint16_t)(expected + read_le16(header + word_index * 2U));
        expected = (uint16_t)(expected ^ read_le16(header + (word_index + 1U) * 2U));
        expected = (uint16_t)(expected - read_le16(header + (word_index + 2U) * 2U));
        expected = (uint16_t)(expected ^ read_le16(header + (word_index + 3U) * 2U));
    }
    return expected;
}

static void build_valid_header(uint8_t header[512], uint16_t part_keys[3],
                               uint16_t part_checksums[3])
{
    uint16_t expected;
    uint16_t key;
    uint16_t second_half_sum;
    size_t i;

    memset(header, 0, 512U);
    for (i = 0U; i < 128U; ++i) {
        write_le16(header + i * 2U, (uint16_t)(0x0100U + i * 3U));
    }
    key = 0x5A3CU;
    write_le16(header + REDMCSB_F1918_PC34_HEADER_KEY_WORD_INDEX * 2U, key);

    expected = header_expected_checksum(header);
    for (i = 0U; i < 128U; ++i) {
        write_le16(header + 256U + i * 2U, 0U);
    }
    for (i = 0U; i < 3U; ++i) {
        write_le16(header + REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET + i * 2U,
                   part_keys[i]);
        write_le16(header + REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET + i * 2U,
                   part_checksums[i]);
    }
    second_half_sum = 0U;
    for (i = 1U; i < 128U; ++i) {
        second_half_sum = (uint16_t)(second_half_sum +
            read_le16(header + 256U + i * 2U));
    }
    write_le16(header + 256U, (uint16_t)(expected - second_half_sum));
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        header + 256U, 256U, key);
}

static void fill_part(uint8_t *part, size_t byte_count, uint8_t seed)
{
    size_t i;

    for (i = 0U; i < byte_count; ++i) {
        part[i] = (uint8_t)(seed + (uint8_t)(i * 7U));
    }
}

static void build_stream(uint8_t *stream, size_t *stream_size,
                         uint8_t global_data[8], uint8_t active_groups[10],
                         uint8_t party[12])
{
    uint8_t header[512];
    uint16_t keys[3] = {0x1111U, 0x2222U, 0x3333U};
    uint16_t sums[3];
    size_t offset = 0U;

    fill_part(global_data, 8U, 0x10U);
    fill_part(active_groups, 10U, 0x30U);
    fill_part(party, 12U, 0x50U);
    sums[0] = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        global_data, 8U, keys[0]);
    sums[1] = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        active_groups, 10U, keys[1]);
    sums[2] = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        party, 12U, keys[2]);
    build_valid_header(header, keys, sums);

    memcpy(stream + offset, header, sizeof(header));
    offset += sizeof(header);
    memcpy(stream + offset, global_data, 8U);
    offset += 8U;
    memcpy(stream + offset, active_groups, 10U);
    offset += 10U;
    memcpy(stream + offset, party, 12U);
    offset += 12U;
    *stream_size = offset;
}

int main(void)
{
    uint8_t stream_bytes[512U + 8U + 10U + 12U];
    uint8_t global_source[8];
    uint8_t active_source[10];
    uint8_t party_source[12];
    uint8_t global_out[8];
    uint8_t active_out[10];
    uint8_t party_out[12];
    size_t stream_size = 0U;
    Stream stream;
    CSB_V1_F1918_LoadReceiptPc34 receipt;
    int rc;

    build_stream(stream_bytes, &stream_size, global_source, active_source,
                 party_source);

    memset(&receipt, 0, sizeof(receipt));
    receipt.parts[0].bytes = global_out;
    receipt.parts[0].byte_count = sizeof(global_out);
    receipt.parts[1].bytes = active_out;
    receipt.parts[1].byte_count = sizeof(active_out);
    receipt.parts[2].bytes = party_out;
    receipt.parts[2].byte_count = sizeof(party_out);
    stream.bytes = stream_bytes;
    stream.byte_count = stream_size;
    stream.offset = 0U;
    stream.calls = 0U;

    rc = csb_v1_f1918_load_game_cpsx_pc34(read_exact, &stream, &receipt);
    CHECK(rc == REDMCSB_F1918_PC34_RESULT_OK,
          "F1918 returns source OK result after header plus three parts");
    CHECK(receipt.header_valid == 1, "F1918 records accepted header");
    CHECK(receipt.parts_loaded == 3U, "F1918 records all three loaded parts");
    CHECK(stream.offset == stream_size, "F1918 consumes exact stream bytes");
    CHECK(stream.calls == 4U, "F1918 uses one header read plus three part reads");
    CHECK(memcmp(global_out, (uint8_t[]){0x10U,0x17U,0x1EU,0x25U,0x2CU,0x33U,0x3AU,0x41U}, 8U) == 0,
          "GLOBAL_DATA is deobfuscated back to caller plaintext");
    CHECK(csb_v1_f1919_post_f1918_load_game_cpsx_pc34(&receipt) == 1,
          "F1919 post accepts only the completed F1918 transaction");

    stream_bytes[0] ^= 0x55U;
    memset(&receipt, 0, sizeof(receipt));
    receipt.parts[0].bytes = global_out;
    receipt.parts[0].byte_count = sizeof(global_out);
    receipt.parts[1].bytes = active_out;
    receipt.parts[1].byte_count = sizeof(active_out);
    receipt.parts[2].bytes = party_out;
    receipt.parts[2].byte_count = sizeof(party_out);
    stream.offset = 0U;
    rc = csb_v1_f1918_load_game_cpsx_pc34(read_exact, &stream, &receipt);
    CHECK(rc == REDMCSB_F1918_PC34_RESULT_HEADER_READ_FAILED,
          "F1918 rejects a bad header before loading parts");
    CHECK(csb_v1_f1919_post_f1918_load_game_cpsx_pc34(&receipt) == 0,
          "F1919 post rejects failed header receipt");
    stream_bytes[0] ^= 0x55U;

    stream_bytes[512U + 8U] ^= 0x01U;
    memset(&receipt, 0, sizeof(receipt));
    receipt.parts[0].bytes = global_out;
    receipt.parts[0].byte_count = sizeof(global_out);
    receipt.parts[1].bytes = active_out;
    receipt.parts[1].byte_count = sizeof(active_out);
    receipt.parts[2].bytes = party_out;
    receipt.parts[2].byte_count = sizeof(party_out);
    stream.offset = 0U;
    rc = csb_v1_f1918_load_game_cpsx_pc34(read_exact, &stream, &receipt);
    CHECK(rc == REDMCSB_F1918_PC34_RESULT_ACTIVE_GROUPS_FAILED,
          "F1918 returns the source active-groups failure slot");
    CHECK(receipt.parts_loaded == 1U, "F1918 stops after the last accepted part");
    CHECK(csb_v1_f1919_post_f1918_load_game_cpsx_pc34(&receipt) == 0,
          "F1919 post rejects partial F1918 transaction");
    stream_bytes[512U + 8U] ^= 0x01U;

    memset(&receipt, 0, sizeof(receipt));
    receipt.parts[0].bytes = global_out;
    receipt.parts[0].byte_count = sizeof(global_out);
    receipt.parts[1].bytes = 0;
    receipt.parts[1].byte_count = sizeof(active_out);
    receipt.parts[2].bytes = party_out;
    receipt.parts[2].byte_count = sizeof(party_out);
    stream.offset = 0U;
    rc = csb_v1_f1918_load_game_cpsx_pc34(read_exact, &stream, &receipt);
    CHECK(rc == REDMCSB_F1918_PC34_RESULT_ACTIVE_GROUPS_FAILED,
          "F1918 rejects missing caller-owned ACTIVE_GROUPS storage");

    CHECK(csb_v1_f1919_post_f1918_load_game_cpsx_pc34(0) == 0,
          "F1919 post rejects NULL receipt");
    CHECK(strstr(csb_v1_f1918_hintload_initial_load_source_evidence_pc34(),
                 "F1918_LoadGame_CPSX") != 0,
          "source evidence names F1918");
    CHECK(strstr(csb_v1_f1918_hintload_initial_load_source_evidence_pc34(),
                 "F1919_Post_F1918_LoadGame_CPSX") != 0,
          "source evidence names F1919");

    if (g_failures != 0) {
        printf("%d failure(s)\n", g_failures);
        return 1;
    }
    printf("all CSB F1918/F1919 checks passed\n");
    return 0;
}
