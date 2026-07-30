#include <stdio.h>
#include <string.h>

#include "csb_v1_csbwin_layout_0232.h"

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static void put_be16(uint8_t *bytes, size_t offset, int value)
{
    bytes[offset] = (uint8_t)((unsigned)value >> 8);
    bytes[offset + 1u] = (uint8_t)value;
}

static void put_rect(uint8_t *bytes, size_t offset,
                     int x1, int x2, int y1, int y2)
{
    put_be16(bytes, offset, x1);
    put_be16(bytes, offset + 2u, x2);
    put_be16(bytes, offset + 4u, y1);
    put_be16(bytes, offset + 6u, y2);
}

int main(void)
{
    uint8_t graphic[CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE];
    CSB_V1_CSBWinLayout0232 layout;
    int index;

    memset(graphic, 0, sizeof(graphic));
    for (index = 0; index < 4; ++index) {
        put_rect(graphic, 376u + (size_t)index * 8u,
                 10 + index * 20, 29 + index * 20, 30, 45);
    }
    put_rect(graphic, 424u, 100, 115, 48, 59);
    put_rect(graphic, 432u, 100, 115, 64, 75);
    put_rect(graphic, 864u, 110, 157, 80, 91);
    put_rect(graphic, 904u, 110, 181, 69, 100);
    put_rect(graphic, 1802u, 216, 319, 88, 159);
    put_rect(graphic, 1818u, 216, 319, 160, 199);

    CHECK(csb_v1_csbwin_layout_0232_decode(graphic, sizeof(graphic), &layout));
    CHECK(layout.valid);
    CHECK(layout.party_direction[3].x1 == 70 && layout.party_direction[3].x2 == 89);
    CHECK(layout.eye_box.y1 == 48 && layout.mouth_box.y1 == 64);
    CHECK(layout.food_water_box.x2 == 181 && layout.poison_box.y2 == 91);
    CHECK(layout.movement_box.x1 == 216 && layout.movement_box.y2 == 159);
    CHECK(layout.magic_box.x2 == 319 && layout.magic_box.y1 == 160);
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.movement_box));
    CHECK(!csb_v1_csbwin_layout_0232_rect_is_screen_valid(NULL));
    layout.magic_box.y2 = 200;
    CHECK(!csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.magic_box));
    CHECK(!csb_v1_csbwin_layout_0232_decode(graphic, sizeof(graphic) - 1u, &layout));
    CHECK(!csb_v1_csbwin_layout_0232_decode(NULL, sizeof(graphic), &layout));
    CHECK(!csb_v1_csbwin_layout_0232_decode(graphic, sizeof(graphic), NULL));

    if (failures) return 1;
    puts("PASS: CSBWin GRAPHICS.DAT 0x232 layout decode");
    return 0;
}
