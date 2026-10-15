#include "dm1_v1_fmtowns_layout_crop.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #x); ++failures; \
} } while (0)

static void check_crop(unsigned int zone, int width, int height,
                       int sx, int sy, int dx, int dy, int cw, int ch)
{
    DM1_V1_FmtownsLayoutCrop crop;
    int ok = dm1_v1_fmtowns_layout_resolve_crop(zone, width, height, &crop);
    if (!ok || crop.source_x != sx || crop.source_y != sy ||
        crop.destination_x != dx || crop.destination_y != dy ||
        crop.width != cw || crop.height != ch) {
        fprintf(stderr, "crop zone=%u ok=%d got=%d,%d %dx%d src=%d,%d expected=%d,%d %dx%d src=%d,%d\n",
                zone, ok, crop.destination_x, crop.destination_y, crop.width,
                crop.height, crop.source_x, crop.source_y, dx, dy, cw, ch, sx, sy);
    }
    CHECK(ok);
    CHECK(crop.source_x == sx && crop.source_y == sy);
    CHECK(crop.destination_x == dx && crop.destination_y == dy);
    CHECK(crop.width == cw && crop.height == ch);
}

int main(void)
{
    uint8_t source[248 * 111];
    uint8_t destination[224 * 136];
    DM1_V1_FmtownsLayoutCrop crop;
    int x;
    int y;

    /* ReDMCSB DEFS.H F20 M661..M663/C086..C088 and DUNVIEW.C
     * F0128/F0104 select a distinct layout-696 zone for each wall semantic. */
    {
        static const struct { unsigned int pc, graphic, zone; } routes[] = {
            {93, 84, 717}, {94, 85, 716}, {95, 86, 714},
            {96, 86, 713}, {97, 86, 712}, {98, 87, 708},
            {99, 87, 707}, {100, 87, 711}, {101, 87, 710},
            {102, 87, 709}, {103, 89, 703}, {104, 89, 702},
            {105, 88, 706}, {106, 88, 705}, {107, 88, 704}
        };
        DM1_V1_FmtownsWallPanelRoute route;
        for (x = 0; x < (int)(sizeof(routes) / sizeof(routes[0])); ++x) {
            CHECK(dm1_v1_fmtowns_wall_panel_route(routes[x].pc, 0, &route));
            CHECK(route.source_graphic_base == routes[x].graphic);
            CHECK(route.layout_zone == routes[x].zone);
            CHECK(route.flip_horizontally == (routes[x].pc == 103));
            CHECK(dm1_v1_fmtowns_wall_panel_route(routes[x].pc, 1, &route));
            CHECK(route.layout_zone == routes[x].zone);
            CHECK(route.flip_horizontally == (routes[x].pc != 104));
            if (routes[x].pc == 93) CHECK(route.source_graphic_base == 85);
            else if (routes[x].pc == 94) CHECK(route.source_graphic_base == 84);
            else CHECK(route.source_graphic_base == routes[x].graphic);
        }
        CHECK(!dm1_v1_fmtowns_wall_panel_route(92u, 0, &route));
        CHECK(!dm1_v1_fmtowns_wall_panel_route(108u, 0, &route));
    }

    /* ReDMCSB COORD.C G3019/G3016 exact F20E/F20J geometry. */
    check_crop(705, 158, 100, 0, 0, 0, 25, 158, 100); /* D3L */
    check_crop(706, 20, 39, 0, 0, 204, 25, 20, 39);   /* D3R */
    check_crop(707, 136, 71, 0, 0, 44, 20, 136, 71); /* D2L2 */
    check_crop(708, 32, 44, 0, 0, 0, 20, 32, 44);    /* D2R2 */
    check_crop(709, 158, 100, 0, 0, 66, 20, 158, 100);/* D2C */
    check_crop(710, 32, 44, 0, 0, 96, 9, 32, 44);    /* D2L */
    check_crop(711, 20, 39, 0, 0, 0, 9, 20, 39);    /* D2R */
    check_crop(712, 158, 100, 0, 0, 66, 9, 158, 100); /* D1C */
    check_crop(713, 32, 44, 0, 0, 0, 0, 32, 44);     /* D1L */
    check_crop(714, 20, 39, 0, 0, 0, 0, 20, 39);     /* D1R */
    check_crop(715, 158, 100, 0, 0, 66, 0, 158, 100); /* D0C */
    check_crop(716, 32, 44, 0, 0, 12, 28, 32, 44);   /* D0L */
    check_crop(717, 158, 100, 0, 0, 54, 28, 158, 100);/* D0R */
    CHECK(!dm1_v1_fmtowns_layout_resolve_crop(718, 20, 39, &crop));
    CHECK(!dm1_v1_fmtowns_layout_resolve_crop(705, 0, 39, &crop));

    /* Wide native LCR source: right-aligned F0635 crop advances source x. */
    check_crop(706, 248, 111, 24, 0, 0, 25, 224, 111);
    memset(source, 3, sizeof(source));
    source[24] = 10;
    source[25] = 7;
    memset(destination, 0x55, sizeof(destination));
    CHECK(dm1_v1_fmtowns_layout_blit(destination, sizeof(destination), 224,
        source, sizeof(source), 248, 248, 111, 706, 10));
    CHECK(destination[25u * 224u] == 0x55);
    CHECK(destination[25u * 224u + 1u] == 7);
    CHECK(destination[24u * 224u] == 0x55);

    /* Validate complete extents before writing, so a rejected first blit is
     * transactional for both source and destination buffer bounds. */
    memset(destination, 0x55, sizeof(destination));
    CHECK(!dm1_v1_fmtowns_layout_blit(destination, sizeof(destination), 224,
        source, sizeof(source) - 1u, 248, 248, 111, 706, 10));
    CHECK(destination[0] == 0x55 && destination[sizeof(destination) - 1] == 0x55);
    CHECK(!dm1_v1_fmtowns_layout_blit(destination, sizeof(destination), 223,
        source, sizeof(source), 248, 248, 111, 706, 10));
    CHECK(destination[0] == 0x55 && destination[sizeof(destination) - 1] == 0x55);

    /* Explicitly cover the authentic IMG2 dimension table used by the
     * archive receipt. Each image is bounded by its decoded native stride. */
    {
        static const struct { unsigned int graphic, zone; int width, height; } images[] = {
            {78, 701, 25, 94}, {79, 700, 21, 65}, {95, 712, 158, 100},
            {96, 713, 32, 44}, {101, 710, 32, 91},
            {105, 706, 20, 43}, {106, 705, 20, 39}
        };
        for (x = 0; x < (int)(sizeof(images) / sizeof(images[0])); ++x) {
            CHECK(dm1_v1_fmtowns_layout_resolve_crop(
                images[x].zone, images[x].width, images[x].height, &crop));
            CHECK(crop.width <= images[x].width &&
                  crop.height <= images[x].height);
        }
    }
    (void)y;
    if (failures) return 1;
    puts("PASS: DM1 FM Towns G2174 F0635 layout crops");
    return 0;
}
