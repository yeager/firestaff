/*
 * firestaff_theron_v1_track02_descriptor_entry_semantic_probe.c
 *
 * Theron's Quest V1 -- bounded Track 02 descriptor-entry semantic binding
 * probe.
 *
 * Scope:
 *   Binds one descriptor-table entry to one semantic role.  The probe
 *   regression-locks the audio-bank-pointer role on raw Track 02 BIN
 *   descriptor entries (the 12-byte `00 ff*10 00` sentinel followed by a
 *   4-byte LE audio-bank id word is the same fingerprint already proven
 *   at the per-anchor post-boundary spans in the bank-signal module),
 *   the descriptor-table role on entry 5, the zero-fill role on
 *   all-zero entries, and the structured-data role on entries that
 *   carry small nonzero tails without an audio-bank marker.
 *
 * Honest scope: this is a single-entry semantic role classification based
 * on a byte-level fingerprint that the bank-signal module has already
 * proven at the per-anchor post-boundary spans.  It does NOT claim that
 * the marker is decoded into an actual audio bank, that the descriptor
 * entries map to specific dungeon/object/text/palette records, or that
 * any runtime handoff reaches the dungeon loader.
 *
 * Source/evidence:
 *   src/theron/theron_v1_track02.c
 *     - g_audio_bank_prefix (12-byte `00 ff*10 00` sentinel)
 *     - TQR_US_ISO_BANK_STRIDE_OFFSET=0x1584
 *     - TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES, TQR_RAW_BIN_AUDIO_BANK_ID_BYTES
 *     - theron_v1_track02_find_audio_bank_marker (post-boundary span
 *       fingerprint, already CTest-gated via theron_v1_track02_bank)
 *   docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md
 *     - US/JP raw BIN anchor coordinates
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §10.2
 *     - ADPCM audio data block location (STUB; this fingerprint is one
 *       ADPCM-bank anchor candidate).
 *
 * What the probe covers:
 *   1. Synthetic positive: a single descriptor entry that contains the
 *      sentinel + a non-zero 4-byte LE id word is classified
 *      AUDIO_BANK_POINTER and reports the marker offset + id.
 *   2. Synthetic positive: an entry whose 0x0400 bytes are all zero is
 *      classified ZERO_FILL.
 *   3. Synthetic positive: an entry whose 0x0400 bytes contain a
 *      small nonzero tail (38 bytes, 8 unique nonzero values,
 *      mirroring the on-disk entry-1 tail) without an audio-bank
 *      marker is classified STRUCTURED_DATA.
 *   4. Synthetic positive: entry 5 of a synthetic descriptor table
 *      (containing the 18-byte descriptor bytes) is classified
 *      DESCRIPTOR_TABLE.
 *   5. Negative fixtures: out-of-range entry index, NULL data, NULL
 *      out, truncated descriptor table, malformed descriptor bytes
 *      (not strictly ascending).  All return the documented failure
 *      status.
 *   6. Hash-gated real-data classification for every descriptor entry
 *      of the US Track 02 ISO (partial extract) and the US/JP raw
 *      Track 02 BIN anchors.  The probe asserts the binary invariant
 *      that raw BIN descriptor regions carry at least one
 *      AUDIO_BANK_POINTER entry and exactly one DESCRIPTOR_TABLE
 *      entry; the US ISO partial extract carries zero
 *      AUDIO_BANK_POINTER entries plus at least one STRUCTURED_DATA
 *      and at least one ZERO_FILL.  Per-entry role assertions are not
 *      made because real Track 02 entry content is anchor-shuffled
 *      (anchor 0 vs anchor 2 place the audio-bank marker at different
 *      descriptor entry indices).
 *
 * If Track 02 images are absent the probe skips real-data assertions
 * (matching the existing descriptor-table / bank / level-handoff
 * probes' skip-when-absent pattern).
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
#define DESCRIPTOR_WINDOW_SIZE 0x0400u

static int g_fail = 0;
static int g_skip = 0;

/* Canonical 9-word little-endian stride table mirroring
 * g_us_iso_bank_stride_descriptor in src/theron/theron_v1_track02.c. */
