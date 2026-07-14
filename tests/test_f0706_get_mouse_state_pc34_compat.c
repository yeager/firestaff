#include "f0706_get_mouse_state_pc34_compat.h"

#include <stdio.h>

static int16_t *seenX;
static int16_t *seenY;
static int16_t *seenButtons;
static int callCount;

static void capture_get_mouse_state(
    int16_t *outX,
    int16_t *outY,
    int16_t *outButtons)
{
    seenX = outX;
    seenY = outY;
    seenButtons = outButtons;
    ++callCount;
    *outX = -17;
    *outY = 203;
    *outButtons = (int16_t)0x8003;
}

static int expect_int(const char *label, int actual, int expected)
{
    if (actual == expected) {
        return 1;
    }
    fprintf(stderr, "%s: got %d, expected %d\n", label, actual, expected);
    return 0;
}

int main(void)
{
    const ReDMCSB_F0706_IODriverPc34 ioDriver = { capture_get_mouse_state };
    int16_t x = 0;
    int16_t y = 0;
    int16_t buttons = 0;
    int ok = 1;

    ReDMCSB_F0706_GetMouseStatePc34Compat(&ioDriver, &x, &y, &buttons);

    ok &= expect_int("one IODRV_13 call", callCount, 1);
    ok &= expect_int("x pointer forwarded", seenX == &x, 1);
    ok &= expect_int("y pointer forwarded", seenY == &y, 1);
    ok &= expect_int("button pointer forwarded", seenButtons == &buttons, 1);
    ok &= expect_int("driver x preserved", x, -17);
    ok &= expect_int("driver y preserved", y, 203);
    ok &= expect_int("driver buttons preserved", (unsigned short)buttons, 0x8003);

    ReDMCSB_F0706_GetMouseStatePc34Compat(0, &x, &y, &buttons);
    ok &= expect_int("missing driver is a no-op", callCount, 1);

    return ok ? 0 : 1;
}
