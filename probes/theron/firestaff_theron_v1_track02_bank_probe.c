/*
 * firestaff_theron_v1_track02_bank_probe.c
 *
 * Theron's Quest V1 -- Track 02 bank evidence probe.
 *
 * This probe does not prove a dungeon map grid or promote runtime loading.
 * It regression-locks hash-gated JP/US Track 02 bank-stride anchors and keeps
 * the JP Rev 1 zero-filled ISO outcome explicit.
 */

#include "asset_status_m12.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

static int g_fail = 0;
static int g_skip = 0;

static const uint8_t g_descriptor[18] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static const uint8_t g_post_boundary_span[44] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
    0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
    0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
    0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
    0x93, 0x80, 0x00, 0x3f
};

static const size_t g_us_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};

static const size_t g_jp_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static const size_t g_us_bin_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d53e0u, 0x47d040u, 0x712840u
};

static const size_t g_jp_bin_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d4ab0u, 0x47c710u, 0x711f10u
};

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        ++g_fail;
    }
}

static void check_size(const char *label, size_t got, size_t want) {
    if (got != want) {
        printf("FAIL %s: got %zu want %zu\n", label, got, want);
        ++g_fail;
    }
}

static void check_u16(const char *label, uint16_t got, uint16_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%04x want 0x%04x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_fail;
    }
}

