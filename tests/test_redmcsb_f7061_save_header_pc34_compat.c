#include "redmcsb_f7055_saveutil_pc34_compat.h"
#include "redmcsb_f7061_save_header_pc34_compat.h"

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

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static void make_obfuscated_header(uint8_t *header)
{
    uint16_t tail_checksum = (uint16_t)(0x1234U + 0xabcdU);

    memset(header, 0, 512U);
    write_le16(header + 5U * 2U, 0x11b8U);
    write_le16(header + 127U * 2U, (uint16_t)(0x11b8U ^ tail_checksum));
    write_le16(header + 256U, 0x1234U);
    write_le16(header + 256U + 127U * 2U, 0xabcdU);
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        header + 256U, 256U, 0x11b8U);
}

static void test_valid_header(void)
{
    uint8_t header[512];
    uint8_t expected[512];

    make_obfuscated_header(header);
    memcpy(expected, header, sizeof(expected));
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        expected + 256U, 256U, 0x11b8U);

    CHECK(redmcsb_f7061_is_read_save_header_successful_pc34(
              header, sizeof(header), 5U) == 1,
          "F7061 accepts a checksum-valid obfuscated PC34 header");
    CHECK(memcmp(header, expected, sizeof(header)) == 0,
          "F7061 leaves a valid header's tail deobfuscated");
}

static void test_failed_checksum_still_deobfuscates(void)
{
    uint8_t header[512];
    uint8_t plaintext[512];

    make_obfuscated_header(header);
    memcpy(plaintext, header, sizeof(plaintext));
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        plaintext + 256U, 256U, 0x11b8U);
    header[256U + 4U] ^= 0x80U;

    CHECK(redmcsb_f7061_is_read_save_header_successful_pc34(
              header, sizeof(header), 5U) == 0,
          "F7061 rejects a checksum-corrupted header");
    CHECK(memcmp(header + 256U, plaintext + 256U, 256U) != 0,
          "F7061 still deobfuscates the tail before a failed verdict");
}

static void test_fixed_header_boundary(void)
{
    uint8_t header[512];
    uint8_t original[512];

    make_obfuscated_header(header);
    memcpy(original, header, sizeof(original));
    CHECK(redmcsb_f7061_is_read_save_header_successful_pc34(
              header, 511U, 5U) == 0,
          "F7061 refuses a non-512-byte header");
    CHECK(memcmp(header, original, sizeof(header)) == 0,
          "a non-512-byte header is not modified");
    CHECK(redmcsb_f7061_is_read_save_header_successful_pc34(
              header, sizeof(header), 128U) == 0,
          "F7061 refuses a key outside the clear header words");
}

int main(void)
{
    test_valid_header();
    test_failed_checksum_still_deobfuscates();
    test_fixed_header_boundary();
    CHECK(strcmp(redmcsb_f7061_save_header_pc34_source_evidence(),
                 "ReDMCSB CEDTINC6.C F7061/F0429") == 0,
          "source evidence identifies the original routine");

    if (failures != 0) {
        return 1;
    }
    puts("PASSED: ReDMCSB F7061/F0429");
    return 0;
}
