#include "csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat.h"
#include "csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_f0425_source_named_wrapper_matches_render_plan(void)
{
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat helper_plan;
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat wrapper_plan;
    const char text[] = "RESUME";

    CHECK(csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
        text, sizeof(text), 80, 42, &helper_plan) == 1);
    CHECK(F0425_DIALOG_PrintCenteredChoice(
        text, sizeof(text), 80, 42, &wrapper_plan) == 1);

    CHECK(wrapper_plan.text == helper_plan.text);
    CHECK(wrapper_plan.text_length == 6u);
    CHECK(wrapper_plan.text_x == 62);
    CHECK(wrapper_plan.text_y == 42);
    CHECK(wrapper_plan.bitmap_byte_width ==
          CSB_V1_F0425_DIALOG_VIEWPORT_BYTE_WIDTH_PC34);
    CHECK(wrapper_plan.foreground_color ==
          CSB_V1_F0425_DIALOG_FOREGROUND_GOLD_PC34);
    CHECK(wrapper_plan.background_color ==
          CSB_V1_F0425_DIALOG_BACKGROUND_LIGHT_BROWN_PC34);
    return 0;
}

static int test_f0425_rejects_unbounded_text(void)
{
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat plan;
    const char text_without_nul[] = { 'A', 'B', 'C' };

    CHECK(F0425_DIALOG_PrintCenteredChoice(
        text_without_nul, sizeof(text_without_nul), 80, 42, &plan) == -1);
    CHECK(F0425_DIALOG_PrintCenteredChoice(
        0, 0u, 80, 42, &plan) == 0);
    return 0;
}

static int test_f0426_source_named_wrapper_splits_at_midpoint_space(void)
{
    char message[] = "ABCDEFGHIJKLMNO PQRSTUVWXYZ1234";
    char part1[sizeof(message)];
    char part2[sizeof(message)];

    CHECK(F0426_DIALOG_IsMessageOnTwoLines(message, part1, part2) == 1);
    CHECK(strcmp(part1, "ABCDEFGHIJKLMNO") == 0);
    CHECK(strcmp(part2, "PQRSTUVWXYZ1234") == 0);
    return 0;
}

static int test_f0426_short_messages_stay_single_line(void)
{
    char message[] = "SHORT MESSAGE";
    char part1[sizeof(message)];
    char part2[sizeof(message)];

    part1[0] = '\0';
    part2[0] = '\0';
    CHECK(F0426_DIALOG_IsMessageOnTwoLines(message, part1, part2) == 0);
    CHECK(part1[0] == '\0');
    CHECK(part2[0] == '\0');
    return 0;
}

static int test_source_evidence_names_dialog_symbols(void)
{
    CHECK(strstr(
        csb_v1_f0425_dialog_print_centered_choice_source_evidence_pc34(),
        "F0425") != 0);
    CHECK(strstr(
        csb_v1_f0426_dialog_is_message_on_two_lines_source_evidence_pc34(),
        "F0426") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_f0425_source_named_wrapper_matches_render_plan() == 0);
    CHECK(test_f0425_rejects_unbounded_text() == 0);
    CHECK(test_f0426_source_named_wrapper_splits_at_midpoint_space() == 0);
    CHECK(test_f0426_short_messages_stay_single_line() == 0);
    CHECK(test_source_evidence_names_dialog_symbols() == 0);
    return 0;
}
