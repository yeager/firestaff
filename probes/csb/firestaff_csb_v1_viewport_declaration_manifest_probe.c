/* Skip-safe local CSB viewport declaration-manifest admission probe. */
#include "csb_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CSB_V1_VIEWPORT_MANIFEST_MAX_BYTES = 65536 };

static const char *data_dir_arg(int argc, char **argv,
                                char *buffer, size_t buffer_size)
{
    const char *value;
    const char *home;
    if (argc > 1 && argv[1] && argv[1][0]) return argv[1];
    value = getenv("FIRESTAFF_CSBWIN_CSBGRAPHICS_DATA");
    if (value && value[0]) return value;
    value = getenv("FIRESTAFF_DATA_DIR");
    if (value && value[0]) return value;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(buffer, buffer_size, "%s/.firestaff/data", home);
    return buffer;
}

static const char *manifest_path_arg(int argc, char **argv)
{
    const char *value;
    if (argc > 2 && argv[2] && argv[2][0]) return argv[2];
    value = getenv("FIRESTAFF_CSB_VIEWPORT_DECLARATION_MANIFEST");
    return value && value[0] ? value : NULL;
}

static char *read_manifest_text(const char *path)
{
    FILE *file;
    long length;
    char *text;
    if (!path || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) < 1L ||
        length > CSB_V1_VIEWPORT_MANIFEST_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    text = (char *)malloc((size_t)length + 1u);
    if (!text) {
        fclose(file);
        return NULL;
    }
    if (fread(text, 1u, (size_t)length, file) != (size_t)length) {
        fclose(file);
        free(text);
        return NULL;
    }
    if (fclose(file) != 0) {
        free(text);
        return NULL;
    }
    text[length] = '\0';
    return text;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *data_dir = data_dir_arg(argc, argv, default_dir,
                                        sizeof(default_dir));
    const char *manifest_path = manifest_path_arg(argc, argv);
    CSB_V1_BootProfile profile;
    CSB_V1_CSBGraphicsDatPaletteCandidateReport candidates;
    CSB_V1_CSBGraphicsDatPaletteAdmissionSpec palette_spec;
    CSB_V1_ViewportOperatorDeclarationManifestPc34 manifest;
    char *text;
    int rc;

    csb_v1_boot_profile_init(&profile);
    if (data_dir) snprintf(profile.asset_root, sizeof(profile.asset_root), "%s", data_dir);
    rc = csb_v1_boot_scan_csbgraphics(&profile, NULL);
    if (rc == CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND) {
        printf("SKIP: no hash-admitted CSBgraphics.dat corpus is staged.\n");
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    if (rc != CSB_V1_CSBGRAPHICS_RUNTIME_PLAN_OK) {
        printf("FAIL: CSBgraphics scan returned %d.\n", rc);
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (!manifest_path) {
        printf("SKIP: set FIRESTAFF_CSB_VIEWPORT_DECLARATION_MANIFEST or pass a manifest path.\n");
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    memset(&candidates, 0, sizeof(candidates));
    rc = csb_v1_csbgraphics_dat_real_scan_palette_candidates(
        csb_v1_boot_csbgraphics_cache(&profile), &candidates);
    if (rc != CSB_V1_CSBGRAPHICS_DAT_REAL_OK || candidates.candidate_count == 0u) {
        printf("SKIP: admitted corpus has no declared 768-byte palette candidate.\n");
        csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&candidates);
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    memset(&palette_spec, 0, sizeof(palette_spec));
    palette_spec.source_path = candidates.candidates[0].source_path;
    palette_spec.source_md5 = candidates.candidates[0].source_md5;
    palette_spec.entry_index = candidates.candidates[0].entry_span.entry_index;
    palette_spec.decoded_fnv1a = candidates.candidates[0].decoded_fnv1a;
    if (csb_v1_boot_admit_csbgraphics_palette_candidate(&profile, &palette_spec) !=
            CSB_V1_CSBGRAPHICS_DAT_REAL_OK ||
        !csb_v1_boot_csbgraphics_palette_receipt_ready(&profile)) {
        printf("FAIL: exact palette candidate did not produce a boot receipt.\n");
        csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&candidates);
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    text = read_manifest_text(manifest_path);
    if (!text) {
        printf("SKIP: manifest is absent, empty, unreadable, or exceeds %d bytes.\n",
               CSB_V1_VIEWPORT_MANIFEST_MAX_BYTES);
        csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&candidates);
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    memset(&manifest, 0, sizeof(manifest));
    if (!csb_v1_viewport_parse_operator_declaration_manifest_pc34(
            text, csb_v1_boot_csbgraphics_cache(&profile),
            &profile.csbgraphics_palette_receipt, &manifest)) {
        printf("FAIL: manifest did not match the admitted graphics and palette source.\n");
        free(text);
        csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&candidates);
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    printf("PASS: manifest=%s frames=%zu graphics=%s md5=%s; no draw performed.\n",
           manifest_path, manifest.declaration_count, manifest.source_path,
           manifest.source_md5);
    free(text);
    csb_v1_csbgraphics_dat_real_palette_candidate_report_free(&candidates);
    csb_v1_boot_cleanup(&profile);
    return 0;
}
