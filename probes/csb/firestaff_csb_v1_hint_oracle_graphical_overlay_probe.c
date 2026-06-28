/*
 * firestaff_csb_v1_hint_oracle_graphical_overlay_probe.c
 *
 * Headless render probe for the CSB Hint Oracle graphical overlay
 * boundary. The synthetic smoke always runs without game data. If a
 * known HCSB.HTC is staged under the Firestaff data root, the probe
 * also decodes hint 0 from that cache and renders it into an 8-bit
 * framebuffer. Hosts without the file SKIP cleanly after the synthetic
 * render path has already proven the graphics boundary.
 *
 * Source: ReDMCSB HINTLOAD.C:11-18, HINTHTC.C:177-358,
 *         HINTLZW.C:122-212, dmweb Hint Oracle Files page.
 */

#include "csb_hint_oracle_graphical_overlay.h"
#include "csb_hint_oracle_htc_real_scan.h"

#include <stdio.h>
#include <string.h>

static int s_pass = 0;
static int s_fail = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        printf("  PASS: %s\n", name);
        ++s_pass;
    } else {
        printf("  FAIL: %s\n", name);
        ++s_fail;
    }
}

static size_t count_nonzero(const uint8_t *fb, size_t len)
{
    size_t count = 0u;
    size_t i;
    for (i = 0u; i < len; ++i) {
        if (fb[i] != 0u) {
            ++count;
        }
    }
    return count;
}

static size_t count_color(const uint8_t *fb, size_t len, uint8_t color)
{
    size_t count = 0u;
    size_t i;
    for (i = 0u; i < len; ++i) {
        if (fb[i] == color) {
            ++count;
        }
    }
    return count;
}

int main(int argc, char **argv)
{
    const char *dir = NULL;
    uint8_t fb[CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_W *
               CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_H];
    CSB_HintOracleOverlay_Stats stats;
    CSB_HintOracleOverlay_Config cfg;
    CSB_HintOracleHTC_RealCache cache;
    int rc;
    size_t known_count = 0u;
    const CSB_HintOracleHTC_RealKnownHash *known;

    if (argc > 1) {
        dir = argv[1];
    }

    printf("=== CSB V1 Hint Oracle graphical overlay probe ===\n\n");

    csb_hint_oracle_overlay_default_config(&cfg);
    printf("[ synthetic graphical overlay render ]\n");
    memset(fb, 0, sizeof(fb));
    rc = csb_hint_oracle_overlay_render_text(
        "CSB HINT ORACLE",
        "Cast/ZOKATHRA and read the oracle page.",
        fb,
        CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_W,
        CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_H,
        &cfg,
        &stats);
    check("render_text returns OK", rc == CSB_HINT_ORACLE_OVERLAY_OK);
    check("render_text writes background pixels",
          stats.background_pixels > 30000u);
    check("render_text writes border pixels", stats.border_pixels > 1000u);
    check("render_text writes glyph pixels", stats.glyph_pixels > 100u);
    check("framebuffer has nonzero pixels",
          count_nonzero(fb, sizeof(fb)) > 30000u);
    check("border color present", count_color(fb, sizeof(fb), cfg.border) > 0u);
    check("title color present", count_color(fb, sizeof(fb), cfg.title) > 0u);
    check("text color present", count_color(fb, sizeof(fb), cfg.text) > 0u);

    printf("\n[ real HCSB.HTC decoded-page overlay render ]\n");
    known = csb_hint_oracle_htc_real_known_hashes(&known_count);
    check("known HCSB.HTC hash table is non-empty",
          known != NULL && known_count > 0u);

    csb_hint_oracle_htc_real_cache_init(&cache);
    rc = csb_hint_oracle_htc_real_scan_and_load(dir, NULL, 6, &cache);
    printf("  scan result: %d (%s)\n",
           rc, csb_hint_oracle_htc_real_result_name(rc));
    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND ||
        rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR) {
        printf("  SKIP: no known HCSB.HTC staged; synthetic overlay "
               "boundary already ran.\n");
        csb_hint_oracle_htc_real_cache_free(&cache);
        return s_fail == 0 ? 0 : 1;
    }
    check("real scan loads a cache", rc == CSB_HINT_ORACLE_HTC_REAL_OK);
    check("cache reports loaded", cache.loaded == 1);
    if (rc == CSB_HINT_ORACLE_HTC_REAL_OK && cache.loaded) {
        uint8_t fb2[sizeof(fb)];
        CSB_HintOracleOverlay_Stats stats2;
        size_t nonzero1;
        size_t nonzero2;

        memset(fb, 0, sizeof(fb));
        rc = csb_hint_oracle_overlay_render_hint(
            &cache, 0u, fb,
            CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_W,
            CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_H,
            &cfg,
            &stats);
        check("render_hint(0) returns OK",
              rc == CSB_HINT_ORACLE_OVERLAY_OK);
        check("render_hint draws decoded-page glyphs",
              stats.glyph_pixels > 100u);
        check("render_hint draws frame",
              stats.border_pixels > 1000u &&
              stats.background_pixels > 30000u);

        memset(fb2, 0, sizeof(fb2));
        rc = csb_hint_oracle_overlay_render_hint(
            &cache, 0u, fb2,
            CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_W,
            CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_H,
            &cfg,
            &stats2);
        nonzero1 = count_nonzero(fb, sizeof(fb));
        nonzero2 = count_nonzero(fb2, sizeof(fb2));
        check("second render returns OK", rc == CSB_HINT_ORACLE_OVERLAY_OK);
        check("decoded-page render is deterministic",
              nonzero1 == nonzero2 &&
              memcmp(fb, fb2, sizeof(fb)) == 0);
    }

    csb_hint_oracle_htc_real_cache_free(&cache);

    printf("\nProbe summary: %d pass, %d fail\n", s_pass, s_fail);
    return s_fail == 0 ? 0 : 1;
}
