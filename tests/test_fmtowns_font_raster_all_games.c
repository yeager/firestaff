#include "fmtowns_font_raster_all_games.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int locate_in(const char *path, size_t *out_offset, uint8_t out_full[768]) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    if (!buf || fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf); fclose(fp); return 0;
    }
    fclose(fp);
    int ok = fmtowns_font_raster_locate_pc34(buf, (size_t)sz, out_offset);
    if (ok && *out_offset + 768 <= (size_t)sz) {
        memcpy(out_full, buf + *out_offset, 768);
    }
    free(buf);
    return ok;
}

int main(void) {
    assert(FMTOWNS_FONT_RASTER_ALL_GAMES_COUNT == 3);
    /* Look up per-game shipped offsets. */
    assert(strcmp(fmtowns_font_raster_locations[0].game, "DM1") == 0);
    assert(fmtowns_font_raster_locations[1].file_offset == 0x50f1a);
    assert(fmtowns_font_raster_locations[2].file_offset == 0x2f5a3);

    /* Null gate. */
    size_t off;
    uint8_t buf[10] = {0};
    assert(fmtowns_font_raster_locate_pc34(NULL, 10, &off) == 0);
    assert(fmtowns_font_raster_locate_pc34(buf, 10, NULL) == 0);
    assert(fmtowns_font_raster_locate_pc34(buf, 63, &off) == 0);

    /* Real-data: try to locate the font raster in each shipping
     * GRAPHICS.DAT and verify byte-identity across all 3. */
    uint8_t dm1_full[768] = {0};
    uint8_t csb_full[768] = {0};
    uint8_t dm2_full[768] = {0};
    int ran = 0;
    const char *dm1_dir = getenv("FIRESTAFF_DM1_FMTOWNS_DATA_DIR");
    const char *csb_dir = getenv("FIRESTAFF_CSB_FMTOWNS_CDATA_DIR");
    const char *dm2_dir = getenv("FIRESTAFF_DM2_FMTOWNS_DATA_DIR");
    if (dm1_dir) {
        char p[1024]; snprintf(p, sizeof(p), "%s/GRAPHICS.DAT", dm1_dir);
        if (locate_in(p, &off, dm1_full)) ++ran;
    }
    if (csb_dir) {
        char p[1024]; snprintf(p, sizeof(p), "%s/GRAPHICS.DAT", csb_dir);
        if (locate_in(p, &off, csb_full)) { ++ran; assert(off == 0x50f1a); }
    }
    if (dm2_dir) {
        char p[1024]; snprintf(p, sizeof(p), "%s/GRAPHICS.DAT", dm2_dir);
        if (locate_in(p, &off, dm2_full)) { ++ran; assert(off == 0x2f5a3); }
    }
    if (ran >= 2) {
        /* Verify identity across every located pair. */
        if (dm1_dir && csb_dir) assert(memcmp(dm1_full, csb_full, 768) == 0);
        if (dm1_dir && dm2_dir) assert(memcmp(dm1_full, dm2_full, 768) == 0);
        if (csb_dir && dm2_dir) assert(memcmp(csb_full, dm2_full, 768) == 0);
        printf("PASS: %d game(s) verified with byte-identical 768-byte font raster\n", ran);
    } else {
        puts("SKIP: fewer than 2 GRAPHICS.DAT env vars set");
    }

    puts("All fmtowns_font_raster_all_games tests passed.");
    return 0;
}
