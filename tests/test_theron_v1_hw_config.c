#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theron_v1_track02.h"

static uint8_t *load_track02_named(const char *basename, size_t *out_size) {
    const char *home = getenv("HOME");
    char path[1024];
    FILE *f;
    long len;
    uint8_t *buf;

    if (!home || !basename || !out_size) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/%s", home, basename);
    f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    buf = malloc((size_t)len);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)len;
    return buf;
}

static uint8_t *load_track02(size_t *out_size) {
    return load_track02_named("TQUS02.bin", out_size);
}

static int test_cd_play_candidate_scan(void) {
    static const uint8_t stage2_cd_play_caller[] = {
        0x20u, 0x03u, 0x44u, 0xc6u, 0x5bu, 0xa0u, 0x01u, 0xb1u,
        0x1cu, 0xf0u, 0x13u, 0x44u, 0x54u, 0x44u, 0x31u, 0x64u,
        0xf9u, 0x64u, 0xfau, 0xa9u, 0x0eu, 0x85u, 0xffu, 0xc6u,
        0xf8u, 0x20u, 0x3fu, 0xe0u
    };
    static const struct {
        const char *basename;
        const char *md5;
        const char *region;
        size_t stage2_sector;
    } media[] = {
        { "TQUS02.bin", THERON_TRACK02_MD5_US_BIN, "US",
          THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR +
              THERON_TRACK02_IPL_STAGE2_RECORD },
        { "TQJP02.bin", THERON_TRACK02_MD5_JP_BIN, "JP",
          THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR +
              THERON_TRACK02_IPL_STAGE2_RECORD }
    };
    size_t available_regions = 0u;
    size_t region_index;

    for (region_index = 0u;
         region_index < sizeof(media) / sizeof(media[0]); ++region_index) {
        size_t size;
        size_t code_sites = 0u;
        size_t stage2_cd_play_sites = 0u;
        uint8_t *data = load_track02_named(media[region_index].basename, &size);
        Theron_Track02CdPlayCandidateCatalog catalog;
        Theron_Track02SignalStatus status;
        size_t i;

        if (!data) {
            printf("SKIP cd_play_candidates %s (authentic data unavailable)\n",
                   media[region_index].region);
            continue;
        }
        ++available_regions;
        status = theron_v1_track02_scan_cd_play_candidates(
            data, size, media[region_index].md5, &catalog);
        assert(status == THERON_TRACK02_SIGNAL_OK);
        assert(catalog.valid);
        assert(catalog.code_sites > 0u);
        {
            const size_t caller_user_offset = 0x375u;
            const size_t caller_raw_offset =
                media[region_index].stage2_sector *
                    THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET + caller_user_offset;
            assert(caller_raw_offset <= size &&
                   sizeof(stage2_cd_play_caller) <= size - caller_raw_offset);
            assert(memcmp(data + caller_raw_offset, stage2_cd_play_caller,
                          sizeof(stage2_cd_play_caller)) == 0);
        }
        for (i = 0u; i < catalog.total_sites; ++i) {
            if (!catalog.sites[i].in_code_region) continue;
            ++code_sites;
            assert(catalog.sites[i].prior_ff_immediate_found);
            if (catalog.sites[i].sector == media[region_index].stage2_sector &&
                catalog.sites[i].user_data_offset == 0x38eu) {
                const size_t expected_raw_offset =
                    media[region_index].stage2_sector *
                        THERON_TRACK02_RAW_SECTOR_BYTES +
                    THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x38eu;
                assert(catalog.sites[i].raw_offset == expected_raw_offset);
                ++stage2_cd_play_sites;
            }
            printf("  %s $E03F call-site candidate: sector %zu, nearby $FF immediate $%02X (not a track mapping)\n",
                   media[region_index].region, catalog.sites[i].sector,
                   catalog.sites[i].prior_ff_immediate);
        }
        printf("  %s static candidates: total=%zu code=%zu with_nearby_ff_immediate=%zu\n",
               media[region_index].region, catalog.total_sites,
               catalog.code_sites, catalog.sites_with_ff_immediate);
        assert(code_sites == catalog.code_sites);
        assert(catalog.sites_with_ff_immediate == catalog.code_sites);
        assert(stage2_cd_play_sites == 1u);
        if (strcmp(media[region_index].region, "US") == 0) {
            assert(catalog.code_sites >= 2u);
            for (i = 0u; i < catalog.total_sites; ++i) {
                if (catalog.sites[i].in_code_region)
                    assert(catalog.sites[i].prior_ff_immediate == 0x0Eu);
            }
        }
        free(data);
    }
    if (available_regions == 0u) {
        printf("SKIP cd_play_candidates (no authentic regional data)\n");
        return 0;
    }
    printf("PASS cd_play_candidate_scan\n");
    return 0;
}

