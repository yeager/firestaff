#include "redmcsb_f7067_portrait_info_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); failures++; } } while (0)

int main(void)
{
    uint8_t portrait_a[2] = {1, 2};
    uint8_t portrait_b[2] = {3, 4};
    uint8_t *slots[2] = {portrait_a, NULL};
    uint8_t *result = NULL;

    CHECK(redmcsb_f7067_get_champion_portrait_pc34(
              slots, 2U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED,
              0U, &result) == 1 && result == portrait_a,
          "F7067 gets the C31 portrait pointer for excluded-format champions");
    CHECK(redmcsb_f7068_set_champion_portrait_pc34(
              slots, 2U, REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED,
              1U, portrait_b) == 1 && slots[1] == portrait_b,
          "F7068 sets the C31 portrait pointer for included-format champions");
    result = portrait_a;
    CHECK(redmcsb_f7067_get_champion_portrait_pc34(slots, 2U, 99U, 0U, &result) == 0 &&
              result == portrait_a,
          "unknown champion format has no portrait fallback");
    CHECK(redmcsb_f7068_set_champion_portrait_pc34(slots, 2U,
              REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED, 2U, portrait_a) == 0 &&
              slots[1] == portrait_b,
          "out-of-range champion does not mutate a portrait slot");
    CHECK(strcmp(redmcsb_f7067_portrait_info_pc34_source_evidence(),
                 "ReDMCSB CEDT007.C F7067/F7068 C31_CHAMPION_INFO_PORTRAIT") == 0,
          "source evidence identifies F7067/F7068 portrait access");
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F7067/F7068 portrait info");
    return 0;
}
