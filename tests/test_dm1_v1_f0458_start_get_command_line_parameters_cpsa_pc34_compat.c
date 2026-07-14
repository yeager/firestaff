#include "firestaff/dm1/v1/f0458_start_get_command_line_parameters_cpsa_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_equal(int actual, int expected, const char *description)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %d; expected %d)\n", description,
                actual, expected);
        ++failures;
    }
}

static void expect_byte(unsigned char actual, unsigned char expected,
                        const char *description)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %u; expected %u)\n", description,
                (unsigned int)actual, (unsigned int)expected);
        ++failures;
    }
}

static void test_copies_the_reference_length_prefixed_tail(void)
{
    unsigned char base_page[256] = {0};
    char out[8];

    memset(out, 'X', sizeof(out));
    base_page[DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34] = 4;
    memcpy(base_page + DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34 + 1,
           "AUTO", 4);

    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page), out, sizeof(out)),
        4, "returns the base-page command-tail length");
    expect_byte((unsigned char)out[0], 'A', "copies byte zero");
    expect_byte((unsigned char)out[3], 'O', "copies the final tail byte");
    expect_byte((unsigned char)out[4], 'X',
                "preserves the reference unwritten byte after the tail");
    expect_byte((unsigned char)out[5], '\0',
                "writes the reference terminator at length plus one");
}

static void test_zero_length_tail_preserves_the_first_output_byte(void)
{
    unsigned char base_page[129] = {0};
    char out[2] = {'X', 'Y'};

    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page), out, sizeof(out)),
        0, "accepts an empty command tail");
    expect_byte((unsigned char)out[0], 'X',
                "does not synthesize a conventional empty string");
    expect_byte((unsigned char)out[1], '\0',
                "terminates empty tail at index one");
}

static void test_rejects_truncated_or_too_small_buffers_without_writing(void)
{
    unsigned char base_page[133] = {0};
    char out[6] = {'X', 'X', 'X', 'X', 'X', 'X'};

    base_page[DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34] = 4;
    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page) - 1U, out, sizeof(out)),
        -1, "rejects a truncated command-tail source");
    expect_byte((unsigned char)out[0], 'X',
                "truncated source leaves output unchanged");

    memcpy(base_page + DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34 + 1,
           "AUTO", 4);
    memset(out, 'X', sizeof(out));
    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page), out, 5),
        -1, "rejects output without the reference terminator slot");
    expect_byte((unsigned char)out[0], 'X',
                "short output leaves destination unchanged");
}

static void test_rejects_missing_inputs(void)
{
    unsigned char base_page[129] = {0};
    char out[2] = {'X', 'X'};

    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            NULL, 0, out, sizeof(out)),
        -1, "rejects a missing base page");
    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page), NULL, 0),
        -1, "rejects a missing output buffer");
}

static void test_accepts_the_tos_limit_and_rejects_longer_tails(void)
{
    unsigned char base_page[256] = {0};
    char out[129];

    memset(out, 'X', sizeof(out));
    base_page[DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34] =
        DM1_V1_F0458_MAX_COMMAND_TAIL_LENGTH_PC34;
    memset(base_page + DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34 + 1,
           'A', DM1_V1_F0458_MAX_COMMAND_TAIL_LENGTH_PC34);
    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page), out, sizeof(out)),
        DM1_V1_F0458_MAX_COMMAND_TAIL_LENGTH_PC34,
        "accepts the 127-byte TOS command tail");
    expect_byte((unsigned char)out[127], 'X',
                "retains the reference gap at the TOS tail limit");
    expect_byte((unsigned char)out[128], '\0',
                "terminates at the bounded reference position");

    base_page[DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34] = 128;
    memset(out, 'X', sizeof(out));
    expect_equal(
        DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
            base_page, sizeof(base_page), out, sizeof(out)),
        -1, "rejects a command tail beyond the TOS limit");
    expect_byte((unsigned char)out[0], 'X',
                "over-limit tail leaves destination unchanged");
}

int main(void)
{
    test_copies_the_reference_length_prefixed_tail();
    test_zero_length_tail_preserves_the_first_output_byte();
    test_rejects_truncated_or_too_small_buffers_without_writing();
    test_rejects_missing_inputs();
    test_accepts_the_tos_limit_and_rejects_longer_tails();
    return failures != 0;
}