static const uint8_t g_canonical_descriptor[DESCRIPTOR_BYTE_COUNT] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

/* Audio-bank marker sentinel: 0x00, 0xff*10, 0x00 (12 bytes).  Mirrors
 * g_audio_bank_prefix in src/theron/theron_v1_track02.c. */
static const uint8_t g_audio_bank_prefix[12] = {
    0x00,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0x00
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

static void check_u32(const char *label, uint32_t got, uint32_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%08x want 0x%08x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_fail;
    }
}

static void check_role(const char *label,
                       Theron_Track02DescriptorTableRole got,
                       Theron_Track02DescriptorTableRole want) {
    if (got != want) {
        printf("FAIL %s: got %s want %s\n",
               label,
               theron_v1_track02_descriptor_table_role_name(got),
               theron_v1_track02_descriptor_table_role_name(want));
        ++g_fail;
    }
}

static void write_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
    p[2] = (uint8_t)((v >> 16) & 0xffu);
    p[3] = (uint8_t)((v >> 24) & 0xffu);
}

/* Synthetic fixture: a 0x3000-byte track with the descriptor table at
 * 0x1584, entry 2 (relative offset 0x0820) carrying the audio-bank
 * sentinel + a non-zero id word.  Asserts entry 2 classifies as
 * AUDIO_BANK_POINTER with the expected id, and that all other entries
 * fall into the documented taxonomy for the fixture. */
static void probe_synthetic_audio_bank_pointer(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorEntrySemantic semantic;
    Theron_Track02TableDecodeStatus status;
    static const uint32_t want_audio_id = 0x01002401u;

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));

    /* Place the audio-bank marker at the start of entry 2's window
     * (descriptor-relative offset 0x0820, absolute 0x0820 in this
     * synthetic fixture because base = descriptor_offset - 0x1584 = 0). */
    memcpy(track + 0x0820u, g_audio_bank_prefix, sizeof(g_audio_bank_prefix));
    write_le32(track + 0x0820u + 12u, want_audio_id);

    /* Entry 2: 12-byte sentinel + 4-byte id -> AUDIO_BANK_POINTER. */
    status = theron_v1_track02_classify_descriptor_entry(
        track,
        sizeof(track),
        0x1584u,
        2u,
        &semantic);
    printf("synthetic entry 2: status=%s role=%s marker_offset=0x%zx marker_recognized=%d audio_id=0x%08x\n",
           theron_v1_track02_table_decode_status_name(status),
           theron_v1_track02_descriptor_table_role_name(semantic.role),
           semantic.audio_bank_marker_offset,
           semantic.audio_bank_marker_recognized,
           (unsigned)semantic.audio_bank_id);

    check_int("synthetic entry 2 status",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);
    check_role("synthetic entry 2 role",
               semantic.role,
               THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_AUDIO_BANK_POINTER);
    check_int("synthetic entry 2 marker recognized",
              semantic.audio_bank_marker_recognized,
              1);
    check_size("synthetic entry 2 marker offset",
               semantic.audio_bank_marker_offset,
               0x0820u);
    check_u32("synthetic entry 2 audio id",
              semantic.audio_bank_id,
              want_audio_id);

    /* Entry 5 must still be DESCRIPTOR_TABLE (no overlap with our marker
     * since we placed the marker only inside entry 2). */
    memset(&semantic, 0, sizeof(semantic));
    status = theron_v1_track02_classify_descriptor_entry(
        track, sizeof(track), 0x1584u, 5u, &semantic);
    check_int("synthetic entry 5 status",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);
    check_role("synthetic entry 5 role",
               semantic.role,
               THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_DESCRIPTOR_TABLE);
}

/* Synthetic fixture: a 38-byte nonzero tail inside entry 1 (no marker)
 * with 8 unique nonzero byte values (the rest of the tail is zero),
 * mirroring the observed on-disk shape of entry 1 in the raw BIN
 * anchors (38 bytes nonzero, mostly zero, no audio-bank marker).
 * Asserts STRUCTURED_DATA classification. */
