#include "redmcsb_f0049_text_messagearea_print_integer_unreferenced_pc34_compat.h"

#include <stdio.h>
#include <string.h>

struct print_capture {
    int calls;
    int16_t color;
    char message[8];
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

    ok &= check(F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced_PC34(
                    capture_print_message, &capture, 8, 0),
                "accepts a print-message delegate for zero");
    ok &= check(capture.calls == 1 && capture.color == 8 &&
                    strcmp(capture.message, "0") == 0,
                "formats zero exactly as TEXT.C F0049");

    ok &= check(F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced_PC34(
                    capture_print_message, &capture, 4, 1234),
                "accepts a multi-digit value");
    ok &= check(capture.calls == 2 && capture.color == 4 &&
                    strcmp(capture.message, "1234") == 0,
                "forwards the requested color and decimal suffix");

    ok &= check(F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced_PC34(
                    capture_print_message, &capture, 15, UINT16_MAX),
                "accepts the source unsigned 16-bit maximum");
    ok &= check(capture.calls == 3 && capture.color == 15 &&
                    strcmp(capture.message, "65535") == 0,
                "keeps the source buffer within its bounded input range");

    ok &= check(!F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced_PC34(
                    NULL, &capture, 1, 7),
                "rejects a missing print-message delegate");
    ok &= check(capture.calls == 3,
                "rejected call does not invoke the delegate");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0049_text_messagearea_print_integer_unreferenced_pc34_compat");
    return 0;
}
