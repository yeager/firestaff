#include "dm1_v20_startup_presentation_timing_pc34.h"

uint32_t dm1_v20_startup_presentation_remaining_delay_ms_pc34(
    uint32_t source_delay_ms,
    uint64_t presentation_elapsed_ms)
{
    if (presentation_elapsed_ms >= (uint64_t)source_delay_ms) {
        return 0U;
    }
    return source_delay_ms - (uint32_t)presentation_elapsed_ms;
}
