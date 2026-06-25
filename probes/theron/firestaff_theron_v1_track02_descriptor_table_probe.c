/*
 * firestaff_theron_v1_track02_descriptor_table_probe.c
 *
 * Theron's Quest V1 -- narrow Track 02 semantic dungeon-descriptor table
 * decoder probe.
 *
 * Scope:
 *   This probe locks ONE narrow slice of semantic Track 02 decoding:
 *   reading the documented 9-word little-endian stride table that the
 *   bank-signal module has already located.  It does NOT claim a per-
 *   dungeon level binding, a per-entry semantic type, a runtime loader
 *   handoff, or any data-source interpretation.  Its job is to regression-
 *   lock the byte-shape contract so future semantic work can build on it.
 *
 * Source/evidence:
 *   src/theron/theron_v1_track02.c (g_us_iso_bank_stride_descriptor,
 *   TQR_US_ISO_BANK_STRIDE_OFFSET=0x1584, TQR_US_ISO_BANK_STRIDE_COUNT=9,
 *   TQR_US_ISO_BANK_STRIDE_STEP=0x0400).
 *   docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md
 *   (the 0x1584 descriptor and the three JP/US raw BIN anchors).
 *
 * What the probe covers:
 *   1. 9-word canonical table -> OK with all 9 entries populated.
 *   2. Negative: not-strictly-ascending, wrong stride, truncated input,
 *      NULL input, wrong entry count (handled by NOT_FOUND on shape).
 *   3. range_inclusive flag matches the documented 0x0020..0x2020+0x0400.
 *   4. exclusive_upper_bound == last_value + stride.
 *   5. Hash-gated real-data round-trip from the bank-signal module:
 *      - US Track 02 ISO descriptor bytes at 0x1584 (when present).
 *      - All three US raw Track 02 BIN descriptor anchors (when present).
 *      - All three JP raw Track 02 BIN descriptor anchors (when present).
 *      - JP Rev 1 ISO is a zero-filled image; bank-signal returns
 *        INSUFFICIENT_ZERO_IMAGE, so the descriptor-table round-trip is
 *        not asserted.
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

#define DESCRIPTOR_BYTE_COUNT (THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES * 2u)

static int g_fail = 0;
static int g_skip = 0;

/* Canonical documented descriptor bytes.  Mirrors g_us_iso_bank_stride_descriptor
 * in src/theron/theron_v1_track02.c. */
static const uint8_t g_canonical_descriptor[DESCRIPTOR_BYTE_COUNT] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

/* Documented stride: 0x0400 (1024 bytes between adjacent entries). */
#define DOCUMENTED_STRIDE 0x0400u

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

static void default_data_path(const char *relative_name, char out_path[512]) {
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = ".";
    snprintf(out_path, 512, "%s%s.firestaff%sdata%s%s",
             home, PATH_SEP, PATH_SEP, PATH_SEP, relative_name);
}

/* Round-trip decoder on the canonical synthetic descriptor and verify
 * every shape field. */
