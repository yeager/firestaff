#include "csb_hint_oracle_graphics_surface.h"
#include "csb_hint_oracle_htc_real_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int frames_differ(const uint8_t *a, const uint8_t *b, size_t size)
{
    return memcmp(a, b, size) != 0;
}

int main(int argc, char **argv)
{
    const char *graphics_dir = argc > 1 ? argv[1]
                                        : getenv("FIRESTAFF_CSB_HCSB_DAT_DATA");
    const char *htc_dir = getenv("FIRESTAFF_CSB_HTC_DATA");
    CSB_HintOracleGraphicsSurface surface;
    CSB_HintOracleHTC_RealCache htc_cache;
    CSB_HintOracleHTC_Hint hint;
    uint8_t page_text[2240];
    uint8_t frame1[320u * 200u];
    uint8_t frame2[320u * 200u];
    uint8_t frame3[320u * 200u];
    size_t page_size = 0u;
    int rc;

    if (!graphics_dir || !*graphics_dir) {
        puts("SKIP: set FIRESTAFF_CSB_HCSB_DAT_DATA.");
        return 0;
    }
    if (!htc_dir || !*htc_dir) {
        htc_dir = graphics_dir;
    }

    csb_hint_oracle_graphics_surface_init(&surface);
    rc = csb_hint_oracle_graphics_surface_load(&surface, graphics_dir, 6, NULL);
    if (rc) {
        fprintf(stderr, "FAIL: HCSB.DAT surface load %d\n", rc);
        return 1;
    }
    if (!surface.pixels || surface.width != 320u || surface.height != 200u ||
        !surface.font_pixels || surface.font_width != 256u ||
        (surface.font_height != CSB_HINT_ORACLE_FONT_SURFACE_HEIGHT_ST &&
         surface.font_height != CSB_HINT_ORACLE_FONT_SURFACE_HEIGHT_AU2) ||
        surface.controls[0] != 2u ||
        surface.controls[1] != 0x0620u) {
        fprintf(stderr, "FAIL: invalid surface or source graphics controls\n");
        csb_hint_oracle_graphics_surface_free(&surface);
        return 1;
    }
    printf("surface %ux%u font %ux%u controls=%u/%04x md5=%s\n",
           surface.width, surface.height, surface.font_width,
           surface.font_height, surface.controls[0], surface.controls[1],
           surface.source.matched_md5);

    csb_hint_oracle_htc_real_cache_init(&htc_cache);
    rc = csb_hint_oracle_htc_real_scan_and_load(htc_dir, NULL, 6, &htc_cache);
    if (rc) {
        fprintf(stderr, "FAIL: HCSB.HTC real-media scan %d (%s)\n", rc,
                csb_hint_oracle_htc_real_result_name(rc));
        csb_hint_oracle_graphics_surface_free(&surface);
        csb_hint_oracle_htc_real_cache_free(&htc_cache);
        return 1;
    }
    if (csb_hint_oracle_htc_get_hint(&htc_cache.htc, 0u, &hint) !=
            CSB_HINT_ORACLE_HTC_OK || hint.page_count < 3u) {
        fprintf(stderr, "FAIL: real Hint 0 lacks three authored pages\n");
        csb_hint_oracle_graphics_surface_free(&surface);
        csb_hint_oracle_htc_real_cache_free(&htc_cache);
        return 1;
    }

    for (size_t page = 1u; page <= 3u; ++page) {
        rc = csb_hint_oracle_htc_decode_hint_page(
            &htc_cache.htc, 0u, page, page_text, sizeof(page_text),
            &page_size);
        if (rc != CSB_HINT_ORACLE_HTC_OK || page_size == 0u) {
            fprintf(stderr, "FAIL: real Hint 0 page %zu decode %d\n", page, rc);
            csb_hint_oracle_graphics_surface_free(&surface);
            csb_hint_oracle_htc_real_cache_free(&htc_cache);
            return 1;
        }
        printf("Hint 0 page %zu (%zu bytes): %s\n", page, page_size,
               (const char *)page_text);
    }

    if (!csb_hint_oracle_graphics_surface_render_st_hint_page(
            &surface, &htc_cache.htc, 0u, 1u, frame1, sizeof(frame1)) ||
        !csb_hint_oracle_graphics_surface_render_st_hint_page(
            &surface, &htc_cache.htc, 0u, 2u, frame2, sizeof(frame2)) ||
        !csb_hint_oracle_graphics_surface_render_st_hint_page(
            &surface, &htc_cache.htc, 0u, 3u, frame3, sizeof(frame3)) ||
        !frames_differ(frame1, frame2, sizeof(frame1)) ||
        !frames_differ(frame2, frame3, sizeof(frame2))) {
        fprintf(stderr, "FAIL: real later Hint Oracle pages render distinct text\n");
        csb_hint_oracle_graphics_surface_free(&surface);
        csb_hint_oracle_htc_real_cache_free(&htc_cache);
        return 1;
    }

    puts("PASS: real Amiga Utility Disk Hint Oracle pages render separately");
    csb_hint_oracle_graphics_surface_free(&surface);
    csb_hint_oracle_htc_real_cache_free(&htc_cache);
    return 0;
}
