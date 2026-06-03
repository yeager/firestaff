/*
 * firestaff_theron_v1_track02_bank_probe.c
 *
 * Theron's Quest V1 -- Track 02 bank evidence probe.
 *
 * This probe does not prove a dungeon map grid or promote runtime loading.
 * It only regression-locks one hash-gated US Track 02 bank-stride descriptor
 * and keeps the JP Rev 1 zero-filled ISO outcome explicit.
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

static void default_path(const char *file_name, char out_path[512]) {
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = ".";
    snprintf(out_path, 512, "%s%s.firestaff%sdata%stheron%s%s",
             home, PATH_SEP, PATH_SEP, PATH_SEP, PATH_SEP, file_name);
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
        default_path(default_file, path);
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
        printf("%s: descriptor=0x%zx bytes=%zu boundary=0x%zx zero_bytes=%zu span_bytes=%zu span_occurrences=%zu\n",
               label,
               signal.descriptor_offset,
               signal.descriptor_size,
               signal.next_nonzero_offset,
               signal.post_descriptor_zero_bytes,
               signal.post_boundary_span_size,
               signal.post_boundary_span_occurrence_count);
        check_int("variant", signal.variant, THERON_TRACK02_VARIANT_US_ISO);
        check_size("descriptor offset", signal.descriptor_offset, 0x1584u);
        check_size("descriptor size", signal.descriptor_size, 18u);
        check_size("descriptor occurrence count", signal.occurrence_count, 1u);
        check_size("descriptor value count", signal.value_count, 9u);
        check_u16("first descriptor value", signal.first_value, 0x0020u);
        check_u16("last descriptor value", signal.last_value, 0x2020u);
        check_u16("descriptor stride", signal.stride, 0x0400u);
        check_size("post descriptor zero offset", signal.post_descriptor_zero_offset, 0x1596u);
        check_size("post descriptor zero bytes", signal.post_descriptor_zero_bytes, 0x1a6au);
        check_size("next nonzero offset", signal.next_nonzero_offset, 0x3000u);
        check_size("boundary prefix size", signal.boundary_prefix_size, 16u);
        check_size("boundary prefix occurrence count",
                   signal.boundary_prefix_occurrence_count,
                   1u);
        check_size("post-boundary span size", signal.post_boundary_span_size, 44u);
        check_size("post-boundary span occurrence count",
                   signal.post_boundary_span_occurrence_count,
                   1u);
        check_u16("post-boundary span first word",
                  signal.post_boundary_span_first_word,
                  0x80beu);
        check_u16("post-boundary span last word",
                  signal.post_boundary_span_last_word,
                  0x3f00u);
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
    static const uint8_t descriptor[18] = {
        0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
        0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
    };
    uint8_t *fixture = (uint8_t *)calloc(1u, 0x3010u);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL descriptor-only fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    memcpy(fixture + 0x1584u, descriptor, sizeof(descriptor));
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
    static const uint8_t descriptor[18] = {
        0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
        0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
    };
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

    memcpy(fixture + 0x1584u, descriptor, sizeof(descriptor));
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

int main(void) {
    printf("=== Theron V1 Track 02 Bank Evidence Probe ===\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_negative_fixture();
    probe_jp_zero_image_fixture();
    probe_descriptor_only_negative_fixture();
    probe_boundary_prefix_only_negative_fixture();
    probe_track("US ISO bank descriptor",
                "FIRESTAFF_THERON_TRACK02_US",
                "TQUS02End.iso",
                THERON_TRACK02_MD5_US_ISO,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("JP Rev 1 ISO zero-image guard",
                "FIRESTAFF_THERON_TRACK02_JP_REV1",
                "TQJP02End.iso",
                THERON_TRACK02_MD5_JP_REV1_ISO,
                THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