static int file_exists(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return 0;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static void check_no_reported_offset(const char *label,
                                     const Theron_Track02BankSignal *signal) {
    check_size(label, signal->descriptor_offset, 0u);
    check_size(label, signal->descriptor_size, 0u);
    check_size(label, signal->post_descriptor_zero_offset, 0u);
    check_size(label, signal->post_descriptor_zero_bytes, 0u);
    check_size(label, signal->next_nonzero_offset, 0u);
    check_size(label, signal->boundary_prefix_size, 0u);
    check_size(label, signal->post_boundary_span_size, 0u);
}

static void default_data_path(const char *relative_name, char out_path[512]) {
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = ".";
    snprintf(out_path, 512, "%s%s.firestaff%sdata%s%s",
             home, PATH_SEP, PATH_SEP, PATH_SEP, relative_name);
}

static void check_iso_signal(const Theron_Track02BankSignal *signal) {
    check_int("variant", signal->variant, THERON_TRACK02_VARIANT_US_ISO);
    check_size("anchor count", signal->anchor_count, 1u);
    check_size("descriptor offset", signal->descriptor_offset, 0x1584u);
    check_size("descriptor offsets[0]", signal->descriptor_offsets[0], 0x1584u);
    check_size("descriptor size", signal->descriptor_size, 18u);
    check_size("descriptor occurrence count", signal->occurrence_count, 1u);
    check_size("descriptor value count", signal->value_count, 9u);
    check_u16("first descriptor value", signal->first_value, 0x0020u);
    check_u16("last descriptor value", signal->last_value, 0x2020u);
    check_u16("descriptor stride", signal->stride, 0x0400u);
    check_size("post descriptor zero offset", signal->post_descriptor_zero_offset, 0x1596u);
    check_size("post descriptor zero bytes", signal->post_descriptor_zero_bytes, 0x1a6au);
    check_size("next nonzero offset", signal->next_nonzero_offset, 0x3000u);
    check_size("boundary prefix size", signal->boundary_prefix_size, 16u);
    check_size("boundary prefix occurrence count",
               signal->boundary_prefix_occurrence_count,
               1u);
    check_size("post-boundary span size", signal->post_boundary_span_size, 44u);
    check_size("post-boundary span offsets[0]",
               signal->post_boundary_span_offsets[0],
               0x3000u);
    check_size("post-boundary span occurrence count",
               signal->post_boundary_span_occurrence_count,
               1u);
    check_u16("post-boundary span first word",
              signal->post_boundary_span_first_word,
              0x80beu);
    check_u16("post-boundary span last word",
              signal->post_boundary_span_last_word,
              0x3f00u);
}

static void check_raw_signal(const char *label,
                             const Theron_Track02BankSignal *signal,
                             Theron_Track02Variant variant,
                             const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                             const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                             size_t first_descriptor_sector,
                             size_t first_span_sector) {
    size_t i;

    check_int("raw variant", signal->variant, variant);
    check_size("raw anchor count",
               signal->anchor_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw descriptor size", signal->descriptor_size, 18u);
    check_size("raw descriptor occurrence count",
               signal->occurrence_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw descriptor value count", signal->value_count, 9u);
    check_u16("raw first descriptor value", signal->first_value, 0x0020u);
    check_u16("raw last descriptor value", signal->last_value, 0x2020u);
    check_u16("raw descriptor stride", signal->stride, 0x0400u);
    check_size("raw post-boundary span occurrence count",
               signal->post_boundary_span_occurrence_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw boundary prefix occurrence count",
               signal->boundary_prefix_occurrence_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw sector bytes", signal->raw_sector_bytes, 2352u);
    check_size("raw sector user-data offset",
               signal->raw_sector_user_data_offset,
               0x10u);

    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char name[96];
        snprintf(name, sizeof(name), "%s descriptor offset[%zu]", label, i);
        check_size(name, signal->descriptor_offsets[i], descriptor_offsets[i]);
        snprintf(name, sizeof(name), "%s span offset[%zu]", label, i);
        check_size(name, signal->post_boundary_span_offsets[i], span_offsets[i]);
    }

    check_size("raw descriptor sector[0]",
               signal->descriptor_raw_sector_numbers[0],
               first_descriptor_sector);
    check_size("raw descriptor sector[1]",
               signal->descriptor_raw_sector_numbers[1],
               first_descriptor_sector + 4u);
    check_size("raw descriptor sector[2]",
               signal->descriptor_raw_sector_numbers[2],
               first_descriptor_sector + 8u);
    check_size("raw descriptor user offset[0]",
               signal->descriptor_raw_sector_user_offsets[0],
               0x406u);
    check_size("raw descriptor user offset[1]",
               signal->descriptor_raw_sector_user_offsets[1],
               0x406u);
    check_size("raw descriptor user offset[2]",
               signal->descriptor_raw_sector_user_offsets[2],
               0x584u);
    check_size("raw span sector[0]",
               signal->post_boundary_span_raw_sector_numbers[0],
               first_span_sector);
    check_size("raw span sector[1]",
               signal->post_boundary_span_raw_sector_numbers[1],
               first_span_sector + 738u);
    check_size("raw span sector[2]",
               signal->post_boundary_span_raw_sector_numbers[2],
               first_descriptor_sector + 12u);
    check_size("raw span user offset[0]",
               signal->post_boundary_span_raw_sector_user_offsets[0],
               0u);
    check_size("raw span user offset[1]",
               signal->post_boundary_span_raw_sector_user_offsets[1],
               0u);
    check_size("raw span user offset[2]",
               signal->post_boundary_span_raw_sector_user_offsets[2],
               0u);
}

static void probe_track(const char *label,
                        const char *env_name,
                        const char *default_file,
                        const char *expected_md5,
                        Theron_Track02SignalStatus expected_status) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char md5_hex[33] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (env_path && env_path[0]) {
        path_to_read = env_path;
    } else {
        default_data_path(default_file, path);
        path_to_read = path;
    }

    if (!file_exists(path_to_read)) {
        printf("SKIP %s: no Track 02 image at %s\n", label, path_to_read);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path_to_read, md5_hex)) {
        printf("FAIL %s: could not compute MD5 for %s\n", label, path_to_read);
        ++g_fail;
        return;
    }
    if (strcmp(md5_hex, expected_md5) != 0) {
        printf("FAIL %s: MD5 %s does not match expected %s\n",
               label, md5_hex, expected_md5);
        ++g_fail;
        return;
    }
    if (!read_file(path_to_read, &data, &size)) {
        printf("FAIL %s: could not read %s\n", label, path_to_read);
        ++g_fail;
        return;
    }

    status = theron_v1_track02_find_bank_signal(data, size, md5_hex, &signal);
    printf("%s: md5=%s size=%zu status=%s variant=%s\n",
           label, md5_hex, size,
           theron_v1_track02_signal_status_name(status),
           theron_v1_track02_variant_name(signal.variant));

    check_int("track status", status, expected_status);
    if (expected_status == THERON_TRACK02_SIGNAL_OK) {
        printf("%s: anchors=%zu descriptor=0x%zx bytes=%zu boundary=0x%zx zero_bytes=%zu span_bytes=%zu span_occurrences=%zu\n",
               label,
               signal.anchor_count,
               signal.descriptor_offset,
               signal.descriptor_size,
               signal.next_nonzero_offset,
               signal.post_descriptor_zero_bytes,
               signal.post_boundary_span_size,
               signal.post_boundary_span_occurrence_count);
        if (signal.variant == THERON_TRACK02_VARIANT_US_ISO) {
            check_iso_signal(&signal);
        } else if (signal.variant == THERON_TRACK02_VARIANT_US_BIN) {
            check_raw_signal("US raw",
                             &signal,
                             THERON_TRACK02_VARIANT_US_BIN,
                             g_us_bin_descriptor_offsets,
                             g_us_bin_span_offsets,
                             3141u,
                             1263u);
        } else if (signal.variant == THERON_TRACK02_VARIANT_JP_BIN) {
            check_raw_signal("JP raw",
                             &signal,
                             THERON_TRACK02_VARIANT_JP_BIN,
                             g_jp_bin_descriptor_offsets,
                             g_jp_bin_span_offsets,
                             3140u,
                             1262u);
        } else {
            printf("FAIL %s: unexpected OK variant %s\n",
                   label,
                   theron_v1_track02_variant_name(signal.variant));
            ++g_fail;
        }
    } else if (expected_status == THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE) {
        check_int("zero-image variant", signal.variant, THERON_TRACK02_VARIANT_JP_REV1_ISO);
        check_no_reported_offset("zero-image leaves no offset", &signal);
    }

    free(data);
}

static void probe_negative_fixture(void) {
    uint8_t zeros[64] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status =
        theron_v1_track02_find_bank_signal(zeros,
                                           sizeof(zeros),
                                           "00000000000000000000000000000000",
                                           &signal);
    check_int("unsupported fixture stays unsupported",
              status,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
}

static void probe_jp_zero_image_fixture(void) {
    uint8_t zeros[4096] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status =
        theron_v1_track02_find_bank_signal(zeros,
                                           sizeof(zeros),
                                           THERON_TRACK02_MD5_JP_REV1_ISO,
                                           &signal);
    check_int("JP Rev 1 zero fixture is insufficient evidence",
              status,
              THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);
    check_int("JP Rev 1 zero fixture is hash-gated",
              signal.variant,
              THERON_TRACK02_VARIANT_JP_REV1_ISO);
    check_no_reported_offset("JP Rev 1 zero fixture leaves no offset", &signal);
}

static void probe_descriptor_only_negative_fixture(void) {
    uint8_t *fixture = (uint8_t *)calloc(1u, 0x3010u);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL descriptor-only fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    memcpy(fixture + 0x1584u, g_descriptor, sizeof(g_descriptor));
    status = theron_v1_track02_find_bank_signal(fixture,
                                                0x3010u,
                                                THERON_TRACK02_MD5_US_ISO,
                                                &signal);
    check_int("descriptor-only fixture needs boundary prefix",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(fixture);
}

static void probe_boundary_prefix_only_negative_fixture(void) {
    static const uint8_t prefix[16] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80
    };
    uint8_t *fixture = (uint8_t *)calloc(1u, 0x302cu);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL boundary-prefix-only fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    memcpy(fixture + 0x1584u, g_descriptor, sizeof(g_descriptor));
    memcpy(fixture + 0x3000u, prefix, sizeof(prefix));
    status = theron_v1_track02_find_bank_signal(fixture,
                                                0x302cu,
                                                THERON_TRACK02_MD5_US_ISO,
                                                &signal);
    check_int("boundary prefix alone is not a post-boundary span",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(fixture);
}

static void fill_raw_anchor_fixture(uint8_t *fixture,
                                    const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                    const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS]) {
    size_t i;
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        memcpy(fixture + descriptor_offsets[i], g_descriptor, sizeof(g_descriptor));
        memcpy(fixture + span_offsets[i],
               g_post_boundary_span,
               sizeof(g_post_boundary_span));
    }
}

static void probe_raw_bin_positive_fixture(const char *label,
                                           const char *md5_hex,
                                           Theron_Track02Variant variant,
                                           const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                           const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                           size_t first_descriptor_sector,
                                           size_t first_span_sector) {
    const size_t fixture_size = span_offsets[2] + sizeof(g_post_boundary_span) + 1u;
    uint8_t *fixture = (uint8_t *)calloc(1u, fixture_size);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL %s: allocation failed\n", label);
        ++g_fail;
        return;
    }

    fill_raw_anchor_fixture(fixture, descriptor_offsets, span_offsets);
    status = theron_v1_track02_find_bank_signal(fixture,
                                                fixture_size,
                                                md5_hex,
                                                &signal);
    check_int(label, status, THERON_TRACK02_SIGNAL_OK);
    check_raw_signal(label,
                     &signal,
                     variant,
                     descriptor_offsets,
                     span_offsets,
                     first_descriptor_sector,
                     first_span_sector);
    free(fixture);
}

