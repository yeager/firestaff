#include "redmcsb_f8151_f8154_vidrv_source_bound_pc34_compat.h"

#include <limits.h>
#include <string.h>

static int surface_valid(const RedmcsbF8151F8154SourceSurfacePc34 *source)
{
    size_t required;

    if (!source || !source->pixels || source->width <= 0 || source->height <= 0 ||
        source->frame_fingerprint == 0u || source->palette_fingerprint == 0u ||
        !source->source_frame_verified || !source->no_synthetic_fallback ||
        (size_t)source->height > SIZE_MAX / (size_t)source->width) {
        return 0;
    }
    required = (size_t)source->width * (size_t)source->height;
    return source->pixel_count >= required;
}

int redmcsb_f8151_f8154_vidrv_bind_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    uint8_t *target_pixels,
    size_t target_pixel_count,
    int target_width,
    int target_height,
    int target_pitch,
    uint32_t source_frame_fingerprint,
    const uint8_t *palette,
    size_t palette_bytes,
    int source_frame_verified,
    int no_synthetic_fallback,
    RedmcsbF8151F8154VideoStatusReaderPc34 read_status,
    void *status_context,
    size_t maximum_vblank_polls)
{
    RedmcsbF8151F8154VideoDriverPc34 candidate;

    if (!driver || !read_status || maximum_vblank_polls == 0u ||
        !redmcsb_f0134_f0135_presentation_target_bind_pc34(
            &candidate.target, target_pixels, target_pixel_count, target_width,
            target_height, target_pitch, source_frame_fingerprint, palette,
            palette_bytes, source_frame_verified, no_synthetic_fallback)) {
        return 0;
    }
    candidate.read_status = read_status;
    candidate.status_context = status_context;
    candidate.maximum_vblank_polls = maximum_vblank_polls;
    *driver = candidate;
    return 1;
}

int redmcsb_f8151_vidrv_source_blit_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    const RedmcsbF8151F8154SourceSurfacePc34 *source,
    int dst_x,
    int dst_y,
    int transparent_color)
{
    if (!driver || !redmcsb_f0134_f0135_presentation_target_is_bound_pc34(
                       &driver->target) || !surface_valid(source) ||
        source->palette_fingerprint != driver->target.palette_fingerprint) {
        return 0;
    }
    return redmcsb_f0134_f0135_blit_indexed_pc34(
        &driver->target, source->pixels, source->pixel_count, source->width,
        source->height, source->palette_fingerprint, dst_x, dst_y,
        transparent_color);
}

int redmcsb_f8152_vidrv_source_fill_box_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    const int16_t box[4],
    uint16_t color)
{
    if (!driver || !redmcsb_f0134_f0135_presentation_target_is_bound_pc34(
                       &driver->target)) {
        return 0;
    }
    return redmcsb_f0135_fill_box_pc34(&driver->target, box, color);
}

int redmcsb_f8153_vidrv_wait_vertical_blank_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver)
{
    size_t poll;

    if (!driver || !driver->read_status || driver->maximum_vblank_polls == 0u) {
        return 0;
    }
    for (poll = 0u; poll < driver->maximum_vblank_polls; ++poll) {
        if ((driver->read_status(driver->status_context) & 0x08u) == 0u) break;
    }
    if (poll == driver->maximum_vblank_polls) return 0;
    for (poll = 0u; poll < driver->maximum_vblank_polls; ++poll) {
        if ((driver->read_status(driver->status_context) & 0x08u) != 0u) return 1;
    }
    return 0;
}

int redmcsb_f8154_vidrv_source_invert_box_pc34(
    RedmcsbF8151F8154VideoDriverPc34 *driver,
    int16_t x1,
    int16_t x2,
    int16_t y1,
    int16_t y2)
{
    int y;

    if (!driver || !redmcsb_f0134_f0135_presentation_target_is_bound_pc34(
                       &driver->target) || x1 < 0 || y1 < 0) {
        return 0;
    }
    if (x2 < x1 || y2 < y1) return 1;
    if (x2 >= driver->target.width || y2 >= driver->target.height) return 0;
    for (y = y1; y <= y2; ++y) {
        int x;
        for (x = x1; x <= x2; ++x) {
            driver->target.pixels[(size_t)y * (size_t)driver->target.pitch +
                                  (size_t)x] ^= 0x04u;
        }
    }
    return 1;
}

const char *redmcsb_f8151_f8154_vidrv_source_bound_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C F8151/F8152/F8153/F8154: source blit, "
           "inclusive fill, 0x3DA inactive-to-active VBlank, and xor-0x04 "
           "invert. Firestaff routes all writable pixels through the "
           "F0134/F0135 verified indexed-presentation target.";
}
