#include "dm2_v1_fmtowns_disc.h"
#include "dm2_v1_fmtowns_graphics_dat.h"
#include "dm2_v1_fmtowns_cd_dat.h"
#include "firestaff_zip_extract.h"

/* This test deliberately keeps assert() enabled.  Several checks call the
 * probe/extraction API inside the assertion; compiling those calls away
 * under the project's release-wide NDEBUG flag would turn a real-media
 * verification test into a false pass with uninitialised receipts. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_api_null_guards(void) {
    DM2_V1_FmtownsDiscReceipt receipt;
    assert(dm2_v1_fmtowns_disc_probe(NULL, 0, &receipt) != 0);
    assert(dm2_v1_fmtowns_disc_probe(NULL, 0, NULL) != 0);
    printf("  PASS: null guards\n");
}

static void test_too_small_image(void) {
    uint8_t buf[100];
    DM2_V1_FmtownsDiscReceipt receipt;
    memset(buf, 0, sizeof(buf));
    assert(dm2_v1_fmtowns_disc_probe(buf, sizeof(buf), &receipt) != 0);
    printf("  PASS: too-small image rejected\n");
}

static void test_disc_image(const uint8_t *image, size_t image_size) {
    DM2_V1_FmtownsDiscReceipt receipt;

    assert(dm2_v1_fmtowns_disc_probe(image, image_size, &receipt) == 0);
    assert(receipt.valid);

    printf("  Volume ID: %s\n", receipt.volume_id);
    printf("  System ID: %s\n", receipt.system_id);
    printf("  Data track sectors: %u\n", receipt.data_track_sectors);

    assert(receipt.has_cd_dat);
    assert(receipt.has_dungeon_dat);
    assert(receipt.has_graphics_dat);
    assert(receipt.startup_media_complete);
    assert(receipt.has_autoexec_bat);
    assert(receipt.has_swoosh);
    assert(receipt.has_title);
    assert(receipt.has_twanim_exp);
    assert(receipt.has_skull_exp);
    assert(receipt.has_end);

    printf("  CD.DAT: LBA=%u size=%u\n", receipt.cd_dat.lba, receipt.cd_dat.size);
    printf("  DUNGEON.DAT: LBA=%u size=%u\n", receipt.dungeon_dat.lba, receipt.dungeon_dat.size);
    printf("  GRAPHICS.DAT: LBA=%u size=%u\n", receipt.graphics_dat.lba, receipt.graphics_dat.size);

    assert(receipt.cd_dat.size == 40);
    assert(receipt.dungeon_dat.size == 37954);
    assert(receipt.graphics_dat.size == 2783791);

    {
        DM2_V1_FmtownsStartupPlan plan;
        assert(dm2_v1_fmtowns_disc_startup_plan(image, image_size,
                                                 &receipt, &plan) == 0);
        assert(plan.valid && plan.stage_count == 4);
        assert(plan.stages[0] == DM2_FMTOWNS_STARTUP_STAGE_SWOOSH);
        assert(plan.stages[1] == DM2_FMTOWNS_STARTUP_STAGE_TITLE);
        assert(plan.stages[2] == DM2_FMTOWNS_STARTUP_STAGE_SKULL);
        assert(plan.stages[3] == DM2_FMTOWNS_STARTUP_STAGE_END);
        printf("  PASS: AUTOEXEC native startup order\n");
    }

    /* Extract and validate CD.DAT */
    {
        uint8_t *cd_data = NULL;
        size_t cd_size = 0;
        DM2_V1_FmtownsCdDatReceipt cd_receipt;

        assert(dm2_v1_fmtowns_disc_extract_alloc(image, image_size,
               &receipt.cd_dat, &cd_data, &cd_size) == 0);
        assert(cd_size == 40);
        assert(dm2_v1_fmtowns_cd_dat_parse(cd_data, cd_size, &cd_receipt) == 0);
        assert(cd_receipt.valid);
        assert(cd_receipt.data_entries == 1);
        assert(cd_receipt.audio_entries == 9);
        assert(dm2_v1_fmtowns_cd_dat_disc_track(&cd_receipt, 0) == 2);
        assert(dm2_v1_fmtowns_cd_dat_disc_track(&cd_receipt, 6) == 8);
        free(cd_data);
        printf("  PASS: CD.DAT extraction + parse\n");
    }

    /* Extract and validate GRAPHICS.DAT header */
    {
        uint8_t *gfx_data = NULL;
        size_t gfx_size = 0;

        assert(dm2_v1_fmtowns_disc_extract_alloc(image, image_size,
               &receipt.graphics_dat, &gfx_data, &gfx_size) == 0);
        assert(gfx_size == 2783791);
        assert(dm2_v1_fmtowns_gdat_probe(gfx_data, gfx_size) == 1);
        free(gfx_data);
        printf("  PASS: GRAPHICS.DAT extraction + probe\n");
    }

    printf("  PASS: disc image probe + extraction\n");
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM2_FMTOWNS_ARCHIVE");
    uint8_t *image = NULL;
    size_t image_size = 0u;
    printf("dm2_v1_fmtowns_disc tests:\n");

    test_api_null_guards();
    test_too_small_image();

    if (archive && archive[0]) {
        assert(firestaff_zip_extract_by_suffix(archive, ".img", &image,
                                                &image_size) == 0 && image);
        test_disc_image(image, image_size);
        free(image);
    } else {
        printf("  SKIP: FIRESTAFF_DM2_FMTOWNS_ARCHIVE not set\n");
    }

    printf("All dm2_v1_fmtowns_disc tests passed.\n");
    return 0;
}
