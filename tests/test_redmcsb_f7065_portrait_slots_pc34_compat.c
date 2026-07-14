#include "redmcsb_f7065_portrait_slots_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); failures++; } \
} while (0)

static void test_clear_and_rebind(void)
{
    uint8_t portraits[3][4] = {{1}, {2}, {3}};
    uint8_t *slots[3] = {portraits[0], portraits[1], portraits[2]};

    redmcsb_f7065_clear_portrait_slots_before_save_pc34(
        slots, 3U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED);
    CHECK(slots[0] == NULL && slots[1] == NULL && slots[2] == NULL,
          "F7065 clears every excluded-format portrait pointer before save");
    CHECK(redmcsb_f7066_bind_portrait_slots_after_load_pc34(
              slots, 3U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED,
              &portraits[0][0], sizeof(portraits), 4U) == 1,
          "F7066 binds the complete stored portrait buffer");
    CHECK(slots[0] == portraits[0] && slots[1] == portraits[1] &&
              slots[2] == portraits[2],
          "F7066 restores source sequential portrait addresses");
}

static void test_nonexcluded_format_untouched(void)
{
    uint8_t portrait[4] = {0};
    uint8_t *slot = portrait;

    redmcsb_f7065_clear_portrait_slots_before_save_pc34(
        &slot, 1U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED);
    CHECK(slot == portrait, "included-format portrait slot is not cleared");
    CHECK(redmcsb_f7066_bind_portrait_slots_after_load_pc34(
              &slot, 1U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED,
              NULL, 0U, 0U) == 1,
          "included-format portrait slot needs no external portrait bytes");
    CHECK(slot == portrait, "included-format portrait slot remains untouched");
}

static void test_no_partial_rebind(void)
{
    uint8_t portraits[8] = {0};
    uint8_t *slots[3] = {(uint8_t *)1, (uint8_t *)2, (uint8_t *)3};
    uint8_t *before[3];

    memcpy(before, slots, sizeof(before));
    CHECK(redmcsb_f7066_bind_portrait_slots_after_load_pc34(
              slots, 3U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED,
              portraits, sizeof(portraits), 4U) == 0,
          "undersized portrait storage is refused");
    CHECK(memcmp(slots, before, sizeof(slots)) == 0,
          "undersized portrait storage causes no partial pointer rebind");
}

int main(void)
{
    test_clear_and_rebind();
    test_nonexcluded_format_untouched();
    test_no_partial_rebind();
    CHECK(strcmp(redmcsb_f7065_portrait_slots_pc34_source_evidence(),
                 "ReDMCSB CEDTINCS.C F7065/F7066") == 0,
          "source evidence identifies F7065/F7066");
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F7065/F7066 portrait slots");
    return 0;
}
