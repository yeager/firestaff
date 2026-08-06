#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_cue_parse(void) {
    const char *cue =
        "FILE \"test.bin\" BINARY\n"
        "  TRACK 01 MODE1/2048\n"
        "    INDEX 01 00:00:00\n"
        "  TRACK 02 AUDIO\n"
        "    PREGAP 00:02:00\n"
        "    INDEX 01 00:28:50\n"
        "  TRACK 03 AUDIO\n"
        "    INDEX 01 02:28:50\n";

    uint32_t starts[4] = {0};
    int count = fmtowns_cue_parse_track_starts(cue, strlen(cue), starts, 4);
    if (count < 3) {
        fprintf(stderr, "FAIL: expected 3 tracks, got %d\n", count);
        return 1;
    }
    if (starts[1] != 0) {
        fprintf(stderr, "FAIL: track 1 should start at 0, got %u\n", starts[1]);
        return 1;
    }
    /* 00:28:50 = 28*75+50 = 2150 */
    if (starts[2] != 2150) {
        fprintf(stderr, "FAIL: track 2 should start at 2150, got %u\n", starts[2]);
        return 1;
    }
    /* 02:28:50 = 2*60*75+28*75+50 = 9000+2100+50 = 11150 */
    if (starts[3] != 11150) {
        fprintf(stderr, "FAIL: track 3 should start at 11150, got %u\n", starts[3]);
        return 1;
    }
    printf("OK: CUE parse\n");
    return 0;
}

static int test_dm2_fmtowns_probe(void) {
    const char *zip = getenv("FIRESTAFF_DM2_FMTOWNS_ZIP");
    if (!zip || !zip[0]) {
        printf("SKIP: FIRESTAFF_DM2_FMTOWNS_ZIP not set\n");
        return 0;
    }

    uint8_t *img = NULL;
    size_t img_size = 0;
    if (firestaff_zip_extract_by_suffix(zip, ".img", &img, &img_size) != 0) {
        fprintf(stderr, "FAIL: could not extract .img\n");
        return 1;
    }

    FmtownsDiscProbeResult result;
    if (fmtowns_disc_probe(img, img_size, FMTOWNS_SECTOR_2352, &result) != 0) {
        fprintf(stderr, "FAIL: disc probe failed\n");
        free(img);
        return 1;
    }

    printf("Volume: %s, System: %s, entries: %d\n",
           result.volume_id, result.system_id, result.entry_count);

    const FmtownsIsoEntry *gfx = fmtowns_disc_find(&result, "DATA/GRAPHICS.DAT");
    if (!gfx) {
        fprintf(stderr, "FAIL: GRAPHICS.DAT not found\n");
        free(img);
        return 1;
    }
    printf("OK: GRAPHICS.DAT at LBA %u, size %u\n", gfx->lba, gfx->size);

    free(img);
    return 0;
}

static int test_dm1_fmtowns_probe(void) {
    const char *zip = getenv("FIRESTAFF_DM1_FMTOWNS_ZIP");
    if (!zip || !zip[0]) {
        printf("SKIP: FIRESTAFF_DM1_FMTOWNS_ZIP not set\n");
        return 0;
    }

    uint8_t *img = NULL;
    size_t img_size = 0;
    if (firestaff_zip_extract_by_suffix(zip, ".bin", &img, &img_size) != 0) {
        fprintf(stderr, "FAIL: could not extract .bin\n");
        return 1;
    }

    FmtownsDiscProbeResult result;
    if (fmtowns_disc_probe(img, img_size, FMTOWNS_SECTOR_2048, &result) != 0) {
        fprintf(stderr, "FAIL: disc probe failed\n");
        free(img);
        return 1;
    }

    printf("Volume: %s, System: %s, entries: %d\n",
           result.volume_id, result.system_id, result.entry_count);

    const FmtownsIsoEntry *gfx = fmtowns_disc_find(&result, "DATA/GRAPHICS.DAT");
    if (!gfx) {
        fprintf(stderr, "FAIL: DATA/GRAPHICS.DAT not found\n");
        free(img);
        return 1;
    }
    printf("OK: DATA/GRAPHICS.DAT at LBA %u, size %u\n", gfx->lba, gfx->size);

    const FmtownsIsoEntry *dgn = fmtowns_disc_find(&result, "DATA/DUNGEON.DAT");
    if (dgn)
        printf("OK: DATA/DUNGEON.DAT at LBA %u, size %u\n", dgn->lba, dgn->size);

    /* DM1 FM Towns startup is owned by the original language executable and
     * Towns launcher, not by the PC34 TITLE/SWOOSH files.  Keep the media
     * inventory gate explicit so a future startup handoff cannot silently
     * lose these real files. */
    const FmtownsIsoEntry *jgfx = fmtowns_disc_find(&result, "JDATA/GRAPHICS.DAT");
    const FmtownsIsoEntry *jdgn = fmtowns_disc_find(&result, "JDATA/DUNGEON.DAT");
    const FmtownsIsoEntry *edm = fmtowns_disc_find(&result, "EDM.EXP");
    const FmtownsIsoEntry *jdm = fmtowns_disc_find(&result, "JDM.EXP");
    const FmtownsIsoEntry *tmenu = fmtowns_disc_find(&result, "TMENU.EXP");
    const FmtownsIsoEntry *tmenuIcon = fmtowns_disc_find(&result, "TMENU.ICN");
    if (!jgfx || jgfx->size != 396407U || !jdgn || jdgn->size != 33931U ||
        !edm || edm->size != 310518U || !jdm || jdm->size != 290221U ||
        !tmenu || tmenu->size != 235476U || !tmenuIcon ||
        tmenuIcon->size != 32768U) {
        fprintf(stderr, "FAIL: DM1 FM Towns startup/menu inventory incomplete\n");
        free(img);
        return 1;
    }
    printf("OK: DM1 FM Towns EN/JP startup and menu inventory\n");

    free(img);
    return 0;
}

int main(void) {
    int fails = 0;
    fails += test_cue_parse();
    fails += test_dm2_fmtowns_probe();
    fails += test_dm1_fmtowns_probe();
    return fails ? 1 : 0;
}