static void probe_raw_bin_missing_anchor_fixture(void) {
    const size_t fixture_size = g_us_bin_span_offsets[2] + sizeof(g_post_boundary_span) + 1u;
    uint8_t *fixture = (uint8_t *)calloc(1u, fixture_size);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL raw missing-anchor fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    fill_raw_anchor_fixture(fixture, g_us_bin_descriptor_offsets, g_us_bin_span_offsets);
    fixture[g_us_bin_descriptor_offsets[1]] ^= 0x01u;
    status = theron_v1_track02_find_bank_signal(fixture,
                                                fixture_size,
                                                THERON_TRACK02_MD5_US_BIN,
                                                &signal);
    check_int("raw BIN requires all three descriptor anchors",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(fixture);
}

int main(void) {
    printf("=== Theron V1 Track 02 Bank Evidence Probe ===\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_negative_fixture();
    probe_jp_zero_image_fixture();
    probe_descriptor_only_negative_fixture();
    probe_boundary_prefix_only_negative_fixture();
    probe_raw_bin_positive_fixture("US raw BIN synthetic anchors",
                                   THERON_TRACK02_MD5_US_BIN,
                                   THERON_TRACK02_VARIANT_US_BIN,
                                   g_us_bin_descriptor_offsets,
                                   g_us_bin_span_offsets,
                                   3141u,
                                   1263u);
    probe_raw_bin_positive_fixture("JP raw BIN synthetic anchors",
                                   THERON_TRACK02_MD5_JP_BIN,
                                   THERON_TRACK02_VARIANT_JP_BIN,
                                   g_jp_bin_descriptor_offsets,
                                   g_jp_bin_span_offsets,
                                   3140u,
                                   1262u);
    probe_raw_bin_missing_anchor_fixture();
    probe_track("US ISO bank descriptor",
                "FIRESTAFF_THERON_TRACK02_US",
                "theron/TQUS02End.iso",
                THERON_TRACK02_MD5_US_ISO,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("US raw Track 02 bank anchors",
                "FIRESTAFF_THERON_TRACK02_US_BIN",
                "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                THERON_TRACK02_MD5_US_BIN,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("JP raw Track 02 bank anchors",
                "FIRESTAFF_THERON_TRACK02_JP_BIN",
                "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                THERON_TRACK02_MD5_JP_BIN,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("JP Rev 1 ISO zero-image guard",
                "FIRESTAFF_THERON_TRACK02_JP_REV1",
                "theron/TQJP02End.iso",
                THERON_TRACK02_MD5_JP_REV1_ISO,
                THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
