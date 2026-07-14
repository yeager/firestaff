#include "redmcsb_f0048_text_messagearea_print_character_pc34_compat.h"

#include <stdio.h>

struct print_capture {
    int calls;
    int16_t color;
    char message[2];
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
    capture->message[0] = message[0];
    capture->message[1] = message[1];
}

int main(void)
{
    struct print_capture capture = { 0, 0, { 0, 0 } };
    int ok = 1;

    ok &= check(F0048_TEXT_MESSAGEAREA_PrintCharacter_PC34(
                    capture_print_message, &capture, 8, '7'),
                "accepts a print-message delegate");
    ok &= check(capture.calls == 1 && capture.color == 8 &&
                    capture.message[0] == '7' && capture.message[1] == '\0',
                "forwards the character as a NUL-terminated source string");

    ok &= check(F0048_TEXT_MESSAGEAREA_PrintCharacter_PC34(
                    capture_print_message, &capture, 15, '\0'),
                "forwards a NUL character");
    ok &= check(capture.calls == 2 && capture.color == 15 &&
                    capture.message[0] == '\0' && capture.message[1] == '\0',
                "preserves the source's empty-string F0047 call for NUL");

    ok &= check(!F0048_TEXT_MESSAGEAREA_PrintCharacter_PC34(
                    NULL, &capture, 4, 'X'),
                "rejects a missing print-message delegate");
    ok &= check(capture.calls == 2,
                "rejected call does not invoke the delegate");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0048_text_messagearea_print_character_pc34_compat");
    return 0;
}
