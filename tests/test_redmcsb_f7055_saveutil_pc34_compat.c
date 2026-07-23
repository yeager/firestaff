#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f7055_saveutil_pc34_compat.h"

static int failures;

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            ++failures; \
            printf("FAIL: %s\n", message); \
        } \
    } while (0)

static void test_known_word_sequence(void)
{
    uint8_t words[] = { 0x34u, 0x12u, 0xCDu, 0xABu, 0xFFu, 0x00u };
    static const uint8_t expected[] = {
        0x8Cu, 0x03u, 0x76u, 0xBAu, 0x42u, 0x11u
    };
    uint16_t checksum;

    checksum = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        words, sizeof(words), 0x11B8u);
    CHECK(checksum == 0x9FFCu, "F7055 source-order checksum matches");
    CHECK(memcmp(words, expected, sizeof(words)) == 0,
          "F7055 writes PC34 little-endian XOR words");
    CHECK(redmcsb_f7056_saveutil_get_checksum_pc34(
              words, sizeof(words), 0x11B8u) == checksum,
          "F7056 validates F7055 ciphertext");
}

static void test_read_write_transaction(void)
{
    uint8_t plaintext[] = { 0xFFu, 0xFFu, 0x02u, 0x80u, 0x00u, 0x7Fu, 0x01u, 0x00u };
    uint8_t ciphertext[sizeof(plaintext)];
    uint16_t checksum = 0u;

    CHECK(redmcsb_f7058_write_save_part_with_checksum_pc34(
              plaintext, sizeof(plaintext), 0xFFFEu, &checksum) == 1,
          "F7058 accepts a valid even save section");
    CHECK(memcmp(plaintext, (uint8_t[]){
              0xFFu, 0xFFu, 0x02u, 0x80u, 0x00u, 0x7Fu, 0x01u, 0x00u
          }, sizeof(plaintext)) == 0,
          "F7058 restores caller plaintext after write obfuscation");
    memcpy(ciphertext, plaintext, sizeof(ciphertext));
    CHECK(redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
              ciphertext, sizeof(ciphertext), 0xFFFEu) == checksum,
          "F7058 reports the F7055 checksum");
    CHECK(redmcsb_f7057_read_save_part_with_checksum_pc34(
              ciphertext, sizeof(ciphertext), 0xFFFEu, checksum) == 1,
          "F7057 accepts the original checksum");
    CHECK(memcmp(ciphertext, plaintext, sizeof(plaintext)) == 0,
          "F7057 restores original plaintext");
    CHECK(redmcsb_f7057_read_save_part_with_checksum_pc34(
              plaintext, sizeof(plaintext), 0xFFFEu,
              (uint16_t)(checksum + 1u)) == 0,
          "F7057 rejects a foreign checksum");
}

static void test_rejects_non_source_sections(void)
{
    uint8_t bytes[] = { 1u, 2u, 3u };
    uint8_t original[sizeof(bytes)];
    uint16_t checksum = 0xBEEFu;

    memcpy(original, bytes, sizeof(bytes));
    CHECK(redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
              bytes, sizeof(bytes), 1u) == 0u,
          "F7055 rejects odd byte sections");
    CHECK(memcmp(bytes, original, sizeof(bytes)) == 0,
          "F7055 odd rejection leaves bytes unchanged");
    CHECK(redmcsb_f7056_saveutil_get_checksum_pc34(NULL, 2u, 1u) == 0u,
          "F7056 rejects null input");
    CHECK(redmcsb_f7057_read_save_part_with_checksum_pc34(
              bytes, 0u, 1u, 0u) == 0,
          "F7057 rejects empty sections");
    CHECK(redmcsb_f7058_write_save_part_with_checksum_pc34(
              bytes, 2u, 1u, NULL) == 0,
          "F7058 requires checksum ownership");
    CHECK(checksum == 0xBEEFu, "caller checksum sentinel remains owned");
}

int main(void)
{
    test_known_word_sequence();
    test_read_write_transaction();
    test_rejects_non_source_sections();
    CHECK(strstr(redmcsb_f7055_saveutil_source_evidence_pc34(),
                 "CEDTINC6.C") != NULL,
          "source evidence names ReDMCSB CEDTINC6.C");
    if (failures != 0) {
        printf("FAILED: %d\n", failures);
        return 1;
    }
    printf("PASSED: ReDMCSB F7055/F7056/F7057/F7058\n");
    return 0;
}
