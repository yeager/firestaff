#include "csb_v1_f0496_lzw_output_character_pc34_compat.h"

#include <string.h>

int csb_v1_f0496_lzw_output_character_pc34_compat(
    CsbV1F0496LzwDecoderStatePc34Compat *state,
    uint8_t character)
{
    size_t output_needed;

    if (state == NULL || state->output == NULL ||
        state->output_count > state->output_capacity) {
        return 0;
    }

    if (state->repeat_flag == 0u) {
        if (character == 0x90u) {
            state->repeat_flag = 1u;
            return 1;
        }
        if (state->output_count == state->output_capacity) {
            return 0;
        }
        state->output[state->output_count] = character;
        ++state->output_count;
        state->repeat_character = character;
        return 1;
    }

    if (state->repeat_flag != 1u) {
        return 0;
    }

    output_needed = character == 0u ? 1u : (size_t)character - 1u;
    if (output_needed > state->output_capacity - state->output_count) {
        return 0;
    }

    if (character == 0u) {
        state->output[state->output_count] = 0x90u;
    } else {
        memset(state->output + state->output_count,
               state->repeat_character, output_needed);
    }
    state->output_count += output_needed;
    state->repeat_flag = 0u;
    return 1;
}

const char *csb_v1_f0496_lzw_output_character_pc34_compat_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/LZW.C "
           "F0496_LZW_OutputCharacter; DEFS.H:8887";
}
