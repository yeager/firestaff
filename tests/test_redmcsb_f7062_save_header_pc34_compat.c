#include "redmcsb_f7061_save_header_pc34_compat.h"
#include "redmcsb_f7062_save_header_pc34_compat.h"

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

static void make_plain_header(uint8_t *header)
{
    memset(header, 0, REDMCSB_F7062_HEADER_BYTES);
    write_le16(header + 256U, 0x1234U);
    write_le16(header + 256U + 127U * 2U, 0xabcdU);
}

static void make_random_words(uint16_t *words)
{
    size_t index;

    for (index = 0; index < REDMCSB_F7062_RANDOM_WORDS; ++index) {
        words[index] = (uint16_t)(0x9000U + index * 37U);
    }
}

static void test_prepare_and_verify(void)
{
    uint8_t header[REDMCSB_F7062_HEADER_BYTES];
    uint8_t plaintext[REDMCSB_F7062_HEADER_BYTES];
    uint8_t encoded[REDMCSB_F7062_HEADER_BYTES];
    uint8_t decoded[REDMCSB_F7062_HEADER_BYTES];
    uint16_t random_words[REDMCSB_F7062_RANDOM_WORDS];

    make_plain_header(header);
    memcpy(plaintext, header, sizeof(plaintext));
    make_random_words(random_words);
    CHECK(redmcsb_f7062_prepare_obfuscated_save_header_pc34(
              header, sizeof(header), 5U, random_words,
              REDMCSB_F7062_RANDOM_WORDS, encoded, sizeof(encoded)) == 1,
          "F7062 prepares an obfuscated PC34 header from source RNG words");
    CHECK(memcmp(header + 256U, plaintext + 256U, 256U) == 0,
          "F7062 restores the caller's plaintext header tail");
    CHECK(memcmp(encoded + 256U, plaintext + 256U, 256U) != 0,
          "F7062 retains the obfuscated tail in the written byte image");

    memcpy(decoded, encoded, sizeof(decoded));
    CHECK(redmcsb_f7061_is_read_save_header_successful_pc34(
              decoded, sizeof(decoded), 5U) == 1,
          "F7062 output is accepted by the exact F7061 checksum gate");
    CHECK(memcmp(decoded + 256U, plaintext + 256U, 256U) == 0,
          "F7061 recovers F7062's plaintext tail exactly");
}

static void test_exact_input_boundaries(void)
{
    uint8_t header[REDMCSB_F7062_HEADER_BYTES];
    uint8_t output[REDMCSB_F7062_HEADER_BYTES];
    uint8_t header_before[REDMCSB_F7062_HEADER_BYTES];
    uint8_t output_before[REDMCSB_F7062_HEADER_BYTES];
    uint16_t random_words[REDMCSB_F7062_RANDOM_WORDS];

    make_plain_header(header);
    memset(output, 0xa5, sizeof(output));
    memcpy(header_before, header, sizeof(header));
    memcpy(output_before, output, sizeof(output));
    make_random_words(random_words);
    CHECK(redmcsb_f7062_prepare_obfuscated_save_header_pc34(
              header, sizeof(header), 5U, random_words,
              REDMCSB_F7062_RANDOM_WORDS - 1U, output, sizeof(output)) == 0,
          "F7062 refuses an incomplete source RNG sequence");
    CHECK(memcmp(header, header_before, sizeof(header)) == 0,
          "an incomplete RNG sequence does not alter the header");
    CHECK(memcmp(output, output_before, sizeof(output)) == 0,
          "an incomplete RNG sequence does not emit a fallback header");
}

int main(void)
{
    test_prepare_and_verify();
    test_exact_input_boundaries();
    CHECK(strcmp(redmcsb_f7062_save_header_pc34_source_evidence(),
                 "ReDMCSB CEDTINC6.C F7062/F0430") == 0,
          "source evidence identifies the original routine");

    if (failures != 0) {
        return 1;
    }
    puts("PASSED: ReDMCSB F7062/F0430");
    return 0;
}
