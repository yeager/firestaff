#include "csb_v1_f0496_lzw_output_character_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    uint8_t output[5] = {0xa5u, 0xa5u, 0xa5u, 0xa5u, 0xa5u};
    uint8_t before[5];
    CsbV1F0496LzwDecoderStatePc34Compat state = {
        output, sizeof(output), 0u, 0u, 0u
    };
    int ok = 1;

    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0x41u) == 1 &&
                    state.output_count == 1u && output[0] == 0x41u &&
                    state.repeat_character == 0x41u,
                "ordinary character is stored and becomes the repeat character");
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0x90u) == 1 &&
                    state.output_count == 1u && state.repeat_flag == 1u,
                "repeat escape is held without output");
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 4u) == 1 &&
                    state.output_count == 4u && state.repeat_flag == 0u &&
                    memcmp(output, "AAAA", 4u) == 0,
                "nonzero repeat count emits count minus one prior characters");
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0x90u) == 1 &&
                    csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0u) == 1 &&
                    state.output_count == 5u && output[4] == 0x90u,
                "zero repeat count emits a literal escape character");

    memcpy(before, output, sizeof(output));
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0x42u) == 0 &&
                    state.output_count == 5u &&
                    memcmp(output, before, sizeof(output)) == 0,
                "full output state rejects without a write or count change");

    state.output = output;
    state.output_capacity = 3u;
    state.output_count = 1u;
    state.repeat_flag = 1u;
    state.repeat_character = 0x42u;
    memcpy(before, output, sizeof(output));
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 4u) == 0 &&
                    state.output_count == 1u && state.repeat_flag == 1u &&
                    memcmp(output, before, sizeof(output)) == 0,
                "oversized repeat expansion is rejected atomically");

    state.output = NULL;
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0x43u) == 0 &&
                    state.output_count == 1u,
                "missing caller-owned output storage is rejected");

    state.output = output;
    state.repeat_flag = 2u;
    memcpy(before, output, sizeof(output));
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(&state, 0x43u) == 0 &&
                    state.repeat_flag == 2u &&
                    memcmp(output, before, sizeof(output)) == 0,
                "invalid repeat state is rejected without recovery behavior");
    ok &= check(csb_v1_f0496_lzw_output_character_pc34_compat(NULL, 0x44u) == 0,
                "missing decoder state is rejected");
    ok &= check(strstr(csb_v1_f0496_lzw_output_character_pc34_compat_source_evidence(),
                       "F0496_LZW_OutputCharacter") != NULL,
                "source evidence identifies the exact callable");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_f0496_lzw_output_character_pc34_compat");
    return 0;
}
