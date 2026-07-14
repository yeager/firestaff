#include "redmcsb_f0050_text_messagearea_print_space_unreferenced_pc34_compat.h"

#include <stdio.h>
#include <string.h>

struct print_capture {
    int calls;
    int16_t color;
    char message[4];
};

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static void capture_print_message(void *context, int16_t color,
                                  const char *message)
{
    struct print_capture *capture = context;

    capture->calls++;
    capture->color = color;
    strncpy(capture->message, message, sizeof(capture->message) - 1U);
    capture->message[sizeof(capture->message) - 1U] = '\0';
}

int main(void)
{
    struct print_capture capture = { 0, 0, "" };
    int ok = 1;

    ok &= check(F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(
                    capture_print_message, &capture),
                "accepts a print-message delegate");
    ok &= check(capture.calls == 1 && capture.color == 15 &&
                    strcmp(capture.message, " ") == 0,
                "delegates one white space exactly as TEXT.C F0050");

    ok &= check(!F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(
                    NULL, &capture),
                "rejects a missing print-message delegate");
    ok &= check(capture.calls == 1,
                "rejected call does not invoke the delegate");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0050_text_messagearea_print_space_unreferenced_pc34_compat");
    return 0;
}
