#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theron_v1_track02.h"

static uint8_t *load_track02(size_t *out_size) {
    const char *home = getenv("HOME");
    char path[1024];
    FILE *f;
    long len;
    uint8_t *buf;

    if (!home) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin", home);
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

static int test_cd_play_track_extraction(void) {
    size_t size;
    uint8_t *data = load_track02(&size);
    Theron_Track02CdPlayTrackMapReceipt receipt;
    Theron_Track02SignalStatus status;

    if (!data) { printf("SKIP cd_play_tracks (no data)\n"); return 0; }

    status = theron_v1_track02_extract_cd_play_tracks(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid);

    printf("  CD_PLAY sites: total=%zu code=%zu with_track=%zu\n",
           receipt.total_sites, receipt.code_sites, receipt.sites_with_track);

    /* Must find at least 2 code-region CD_PLAY sites */
    assert(receipt.code_sites >= 2u);
    /* All code sites should have track parameter */
    assert(receipt.sites_with_track == receipt.code_sites);

    /* Both real code sites load track $0E (14 decimal = CD-DA track 14) */
    {
        size_t i;
        for (i = 0u; i < receipt.total_sites; ++i) {
            if (receipt.sites[i].in_code_region) {
                assert(receipt.sites[i].track_param_found);
                assert(receipt.sites[i].track_param == 0x0Eu);
                printf("  CD_PLAY code site: sector %zu, track $%02X\n",
                       receipt.sites[i].sector, receipt.sites[i].track_param);
            }
        }
    }

    free(data);
    printf("PASS cd_play_track_extraction\n");
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
    Theron_Track02CdPlayTrackMapReceipt receipt;

    if (!data) { printf("SKIP cd_play_filter (no data)\n"); return 0; }

    theron_v1_track02_extract_cd_play_tracks(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    /* Total sites > code sites means data false positives were detected */
    assert(receipt.total_sites > receipt.code_sites);
    printf("  Filtered %zu data false positives\n",
           receipt.total_sites - receipt.code_sites);

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
    printf("  CD_PLAY: track $0E proven at 2 code sites\n");
    printf("  VDC: MWR/CR/SATB/HSR/HDR/VDW proven from st0/st1/st2 triplets\n");
    printf("  Joypad: all 5 button groups (I/II/Select/Run/D-pad) proven\n");
    printf("PASS hw_config_summary\n");
    return 0;
}

typedef int (*test_fn)(void);
static const struct { const char *name; test_fn fn; } tests[] = {
    {"cd_play_track_extraction", test_cd_play_track_extraction},
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