static void probe_canonical_synthetic_descriptor(void) {
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status =
        theron_v1_track02_decode_descriptor_table(
            g_canonical_descriptor,
            sizeof(g_canonical_descriptor),
            DOCUMENTED_STRIDE,
            &table);

    printf("canonical descriptor: status=%s count=%zu first=0x%04x last=0x%04x stride=0x%04x upper=0x%04x range_inclusive=%d\n",
           theron_v1_track02_table_decode_status_name(status),
           table.entry_count,
           (unsigned)table.first_value,
           (unsigned)table.last_value,
           (unsigned)table.stride,
           (unsigned)table.exclusive_upper_bound,
           table.range_inclusive);

    check_int("canonical status", status, THERON_TRACK02_TABLE_DECODE_OK);
    check_size("canonical entry_count",
               table.entry_count,
               THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    check_u16("canonical first_value", table.first_value, 0x0020u);
    check_u16("canonical last_value", table.last_value, 0x2020u);
    check_u16("canonical stride", table.stride, DOCUMENTED_STRIDE);
    check_u16("canonical exclusive_upper_bound",
              table.exclusive_upper_bound,
              0x2420u);
    check_int("canonical range_inclusive", table.range_inclusive, 1);

    /* Spot-check entries [0], [4], [8]. */
    check_u16("canonical entries[0]", table.entries[0], 0x0020u);
    check_u16("canonical entries[4]", table.entries[4], 0x1020u);
    check_u16("canonical entries[8]", table.entries[8], 0x2020u);
}

/* Truncated input must be rejected as BAD_INPUT. */
static void probe_truncated_input_negative_fixture(void) {
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status =
        theron_v1_track02_decode_descriptor_table(
            g_canonical_descriptor,
            DESCRIPTOR_BYTE_COUNT - 2u, /* missing last entry */
            DOCUMENTED_STRIDE,
            &table);

    check_int("truncated input is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);
}

/* NULL descriptor must be rejected. */
static void probe_null_descriptor_negative_fixture(void) {
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status =
        theron_v1_track02_decode_descriptor_table(
            NULL,
            DESCRIPTOR_BYTE_COUNT,
            DOCUMENTED_STRIDE,
            &table);

    check_int("NULL descriptor is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);
}

/* Zero expected stride is a configuration error. */
static void probe_zero_stride_negative_fixture(void) {
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status =
        theron_v1_track02_decode_descriptor_table(
            g_canonical_descriptor,
            sizeof(g_canonical_descriptor),
            0u,
            &table);

    check_int("zero expected stride is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);
}

/* Descending entries (reversed) must be rejected as not-strictly-ascending. */
static void probe_descending_entries_negative_fixture(void) {
    uint8_t reversed[DESCRIPTOR_BYTE_COUNT];
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status;
    size_t i;

    /* Reverse the byte order of the entries (so the LE words are reversed
     * but still strictly descending). */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        const size_t src =
            (THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES - 1u - i) * 2u;
        reversed[i * 2u + 0u] = g_canonical_descriptor[src + 0u];
        reversed[i * 2u + 1u] = g_canonical_descriptor[src + 1u];
    }
    status = theron_v1_track02_decode_descriptor_table(
        reversed,
        sizeof(reversed),
        DOCUMENTED_STRIDE,
        &table);

    check_int("descending entries rejected",
              status,
              THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING);
}

/* Equal-adjacent entries (no strict monotonicity) must be rejected. */
static void probe_non_strict_ascending_negative_fixture(void) {
    uint8_t duplicate[DESCRIPTOR_BYTE_COUNT];
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status;

    /* All 9 entries == 0x0020 (duplicate). */
    memset(duplicate, 0, sizeof(duplicate));
    duplicate[0] = 0x20u; duplicate[1] = 0x00u;
    /* Subsequent 8 entries are also 0x0020, so duplicates, not strictly
     * ascending. */
    status = theron_v1_track02_decode_descriptor_table(
        duplicate,
        sizeof(duplicate),
        DOCUMENTED_STRIDE,
        &table);

    check_int("non-strict-ascending rejected",
              status,
              THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING);
}

/* Wrong stride between adjacent entries (e.g. 0x0800 instead of 0x0400) must
 * be rejected as WRONG_STRIDE. */
static void probe_wrong_stride_negative_fixture(void) {
    uint8_t double_stride[DESCRIPTOR_BYTE_COUNT];
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status;
    size_t i;
    uint16_t v = 0x0020u;

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        double_stride[i * 2u + 0u] = (uint8_t)(v & 0xFFu);
        double_stride[i * 2u + 1u] = (uint8_t)((v >> 8) & 0xFFu);
        v = (uint16_t)(v + 0x0800u); /* doubled stride; same last value 0x4020 */
    }
    status = theron_v1_track02_decode_descriptor_table(
        double_stride,
        sizeof(double_stride),
        DOCUMENTED_STRIDE,
        &table);

    check_int("wrong stride rejected",
              status,
              THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE);
}

/* Acceptable ascending 9-word stride table that uses a different stride
 * (0x0200 instead of 0x0400) and a different range.  Verifies the decoder
 * is shape-driven, not magic-number-driven. */
static void probe_alternative_stride_positive_fixture(void) {
    uint8_t alt_stride[DESCRIPTOR_BYTE_COUNT];
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status;
    size_t i;
    uint16_t v = 0x0100u;
    const uint16_t alt = 0x0200u;

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        alt_stride[i * 2u + 0u] = (uint8_t)(v & 0xFFu);
        alt_stride[i * 2u + 1u] = (uint8_t)((v >> 8) & 0xFFu);
        v = (uint16_t)(v + alt);
    }
    status = theron_v1_track02_decode_descriptor_table(
        alt_stride,
        sizeof(alt_stride),
        alt,
        &table);

    check_int("alt stride positive", status, THERON_TRACK02_TABLE_DECODE_OK);
    check_u16("alt stride first_value", table.first_value, 0x0100u);
    check_u16("alt stride last_value", table.last_value, 0x1100u);
    check_u16("alt stride exclusive_upper_bound",
              table.exclusive_upper_bound,
              0x1300u);
    /* The alt range [0x0100, 0x1300) fits inside the documented
     * inclusive window [0x0020, 0x2420], so range_inclusive stays 1.
     * This proves the range check is value-driven, not magic-number-driven. */
    check_int("alt stride range_inclusive", table.range_inclusive, 1);
}

/* Strictly ascending 9-word stride table that lands *outside* the
 * documented [0x0020, 0x2420] inclusive window.  Verifies that
 * range_inclusive flips to 0 even when the shape (count, strict-
 * ascending, stride, exclusive_upper > last) is valid. */
static void probe_out_of_range_positive_fixture(void) {
    uint8_t oor[DESCRIPTOR_BYTE_COUNT];
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status;
    size_t i;
    uint16_t v = 0x8000u;
    const uint16_t alt = 0x0200u;

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        oor[i * 2u + 0u] = (uint8_t)(v & 0xFFu);
        oor[i * 2u + 1u] = (uint8_t)((v >> 8) & 0xFFu);
        v = (uint16_t)(v + alt);
    }
    status = theron_v1_track02_decode_descriptor_table(
        oor,
        sizeof(oor),
        alt,
        &table);

    check_int("out-of-range shape OK", status, THERON_TRACK02_TABLE_DECODE_OK);
    check_int("out-of-range range_inclusive flips to 0",
              table.range_inclusive,
              0);
}

