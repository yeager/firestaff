#include "csb_v1_atari_f0113_fluxcage_blit.h"

#include "csb_v1_atari_f0132_clip.h"

#include <stddef.h>

int csb_v1_atari_f0113_blit_fluxcage(
    const CSB_V1_AtariF0113FluxcagePlan *plan,
    const uint8_t *field_pixels,
    int field_width,
    int field_height,
    const uint8_t *mask_pixels,
    int mask_width,
    int mask_height,
    int last_unit,
    int first_unit,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int viewport_x,
    int viewport_y)
{
    const int x1 = plan ? plan->destination_frame[0] : 0;
    const int x2 = plan ? plan->destination_frame[1] : -1;
    const int y1 = plan ? plan->destination_frame[2] : 0;
    const int y2 = plan ? plan->destination_frame[3] : -1;
    const int box_width = x2 - x1 + 1;
    const int box_height = y2 - y1 + 1;
    const int padded_width = ((x2 - x1) / 16 + 1) * 16;
    const int source_units_per_row = field_width / 16;
    const int source_unit_count = source_units_per_row * field_height;
    const int mask_required = plan && plan->mask_graphic_index >= 0;
    CSB_V1_AtariF0132VisibleSpan visible_span;
    int written = 0;
    int y;

    if (!plan || plan->kind != CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD ||
        !field_pixels || (mask_required && !mask_pixels) || !framebuffer ||
        field_width <= 0 || field_width % 16 != 0 || field_height <= 0 ||
        (mask_required && (mask_width <= 0 || mask_height <= 0)) ||
        framebuffer_width <= 0 ||
        framebuffer_height <= 0 || viewport_x < 0 || viewport_y < 0 ||
        x1 < 0 || x2 < x1 || y1 < 0 || y2 < y1 ||
        box_width <= 0 || box_height <= 0 ||
        (mask_required &&
         (padded_width != mask_width || mask_height != box_height)) ||
        (mask_required &&
         (plan->source_pixel_width != mask_width ||
          plan->source_height != mask_height)) ||
        (!mask_required && (mask_pixels || mask_width != 0 || mask_height != 0)) ||
        plan->source_x < 0 ||
        plan->source_x + box_width > padded_width ||
        viewport_x + x2 >= framebuffer_width ||
        viewport_y + y2 >= framebuffer_height ||
        source_units_per_row <= 0 || last_unit <= first_unit ||
        first_unit < 0 || last_unit > source_unit_count ||
        !csb_v1_atari_f0132_visible_span(
            (int16_t)plan->source_x, (uint16_t)padded_width,
            (uint16_t)box_width, &visible_span) ||
        visible_span.pixel_count != (uint16_t)box_width) {
        return -1;
    }

    /* MEDIA020 copies and, for mask bit 7, flips the whole mask before F0133.
     * F0133 consumes one 8-byte (16-pixel) mask unit per destination unit.
     * In the authenticated Atari graphics, each expanded mask pixel is 0 or
     * 15 because the source mask word is applied to all four color planes. */
    if (mask_required) {
        const size_t mask_size = (size_t)mask_width * (size_t)mask_height;
        size_t i;
        for (i = 0u; i < mask_size; ++i) {
            if (mask_pixels[i] != 0u && mask_pixels[i] != 15u) return -1;
        }
    }

    for (y = 0; y < box_height; ++y) {
        const int destination_y = viewport_y + y1 + y;
        const int mask_y = y;
        int x;
        if (destination_y < 0 || destination_y >= framebuffer_height) continue;
        for (x = 0; x < visible_span.pixel_count; ++x) {
            const int temporary_x = (int)visible_span.source_offset + x;
            const int destination_x = viewport_x + x1 +
                (int)visible_span.destination_offset + x;
            const int destination_unit =
                (mask_y * (padded_width / 16)) + temporary_x / 16;
            const int source_unit = (first_unit + destination_unit) % last_unit;
            const int source_x = (source_unit % source_units_per_row) * 16 +
                temporary_x % 16;
            const int source_y = source_unit / source_units_per_row;
            const int mask_x = plan->mask_flip_horizontal
                ? mask_width - 1 - temporary_x : temporary_x;
            const uint8_t mask = mask_required ? mask_pixels[
                (size_t)mask_y * (size_t)mask_width + (size_t)mask_x] : 15u;
            const uint8_t color = field_pixels[
                (size_t)source_y * (size_t)field_width + (size_t)source_x];

            if (destination_x < 0 || destination_x >= framebuffer_width ||
                mask != 15u || color == (uint8_t)plan->transparent_color) {
                continue;
            }
            framebuffer[(size_t)destination_y * (size_t)framebuffer_width +
                        (size_t)destination_x] = color & 0x0fu;
            ++written;
        }
    }
    return written;
}