static void probe_synthetic_structured_data(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorEntrySemantic semantic;
    Theron_Track02TableDecodeStatus status;
    /* 38 bytes: 8 nonzero (`f3 8f 3e d1`, `f5 c9 a9 f5`) + 30 zero.
     * This mirrors the on-disk 38-byte sparse tail at descriptor entry
     * 1 in the raw US/JP Track 02 BIN anchors. */
    static const uint8_t tail[38] = {
        0xf3, 0x8f, 0x3e, 0xd1, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xf5, 0xc9, 0xa9, 0xf5,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    /* 8 nonzero bytes inside the 38-byte tail. */
    const size_t want_nonzero = 8u;

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));
    memcpy(track + 0x0420u + 0x2feu, tail, sizeof(tail));

    status = theron_v1_track02_classify_descriptor_entry(
        track, sizeof(track), 0x1584u, 1u, &semantic);
    printf("synthetic entry 1: status=%s role=%s nonzero=%zu\n",
           theron_v1_track02_table_decode_status_name(status),
           theron_v1_track02_descriptor_table_role_name(semantic.role),
           semantic.nonzero_byte_count);

    check_int("synthetic entry 1 status",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);
    check_role("synthetic entry 1 role",
               semantic.role,
               THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_STRUCTURED_DATA);
    check_size("synthetic entry 1 nonzero bytes",
               semantic.nonzero_byte_count,
               want_nonzero);
    check_int("synthetic entry 1 marker recognized",
              semantic.audio_bank_marker_recognized,
              0);
    check_u32("synthetic entry 1 audio id",
              semantic.audio_bank_id,
              0u);
}

/* Synthetic fixture: an entry that is fully zero-fill should classify
 * ZERO_FILL with no recognized audio marker. */
static void probe_synthetic_zero_fill(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorEntrySemantic semantic;
    Theron_Track02TableDecodeStatus status;

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));

    status = theron_v1_track02_classify_descriptor_entry(
        track, sizeof(track), 0x1584u, 0u, &semantic);
    printf("synthetic entry 0: status=%s role=%s nonzero=%zu\n",
           theron_v1_track02_table_decode_status_name(status),
           theron_v1_track02_descriptor_table_role_name(semantic.role),
           semantic.nonzero_byte_count);

    check_int("synthetic entry 0 status",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);
    check_role("synthetic entry 0 role",
               semantic.role,
               THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_ZERO_FILL);
    check_size("synthetic entry 0 nonzero bytes",
               semantic.nonzero_byte_count,
               0u);
    check_int("synthetic entry 0 marker recognized",
              semantic.audio_bank_marker_recognized,
              0);
}

/* Negative fixtures: out-of-range entry index, NULL data, NULL out,
 * truncated descriptor table, malformed (not strictly ascending) table. */
