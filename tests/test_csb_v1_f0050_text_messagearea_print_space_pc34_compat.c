#include "redmcsb_f0050_text_messagearea_print_space_unreferenced_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

typedef struct F0050Capture {
    int call_count;
    void *context_seen;
    int16_t color_seen;
    char message_seen[4];
} F0050Capture;

static void capture_print_message(void *context, int16_t text_color, const char *message)
{
    F0050Capture *capture = (F0050Capture *)context;

    ++capture->call_count;
    capture->context_seen = context;
    capture->color_seen = text_color;
    snprintf(capture->message_seen, sizeof(capture->message_seen), "%s", message);
}

static int test_source_named_wrapper_prints_one_white_space(void)
{
    F0050Capture capture = {0, 0, 0, {0}};

    CHECK(F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced(
        capture_print_message,
        &capture) == true);
    CHECK(capture.call_count == 1);
    CHECK(capture.context_seen == &capture);
    CHECK(capture.color_seen == 15);
    CHECK(strcmp(capture.message_seen, " ") == 0);
    return 0;
}

static int test_pc34_entrypoint_matches_source_named_wrapper(void)
{
    F0050Capture wrapper_capture = {0, 0, 0, {0}};
    F0050Capture pc34_capture = {0, 0, 0, {0}};

    CHECK(F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced(
        capture_print_message,
        &wrapper_capture) == true);
    CHECK(F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(
        capture_print_message,
        &pc34_capture) == true);
    CHECK(wrapper_capture.call_count == pc34_capture.call_count);
    CHECK(wrapper_capture.color_seen == pc34_capture.color_seen);
    CHECK(strcmp(wrapper_capture.message_seen, pc34_capture.message_seen) == 0);
    return 0;
}

static int test_missing_delegate_is_bounded_noop(void)
{
    CHECK(F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced(0, 0) == false);
    CHECK(F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(0, 0) == false);
    return 0;
}

static int test_source_evidence_names_f0050(void)
{
    const char *evidence =
        redmcsb_f0050_text_messagearea_print_space_source_evidence_pc34();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced") != 0);
    CHECK(strstr(evidence, "C15") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_prints_one_white_space() == 0);
    CHECK(test_pc34_entrypoint_matches_source_named_wrapper() == 0);
    CHECK(test_missing_delegate_is_bounded_noop() == 0);
    CHECK(test_source_evidence_names_f0050() == 0);
    return 0;
}
