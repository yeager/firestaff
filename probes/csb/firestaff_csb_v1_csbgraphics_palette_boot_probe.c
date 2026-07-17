/* Skip-safe CSBgraphics palette candidate -> boot provenance probe. */
#include "csb_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *data_dir_arg(int argc, char **argv,
                                char *buffer, size_t buffer_size)
{
    const char *value;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0]) {
        return argv[1];
    }
    value = getenv("FIRESTAFF_CSBWIN_CSBGRAPHICS_DATA");
    if (value && value[0]) {
        return value;
    }
    value = getenv("FIRESTAFF_DATA_DIR");
    if (value && value[0]) {
        return value;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(buffer, buffer_size, "%s/.firestaff/data", home);
    return buffer;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *data_dir = data_dir_arg(argc, argv, default_dir,
                                        sizeof(default_dir));
    CSB_V1_BootProfile profile;
    CSB_V1_CSBGraphicsDatPaletteCandidateReport report;
    CSB_V1_CSBGraphicsDatPaletteAdmissionSpec spec;
    CSB_V1_BootStartupCSBGraphicsPaletteReadiness_PC34 readiness;
    int rc;

    csb_v1_boot_profile_init(&profile);
    if (data_dir) {
        snprintf(profile.asset_root, sizeof(profile.asset_root), "%s", data_dir);
    }
    rc = csb_v1_boot_scan_csbgraphics(&profile, NULL);
    if (rc == CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND) {
        printf("SKIP: no hash-admitted CSBgraphics.dat corpus is staged.\n");
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    if (rc != CSB_V1_CSBGRAPHICS_RUNTIME_PLAN_OK) {
        printf("FAIL: boot scan returned %d.\n", rc);
        csb_v1_boot_cleanup(&profile);
        return 1;
    }

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_real_scan_palette_candidates(
        csb_v1_boot_csbgraphics_cache(&profile), &report);
    if (rc != CSB_V1_CSBGRAPHICS_DAT_REAL_OK) {
        printf("FAIL: candidate scan returned %d.\n", rc);
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (report.candidate_count == 0u) {
        printf("SKIP: admitted corpus has no exact 768-byte palette candidate.\n");
        csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&report);
        csb_v1_boot_cleanup(&profile);
        return 0;
    }

    memset(&spec, 0, sizeof(spec));
    spec.source_path = report.candidates[0].source_path;
    spec.source_md5 = report.candidates[0].source_md5;
    spec.entry_index = report.candidates[0].entry_span.entry_index;
    spec.decoded_fnv1a = report.candidates[0].decoded_fnv1a;
    rc = csb_v1_boot_admit_csbgraphics_palette_candidate(&profile, &spec);
    if (rc != CSB_V1_CSBGRAPHICS_DAT_REAL_OK ||
        !csb_v1_boot_startup_csbgraphics_palette_readiness_pc34(
            &profile, &readiness) || !readiness.palette_receipt_ready) {
        printf("FAIL: exact candidate did not produce boot palette provenance.\n");
        csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&report);
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    printf("PASS: palette entry=%u fnv=%08x boot receipt is ready; "
           "title=%d door=%d hud=%d.\n",
           (unsigned)readiness.palette_entry_index,
           (unsigned)readiness.palette_decoded_fnv1a,
           readiness.title_palette_ready, readiness.door_palette_ready,
           readiness.hud_palette_ready);
    csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&report);
    csb_v1_boot_cleanup(&profile);
    return 0;
}
