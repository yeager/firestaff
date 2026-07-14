#include "dm1_v1_object_draw_icon_to_screen_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int assertions;

static void check(const char *name, int condition)
{
    ++assertions;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", name);
    }
}

typedef struct {
    unsigned char pixels[DM1_V1_OBJECT_ICON_WIDTH_PC34 *
                         DM1_V1_OBJECT_ICON_HEIGHT_PC34];
    int available;
} IconFixture;

static const unsigned char *lookup_icon(void *context,
                                         int icon_index,
                                         int *out_row_bytes)
{
    IconFixture *fixture = (IconFixture *)context;

    if (!fixture || !fixture->available || icon_index != 37) return 0;
    *out_row_bytes = DM1_V1_OBJECT_ICON_WIDTH_PC34;
    return fixture->pixels;
}

int main(void)
{
    unsigned char screen[40 * 32];
    unsigned char before[sizeof(screen)];
    IconFixture fixture;
    DM1_V1_ObjectDrawIconSurfacePc34 surface;
    int row;

    memset(screen, 0x55, sizeof(screen));
    memset(&fixture, 0, sizeof(fixture));
    memset(&surface, 0, sizeof(surface));
    fixture.available = 1;
    for (row = 0; row < DM1_V1_OBJECT_ICON_HEIGHT_PC34; ++row) {
        fixture.pixels[row * DM1_V1_OBJECT_ICON_WIDTH_PC34] = 10;
        fixture.pixels[row * DM1_V1_OBJECT_ICON_WIDTH_PC34 + 1] =
            (unsigned char)(20 + row);
    }
    surface.lookup_icon = lookup_icon;
    surface.lookup_context = &fixture;
    surface.screen_pixels = screen;
    surface.screen_width = 40;
    surface.screen_height = 32;
    surface.screen_row_bytes = 40;
    surface.transparent_color = 10;

    check("draws source-locked F0037 icon",
          dm1_v1_object_draw_icon_to_screen_f0037_pc34(&surface, 37, 12, 8));
    check("transparent source leaves destination intact", screen[8 * 40 + 12] == 0x55);
    check("icon pixels copy at F0037 origin", screen[8 * 40 + 13] == 20);
    check("each icon row keeps its source stride", screen[23 * 40 + 13] == 35);
    check("outside icon box stays untouched", screen[8 * 40 + 28] == 0x55);

    memcpy(before, screen, sizeof(screen));
    check("missing F0036 icon rejects draw",
          !dm1_v1_object_draw_icon_to_screen_f0037_pc34(&surface, 38, 0, 0));
    check("missing icon does not mutate screen", !memcmp(screen, before, sizeof(screen)));

    memcpy(before, screen, sizeof(screen));
    check("partially offscreen icon rejects draw",
          !dm1_v1_object_draw_icon_to_screen_f0037_pc34(&surface, 37, 25, 8));
    check("offscreen icon does not mutate screen", !memcmp(screen, before, sizeof(screen)));

    printf("test_dm1_v1_object_draw_icon_to_screen_pc34_compat: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures != 0;
}