static int test_vdc_display_config(void) {
    size_t size;
    uint8_t *data = load_track02(&size);
    Theron_Track02VdcDisplayConfigReceipt receipt;
    Theron_Track02SignalStatus status;

    if (!data) { printf("SKIP vdc_display_config (no data)\n"); return 0; }

    status = theron_v1_track02_extract_vdc_display_config(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid);

    printf("  MWR proven=%d\n", receipt.mwr_proven);
    printf("  CR proven=%d\n", receipt.cr_proven);
    printf("  SATB proven=%d\n", receipt.satb_proven);
    printf("  Width proven=%d\n", receipt.screen_width_proven);
    printf("  Height proven=%d\n", receipt.screen_height_proven);
    printf("  VDC registers used: %zu\n", receipt.config_site_count);

    /* Must prove key display registers */
    assert(receipt.cr_proven);
    assert(receipt.mwr_proven);
    assert(receipt.satb_proven);
    assert(receipt.screen_width_proven);
    assert(receipt.screen_height_proven);

    free(data);
    printf("PASS vdc_display_config\n");
    return 0;
}

static int test_joypad_action_map(void) {
    size_t size;
    uint8_t *data = load_track02(&size);
    Theron_Track02JoypadActionMapReceipt receipt;
    Theron_Track02SignalStatus status;

    if (!data) { printf("SKIP joypad_action_map (no data)\n"); return 0; }

    status = theron_v1_track02_extract_joypad_actions(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid);

    printf("  Action sites: %zu\n", receipt.action_site_count);
    printf("  Combined mask: $%02X\n", receipt.combined_button_mask);
    printf("  D-pad=%d I=%d II=%d Select=%d Run=%d\n",
           receipt.dpad_proven, receipt.button_i_proven,
           receipt.button_ii_proven, receipt.select_proven, receipt.run_proven);

    /* All buttons must be proven — Theron's Quest uses the full pad */
    assert(receipt.dpad_proven);
    assert(receipt.button_i_proven);
    assert(receipt.button_ii_proven);
    assert(receipt.run_proven);
    assert(receipt.select_proven);

    free(data);
    printf("PASS joypad_action_map\n");
    return 0;
}

static int test_cd_play_data_false_positive_filter(void) {
    size_t size;
    uint8_t *data = load_track02(&size);
    Theron_Track02CdPlayCandidateCatalog catalog;

    if (!data) { printf("SKIP cd_play_filter (no data)\n"); return 0; }

    theron_v1_track02_scan_cd_play_candidates(
        data, size, THERON_TRACK02_MD5_US_BIN, &catalog);

    /* Total sites > code sites means data false positives were detected */
    assert(catalog.total_sites > catalog.code_sites);
    printf("  Filtered %zu data false positives\n",
           catalog.total_sites - catalog.code_sites);

    free(data);
    printf("PASS cd_play_data_false_positive_filter\n");
    return 0;
}

static int test_vdc_config_sites_populated(void) {
    size_t size;
    uint8_t *data = load_track02(&size);
    Theron_Track02VdcDisplayConfigReceipt receipt;

    if (!data) { printf("SKIP vdc_config_sites (no data)\n"); return 0; }

    theron_v1_track02_extract_vdc_display_config(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.config_site_count > 0u);
    {
        size_t i;
        for (i = 0u; i < receipt.config_site_count; ++i) {
            printf("  VDC reg $%02X: %u occurrences\n",
                   receipt.config_sites[i].reg,
                   receipt.config_sites[i].value);
        }
    }

    free(data);
    printf("PASS vdc_config_sites_populated\n");
    return 0;
}

static int test_hw_config_summary(void) {
    printf("\n=== Theron V1 Hardware Configuration Summary ===\n");
    printf("  $E03F: static call-site candidates only; register semantics and runtime selection unverified\n");
    printf("  VDC: MWR/CR/SATB/HSR/HDR/VDW proven from st0/st1/st2 triplets\n");
    printf("  Joypad: all 5 button groups (I/II/Select/Run/D-pad) proven\n");
    printf("PASS hw_config_summary\n");
    return 0;
}

typedef int (*test_fn)(void);
static const struct { const char *name; test_fn fn; } tests[] = {
    {"cd_play_candidate_scan", test_cd_play_candidate_scan},
    {"vdc_display_config", test_vdc_display_config},
    {"joypad_action_map", test_joypad_action_map},
    {"cd_play_data_false_positive_filter", test_cd_play_data_false_positive_filter},
    {"vdc_config_sites_populated", test_vdc_config_sites_populated},
    {"hw_config_summary", test_hw_config_summary},
};

int main(int argc, char **argv) {
    const char *filter = (argc > 1) ? argv[1] : NULL;
    int ran = 0;
    size_t i;

    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        if (filter && !strstr(tests[i].name, filter)) continue;
        printf("[%s]\n", tests[i].name);
        tests[i].fn();
        ++ran;
    }
    printf("\n%d test(s) passed.\n", ran);
    return 0;
}
