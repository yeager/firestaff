#include "theron_v1_viewport.h"
#include "theron_v1_palette.h"
#include "theron_v1_vram_trace_loader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define THERON_FIXTURE_OVERRIDEABLE __attribute__((weak))
#else
#define THERON_FIXTURE_OVERRIDEABLE
#endif

/*
 * Production viewport seam.
 *
 * Track 02 proves the initial level bytes, but not the tile/material bank,
 * UI chrome bank, or their square-to-tile mapping.  Keep the source-faithful
 * pixel renderer in fixture targets until those original bindings are
 * decoded.  This seam preserves lifecycle and presentation ownership while
 * refusing to invent viewport pixels in the shipped runtime.
 */

static int theron_vp_try_capture_paths(Theron_V1_Viewport *vp,
                                       const char *vram_snapshot,
                                       const char *vce_snapshot,
                                       const char *vdc_state_snapshot,
                                       const char *vdc_sat_snapshot,
                                       const char *vdc_io_trace,
                                       const char *input_trace,
                                       const char *transition_trace) {
    uint8_t *frame;
    if (!vp || !vram_snapshot || !vram_snapshot[0] ||
        !vce_snapshot || !vce_snapshot[0] ||
        !vdc_state_snapshot || !vdc_state_snapshot[0] ||
        !vdc_sat_snapshot || !vdc_sat_snapshot[0] ||
        !vdc_io_trace || !vdc_io_trace[0])
        return 0;
    if (input_trace && input_trace[0] && transition_trace &&
        transition_trace[0]) {
        if (theron_v1_vram_trace_load_known_atomic_input_capture_bundle(
                vp, vram_snapshot, vce_snapshot, vdc_state_snapshot,
                vdc_sat_snapshot, vdc_io_trace, input_trace,
                transition_trace) != 0 &&
            theron_v1_vram_trace_load_known_atomic_capture_bundle(
                vp, vram_snapshot, vce_snapshot, vdc_state_snapshot,
                vdc_sat_snapshot, vdc_io_trace) != 0)
            return 0;
    } else if (theron_v1_vram_trace_load_known_atomic_capture_bundle(
                   vp, vram_snapshot, vce_snapshot, vdc_state_snapshot,
                   vdc_sat_snapshot, vdc_io_trace) != 0) {
        return 0;
    }
    frame = (uint8_t *)realloc(
        vp->fb.data, (size_t)vp->vdc_display_width *
                     (size_t)vp->vdc_display_height);
    if (!frame) {
        theron_v1_vram_trace_unload(vp);
        return 0;
    }
    vp->fb.data = frame;
    vp->fb.w = vp->vdc_display_width;
    vp->fb.h = vp->vdc_display_height;
    vp->fb.stride = vp->fb.w;
    memset(vp->fb.data, 0, (size_t)vp->fb.w * (size_t)vp->fb.h);
    if (!vp->vdc_state_loaded ||
        theron_v1_vram_trace_populate_tiles(
            vp, 0, vp->vdc_bat_width, vp->vdc_bat_height) <= 0) {
        theron_v1_vram_trace_unload(vp);
        return 0;
    }
    vp->synthetic_rendering_blocked = 1;
    fprintf(stderr,
            "THERON AUTHENTICATED ATOMIC VDC CAPTURE: vram=%s geometry=%dx%d\n",
            vram_snapshot, vp->fb.w, vp->fb.h);
    return 1;
}

static int theron_vp_init_core(Theron_V1_Viewport *vp) {
    if (!vp) return 0;
    memset(vp, 0, sizeof(*vp));
    vp->fb.w = TQR_FB_W;
    vp->fb.h = TQR_FB_H;
    vp->fb.stride = TQR_FB_W;
    vp->fb.data = (uint8_t *)calloc((size_t)TQR_FB_W * TQR_FB_H, 1);
    if (!vp->fb.data) return 0;
    tqr_palette_init_defaults(&vp->palette);
    vp->viewport_x = 0;
    vp->viewport_y = 0;
    vp->initialized = 1;
    return 1;
}

