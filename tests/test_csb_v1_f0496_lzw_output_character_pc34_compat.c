#include "csb_v1_f0496_lzw_output_character_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static CsbV1F0496LzwDecoderStatePc34Compat make_state(
    uint8_t *output,
    size_t capacity)
{
    CsbV1F0496LzwDecoderStatePc34Compat state = {
        output,
        capacity,
        0u,
        0u,
        0u
    };
    return state;
}

static int test_source_named_wrapper_outputs_ordinary_character(void)
{
    uint8_t output[4] = { 0u, 0u, 0u, 0u };
    CsbV1F0496LzwDecoderStatePc34Compat state = make_state(output, sizeof(output));

    CHECK(F0496_LZW_OutputCharacter(&state, 'A') == 1);
    CHECK(state.output_count == 1u);
    CHECK(state.repeat_flag == 0u);
    CHECK(state.repeat_character == 'A');
    CHECK(output[0] == 'A');
    return 0;
}

static int test_repeat_escape_zero_outputs_literal_escape(void)
{
    uint8_t output[4] = { 0u, 0u, 0u, 0u };
    CsbV1F0496LzwDecoderStatePc34Compat state = make_state(output, sizeof(output));

    CHECK(F0496_LZW_OutputCharacter(&state, 0x90u) == 1);
    CHECK(state.output_count == 0u);
    CHECK(state.repeat_flag == 1u);
    CHECK(F0496_LZW_OutputCharacter(&state, 0u) == 1);
    CHECK(state.output_count == 1u);
    CHECK(state.repeat_flag == 0u);
    CHECK(output[0] == 0x90u);
    return 0;
}

static int test_repeat_escape_repeats_previous_character(void)
{
    uint8_t output[8] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
    const uint8_t expected[] = { 'Z', 'Z', 'Z', 'Z' };
    CsbV1F0496LzwDecoderStatePc34Compat state = make_state(output, sizeof(output));

    CHECK(F0496_LZW_OutputCharacter(&state, 'Z') == 1);
    CHECK(F0496_LZW_OutputCharacter(&state, 0x90u) == 1);
    CHECK(F0496_LZW_OutputCharacter(&state, 4u) == 1);
    CHECK(state.output_count == sizeof(expected));
    CHECK(state.repeat_flag == 0u);
    CHECK(memcmp(output, expected, sizeof(expected)) == 0);
    return 0;
}

static int test_capacity_rejection_does_not_mutate_state_or_storage(void)
{
    uint8_t output[2] = { 'Q', 0u };
    const uint8_t before_output[2] = { 'Q', 0u };
    CsbV1F0496LzwDecoderStatePc34Compat state = make_state(output, sizeof(output));
    CsbV1F0496LzwDecoderStatePc34Compat before_state;

    state.output_count = 1u;
    state.repeat_character = 'Q';
    CHECK(F0496_LZW_OutputCharacter(&state, 0x90u) == 1);
    before_state = state;

    CHECK(F0496_LZW_OutputCharacter(&state, 3u) == 0);
    CHECK(memcmp(&state, &before_state, sizeof(state)) == 0);
    CHECK(memcmp(output, before_output, sizeof(output)) == 0);
    return 0;
}

static int test_invalid_repeat_flag_is_rejected_without_mutation(void)
{
    uint8_t output[2] = { 'A', 0u };
    CsbV1F0496LzwDecoderStatePc34Compat state = make_state(output, sizeof(output));
    CsbV1F0496LzwDecoderStatePc34Compat before_state;

    state.repeat_flag = 2u;
    before_state = state;

    CHECK(F0496_LZW_OutputCharacter(&state, 'B') == 0);
    CHECK(memcmp(&state, &before_state, sizeof(state)) == 0);
    CHECK(output[0] == 'A');
    return 0;
}

static int test_wrapper_matches_compat_entrypoint(void)
{
    uint8_t wrapper_output[6] = { 0u, 0u, 0u, 0u, 0u, 0u };
    uint8_t compat_output[6] = { 0u, 0u, 0u, 0u, 0u, 0u };
    CsbV1F0496LzwDecoderStatePc34Compat wrapper_state =
        make_state(wrapper_output, sizeof(wrapper_output));
    CsbV1F0496LzwDecoderStatePc34Compat compat_state =
        make_state(compat_output, sizeof(compat_output));

    CHECK(F0496_LZW_OutputCharacter(&wrapper_state, 'C') == 1);
    CHECK(csb_v1_f0496_lzw_output_character_pc34_compat(
        &compat_state,
        'C') == 1);
    CHECK(F0496_LZW_OutputCharacter(&wrapper_state, 0x90u) == 1);
    CHECK(csb_v1_f0496_lzw_output_character_pc34_compat(
        &compat_state,
        0x90u) == 1);
    CHECK(F0496_LZW_OutputCharacter(&wrapper_state, 3u) == 1);
    CHECK(csb_v1_f0496_lzw_output_character_pc34_compat(
        &compat_state,
        3u) == 1);
    CHECK(wrapper_state.output_count == compat_state.output_count);
    CHECK(wrapper_state.repeat_flag == compat_state.repeat_flag);
    CHECK(wrapper_state.repeat_character == compat_state.repeat_character);
    CHECK(memcmp(wrapper_output, compat_output, sizeof(wrapper_output)) == 0);
    return 0;
}

static int test_source_evidence_names_f0496(void)
{
    const char *evidence =
        csb_v1_f0496_lzw_output_character_pc34_compat_source_evidence();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0496_LZW_OutputCharacter") != 0);
    CHECK(strstr(evidence, "DEFS.H:8887") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_outputs_ordinary_character() == 0);
    CHECK(test_repeat_escape_zero_outputs_literal_escape() == 0);
    CHECK(test_repeat_escape_repeats_previous_character() == 0);
    CHECK(test_capacity_rejection_does_not_mutate_state_or_storage() == 0);
    CHECK(test_invalid_repeat_flag_is_rejected_without_mutation() == 0);
    CHECK(test_wrapper_matches_compat_entrypoint() == 0);
    CHECK(test_source_evidence_names_f0496() == 0);
    return 0;
}