static void probe_negative_fixtures(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorEntrySemantic semantic;
    Theron_Track02TableDecodeStatus status;

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));

    status = theron_v1_track02_classify_descriptor_entry(
        NULL, sizeof(track), 0x1584u, 0u, &semantic);
    check_int("NULL data is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    status = theron_v1_track02_classify_descriptor_entry(
        track, sizeof(track), 0x1584u, 0u, NULL);
    check_int("NULL out is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    status = theron_v1_track02_classify_descriptor_entry(
        track, sizeof(track), 0x1584u,
        THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES, &semantic);
    check_int("out-of-range entry is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    /* Truncated descriptor table. */
    status = theron_v1_track02_classify_descriptor_entry(
        track, 0x1584u + 1u, 0x1584u, 0u, &semantic);
    check_int("truncated descriptor is not-found",
              status,
              THERON_TRACK02_TABLE_DECODE_NOT_FOUND);

    /* Malformed descriptor table (all zero bytes -> not strictly ascending,
     * also rejected by decode_descriptor_table). */
    {
        uint8_t bad[0x3000u];
        memset(bad, 0, sizeof(bad));
        status = theron_v1_track02_classify_descriptor_entry(
            bad, sizeof(bad), 0x1584u, 0u, &semantic);
        check_int("malformed descriptor is not-found",
                  status,
                  THERON_TRACK02_TABLE_DECODE_NOT_FOUND);
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

/* All descriptor offsets documented for the bank-signal module. */
static const size_t g_us_iso_descriptor_offset = 0x1584u;
static const size_t g_us_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};
static const size_t g_jp_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

/* Variant flag that decides which role invariants the real-data
 * classifier locks.  Real Track 02 descriptor entries are
 * content-shuffled per anchor (anchor 2's marker lives at entry 3
 * instead of entry 2, etc.), so we only assert the binary invariant
 * that "at least one entry classifies as AUDIO_BANK_POINTER" on raw
 * BIN variants and "no entry classifies as AUDIO_BANK_POINTER" on the
 * US ISO partial extract.  Per-entry role / audio-id assertions would
 * over-claim what is actually stable across anchors. */
typedef enum {
    THERON_TRACK02_PROBE_VARIANT_US_ISO = 0, /* partial extract; no audio-bank markers in descriptor region */
    THERON_TRACK02_PROBE_VARIANT_RAW_BIN    /* raw BIN descriptor region; audio-bank markers expected */
} Theron_Track02ProbeVariant;

static void classify_real_data(const char *label,
                               const uint8_t *data,
                               size_t size,
                               size_t descriptor_offset,
                               Theron_Track02ProbeVariant variant) {
    Theron_Track02DescriptorEntrySemantic semantic;
    Theron_Track02TableDecodeStatus status;
    size_t audio_pointer_count = 0;
    size_t descriptor_table_count = 0;
    size_t zero_fill_count = 0;
    size_t structured_data_count = 0;
    size_t i;

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        memset(&semantic, 0, sizeof(semantic));
        status = theron_v1_track02_classify_descriptor_entry(
            data, size, descriptor_offset, i, &semantic);
        if (status != THERON_TRACK02_TABLE_DECODE_OK) {
            printf("FAIL %s entry[%zu] status=%s\n",
                   label, i,
                   theron_v1_track02_table_decode_status_name(status));
            ++g_fail;
            continue;
        }

        printf("%s entry[%zu] offset=0x%zx role=%s nonzero=%zu marker_offset=0x%zx marker_rec=%d audio_id=0x%08x\n",
               label,
               i,
               semantic.absolute_offset,
               theron_v1_track02_descriptor_table_role_name(semantic.role),
               semantic.nonzero_byte_count,
               semantic.audio_bank_marker_offset,
               semantic.audio_bank_marker_recognized,
               (unsigned)semantic.audio_bank_id);

        /* Internal invariants that should hold for every entry. */
        {
            char name[96];
            snprintf(name, sizeof(name), "%s entry[%zu] window size",
                     label, i);
            check_size(name, semantic.byte_count, DESCRIPTOR_WINDOW_SIZE);
        }

        switch (semantic.role) {
        case THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_AUDIO_BANK_POINTER:
            ++audio_pointer_count;
            /* Audio-bank role always pairs with a recognized marker and
             * a non-zero audio id. */
            {
                char name[96];
                snprintf(name, sizeof(name), "%s entry[%zu] marker_rec",
                         label, i);
                check_int(name, semantic.audio_bank_marker_recognized, 1);
                snprintf(name, sizeof(name), "%s entry[%zu] non-zero id",
                         label, i);
                check_int(name, semantic.audio_bank_id != 0u, 1);
            }
            break;
        case THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_DESCRIPTOR_TABLE:
            ++descriptor_table_count;
            break;
        case THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_ZERO_FILL:
            ++zero_fill_count;
            break;
        case THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_STRUCTURED_DATA:
            ++structured_data_count;
            break;
        case THERON_TRACK02_DESCRIPTOR_TABLE_ROLE_UNKNOWN:
        default:
            break;
        }
    }

    /* Variant-level invariants. */
    if (variant == THERON_TRACK02_PROBE_VARIANT_RAW_BIN) {
        /* Raw BIN descriptor regions carry at least one audio-bank
         * marker; the post-boundary span fingerprint is observed at
         * descriptors 2/4/6/8 at anchor 0 and at 1/3/6/8 at anchor 2.
         * Lock the binary invariant, not the per-anchor entry index. */
        char name[96];
        snprintf(name, sizeof(name), "%s has audio-bank entries", label);
        check_int(name, audio_pointer_count > 0u, 1);
        /* Each raw BIN anchor also has exactly one descriptor-table
         * entry (always index 5). */
        snprintf(name, sizeof(name), "%s descriptor-table count", label);
        check_int(name, descriptor_table_count, 1);
    } else {
        /* US Track 02 ISO partial extract: descriptor region ends
         * before the post-boundary span starts, so no descriptor entry
         * should classify as AUDIO_BANK_POINTER. */
        char name[96];
        snprintf(name, sizeof(name), "%s no audio-bank entries", label);
        check_int(name, audio_pointer_count, 0);
        snprintf(name, sizeof(name), "%s descriptor-table count", label);
        check_int(name, descriptor_table_count, 1);
        /* US ISO also has STRUCTURED_DATA in entry 3 (20 bytes) and
         * entry 4 (811 bytes); the partial extract leaves entries 0,
         * 1, 2, 6, 7, 8 zero-filled. */
        snprintf(name, sizeof(name), "%s has structured-data", label);
        check_int(name, structured_data_count > 0u, 1);
        snprintf(name, sizeof(name), "%s has zero-fill", label);
        check_int(name, zero_fill_count > 0u, 1);
    }

    printf("%s totals: audio-bank=%zu descriptor-table=%zu structured=%zu zero=%zu\n",
           label,
           audio_pointer_count,
           descriptor_table_count,
           structured_data_count,
           zero_fill_count);
}