THERON_FIXTURE_OVERRIDEABLE int theron_vp_init(Theron_V1_Viewport *vp) {
    const char *vram_snapshot;
    const char *vce_snapshot;
    const char *vdc_state_snapshot;
    const char *vdc_sat_snapshot;
    const char *vdc_io_trace;

    if (!theron_vp_init_core(vp)) return 0;
    /* Preserve the real viewport lifecycle contract.  Only pixel admission
     * is disabled here; callers still need an initialized, explicitly
     * unbound palette and stable letterbox origin. */
    /* An explicit real-capture pair may be mounted for runtime inspection.
     * Never search for or synthesize snapshots implicitly: both paths must
     * be supplied and the loader must authenticate the exact raw sizes.
     * Square-to-tile semantics remain blocked until the HuC6280 consumer is
     * source-bound, but the real bitmap/palette bank is now owned by the
     * production viewport when this evidence is present. */
    vram_snapshot = getenv("FIRESTAFF_THERON_VRAM_SNAPSHOT");
    vce_snapshot = getenv("FIRESTAFF_THERON_VCE_SNAPSHOT");
    vdc_state_snapshot = getenv("FIRESTAFF_THERON_VDC_STATE_SNAPSHOT");
    vdc_sat_snapshot = getenv("FIRESTAFF_THERON_VDC_SAT_SNAPSHOT");
    vdc_io_trace = getenv("FIRESTAFF_THERON_VDC_IO_TRACE");
    (void)theron_vp_try_capture_paths(
        vp, vram_snapshot, vce_snapshot, vdc_state_snapshot,
        vdc_sat_snapshot, vdc_io_trace, NULL, NULL);
    return 1;
}

