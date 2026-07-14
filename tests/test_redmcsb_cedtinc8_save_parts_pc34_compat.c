#include "redmcsb_cedtinc8_save_parts_pc34_compat.h"
#include "redmcsb_f7055_saveutil_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        failures++; \
    } \
} while (0)

static void test_five_part_sequence(void)
{
    uint8_t plain[REDMCSB_CEDTINC8_SAVE_PART_COUNT][4] = {
        {0x01, 0x10, 0x02, 0x20}, {0x03, 0x30, 0x04, 0x40},
        {0x05, 0x50, 0x06, 0x60}, {0x07, 0x70, 0x08, 0x80},
        {0x09, 0x90, 0x0a, 0xa0}
    };
    uint8_t original[REDMCSB_CEDTINC8_SAVE_PART_COUNT][4];
    uint8_t written[REDMCSB_CEDTINC8_SAVE_PART_COUNT][4];
    RedmcsbCedtinc8SavePart parts[REDMCSB_CEDTINC8_SAVE_PART_COUNT];
    uint16_t keys[REDMCSB_CEDTINC8_SAVE_HEADER_KEY_COUNT] = {
        0x11b8, 0x2201, 0x33fe, 0x4402, 0x55a5
    };
    uint16_t checksums[REDMCSB_CEDTINC8_SAVE_PART_COUNT];
    uint16_t index;

    memcpy(original, plain, sizeof(original));
    for (index = 0; index < REDMCSB_CEDTINC8_SAVE_PART_COUNT; ++index) {
        parts[index].plaintext = plain[index];
        parts[index].byte_count = 4U;
        parts[index].written_bytes = written[index];
    }
    CHECK(redmcsb_cedtinc8_prepare_save_parts_pc34(parts, keys, checksums) == 1,
          "CEDTINC8 prepares all five source-owned save parts");
    CHECK(memcmp(plain, original, sizeof(plain)) == 0,
          "CEDTINC8 restores every caller plaintext part after writing");

    for (index = 0; index < REDMCSB_CEDTINC8_SAVE_PART_COUNT; ++index) {
        uint16_t decoded_checksum =
            redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
                written[index], 4U, keys[index]);

        CHECK(decoded_checksum == checksums[index],
              "written part checksum matches its header checksum");
        CHECK(memcmp(written[index], original[index], 4U) == 0,
              "written part decodes to the exact original bytes");
    }
}

static void test_no_partial_fallback(void)
{
    uint8_t bytes[REDMCSB_CEDTINC8_SAVE_PART_COUNT][2] = {{1, 0}, {2, 0},
                                                           {3, 0}, {4, 0},
                                                           {5, 0}};
    uint8_t written[REDMCSB_CEDTINC8_SAVE_PART_COUNT][2];
    uint8_t written_before[REDMCSB_CEDTINC8_SAVE_PART_COUNT][2];
    RedmcsbCedtinc8SavePart parts[REDMCSB_CEDTINC8_SAVE_PART_COUNT];
    uint16_t keys[REDMCSB_CEDTINC8_SAVE_HEADER_KEY_COUNT] = {0};
    uint16_t checksums[REDMCSB_CEDTINC8_SAVE_PART_COUNT] = {0};
    uint16_t index;

    memset(written, 0xa5, sizeof(written));
    memcpy(written_before, written, sizeof(written));
    for (index = 0; index < REDMCSB_CEDTINC8_SAVE_PART_COUNT; ++index) {
        parts[index].plaintext = bytes[index];
        parts[index].byte_count = 2U;
        parts[index].written_bytes = written[index];
    }
    parts[3].byte_count = 1U;
    CHECK(redmcsb_cedtinc8_prepare_save_parts_pc34(parts, keys, checksums) == 0,
          "an odd source part refuses the complete five-part save write");
    CHECK(memcmp(written, written_before, sizeof(written)) == 0,
          "an invalid part emits no partial or fallback save bytes");
}

int main(void)
{
    test_five_part_sequence();
    test_no_partial_fallback();
    CHECK(strcmp(redmcsb_cedtinc8_save_parts_pc34_source_evidence(),
                 "ReDMCSB CEDTINC8.C SAVE_GAME five-part checksum/write sequence") == 0,
          "source evidence identifies the original write sequence");

    if (failures != 0) {
        return 1;
    }
    puts("PASSED: ReDMCSB CEDTINC8 five-part save sequence");
    return 0;
}
