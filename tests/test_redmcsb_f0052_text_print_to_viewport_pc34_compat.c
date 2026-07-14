#include "redmcsb_f0052_text_print_to_viewport_pc34_compat.h"

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
    uint8_t viewport[1] = { 0 };
    const char text[] = "LOAD";
    struct print_capture capture = { 0 };
    int ok = 1;

    ok &= check(F0052_TEXT_PrintToViewport_PC34(
                    capture_text_print, &capture, viewport, 104, 132, 11,
                    text),
                "accepts an F0040 text-print delegate");
    ok &= check(capture.calls == 1 && capture.context == &capture,
                "makes one F0040 call with its context");
    ok &= check(capture.destination == viewport &&
                    capture.byte_width ==
                        REDMCSB_F0052_VIEWPORT_BYTE_WIDTH_PC34,
                "forwards the viewport bitmap with C112 byte width");
    ok &= check(capture.x == 104 && capture.y == 132 &&
                    capture.text_color == 11 && capture.string == text,
                "forwards F0052 caller arguments unchanged");
    ok &= check(capture.background_color ==
                    REDMCSB_F0052_VIEWPORT_BACKGROUND_COLOR_PC34,
                "always selects C12 as TEXT.C F0052 background");
    ok &= check(capture.height == REDMCSB_F0052_VIEWPORT_HEIGHT_PC34,
                "forwards C136 viewport height as F0040's final argument");

    ok &= check(F0052_TEXT_PrintToViewport_PC34(
                    capture_text_print, &capture, NULL, -1, -2, 13, NULL),
                "preserves F0052's unguarded F0040 forwarding");
    ok &= check(capture.calls == 2 && capture.destination == NULL &&
                    capture.x == UINT16_MAX &&
                    capture.y == UINT16_MAX - 1U &&
                    capture.text_color == 13 && capture.string == NULL &&
                    capture.height == REDMCSB_F0052_VIEWPORT_HEIGHT_PC34,
                "converts coordinate words as F0040's unsigned parameters");

    ok &= check(!F0052_TEXT_PrintToViewport_PC34(
                    NULL, &capture, viewport, 0, 0, 0, text),
                "rejects a missing F0040 text-print delegate");
    ok &= check(capture.calls == 2,
                "a rejected call does not synthesize rendering");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0052_text_print_to_viewport_pc34_compat");
    return 0;
}