THERON_FIXTURE_OVERRIDEABLE int theron_vp_init_from_data_dir(Theron_V1_Viewport *vp,
                                 const char *data_dir) {
    static const char *subdirs[] = {"capture", ""};
    char vram[4096], vce[4096], vdc[4096], sat[4096], io[4096];
    char input[4096], transition[4096];
    size_t i;
    if (!theron_vp_init_core(vp)) return 0;
    if (!data_dir || !data_dir[0]) return 1;
    for (i = 0u; i < sizeof(subdirs) / sizeof(subdirs[0]); ++i) {
        const char *sep = subdirs[i][0] ? "/" : "";
        const char *tail = subdirs[i];
        if (snprintf(vram, sizeof(vram), "%s/%s%strace.vram", data_dir,
                     tail, sep) >= (int)sizeof(vram) ||
            snprintf(vce, sizeof(vce), "%s/%s%strace.vce", data_dir,
                     tail, sep) >= (int)sizeof(vce) ||
            snprintf(vdc, sizeof(vdc), "%s/%s%strace.vdc-state", data_dir,
                     tail, sep) >= (int)sizeof(vdc) ||
            snprintf(sat, sizeof(sat), "%s/%s%strace.sat", data_dir,
                     tail, sep) >= (int)sizeof(sat) ||
            snprintf(io, sizeof(io), "%s/%s%strace.vdc-io", data_dir,
                     tail, sep) >= (int)sizeof(io) ||
            snprintf(input, sizeof(input), "%s/%s%strace.input", data_dir,
                     tail, sep) >= (int)sizeof(input) ||
            snprintf(transition, sizeof(transition),
                     "%s/%s%strace.transition", data_dir,
                     tail, sep) >= (int)sizeof(transition))
            continue;
        if (theron_vp_try_capture_paths(vp, vram, vce, vdc, sat, io,
                                        input, transition))
            break;
    }
    return 1;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_free(Theron_V1_Viewport *vp) {
    if (!vp) return;
    free(vp->fb.data);
    if (vp->vram_trace_loaded) {
        theron_v1_vram_trace_unload(vp);
    } else {
        tqr_palette_free_tiles(&vp->palette);
    }
    memset(vp, 0, sizeof(*vp));
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_set_palette(Theron_V1_Viewport *vp,
                           const TQR_PaletteState *palette) {
    if (!vp || !palette) return;
    /* Once an authenticated VCE snapshot owns the viewport, a later generic
     * palette setter must not replace source colors with an unbound/default
     * palette.  The capture loader is the sole owner until unload. */
    if (vp->vram_trace_loaded) return;
    vp->palette = *palette;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_set_synthetic_rendering_blocked(Theron_V1_Viewport *vp,
                                                int blocked) {
    if (!vp) return;
    vp->synthetic_rendering_blocked = blocked ? 1 : 0;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_render_dungeon(Theron_V1_Viewport *vp,
                              const Theron_V1_World *world) {
    (void)world;
    /* An explicit VDC/VCE capture is already a source-owned screen-space
     * BAT/tile binding.  Use the dedicated native-screen consumer rather
     * than reconstructing its 32x28 window at this call site.  This keeps
     * the production route byte-for-byte tied to the authenticated VDC
     * screen while assigning no cell to the still-unproven dungeon
     * square/object model. */
    if (vp && vp->vram_trace_loaded) {
        (void)theron_v1_vram_trace_render_authenticated_screen(vp);
    }
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_render_ui(Theron_V1_Viewport *vp,
                         const Theron_V1_World *world,
                         uint32_t ui_flags) {
    (void)vp;
    (void)world;
    (void)ui_flags;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_draw_bar(TQR_PlanarFramebuffer *fb, int x, int y, int w, int h,
                        int current, int max, uint8_t pal_index,
                        uint8_t bg_index) {
    (void)fb; (void)x; (void)y; (void)w; (void)h;
    (void)current; (void)max; (void)pal_index; (void)bg_index;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_draw_champion_slot(TQR_PlanarFramebuffer *fb, int slot_idx,
                                  int x, int y,
                                  const Theron_V1_Champion *champion) {
    (void)fb; (void)slot_idx; (void)x; (void)y; (void)champion;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_present(const Theron_V1_Viewport *vp,
                       const TQR_PaletteState *palette,
                       unsigned char *m11_fb, int m11_fb_w, int m11_fb_h) {
    int y;
    (void)palette;
    if (!vp || !vp->initialized || !vp->fb.data || !m11_fb ||
        m11_fb_w <= 0 || m11_fb_h <= 0 || !vp->vram_trace_loaded) {
        return;
    }
    /* Present only an explicitly captured indexed frame.  The M11 surface
     * is indexed too, so preserve the real BAT/VCE group index rather than
     * folding it through a procedural palette or inventing a color map. */
    {
        int dst_x = (m11_fb_w - vp->fb.w) / 2;
        int dst_y = (m11_fb_h - vp->fb.h) / 2;
        if (dst_x < 0) dst_x = 0;
        if (dst_y < 0) dst_y = 0;
    for (y = 0; y < vp->fb.h && y + dst_y < m11_fb_h; ++y) {
        int x;
        unsigned char *dst = m11_fb + (y + dst_y) * m11_fb_w;
        const uint8_t *src = vp->fb.data + y * vp->fb.stride;
        for (x = 0; x < vp->fb.w && x + dst_x < m11_fb_w; ++x) {
            dst[x + dst_x] = src[x];
        }
    }
    }
}

THERON_FIXTURE_OVERRIDEABLE int theron_vp_tile_for_square(int square_type, int depth, int is_wall) {
    (void)square_type; (void)depth; (void)is_wall;
    return -1;
}

THERON_FIXTURE_OVERRIDEABLE void theron_vp_clear(Theron_V1_Viewport *vp, uint8_t color_index) {
    if (!vp || !vp->fb.data) return;
    memset(vp->fb.data, color_index,
           (size_t)vp->fb.w * (size_t)vp->fb.h);
}

THERON_FIXTURE_OVERRIDEABLE const char *theron_v1_viewport_source_evidence(void) {
    /* The authenticated VDC/VCE route now owns a screen-space BAT/tile atlas
     * when a known capture is mounted.  It still does not identify a dungeon
     * square/material selector, perspective transform, HUD record or object
     * consumer; keep those claims explicitly separate. */
    return "AUTHENTICATED SCREEN-SPACE VDC/VCE BAT-TILE ATLAS; "
           "HASH-BOUND RIGHT/LEFT INPUT-SCREEN RECEIPTS; "
           "SQUARE/MATERIAL/PERSPECTIVE/HUD/OBJECT CONSUMERS BLOCKED";
}