/* All descriptor offsets documented for the bank-signal module. */
static const size_t g_us_iso_descriptor_offset = 0x1584u;
static const size_t g_us_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};
static const size_t g_jp_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static void check_descriptor_table_from_data(const uint8_t *data,
                                             size_t size,
                                             size_t offset,
                                             const char *label) {
    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus status;
    size_t i;

    if (offset > size || DESCRIPTOR_BYTE_COUNT > size - offset) {
        printf("FAIL %s: offset 0x%zx exceeds data size %zu\n",
               label, offset, size);
        ++g_fail;
        return;
    }
    status = theron_v1_track02_decode_descriptor_table(
        data + offset,
        DESCRIPTOR_BYTE_COUNT,
        DOCUMENTED_STRIDE,
        &table);

    printf("%s: status=%s count=%zu first=0x%04x last=0x%04x upper=0x%04x\n",
           label,
           theron_v1_track02_table_decode_status_name(status),
           table.entry_count,
           (unsigned)table.first_value,
           (unsigned)table.last_value,
           (unsigned)table.exclusive_upper_bound);

    check_int("status", status, THERON_TRACK02_TABLE_DECODE_OK);
    check_size("entry_count",
               table.entry_count,
               THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    check_u16("first_value", table.first_value, 0x0020u);
    check_u16("last_value", table.last_value, 0x2020u);
    check_u16("stride", table.stride, DOCUMENTED_STRIDE);
    check_u16("exclusive_upper_bound", table.exclusive_upper_bound, 0x2420u);
    check_int("range_inclusive", table.range_inclusive, 1);

    /* All 9 entries must round-trip back to their canonical values. */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        const uint16_t want = (uint16_t)(0x0020u + (uint16_t)(i * DOCUMENTED_STRIDE));
        char name[64];
        snprintf(name, sizeof(name), "%s entries[%zu]", label, i);
        check_u16(name, table.entries[i], want);
    }
}

