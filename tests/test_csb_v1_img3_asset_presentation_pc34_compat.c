#include "csb_v1_img3_asset_presentation_pc34_compat.h"

typedef struct {
    int call_count;
    uint8_t packed_pixel;
    uint16_t width;
    uint16_t height;
} presented_img3;

static int present_bitmap(void *context, const uint8_t *pixels,
                          uint16_t width, uint16_t height)
{
    presented_img3 *presented = (presented_img3 *)context;

    if (pixels == 0 || width != 2 || height != 1) {
        return 0;
    }
    presented->call_count++;
    presented->packed_pixel = pixels[0];
    presented->width = width;
    presented->height = height;
    return 1;
}

int main(void)
{
    const uint8_t source[] = {2, 0, 1, 0, 0x09, 0x80, 0, 0x12};
    uint8_t decoded[1] = {0};
    presented_img3 presented = {0, 0, 0, 0};
    csb_v1_img3_asset_presentation_pc34_compat presentation = {
        decoded, sizeof(decoded), present_bitmap, &presented};
    uint16_t width = 0;
    uint16_t height = 0;

    if (!csb_v1_img3_decode_and_present_pc34_compat(
            source, sizeof(source), &presentation, &width, &height) ||
        presented.call_count != 1 || presented.packed_pixel != 0x98 ||
        presented.width != 2 || presented.height != 1 || width != 2 ||
        height != 1) {
        return 1;
    }

    presented.call_count = 0;
    if (csb_v1_img3_decode_and_present_pc34_compat(
            source, sizeof(source) - 1, &presentation, 0, 0) ||
        presented.call_count != 0) {
        return 1;
    }
    return 0;
}
