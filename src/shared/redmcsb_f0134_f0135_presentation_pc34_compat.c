#include "redmcsb_f0134_f0135_presentation_pc34_compat.h"

#include <limits.h>
#include <string.h>

enum { REDMCSB_PC34_VGA_PALETTE_BYTES = 16 * 3 };

static uint32_t fnv1a(const uint8_t *bytes, size_t byte_count)
{
    uint32_t value = 2166136261u;
    size_t index;

    if (!bytes || byte_count == 0u) return 0u;
    for (index = 0u; index < byte_count; ++index) {
        value ^= bytes[index];
        value *= 16777619u;
    }
    return value == 0u ? 1u : value;
}

static int target_valid(const Redmcsb_F0134F0135_PresentationTargetPc34 *target)
{
    size_t required;

    if (!target || !target->pixels || target->width <= 0 ||
        target->height <= 0 || target->pitch < target->width ||
        !target->source_frame_verified || !target->no_synthetic_fallback ||
        target->source_frame_fingerprint == 0u ||
        target->palette_fingerprint == 0u) {
        return 0;
    }
    if ((size_t)target->height > SIZE_MAX / (size_t)target->pitch) return 0;
    required = (size_t)target->height * (size_t)target->pitch;
    return target->pixel_count >= required;
}

int redmcsb_f0134_f0135_presentation_target_bind_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *out_target,
    uint8_t *pixels,
    size_t pixel_count,
    int width,
    int height,
    int pitch,
    uint32_t source_frame_fingerprint,
    const uint8_t *palette,
    size_t palette_bytes,
    int source_frame_verified,
    int no_synthetic_fallback)
{
    Redmcsb_F0134F0135_PresentationTargetPc34 target;

    if (!out_target || !pixels || width <= 0 || height <= 0 ||
        pitch < width || source_frame_fingerprint == 0u || !palette ||
        palette_bytes != REDMCSB_PC34_VGA_PALETTE_BYTES ||
        !source_frame_verified || !no_synthetic_fallback ||
        (size_t)height > SIZE_MAX / (size_t)pitch ||
        pixel_count < (size_t)height * (size_t)pitch) {
        return 0;
    }
    memset(&target, 0, sizeof(target));
    target.pixels = pixels;
    target.pixel_count = pixel_count;
    target.width = width;
    target.height = height;
    target.pitch = pitch;
    target.source_frame_fingerprint = source_frame_fingerprint;
    target.palette_fingerprint = fnv1a(palette, palette_bytes);
    target.source_frame_verified = 1;
    target.no_synthetic_fallback = 1;
    if (!target_valid(&target)) return 0;
    *out_target = target;
    return 1;
}

int redmcsb_f0134_fill_bitmap_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *target,
    uint8_t color)
{
    int y;

    if (!target_valid(target) || color > 15u) return 0;
    for (y = 0; y < target->height; ++y) {
        memset(target->pixels + (size_t)y * (size_t)target->pitch,
               color, (size_t)target->width);
    }
    return 1;
}

int redmcsb_f0135_fill_box_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *target,
    const int16_t box[4],
    uint16_t color)
{
    int left;
    int right;
    int top;
    int bottom;
    int y;
    const uint8_t fill_color = (uint8_t)(color & 15u);
    const int alternate = (color & UINT16_C(0x8000)) != 0u;

    if (!target_valid(target) || !box) return 0;
    left = box[0];
    right = box[1];
    top = box[2];
    bottom = box[3];
    if (left > right || top > bottom) return 0;
    if (right < 0 || bottom < 0 || left >= target->width || top >= target->height) {
        return 1;
    }
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right >= target->width) right = target->width - 1;
    if (bottom >= target->height) bottom = target->height - 1;
    for (y = top; y <= bottom; ++y) {
        int x;
        for (x = left; x <= right; ++x) {
            /* Alternate phase is anchored to the caller's original box. */
            if (alternate && (((x - (int)box[0]) + (y - (int)box[2])) & 1) != 0) {
                continue;
            }
            target->pixels[(size_t)y * (size_t)target->pitch + (size_t)x] =
                fill_color;
        }
    }
    return 1;
}

int redmcsb_f0134_f0135_blit_indexed_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *target,
    const uint8_t *source_pixels,
    size_t source_pixel_count,
    int source_width,
    int source_height,
    uint32_t source_palette_fingerprint,
    int dst_x,
    int dst_y,
    int transparent_color)
{
    int source_y;

    if (!target_valid(target) || !source_pixels || source_width <= 0 ||
        source_height <= 0 || source_palette_fingerprint == 0u ||
        source_palette_fingerprint != target->palette_fingerprint ||
        (transparent_color < -1 || transparent_color > 15) ||
        (size_t)source_height > SIZE_MAX / (size_t)source_width ||
        source_pixel_count < (size_t)source_width * (size_t)source_height) {
        return 0;
    }
    for (source_y = 0; source_y < source_height; ++source_y) {
        const int target_y = dst_y + source_y;
        int source_x;

        if (target_y < 0 || target_y >= target->height) continue;
        for (source_x = 0; source_x < source_width; ++source_x) {
            const int target_x = dst_x + source_x;
            const uint8_t pixel = source_pixels[
                (size_t)source_y * (size_t)source_width + (size_t)source_x];

            if (target_x < 0 || target_x >= target->width) continue;
            if (pixel > 15u) return 0;
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            target->pixels[(size_t)target_y * (size_t)target->pitch +
                           (size_t)target_x] = pixel;
        }
    }
    return 1;
}

const char *redmcsb_f0134_f0135_presentation_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEO.C F0134_VIDEO_FillBitmap; FILLBOX.C "
           "F0135_VIDEO_FillBox; BLITFILL.C caller-owned bitmap paths. "
           "PC34 presentation remains indexed VGA4, source-palette-bound, "
           "and rejects generated material.";
}
