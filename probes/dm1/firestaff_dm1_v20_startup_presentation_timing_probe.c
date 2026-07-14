#include "dm1_v20_startup_presentation_timing_pc34.h"

#include <stdint.h>
#include <stdio.h>

static int failures;

#define CHECK(condition, message) do { \
    if (condition) { \
        printf("  [PASS] %s\n", message); \
    } else { \
        printf("  [FAIL] %s\n", message); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    printf("=== DM1 V2.0 startup presentation timing probe ===\n");

    CHECK(dm1_v20_startup_presentation_remaining_delay_ms_pc34(20U, 0U) == 20U,
          "source VBlank delay is unchanged with no presentation work");
    CHECK(dm1_v20_startup_presentation_remaining_delay_ms_pc34(20U, 7U) == 13U,
          "V2.0 title or entrance post-pass consumes the source VBlank budget");
    CHECK(dm1_v20_startup_presentation_remaining_delay_ms_pc34(20U, 20U) == 0U,
          "a presentation finishing at the source boundary adds no delay");
    CHECK(dm1_v20_startup_presentation_remaining_delay_ms_pc34(20U, 31U) == 0U,
          "an over-budget V2.0 presentation never adds a second source delay");
    CHECK(dm1_v20_startup_presentation_remaining_delay_ms_pc34(
              UINT32_MAX, (uint64_t)UINT32_MAX - 1U) == 1U,
          "large source delays retain exact unsigned arithmetic");

    printf("RESULT: %s\n", failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
