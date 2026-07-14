#include "redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat.h"

static uint8_t redmcsb_f0683_source_pixel(const uint8_t *source, size_t pixel_index)
{
    const uint8_t packed = source[pixel_index >> 1u];

    return (pixel_index & 1u) == 0u ? (uint8_t)(packed >> 4u) :
                                      (uint8_t)(packed & 0x0fu);
}

int redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(
    const uint8_t *source_packed_4bpp,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *vga_a000_aperture,
    size_t aperture_bytes,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color,
    uint8_t color_offset)
{
    size_t source_end;
    size_t destination_end;
    size_t offset;

    if (source_packed_4bpp == NULL || vga_a000_aperture == NULL ||
        transparent_color > 0x0fu || source_bytes > SIZE_MAX / 2u ||
        source_pixel_index > SIZE_MAX - pixel_count ||
        destination_pixel_index > SIZE_MAX - pixel_count) {
        return 0;
    }

    source_end = source_pixel_index + pixel_count;
    destination_end = destination_pixel_index + pixel_count;
    if (source_end > source_bytes * 2u || destination_end > aperture_bytes) {
        return 0;
    }

    for (offset = 0u; offset < pixel_count; ++offset) {
        const uint8_t pixel = redmcsb_f0683_source_pixel(
            source_packed_4bpp, source_pixel_index + pixel_count - offset - 1u);

        if (pixel != transparent_color) {
            /* IMAGE5.C C25 VGA: OR, not palette-index addition or masking. */
            vga_a000_aperture[destination_pixel_index + offset] =
                (uint8_t)(pixel | color_offset);
        }
    }

    return 1;
}

const char *redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat_source_evidence(void)
{
    return "ReDMCSB IMAGE3.C:612-829 F0683 reverse nibble traversal; "
           "VIDEODRV.C:2656-2661 C25 VGA F0683 route; "
           "IMAGE5.C:936-1010 C25 VGA A000 byte aperture and palette OR.";
}
