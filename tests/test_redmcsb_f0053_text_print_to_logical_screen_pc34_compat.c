#include "redmcsb_f0053_text_print_to_logical_screen_pc34_compat.h"

#include <stdio.h>

struct print_capture {
    int calls;
    void *context;
    uint8_t *destination;
    uint16_t byte_width;
    uint16_t x;
    uint16_t y;
    int16_t text_color;
    int16_t background_color;
    const char *string;
    uint16_t height;
};

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static void capture_text_print(void *context, uint8_t *destination,
                               uint16_t byte_width, uint16_t x, uint16_t y,
                               int16_t text_color, int16_t background_color,
                               const char *string, uint16_t height)
{
    struct print_capture *capture = context;

    capture->calls++;
    capture->context = context;
    capture->destination = destination;
    capture->byte_width = byte_width;
    capture->x = x;
    capture->y = y;
    capture->text_color = text_color;
    capture->background_color = background_color;
    capture->string = string;
    capture->height = height;
}

int main(void)
{
    uint8_t logical_screen[1] = { 0 };
    const char text[] = "CHAOS";
    struct print_capture capture = { 0 };
    int ok = 1;

    ok &= check(F0053_TEXT_PrintToLogicalScreen_PC34(
                    capture_text_print, &capture, logical_screen, 319, 199,
                    15, 0, text),
                "accepts an F0040 text-print delegate");
    ok &= check(capture.calls == 1 && capture.context == &capture,
                "makes one F0040 call with its context");
    ok &= check(capture.destination == logical_screen &&
                    capture.byte_width ==
                        REDMCSB_F0053_LOGICAL_SCREEN_BYTE_WIDTH_PC34,
                "forwards the logical screen with C160 byte width");
    ok &= check(capture.x == 319 && capture.y == 199 &&
                    capture.text_color == 15 && capture.background_color == 0 &&
                    capture.string == text,
                "forwards every F0053 caller argument unchanged");
    ok &= check(capture.height == REDMCSB_F0053_LOGICAL_SCREEN_HEIGHT_PC34,
                "forwards C200 logical-screen height as F0040's final argument");

    ok &= check(F0053_TEXT_PrintToLogicalScreen_PC34(
                    capture_text_print, &capture, NULL, -1, -2, 4, 12, NULL),
                "preserves F0053's unguarded F0040 forwarding");
    ok &= check(capture.calls == 2 && capture.destination == NULL &&
                    capture.byte_width ==
                        REDMCSB_F0053_LOGICAL_SCREEN_BYTE_WIDTH_PC34 &&
                    capture.x == UINT16_MAX && capture.y == UINT16_MAX - 1U &&
                    capture.text_color == 4 && capture.background_color == 12 &&
                    capture.string == NULL &&
                    capture.height == REDMCSB_F0053_LOGICAL_SCREEN_HEIGHT_PC34,
                "keeps fixed F0040 arguments and converts coordinate words");

    ok &= check(!F0053_TEXT_PrintToLogicalScreen_PC34(
                    NULL, &capture, logical_screen, 0, 0, 0, 0, text),
                "rejects a missing F0040 text-print delegate");
    ok &= check(capture.calls == 2,
                "a rejected call does not synthesize rendering");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0053_text_print_to_logical_screen_pc34_compat");
    return 0;
}
