#include "csb_v1_fmtowns_cd.h"
#include "csb_v1_fmtowns_graphics_dat.h"
#include "firestaff_zip_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

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
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    uint8_t *bin = NULL, *cue = NULL;
    size_t bin_size = 0u, cue_size = 0u;
    CSB_V1_FmtownsCdLayout layout;

    if (!archive || !archive[0]) {
        printf("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set\n");
        return;
    }
    ASSERT(firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) == 0 && cue,
           "read source CUE from ZIP in memory");
    ASSERT(firestaff_zip_extract_by_suffix(archive, ".img", &bin, &bin_size) == 0 && bin,
           "read source IMG from ZIP in memory");
    if (!cue || !bin) { free(cue); free(bin); return; }
    printf("  Loaded IMG: %zu bytes (%.1f MB)\n", bin_size,
           (double)bin_size / (1024.0 * 1024.0));

    ASSERT(csb_v1_fmtowns_cd_probe(bin, bin_size) == 1,
           "probe accepts CSB FM Towns IMG");
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
            uint8_t *gfx_data = NULL;
            size_t gfx_size = 0u;
            gfx_data = (uint8_t *)malloc(gfx_en->size);
            gfx_size = gfx_en->size;
            ASSERT(gfx_data != NULL &&
                   csb_v1_fmtowns_cd_extract(bin, bin_size, gfx_en,
                                              gfx_data, gfx_en->size) == 0,
                   "extract GRAPHICS.DAT succeeds");
            if (gfx_data && gfx_size == gfx_en->size) {
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

        /* ReDMCSB Fujitsu FM-Towns/Source/MKFF.BAT links CHTWE.EXP and
         * CHTWJ.EXP as the two Game programs.  Keep the actual CD members,
         * which SWITCHTW reaches through AUTOEXEC.BAT, distinct from the
         * smaller Utility and Switch executables. */
        {
            const CSB_V1_FmtownsCdFile *game_en = csb_v1_fmtowns_cd_find(
                &layout, "", "CHTWE.EXP");
            const CSB_V1_FmtownsCdFile *game_ja = csb_v1_fmtowns_cd_find(
                &layout, "", "CHTWJ.EXP");
            const CSB_V1_FmtownsCdFile *utility_en = csb_v1_fmtowns_cd_find(
                &layout, "", "UTILE.EXP");
            const CSB_V1_FmtownsCdFile *utility_ja = csb_v1_fmtowns_cd_find(
                &layout, "", "UTILJ.EXP");
            const CSB_V1_FmtownsCdFile *switch_program = csb_v1_fmtowns_cd_find(
                &layout, "", "SWITCHTW.EXP");
            ASSERT(game_en != NULL && game_en->size == 283936u,
                   "find English Game executable with retail byte count");
            ASSERT(game_ja != NULL && game_ja->size == 284416u,
                   "find Japanese Game executable with retail byte count");
            ASSERT(utility_en != NULL && utility_en->size == 152387u,
                   "find English Utility executable with retail byte count");
            ASSERT(utility_ja != NULL && utility_ja->size == 152499u,
                   "find Japanese Utility executable with retail byte count");
            ASSERT(switch_program != NULL && switch_program->size == 90456u,
                   "find Switch executable with retail byte count");
        }
    }

    /* Parse CUE for CDDA tracks */
    {
        CSB_V1_FmtownsCddaLayout cdda;

        if (cue) {
            ASSERT(csb_v1_fmtowns_cdda_parse_cue((const char *)cue, cue_size, &cdda) == 0,
                   "source CUE parse succeeds");
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
            if (cdda.track_count > 0) {
                const CSB_V1_FmtownsCddaTrack *last =
                    &cdda.tracks[cdda.track_count - 1];
                size_t last_capacity = bin_size - last->byte_offset;
                uint8_t *last_pcm = (uint8_t *)malloc(last_capacity);
                ASSERT(last_pcm != NULL, "allocates final CDDA track buffer");
                if (last_pcm) {
                    int extracted = csb_v1_fmtowns_cdda_extract(
                        bin, bin_size, last, last_pcm, last_capacity);
                    ASSERT(extracted > 0, "extracts final CUE track to image end");
                    ASSERT((size_t)extracted == last_capacity,
                           "final CUE track ends at real image boundary");
                    free(last_pcm);
                }
            }
        } else {
            printf("SKIP: CUE member not available\n");
        }
    }

    free(cue);
    free(bin);
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set");
        return 77;
    }
    test_probe_null();
    test_cue_index_one_without_zero_padding();
    test_real_bin();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
