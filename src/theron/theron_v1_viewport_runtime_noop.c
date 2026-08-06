#include "theron_v1_viewport.h"
#include "theron_v1_palette.h"
#include "theron_v1_vram_trace_loader.h"

#include <stdlib.h>
#include <string.h>

/*
 * Production viewport seam.
 *
 * Track 02 proves the initial level bytes, but not the tile/material bank,
 * UI chrome bank, or their square-to-tile mapping.  Keep the source-faithful
 * pixel renderer in fixture targets until those original bindings are
 * decoded.  This seam preserves lifecycle and presentation ownership while
 * refusing to invent viewport pixels in the shipped runtime.
 */

int theron_vp_init(Theron_V1_Viewport *vp) {
    const char *vram_snapshot;
    const char *vce_snapshot;

    if (!vp) return 0;
    memset(vp, 0, sizeof(*vp));
    vp->fb.w = TQR_FB_W;
    vp->fb.h = TQR_FB_H;
    vp->fb.stride = TQR_FB_W;
    vp->fb.data = (uint8_t *)calloc((size_t)TQR_FB_W * TQR_FB_H, 1);
    if (!vp->fb.data) return 0;
    /* Preserve the real viewport lifecycle contract.  Only pixel admission
     * is disabled here; callers still need an initialized, explicitly
     * unbound palette and stable letterbox origin. */
    tqr_palette_init_defaults(&vp->palette);
    vp->viewport_x = 0;
    vp->viewport_y = 0;
    vp->initialized = 1;

    /* An explicit real-capture pair may be mounted for runtime inspection.
     * Never search for or synthesize snapshots implicitly: both paths must
     * be supplied and the loader must authenticate the exact raw sizes.
     * Square-to-tile semantics remain blocked until the HuC6280 consumer is
     * source-bound, but the real bitmap/palette bank is now owned by the
     * production viewport when this evidence is present. */
    vram_snapshot = getenv("FIRESTAFF_THERON_VRAM_SNAPSHOT");
    vce_snapshot = getenv("FIRESTAFF_THERON_VCE_SNAPSHOT");
    if (vram_snapshot && vram_snapshot[0] && vce_snapshot && vce_snapshot[0] &&
        theron_v1_vram_trace_load_files(vp, vram_snapshot, vce_snapshot) == 0 &&
        theron_v1_vram_trace_populate_tiles(vp, 0, 64, 32) > 0) {
        vp->synthetic_rendering_blocked = 1;
    } else if (vp->vram_trace_loaded) {
        theron_v1_vram_trace_unload(vp);
    }
    return 1;
}

void theron_vp_free(Theron_V1_Viewport *vp) {
    if (!vp) return;
    free(vp->fb.data);
    if (vp->vram_trace_loaded) {
        theron_v1_vram_trace_unload(vp);
    } else {
        tqr_palette_free_tiles(&vp->palette);
    }
    memset(vp, 0, sizeof(*vp));
}

void theron_vp_set_palette(Theron_V1_Viewport *vp,
                           const TQR_PaletteState *palette) {
    if (!vp || !palette) return;
    vp->palette = *palette;
}

void theron_vp_set_synthetic_rendering_blocked(Theron_V1_Viewport *vp,
                                                int blocked) {
    if (!vp) return;
    vp->synthetic_rendering_blocked = blocked ? 1 : 0;
}

void theron_vp_render_dungeon(Theron_V1_Viewport *vp,
                              const Theron_V1_World *world) {
    (void)vp;
    (void)world;
}

void theron_vp_render_ui(Theron_V1_Viewport *vp,
                         const Theron_V1_World *world,
                         uint32_t ui_flags) {
    (void)vp;
    (void)world;
    (void)ui_flags;
}

void theron_vp_draw_bar(TQR_PlanarFramebuffer *fb, int x, int y, int w, int h,
                        int current, int max, uint8_t pal_index,
                        uint8_t bg_index) {
    (void)fb; (void)x; (void)y; (void)w; (void)h;
    (void)current; (void)max; (void)pal_index; (void)bg_index;
}

void theron_vp_draw_champion_slot(TQR_PlanarFramebuffer *fb, int slot_idx,
                                  int x, int y,
                                  const Theron_V1_Champion *champion) {
    (void)fb; (void)slot_idx; (void)x; (void)y; (void)champion;
}

void theron_vp_present(const Theron_V1_Viewport *vp,
                       const TQR_PaletteState *palette,
                       unsigned char *m11_fb, int m11_fb_w, int m11_fb_h) {
    (void)vp; (void)palette; (void)m11_fb; (void)m11_fb_w; (void)m11_fb_h;
}

int theron_vp_tile_for_square(int square_type, int depth, int is_wall) {
    (void)square_type; (void)depth; (void)is_wall;
    return -1;
}

void theron_vp_clear(Theron_V1_Viewport *vp, uint8_t color_index) {
    if (!vp || !vp->fb.data) return;
    memset(vp->fb.data, color_index,
           (size_t)vp->fb.w * (size_t)vp->fb.h);
}

const char *theron_v1_viewport_source_evidence(void) {
    /* This reports only the capture-side bank ownership. It does not claim
     * that a dungeon square or UI record has been mapped to these tiles. */
    return "NO VERIFIED TRACK02 TILE/MATERIAL/UI BANK";
}
