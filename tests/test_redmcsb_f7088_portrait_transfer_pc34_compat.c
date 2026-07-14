#include "redmcsb_f7088_portrait_transfer_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); failures++; } \
} while (0)

static void fill_portrait(uint8_t *portrait, uint8_t value)
{
    memset(portrait, value, REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT);
}

static void test_exact_transfer_and_bind(void)
{
    uint8_t source_bytes[REDMCSB_F7088_PC34_PORTRAIT_COUNT]
                        [REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT];
    uint8_t *source_slots[REDMCSB_F7088_PC34_PORTRAIT_COUNT];
    uint8_t destination_bytes[REDMCSB_F7088_PC34_PORTRAIT_COUNT]
                             [REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT];
    uint8_t *destination_slots[REDMCSB_F7088_PC34_PORTRAIT_COUNT] = {NULL};
    uint16_t portrait_index;

    for (portrait_index = 0U;
         portrait_index < REDMCSB_F7088_PC34_PORTRAIT_COUNT;
         ++portrait_index) {
        fill_portrait(source_bytes[portrait_index], (uint8_t)(portrait_index + 1U));
        source_slots[portrait_index] = source_bytes[portrait_index];
    }
    CHECK(redmcsb_f7088_copy_included_portraits_to_excluded_pc34(
              source_slots, REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED,
              destination_slots, REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED,
              &destination_bytes[0][0], sizeof(destination_bytes),
              REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT) == 1,
          "F7088 transfers the exact PC34 four-portrait route");
    for (portrait_index = 0U;
         portrait_index < REDMCSB_F7088_PC34_PORTRAIT_COUNT;
         ++portrait_index) {
        CHECK(destination_slots[portrait_index] == destination_bytes[portrait_index],
              "F7088 rebinds each excluded-format portrait slot");
        CHECK(memcmp(destination_bytes[portrait_index], source_bytes[portrait_index],
                     REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT) == 0,
              "F7088 copies each source portrait's original bytes");
    }
}

static void test_mismatch_refusal(void)
{
    uint8_t source[REDMCSB_F7088_PC34_PORTRAIT_COUNT]
                  [REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT];
    uint8_t *source_slots[REDMCSB_F7088_PC34_PORTRAIT_COUNT];
    uint8_t destination[REDMCSB_F7088_PC34_PORTRAIT_COUNT]
                       [REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT];
    uint8_t *destination_slots[REDMCSB_F7088_PC34_PORTRAIT_COUNT] = {NULL};
    uint8_t before[sizeof(destination)];
    uint16_t portrait_index;

    memset(destination, 0xA5, sizeof(destination));
    memcpy(before, destination, sizeof(before));
    for (portrait_index = 0U;
         portrait_index < REDMCSB_F7088_PC34_PORTRAIT_COUNT;
         ++portrait_index) {
        fill_portrait(source[portrait_index], (uint8_t)(portrait_index + 1U));
        source_slots[portrait_index] = source[portrait_index];
    }
    CHECK(redmcsb_f7088_copy_included_portraits_to_excluded_pc34(
              source_slots, REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED,
              destination_slots, REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED,
              &destination[0][0], sizeof(destination),
              REDMCSB_F7088_PC34_PORTRAIT_COUNT, 463U) == 0,
          "F7088 refuses a non-source portrait byte count");
    CHECK(memcmp(destination, before, sizeof(destination)) == 0,
          "F7088 mismatch refuses before destination bytes change");
    source_slots[2] = NULL;
    CHECK(redmcsb_f7088_copy_included_portraits_to_excluded_pc34(
              source_slots, REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED,
              destination_slots, REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED,
              &destination[0][0], sizeof(destination),
              REDMCSB_F7088_PC34_PORTRAIT_COUNT,
              REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT) == 0,
          "F7088 refuses a missing source portrait without a fallback");
    CHECK(memcmp(destination, before, sizeof(destination)) == 0,
          "F7088 missing-source refusal has no partial transfer");
}

int main(void)
{
    test_exact_transfer_and_bind();
    test_mismatch_refusal();
    CHECK(strcmp(redmcsb_f7088_portrait_transfer_pc34_source_evidence(),
                 "ReDMCSB CEDTINCR.C F7088 C1-to-C2 portrait transfer") == 0,
          "source evidence identifies F7088 portrait transfer");
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F7088 portrait transfer");
    return 0;
}