static void probe_real_data_round_trip(const char *label,
                                       const char *md5_hex,
                                       const char *env_name,
                                       const char *default_file,
                                       size_t descriptor_offset) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char local_md5[33] = {0};

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
    if (!m12_file_md5_hex(path_to_read, local_md5)) {
        printf("FAIL %s: could not compute MD5 for %s\n", label, path_to_read);
        ++g_fail;
        return;
    }
    if (strcmp(local_md5, md5_hex) != 0) {
        printf("FAIL %s: MD5 %s does not match expected %s\n",
               label, local_md5, md5_hex);
        ++g_fail;
        return;
    }
    if (!read_file(path_to_read, &data, &size)) {
        printf("FAIL %s: could not read %s\n", label, path_to_read);
        ++g_fail;
        return;
    }

    {
        Theron_Track02BankSignal signal;
        Theron_Track02SignalStatus bank_status =
            theron_v1_track02_find_bank_signal(data, size, local_md5, &signal);
        check_int("bank-signal status",
                  bank_status,
                  THERON_TRACK02_SIGNAL_OK);
        check_int("bank-signal variant",
                  signal.variant,
                  theron_v1_track02_variant_for_md5(local_md5));
        check_size("bank-signal descriptor size",
                   signal.descriptor_size,
                   DESCRIPTOR_BYTE_COUNT);
        /* The descriptor_offset passed to the table decoder is the
         * per-anchor raw-sector offset when this is a raw BIN; for the
         * US ISO it is the unique 0x1584 offset.  We always decode from
         * the offset the caller provided, which we have documented
         * above. */
        check_descriptor_table_from_data(data, size, descriptor_offset, label);
    }

    free(data);
}

static void probe_real_data_all_anchors(void) {
    size_t i;

    /* US Track 02 ISO descriptor bytes at the unique 0x1584 offset. */
    probe_real_data_round_trip("US ISO descriptor table at 0x1584",
                               THERON_TRACK02_MD5_US_ISO,
                               "FIRESTAFF_THERON_TRACK02_US",
                               "theron/TQUS02End.iso",
                               g_us_iso_descriptor_offset);

    /* All three US raw BIN anchors. */
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char label[96];
        snprintf(label, sizeof(label),
                 "US raw BIN descriptor table anchor %zu", i);
        probe_real_data_round_trip(label,
                                   THERON_TRACK02_MD5_US_BIN,
                                   "FIRESTAFF_THERON_TRACK02_US_BIN",
                                   "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                                   g_us_bin_descriptor_offsets[i]);
    }

    /* All three JP raw BIN anchors. */
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char label[96];
        snprintf(label, sizeof(label),
                 "JP raw BIN descriptor table anchor %zu", i);
        probe_real_data_round_trip(label,
                                   THERON_TRACK02_MD5_JP_BIN,
                                   "FIRESTAFF_THERON_TRACK02_JP_BIN",
                                   "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                                   g_jp_bin_descriptor_offsets[i]);
    }
}

int main(void) {
    printf("=== Theron V1 Track 02 Descriptor Table Decoder Probe ===\n");
    printf("decoder contract: %u entries, LE uint16, stride 0x%04x, range [0x0020, 0x2420)\n",
           (unsigned)THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES,
           DOCUMENTED_STRIDE);
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_canonical_synthetic_descriptor();
    probe_truncated_input_negative_fixture();
    probe_null_descriptor_negative_fixture();
    probe_zero_stride_negative_fixture();
    probe_descending_entries_negative_fixture();
    probe_non_strict_ascending_negative_fixture();
    probe_wrong_stride_negative_fixture();
    probe_alternative_stride_positive_fixture();
    probe_out_of_range_positive_fixture();
    probe_real_data_all_anchors();

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
