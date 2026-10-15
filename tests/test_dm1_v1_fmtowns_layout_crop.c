#include "dm1_v1_fmtowns_layout_crop.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #x); ++failures; \
} } while (0)

/* Source oracle: COORD.C G2002 negative-bitmap aliases (169-198),
 * G3019 F20 zones, and DUNVIEW.C F0116..F0126. The native bitmap is
 * mirrored by F1000 before F0630 selects an alias's source columns. */
static const struct {
    unsigned int semantic, graphic, zone, pc_zone;
    int alias_x, alias_width, native_width, height, x, y;
} walls[] = {
    {93,84,715,717,0,0,32,136,192,0},
    {94,85,714,716,0,0,32,136,0,0},
    {95,86,712,714,0,60,248,111,164,9},
    {96,86,711,713,188,60,248,111,0,9},
    {97,86,710,712,44,160,248,111,32,9},
    {100,87,709,711,0,75,136,71,149,20},
    {101,87,708,710,61,75,136,71,0,20},
    {102,87,707,709,16,104,136,71,60,20},
    {103,89,703,703,0,0,16,49,208,25},
    {104,89,702,702,0,0,16,49,0,25},
    {105,88,706,706,0,85,117,51,139,25},
    {106,88,705,705,32,84,117,51,0,25},
    {107,88,704,704,18,76,117,51,74,25}
};

int main(void)
{
    uint8_t source[248 * 136], actual[231 * 136], expected[231 * 136];
    DM1_V1_FmtownsWallPanelRoute route;
    DM1_V1_FmtownsLayoutCrop crop;
    for (size_t i = 0; i < sizeof(walls) / sizeof(walls[0]); ++i) {
        for (int parity = 0; parity < 2; ++parity) {
            const int width = walls[i].alias_width ? walls[i].alias_width : walls[i].native_width;
            const int flip = walls[i].semantic == 103 || (parity && walls[i].semantic != 104);
            unsigned int graphic = walls[i].graphic;
            if (parity && walls[i].semantic == 93) graphic = 85;
            if (parity && walls[i].semantic == 94) graphic = 84;
            CHECK(dm1_v1_fmtowns_wall_panel_route(walls[i].semantic, parity, &route));
            CHECK(route.source_graphic_base == graphic && route.layout_zone == walls[i].zone);
            CHECK(route.pc34_zone == walls[i].pc_zone);
            CHECK(route.source_x == walls[i].alias_x && route.logical_width == walls[i].alias_width);
            CHECK(route.flip_horizontally == flip);
            CHECK(dm1_v1_fmtowns_wall_resolve_crop(&route, walls[i].native_width, walls[i].height, &crop));
            CHECK(crop.source_x == walls[i].alias_x && crop.source_y == 0);
            CHECK(crop.destination_x == walls[i].x && crop.destination_y == walls[i].y);
            CHECK(crop.width == width && crop.height == walls[i].height);
            for (int y = 0; y < walls[i].height; ++y)
                for (int x = 0; x < walls[i].native_width; ++x)
                    source[y * walls[i].native_width + x] = (uint8_t)((x * 3 + y * 7) & 15);
            memset(actual, 0x55, sizeof(actual));
            memset(expected, 0x55, sizeof(expected));
            for (int y = 0; y < walls[i].height; ++y) for (int x = 0; x < width; ++x) {
                int sx = walls[i].alias_x + x;
                if (flip) sx = walls[i].native_width - 1 - sx;
                const uint8_t color = source[y * walls[i].native_width + sx];
                if (color != 10) expected[(walls[i].y + y) * 231 + walls[i].x + x] = color;
            }
            CHECK(dm1_v1_fmtowns_wall_blit(actual, sizeof(actual), 231,
                source, (size_t)walls[i].native_width * walls[i].height,
                walls[i].native_width, walls[i].native_width, walls[i].height, &route, 10));
            CHECK(memcmp(actual, expected, sizeof(actual)) == 0);
            memset(actual, 0x55, sizeof(actual));
            CHECK(!dm1_v1_fmtowns_wall_blit(actual, sizeof(actual), 231,
                source, (size_t)walls[i].native_width * walls[i].height - 1,
                walls[i].native_width, walls[i].native_width, walls[i].height, &route, 10));
            for (size_t j = 0; j < sizeof(actual); ++j) CHECK(actual[j] == 0x55);
        }
    }
    /* F20 has no D2 outer-wall calls (MEDIA720 alone calls F0678/F0679). */
    CHECK(!dm1_v1_fmtowns_wall_panel_route(98, 0, &route));
    CHECK(!dm1_v1_fmtowns_wall_panel_route(99, 1, &route));
    CHECK(!dm1_v1_fmtowns_wall_panel_route(92, 0, &route));
    CHECK(!dm1_v1_fmtowns_wall_panel_route(108, 0, &route));
    CHECK(dm1_v1_fmtowns_layout_resolve_crop(700, 224, 29, &crop));
    CHECK(crop.source_x == 0 && crop.source_y == 0 && crop.destination_x == 0 &&
          crop.destination_y == 0 && crop.width == 224 && crop.height == 29);
    CHECK(dm1_v1_fmtowns_layout_resolve_crop(701, 224, 70, &crop));
    CHECK(crop.source_x == 0 && crop.source_y == 0 && crop.destination_x == 0 &&
          crop.destination_y == 66 && crop.width == 224 && crop.height == 70);
    CHECK(!dm1_v1_fmtowns_layout_resolve_crop(718, 20, 39, &crop));
    CHECK(!dm1_v1_fmtowns_layout_resolve_crop(705, 0, 39, &crop));
    if (failures) return 1;
    puts("PASS: F20's thirteen source aliases, full-native mirror before crop, C10 and bounded writes");
    return 0;
}
