#include "csb_v1_fmtowns_cd.h"
#include "csb_v1_fmtowns_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

static uint8_t *load_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

static void test_probe_null(void) {
    ASSERT(csb_v1_fmtowns_cd_probe(NULL, 0) == 0, "probe rejects NULL");
}

static void test_cue_index_one_without_zero_padding(void) {
    static const char cue[] =
        "FILE \"CHAOS.IMG\" BINARY\n"
        "  TRACK 1 MODE1/2352\n"
        "    INDEX 1 00:00:00\n"
        "  TRACK 2 AUDIO\n"
        "    INDEX 1 01:00:00\n"
        "  TRACK 3 AUDIO\n"
        "    INDEX 01 01:48:00\n";
    CSB_V1_FmtownsCddaLayout cdda;

    ASSERT(csb_v1_fmtowns_cdda_parse_cue(cue, sizeof(cue) - 1u, &cdda) == 0,
           "CUE parser accepts unpadded INDEX 1");
    ASSERT(cdda.track_count == 2, "CUE parser records both audio tracks");
    ASSERT(cdda.tracks[0].track_number == 2 &&
           cdda.tracks[0].start_sector == 4500u,
           "unpadded INDEX 1 preserves first CDDA start");
    ASSERT(cdda.tracks[1].track_number == 3 &&
           cdda.tracks[1].start_sector == 8100u,
           "zero-padded INDEX 01 remains accepted");
    ASSERT(cdda.tracks[0].sector_count == 3600u,
           "CUE parser derives the first CDDA duration");
}

static void test_real_bin(void) {
    char bin_path[512], cue_path[512];
    const char *home = getenv("HOME");
    uint8_t *bin;
    size_t bin_size;
    CSB_V1_FmtownsCdLayout layout;

    if (!home) { printf("SKIP: HOME not set\n"); return; }
    snprintf(bin_path, sizeof(bin_path),
             "%s/.firestaff/data/csb/fmtowns/Chaos Strikes Back for FM-Towns.bin", home);
    snprintf(cue_path, sizeof(cue_path),
             "%s/.firestaff/data/csb/fmtowns/Chaos Strikes Back for FM-Towns.cue", home);

    bin = load_file(bin_path, &bin_size);
    if (!bin) {
        printf("SKIP: FM Towns CSB BIN not available\n");
        return;
    }
    printf("  Loaded BIN: %zu bytes (%.1f MB)\n", bin_size,
           (double)bin_size / (1024.0 * 1024.0));

    /* Probe */
    ASSERT(csb_v1_fmtowns_cd_probe(bin, bin_size) == 1,
           "probe accepts CSB FM Towns BIN");

    /* Parse ISO */
    ASSERT(csb_v1_fmtowns_cd_parse(bin, bin_size, &layout) == 0,
           "parse succeeds");
    ASSERT(strcmp(layout.volume_id, "CHAOS") == 0,
           "volume ID is CHAOS");
    printf("  Volume: %s, %d files, %s sectors\n",
           layout.volume_id, layout.file_count,
           layout.is_raw_2352 ? "raw 2352" : "cooked 2048");

    /* List files */
    {
        int i;
        for (i = 0; i < layout.file_count; i++) {
            const CSB_V1_FmtownsCdFile *f = &layout.files[i];
            printf("    %s%s/%s  lba=%u size=%u\n",
                   f->is_directory ? "[D] " : "    ",
                   f->parent, f->name, f->lba, f->size);
        }
    }

    /* Find key files */
    {
        const CSB_V1_FmtownsCdFile *gfx_en = csb_v1_fmtowns_cd_find(
            &layout, "CDATA", "GRAPHICS.DAT");
        ASSERT(gfx_en != NULL, "find CDATA/GRAPHICS.DAT");
        if (gfx_en) {
            ASSERT(gfx_en->size == CSB_FMTOWNS_GRAPHICS_EN_EXPECTED_SIZE,
                   "EN GRAPHICS.DAT size matches");

            /* Extract and verify with graphics probe */
            uint8_t *gfx_data = (uint8_t *)malloc(gfx_en->size);
            if (gfx_data) {
                ASSERT(csb_v1_fmtowns_cd_extract(bin, bin_size, gfx_en,
                       gfx_data, gfx_en->size) == 0,
                       "extract GRAPHICS.DAT succeeds");
                ASSERT(csb_v1_fmtowns_graphics_probe(gfx_data, gfx_en->size) == 1,
                       "extracted GRAPHICS.DAT passes probe");
                free(gfx_data);
            }
        }

        const CSB_V1_FmtownsCdFile *dun_en = csb_v1_fmtowns_cd_find(
            &layout, "CDATA", "DUNGEON.DAT");
        ASSERT(dun_en != NULL, "find CDATA/DUNGEON.DAT");
        if (dun_en)
            ASSERT(dun_en->size == 2098, "EN DUNGEON.DAT size 2098");

        const CSB_V1_FmtownsCdFile *gfx_jp = csb_v1_fmtowns_cd_find(
            &layout, "CJDATA", "GRAPHICS.DAT");
        ASSERT(gfx_jp != NULL, "find CJDATA/GRAPHICS.DAT");
        if (gfx_jp)
            ASSERT(gfx_jp->size == CSB_FMTOWNS_GRAPHICS_JP_EXPECTED_SIZE,
                   "JP GRAPHICS.DAT size matches");

        const CSB_V1_FmtownsCdFile *portrait = csb_v1_fmtowns_cd_find(
            &layout, "PORTRAIT", "ALEX.CMP");
        ASSERT(portrait != NULL, "find PORTRAIT/ALEX.CMP");
        if (portrait)
            ASSERT(portrait->size == 508, "ALEX.CMP size 508");

        const CSB_V1_FmtownsCdFile *title_anm = csb_v1_fmtowns_cd_find(
            &layout, "", "TITLE.ANM");
        ASSERT(title_anm != NULL, "find TITLE.ANM");
    }

    /* Parse CUE for CDDA tracks */
    {
        char *cue_text;
        size_t cue_size;
        CSB_V1_FmtownsCddaLayout cdda;

        cue_text = (char *)load_file(cue_path, &cue_size);
        if (cue_text) {
            ASSERT(csb_v1_fmtowns_cdda_parse_cue(cue_text, cue_size, &cdda) == 0,
                   "CUE parse succeeds");
            ASSERT(cdda.valid == 1, "CDDA layout valid");
            ASSERT(cdda.track_count == 30, "30 CDDA tracks");
            printf("  CDDA: %d tracks\n", cdda.track_count);

            /* Verify track 2 (first audio) */
            if (cdda.track_count > 0) {
                ASSERT(cdda.tracks[0].track_number == 2, "first audio is track 2");
                ASSERT(cdda.tracks[0].start_sector > 0, "track 2 starts after data");
                printf("    Track 02: sector=%u, %u sectors (%.1fs)\n",
                       cdda.tracks[0].start_sector,
                       cdda.tracks[0].sector_count,
                       (double)cdda.tracks[0].sector_count / 75.0);
            }
            free(cue_text);
        } else {
            printf("SKIP: CUE file not available\n");
        }
    }

    free(bin);
}

int main(void) {
    test_probe_null();
    test_cue_index_one_without_zero_padding();
    test_real_bin();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