static void probe_real_data_descriptor_table(const char *label,
                                             const char *md5_hex,
                                             const char *env_name,
                                             const char *default_file,
                                             size_t descriptor_offset,
                                             Theron_Track02ProbeVariant variant) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char local_md5[33] = {0};

    path_to_read = (env_path && env_path[0]) ? env_path : NULL;
    if (!path_to_read) {
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

    classify_real_data(label, data, size, descriptor_offset, variant);
    free(data);
}

static void probe_real_data_us_iso(void) {
    probe_real_data_descriptor_table("US ISO descriptor entries",
                                    THERON_TRACK02_MD5_US_ISO,
                                    "FIRESTAFF_THERON_TRACK02_US",
                                    "theron/TQUS02End.iso",
                                    g_us_iso_descriptor_offset,
                                    THERON_TRACK02_PROBE_VARIANT_US_ISO);
}

static void probe_real_data_raw_bin_anchor(const char *label,
                                           const char *md5_hex,
                                           const char *env_name,
                                           const char *default_file,
                                           size_t descriptor_offset) {
    probe_real_data_descriptor_table(label,
                                    md5_hex,
                                    env_name,
                                    default_file,
                                    descriptor_offset,
                                    THERON_TRACK02_PROBE_VARIANT_RAW_BIN);
}

static void probe_real_data_if_present(void) {
    size_t i;

    /* US Track 02 ISO descriptor at 0x1584 (partial extract). */
    probe_real_data_us_iso();

    /* All three US raw BIN anchors. */
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char label[96];
        snprintf(label, sizeof(label),
                 "US raw BIN descriptor entries anchor %zu", i);
        probe_real_data_raw_bin_anchor(label,
                                       THERON_TRACK02_MD5_US_BIN,
                                       "FIRESTAFF_THERON_TRACK02_US_BIN",
                                       "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                                       g_us_bin_descriptor_offsets[i]);
    }

    /* All three JP raw BIN anchors. */
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char label[96];
        snprintf(label, sizeof(label),
                 "JP raw BIN descriptor entries anchor %zu", i);
        probe_real_data_raw_bin_anchor(label,
                                       THERON_TRACK02_MD5_JP_BIN,
                                       "FIRESTAFF_THERON_TRACK02_JP_BIN",
                                       "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                                       g_jp_bin_descriptor_offsets[i]);
    }
}

int main(void) {
    printf("=== Theron V1 Track 02 Descriptor Entry Semantic Probe ===\n");
    printf("descriptor contract: %u entries, LE uint16, stride 0x0400\n",
           (unsigned)THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    printf("audio-bank marker: 12-byte 00 ff*10 00 sentinel + 4-byte LE id word\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_synthetic_audio_bank_pointer();
    probe_synthetic_structured_data();
    probe_synthetic_zero_fill();
    probe_negative_fixtures();
    probe_real_data_if_present();

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
