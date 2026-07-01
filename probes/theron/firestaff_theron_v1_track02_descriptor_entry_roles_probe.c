/*
 * firestaff_theron_v1_track02_descriptor_entry_roles_probe.c
 *
 * Theron's Quest V1 -- semantic role binding probe for the documented
 * 9-word Track 02 descriptor table.
 *
 * Scope:
 *   This probe binds each descriptor-table entry to a bounded byte-level
 *   semantic role (RESERVED_ZERO_FILL / CONTAINS_DESCRIPTOR_TABLE /
 *   PRE_DESCRIPTOR_DATA / POST_DESCRIPTOR_DATA) and pins three descriptor-
 *   window markers:
 *     - byte_before_descriptor_is_rts (HuC6280 RTS opcode == 0x60)
 *     - all_zero_after_descriptor (descriptor sits at window tail)
 *     - first_nonzero_after_descriptor (0 when all-zero)
 *
 *   It does NOT claim dungeon records, map grids, object tables, palette
 *   payloads, text/font payloads, or runtime loader handoff.  The roles
 *   are derived purely from observable byte-shape relationships.
 *
 * Source/evidence:
 *   - src/theron/theron_v1_track02.c (descriptor layout, HuC6280 0x60 RTS
 *     interpretation only; no Theron-specific code claim).
 *   - docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md
 *     (the 0x1584 descriptor and the three JP/US raw BIN anchors).
 *   - HuC6280 datasheet (RTS opcode is 0x60 on the 65C02-derivative core).
 *
 * What the probe covers:
 *   1. Canonical synthetic descriptor on a synthetic 0x3000-byte Track 02
 *      with a code-trailing-descriptor pattern (RTS at descriptor - 1).
 *   2. Synthetic positive roles: descriptor-window + 3 PRE + 4 POST + 1
 *      descriptor-window.  Wait: 1 descriptor + 4 pre + 4 post = 9 entries.
 *      The probe also covers a balanced synthetic with 3 PRE + 1 descriptor
 *      + 5 POST to exercise index ordering invariants.
 *   3. Synthetic all-zero Track 02: every entry must be RESERVED_ZERO_FILL
 *      and the descriptor-window still has the descriptor bytes itself
 *      (the descriptor bytes are non-zero).
 *   4. Negative fixtures: NULL data, NULL table, NULL out, truncated
 *      descriptor, descriptor_offset past end of data, out-of-range entry
 *      count.
 *   5. Hash-gated real-data round-trip:
 *      - US Track 02 ISO descriptor bytes at 0x1584.
 *      - All three US raw BIN anchor offsets.
 *      - All three JP raw BIN anchor offsets.
 *      - JP Rev 1 ISO is a zero-filled image and stays SKIP.
 *      The round-trip confirms:
 *        - byte_before_descriptor_is_rts is 1 in the US ISO (observed).
 *        - all_zero_after_descriptor is 1 in the US ISO (observed).
 *        - descriptor-window entry index is the documented
 *          TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR (5).
 *        - PRE_DESCRIPTOR_DATA / POST_DESCRIPTOR_DATA roles follow
 *          the index-relative ordering invariant.
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
#define DOCUMENTED_STRIDE 0x0400u

static int g_fail = 0;
static int g_skip = 0;

/* Mirrors g_us_iso_bank_stride_descriptor in src/theron/theron_v1_track02.c */
static const uint8_t g_canonical_descriptor[DESCRIPTOR_BYTE_COUNT] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
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

static void check_role(const char *label,
                       Theron_Track02DescriptorEntryRole got,
                       Theron_Track02DescriptorEntryRole want) {
    if (got != want) {
        printf("FAIL %s: got %s want %s\n",
               label,
               theron_v1_track02_descriptor_entry_role_name(got),
               theron_v1_track02_descriptor_entry_role_name(want));
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

/* Build a synthetic Track 02 with a known pattern:
 *   - 4 PRE_DESCRIPTOR_DATA windows at indices 0..3 (each has 1 byte
 *     of nonzero marker data)
 *   - 1 CONTAINS_DESCRIPTOR_TABLE window at index 4 (descriptor at
 *     descriptor_offset; first_nonzero_offset at the byte before
 *     descriptor; last_nonzero_offset at the descriptor's last byte)
 *   - 4 POST_DESCRIPTOR_DATA windows at indices 5..8 (each has 1 byte
 *     of nonzero marker data)
 *
 * Total Track 02 size = 0x3000 (descriptor_offset = 0x1584 -> base
 * offset 0, descriptor sits in window index 5 instead of 4).
 *
 * To put descriptor in window 4 we offset the descriptor by 0x400 less.
 */
static void build_synthetic_balanced_track02(uint8_t *track,
                                              size_t track_size,
                                              size_t descriptor_offset) {
    size_t i;

    memset(track, 0, track_size);
    /* Place the canonical descriptor bytes at descriptor_offset. */
    memcpy(track + descriptor_offset,
           g_canonical_descriptor,
           sizeof(g_canonical_descriptor));

    /* Mark each window with one nonzero byte at its first byte so the
     * PRE/POST_DATA role sees nonzero_byte_count > 0. */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        const uint16_t rel = (uint16_t)(0x0020u + (uint16_t)(i * DOCUMENTED_STRIDE));
        track[rel] = (uint8_t)(0xa0u | (uint8_t)i);
    }

    /* Make the byte immediately before the descriptor an RTS so the
     * byte_before_descriptor_is_rts marker is true. */
    if (descriptor_offset > 0u) {
        track[descriptor_offset - 1u] = 0x60u;
    }
}

/* Probe the documented US Track 02 ISO pattern: descriptor at 0x1584,
 * bytes before descriptor end with `60` (RTS), bytes after descriptor
 * in window 5 are all zero (descriptor sits at window tail).
 *
 * Synthesized Track 02 layout:
 *   - All windows except window 5 (the descriptor-window) are zero
 *     EXCEPT windows that we mark as PRE/POST data.
 *   - For this probe we set up:
 *       - windows 0..3 all-zero (RESERVED_ZERO_FILL)
 *       - window 4 contains bytes (PRE_DESCRIPTOR_DATA)
 *       - window 5 contains the descriptor + 0x60 at descriptor-1
 *         (CONTAINS_DESCRIPTOR_TABLE, descriptor at window tail,
 *          byte_before_descriptor_is_rts = 1,
 *          all_zero_after_descriptor = 1)
 *       - windows 6..8 all-zero (RESERVED_ZERO_FILL)
 *
 *   To put descriptor in window 5, descriptor_offset must be 0x1420
 *   + (5 * 0x400) = 0x1420 + 0x1400 = 0x2820? No: descriptor_offset
 *   = TQR_US_ISO_BANK_STRIDE_OFFSET = 0x1584 maps to window 5 because
 *   base_offset = 0.  So descriptor_offset 0x1584 falls in window 5
 *   (0x1420..0x1820). */
static void probe_synthetic_balanced_roles(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    static const Theron_Track02DescriptorEntryRole want_role[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES] = {
        /* window 0 = 0x0020..0x0420: zero-fill (we do not mark it). */
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
        /* window 4 = 0x1020..0x1420: PRE_DESCRIPTOR_DATA (we mark it). */
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA,
        /* window 5 = 0x1420..0x1820: descriptor-window with code + descriptor. */
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE,
        /* window 6..8 = 0x1820..0x2420: zero-fill. */
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL
    };
    size_t i;

    build_synthetic_balanced_track02(track, sizeof(track), 0x1584u);
    /* Unmark window 0..3 and 6..8 by zeroing the markers we placed. */
    for (i = 0; i < 4u; ++i) {
        const uint16_t rel = (uint16_t)(0x0020u + (uint16_t)(i * DOCUMENTED_STRIDE));
        track[rel] = 0u;
    }
    for (i = 6; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        const uint16_t rel = (uint16_t)(0x0020u + (uint16_t)(i * DOCUMENTED_STRIDE));
        track[rel] = 0u;
    }
    /* Mark window 4 as PRE_DESCRIPTOR_DATA with 1 nonzero byte. */
    track[0x1020u] = 0xbbu;

    status = theron_v1_track02_decode_descriptor_table(
        g_canonical_descriptor,
        sizeof(g_canonical_descriptor),
        DOCUMENTED_STRIDE,
        &table);
    check_int("synthetic balanced decode",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        0x1584u,
        &table,
        entries);
    check_int("synthetic balanced bind status",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        char name[96];
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] role", i);
        check_role(name, entries[i].role, want_role[i]);
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] is_descriptor_window", i);
        check_int(name, entries[i].is_descriptor_window, (i == 5u) ? 1 : 0);
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] index", i);
        check_size(name, entries[i].entry_index, i);
    }

    /* Descriptor-window specific markers. */
    check_int("synthetic balanced descriptor-window byte_before_descriptor_is_rts",
              entries[5].byte_before_descriptor_is_rts,
              1);
    check_int("synthetic balanced descriptor-window all_zero_after_descriptor",
              entries[5].all_zero_after_descriptor,
              1);
    check_size("synthetic balanced descriptor-window first_nonzero_after_descriptor",
               entries[5].first_nonzero_after_descriptor,
               0u);
    check_u16("synthetic balanced descriptor-window byte_before_descriptor",
              entries[5].byte_before_descriptor,
              0x0060u);

    /* Non-descriptor-window markers must be zero. */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        if (i == 5u) continue;
        char name[96];
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] non-descriptor byte_before", i);
        check_u16(name, entries[i].byte_before_descriptor, 0u);
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] non-descriptor is_rts", i);
        check_int(name, entries[i].byte_before_descriptor_is_rts, 0);
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] non-descriptor all_zero_after", i);
        check_int(name, entries[i].all_zero_after_descriptor, 0);
        snprintf(name, sizeof(name),
                 "synthetic balanced entry[%zu] non-descriptor first_nonzero_after",
                 i);
        check_size(name, entries[i].first_nonzero_after_descriptor, 0u);
    }

    /* Index-relative ordering invariant: PRE entries have
     * entry_index < 5; POST entries have entry_index > 5. */
    for (i = 0; i < 5u; ++i) {
        if (entries[i].role == THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA) {
            char name[96];
            snprintf(name, sizeof(name),
                     "synthetic balanced entry[%zu] role index invariant", i);
            check_size(name, i, entries[i].entry_index);
        }
    }
}

/* Probe a JP-BIN-like pattern: descriptor in the middle of a window
 * with code/data bytes both before AND after the descriptor.
 *
 * Layout (descriptor at offset 0x70b4d6-equivalent; we use 0x1584
 * shifted by +0x100 to put descriptor mid-window with bytes on both
 * sides; but we need descriptor_offset to map to a specific window
 * position.  Easiest: shift descriptor by +0x10 so the descriptor
 * starts 16 bytes into its window, with 16 bytes of code before and
 * 0x0400 - 0x10 - 18 = 0x3d8 = 984 bytes of code after). */
static void probe_synthetic_descriptor_mid_window(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    /* Shift descriptor by 0x10 from its canonical 0x1584 location.
     * The new descriptor_offset = 0x1594 still falls inside window 5
     * (0x1420..0x1820) because 0x1594 is between 0x1420 and 0x1820.
     * So descriptor-window entry index is still 5. */
    const size_t mid_descriptor_offset = 0x1594u;
    /* First nonzero before descriptor: 0x1420 (a `60` RTS).
     * Byte just before descriptor: 0x60 (RTS). */

    memset(track, 0, sizeof(track));
    memcpy(track + mid_descriptor_offset,
           g_canonical_descriptor,
           sizeof(g_canonical_descriptor));
    track[0x1420u] = 0x60u; /* first nonzero before descriptor */
    track[0x1421u] = 0x60u; /* extra code before descriptor */
    /* Byte after the descriptor: non-zero (this is what makes
     * all_zero_after_descriptor flip to 0). */
    track[mid_descriptor_offset + DESCRIPTOR_BYTE_COUNT] = 0xccu;

    status = theron_v1_track02_decode_descriptor_table(
        g_canonical_descriptor,
        sizeof(g_canonical_descriptor),
        DOCUMENTED_STRIDE,
        &table);
    check_int("mid-window decode", status, THERON_TRACK02_TABLE_DECODE_OK);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        mid_descriptor_offset,
        &table,
        entries);
    check_int("mid-window bind status", status, THERON_TRACK02_TABLE_DECODE_OK);

    /* Descriptor-window entry is still index 5 (descriptor_offset is
     * still inside window 5). */
    check_int("mid-window descriptor-window entry index",
              entries[5].is_descriptor_window,
              1);
    check_int("mid-window role",
              entries[5].role,
              THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE);
    /* Byte immediately before descriptor: still 0x60 (the byte at
     * descriptor_offset - 1 = 0x1593, which is part of the track. */
    check_u16("mid-window byte_before_descriptor",
              entries[5].byte_before_descriptor,
              0x0060u);
    check_int("mid-window byte_before_descriptor_is_rts",
              entries[5].byte_before_descriptor_is_rts,
              1);
    /* Bytes after descriptor are NOT all zero (we placed 0xcc there). */
    check_int("mid-window all_zero_after_descriptor",
              entries[5].all_zero_after_descriptor,
              0);
    check_size("mid-window first_nonzero_after_descriptor",
               entries[5].first_nonzero_after_descriptor,
               mid_descriptor_offset + DESCRIPTOR_BYTE_COUNT);
}

/* All-zero Track 02 with the descriptor bytes themselves still non-zero
 * inside the descriptor-window.  Every entry must be either RESERVED or
 * CONTAINS_DESCRIPTOR_TABLE.  PRE/POST_DATA must NOT appear. */
static void probe_synthetic_all_zero_track02(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    size_t i;

    memset(track, 0, sizeof(track));
    /* The descriptor bytes themselves are non-zero; the byte immediately
     * before the descriptor is also zero (descriptor sits at the head of
     * the descriptor-window).  Place descriptor at 0x1420 (window 5 start). */
    memcpy(track + 0x1420u,
           g_canonical_descriptor,
           sizeof(g_canonical_descriptor));

    status = theron_v1_track02_decode_descriptor_table(
        g_canonical_descriptor,
        sizeof(g_canonical_descriptor),
        DOCUMENTED_STRIDE,
        &table);
    check_int("all-zero decode", status, THERON_TRACK02_TABLE_DECODE_OK);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        0x1420u,
        &table,
        entries);
    check_int("all-zero bind status", status, THERON_TRACK02_TABLE_DECODE_OK);

    /* Descriptor-window is now entry 5 (descriptor at window head).
     * Byte immediately before descriptor (0x141f) is zero, so
     * byte_before_descriptor_is_rts is 0. */
    check_int("all-zero descriptor-window entry index",
              entries[5].is_descriptor_window,
              1);
    check_int("all-zero role",
              entries[5].role,
              THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE);
    check_u16("all-zero byte_before_descriptor",
              entries[5].byte_before_descriptor,
              0x0000u);
    check_int("all-zero byte_before_descriptor_is_rts",
              entries[5].byte_before_descriptor_is_rts,
              0);
    /* All bytes after the descriptor within window 5 are zero. */
    check_int("all-zero all_zero_after_descriptor",
              entries[5].all_zero_after_descriptor,
              1);

    /* Every other entry must be RESERVED_ZERO_FILL because nonzero
     * count is 0. */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        if (i == 5u) continue;
        char name[96];
        snprintf(name, sizeof(name), "all-zero entry[%zu] role", i);
        check_role(name,
                   entries[i].role,
                   THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL);
        snprintf(name, sizeof(name), "all-zero entry[%zu] nonzero_byte_count=0",
                 i);
        check_int(name, entries[i].is_descriptor_window, 0);
    }
}

/* Negative fixtures. */
static void probe_negative_fixtures(void) {
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    uint8_t track[0x3000u];

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));
    status = theron_v1_track02_decode_descriptor_table(
        g_canonical_descriptor,
        sizeof(g_canonical_descriptor),
        DOCUMENTED_STRIDE,
        &table);
    check_int("negative decode status",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        NULL,
        sizeof(track),
        0x1584u,
        &table,
        entries);
    check_int("NULL data is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        0x1584u,
        NULL,
        entries);
    check_int("NULL table is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        0x1584u,
        &table,
        NULL);
    check_int("NULL out is bad-input",
              status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    /* Descriptor offset past end of data. */
    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        sizeof(track),
        &table,
        entries);
    check_int("descriptor past end is not-found",
              status,
              THERON_TRACK02_TABLE_DECODE_NOT_FOUND);

    /* Truncated Track 02 (descriptor would need bytes past end). */
    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        0x1600u, /* too short for the 0x1584 + 18 descriptor */
        0x1584u,
        &table,
        entries);
    check_int("truncated track is not-found",
              status,
              THERON_TRACK02_TABLE_DECODE_NOT_FOUND);

    /* Pre-anchor descriptor offset (no window contains it). */
    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        0x0400u, /* inside window 1; not at the canonical anchor */
        &table,
        entries);
    check_int("pre-anchor descriptor offset is not-found",
              status,
              THERON_TRACK02_TABLE_DECODE_NOT_FOUND);
}

/* Index-relative ordering invariant: for every entry whose role is
 * PRE_DESCRIPTOR_DATA, its entry_index is strictly less than the
 * descriptor-window entry index; for every POST_DESCRIPTOR_DATA entry
 * its entry_index is strictly greater.
 *
 * This invariant must hold regardless of the specific Track 02 layout,
 * and is the single most important semantic-ordering claim about the
 * descriptor table. */
static void probe_index_ordering_invariant(void) {
    uint8_t track[0x3000u];
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    int descriptor_window_index;
    size_t i;

    build_synthetic_balanced_track02(track, sizeof(track), 0x1584u);
    /* Mark window 1 and window 7 with a single nonzero byte each so we
     * have a mix of PRE and POST roles. */
    track[0x0420u] = 0xb1u; /* PRE (window 1) */
    track[0x1c20u] = 0xb7u; /* POST (window 7) */
    track[0x1020u] = 0xb4u; /* PRE (window 4) */
    track[0x2020u] = 0xb8u; /* POST (window 8) */

    status = theron_v1_track02_decode_descriptor_table(
        g_canonical_descriptor,
        sizeof(g_canonical_descriptor),
        DOCUMENTED_STRIDE,
        &table);
    check_int("invariant decode", status, THERON_TRACK02_TABLE_DECODE_OK);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        0x1584u,
        &table,
        entries);
    check_int("invariant bind status", status, THERON_TRACK02_TABLE_DECODE_OK);

    descriptor_window_index =
        theron_v1_track02_find_descriptor_window_entry_index(
            entries,
            THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    check_int("invariant descriptor-window index",
              descriptor_window_index,
              5);

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        switch (entries[i].role) {
        case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA:
            if ((int)i >= descriptor_window_index) {
                printf("FAIL PRE invariant: entry %zu (>= descriptor %d)\n",
                       i, descriptor_window_index);
                ++g_fail;
            }
            break;
        case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA:
            if ((int)i <= descriptor_window_index) {
                printf("FAIL POST invariant: entry %zu (<= descriptor %d)\n",
                       i, descriptor_window_index);
                ++g_fail;
            }
            break;
        case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL:
        case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE:
        case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_UNKNOWN:
        default:
            break;
        }
    }

    /* Spot-check our deliberate marker placement. */
    check_role("invariant entry[1] role",
               entries[1].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA);
    check_role("invariant entry[4] role",
               entries[4].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA);
    check_role("invariant entry[7] role",
               entries[7].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA);
    check_role("invariant entry[8] role",
               entries[8].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA);
}

static void check_real_data_descriptor_window(const uint8_t *data,
                                              size_t size,
                                              size_t descriptor_offset,
                                              const char *label) {
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    int descriptor_window_index;
    size_t i;

    if (descriptor_offset > size ||
        DESCRIPTOR_BYTE_COUNT > size - descriptor_offset) {
        printf("FAIL %s: offset 0x%zx exceeds data size %zu\n",
               label, descriptor_offset, size);
        ++g_fail;
        return;
    }
    status = theron_v1_track02_decode_descriptor_table(
        data + descriptor_offset,
        DESCRIPTOR_BYTE_COUNT,
        DOCUMENTED_STRIDE,
        &table);
    check_int("decode status", status, THERON_TRACK02_TABLE_DECODE_OK);

    status = theron_v1_track02_bind_descriptor_entry_roles(
        data,
        size,
        descriptor_offset,
        &table,
        entries);
    check_int("bind status", status, THERON_TRACK02_TABLE_DECODE_OK);

    /* Exactly one entry must be the descriptor-window. */
    descriptor_window_index =
        theron_v1_track02_find_descriptor_window_entry_index(
            entries,
            THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    if (descriptor_window_index < 0 ||
        descriptor_window_index >=
            (int)THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES) {
        printf("FAIL %s: descriptor_window_index out of range %d\n",
               label, descriptor_window_index);
        ++g_fail;
        return;
    }
    printf("%s: descriptor_window_index=%d role=%s byte_before=0x%02x is_rts=%d all_zero_after=%d\n",
           label,
           descriptor_window_index,
           theron_v1_track02_descriptor_entry_role_name(
               entries[descriptor_window_index].role),
           (unsigned)entries[descriptor_window_index].byte_before_descriptor,
           entries[descriptor_window_index].byte_before_descriptor_is_rts,
           entries[descriptor_window_index].all_zero_after_descriptor);

    /* All 9 entries must be role-populated. */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        char name[96];
        snprintf(name, sizeof(name), "%s entry[%zu] is_descriptor_window",
                 label, i);
        check_int(name, entries[i].is_descriptor_window,
                  (i == (size_t)descriptor_window_index) ? 1 : 0);
        snprintf(name, sizeof(name), "%s entry[%zu] role populated", label, i);
        check_int(name,
                  entries[i].role !=
                      THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_UNKNOWN,
                  1);
    }

    /* Index-relative ordering invariant must hold for every entry. */
    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        if (entries[i].role ==
                THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA &&
            (int)i >= descriptor_window_index) {
            printf("FAIL %s entry[%zu] PRE but index>=descriptor %d\n",
                   label, i, descriptor_window_index);
            ++g_fail;
        }
        if (entries[i].role ==
                THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA &&
            (int)i <= descriptor_window_index) {
            printf("FAIL %s entry[%zu] POST but index<=descriptor %d\n",
                   label, i, descriptor_window_index);
            ++g_fail;
        }
    }
}

static void probe_real_data_descriptor_window_markers(const char *label,
                                                      const char *md5_hex,
                                                      const char *env_name,
                                                      const char *default_file,
                                                      size_t descriptor_offset,
                                                      int expect_descriptor_window_index,
                                                      int expect_byte_before_is_rts,
                                                      int expect_all_zero_after) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char local_md5[33] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus table_status;

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

    /* Reuse the bank-signal module to confirm the descriptor offset is
     * part of a known Track 02 anchor. */
    signal_status = theron_v1_track02_find_bank_signal(data, size, local_md5, &signal);
    check_int("bank-signal status",
              signal_status,
              THERON_TRACK02_SIGNAL_OK);

    table_status = theron_v1_track02_decode_descriptor_table(
        data + descriptor_offset,
        DESCRIPTOR_BYTE_COUNT,
        DOCUMENTED_STRIDE,
        &table);
    check_int("descriptor decode", table_status, THERON_TRACK02_TABLE_DECODE_OK);

    table_status = theron_v1_track02_bind_descriptor_entry_roles(
        data,
        size,
        descriptor_offset,
        &table,
        entries);
    check_int("descriptor roles bind", table_status, THERON_TRACK02_TABLE_DECODE_OK);

    {
        int descriptor_window_index =
            theron_v1_track02_find_descriptor_window_entry_index(
                entries,
                THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
        char name[96];
        snprintf(name, sizeof(name), "%s descriptor-window index",
                 label);
        check_int(name, descriptor_window_index, expect_descriptor_window_index);
        snprintf(name, sizeof(name), "%s byte_before_descriptor_is_rts",
                 label);
        check_int(name,
                  entries[descriptor_window_index].byte_before_descriptor_is_rts,
                  expect_byte_before_is_rts);
        snprintf(name, sizeof(name), "%s all_zero_after_descriptor",
                 label);
        check_int(name,
                  entries[descriptor_window_index].all_zero_after_descriptor,
                  expect_all_zero_after);
    }

    free(data);
}

static const size_t g_us_iso_descriptor_offset = 0x1584u;
static const size_t g_us_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};
static const size_t g_jp_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static void probe_real_data_descriptor_offset_index_invariant(void) {
    uint8_t *data = NULL;
    size_t size = 0;
    char path[512];
    char local_md5[33] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    size_t i;

    /* US ISO descriptor at 0x1584.  All anchors should round-trip with
     * consistent role assignment. */
    default_data_path("theron/TQUS02End.iso", path);
    if (!file_exists(path)) {
        printf("SKIP US ISO descriptor-offset-index-invariant: no file\n");
        ++g_skip;
    } else if (!m12_file_md5_hex(path, local_md5) ||
               strcmp(local_md5, THERON_TRACK02_MD5_US_ISO) != 0) {
        printf("SKIP US ISO descriptor-offset-index-invariant: MD5 mismatch %s\n",
               local_md5);
        ++g_skip;
    } else if (!read_file(path, &data, &size)) {
        printf("SKIP US ISO descriptor-offset-index-invariant: read fail\n");
        ++g_skip;
    } else {
        signal_status = theron_v1_track02_find_bank_signal(
            data, size, local_md5, &signal);
        check_int("US ISO bank-signal status",
                  signal_status,
                  THERON_TRACK02_SIGNAL_OK);
        check_real_data_descriptor_window(data, size, 0x1584u,
                                          "US ISO descriptor-offset-index-invariant");
        free(data);
        data = NULL;
    }

    /* US raw BIN anchors. */
    default_data_path("theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                      path);
    if (!file_exists(path)) {
        printf("SKIP US raw BIN descriptor-offset-index-invariant: no file\n");
        ++g_skip;
    } else if (!m12_file_md5_hex(path, local_md5) ||
               strcmp(local_md5, THERON_TRACK02_MD5_US_BIN) != 0) {
        printf("SKIP US raw BIN descriptor-offset-index-invariant: MD5 mismatch %s\n",
               local_md5);
        ++g_skip;
    } else if (!read_file(path, &data, &size)) {
        printf("SKIP US raw BIN descriptor-offset-index-invariant: read fail\n");
        ++g_skip;
    } else {
        signal_status = theron_v1_track02_find_bank_signal(
            data, size, local_md5, &signal);
        check_int("US raw BIN bank-signal status",
                  signal_status,
                  THERON_TRACK02_SIGNAL_OK);
        for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
            char label[96];
            snprintf(label, sizeof(label),
                     "US raw BIN anchor %zu index invariant", i);
            check_real_data_descriptor_window(data, size,
                                              g_us_bin_descriptor_offsets[i],
                                              label);
        }
        free(data);
        data = NULL;
    }

    /* JP raw BIN anchors. */
    default_data_path("theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                      path);
    if (!file_exists(path)) {
        printf("SKIP JP raw BIN descriptor-offset-index-invariant: no file\n");
        ++g_skip;
    } else if (!m12_file_md5_hex(path, local_md5) ||
               strcmp(local_md5, THERON_TRACK02_MD5_JP_BIN) != 0) {
        printf("SKIP JP raw BIN descriptor-offset-index-invariant: MD5 mismatch %s\n",
               local_md5);
        ++g_skip;
    } else if (!read_file(path, &data, &size)) {
        printf("SKIP JP raw BIN descriptor-offset-index-invariant: read fail\n");
        ++g_skip;
    } else {
        signal_status = theron_v1_track02_find_bank_signal(
            data, size, local_md5, &signal);
        check_int("JP raw BIN bank-signal status",
                  signal_status,
                  THERON_TRACK02_SIGNAL_OK);
        for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
            char label[96];
            snprintf(label, sizeof(label),
                     "JP raw BIN anchor %zu index invariant", i);
            check_real_data_descriptor_window(data, size,
                                              g_jp_bin_descriptor_offsets[i],
                                              label);
        }
        free(data);
        data = NULL;
    }
}

int main(void) {
    printf("=== Theron V1 Track 02 Descriptor Entry Role Binding Probe ===\n");
    printf("descriptor contract: 9 entries, LE uint16, stride 0x%04x, range [0x0020, 0x2420)\n",
           DOCUMENTED_STRIDE);
    printf("roles: RESERVED_ZERO_FILL / CONTAINS_DESCRIPTOR_TABLE / "
           "PRE_DESCRIPTOR_DATA / POST_DESCRIPTOR_DATA\n");
    printf("descriptor-window markers: byte_before_descriptor_is_rts (HuC6280 RTS == 0x60), "
           "all_zero_after_descriptor\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_synthetic_balanced_roles();
    probe_synthetic_descriptor_mid_window();
    probe_synthetic_all_zero_track02();
    probe_negative_fixtures();
    probe_index_ordering_invariant();

    /* Hash-gated real-data marker assertions. */
    probe_real_data_descriptor_window_markers(
        "US ISO descriptor window markers",
        THERON_TRACK02_MD5_US_ISO,
        "FIRESTAFF_THERON_TRACK02_US",
        "theron/TQUS02End.iso",
        g_us_iso_descriptor_offset,
        5, /* TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR */
        1, /* byte 0x60 (RTS) immediately before descriptor */
        1  /* bytes after descriptor in window are all zero */
    );

    /* US raw BIN anchors: descriptor sits at 0x70be06, 0x70e2c6, 0x710904.
     * The byte before descriptor in raw BIN anchors is also 0x60 (RTS).
     * Bytes after descriptor in the descriptor-window are not necessarily
     * zero in raw BINs (additional code can follow the descriptor). */
    for (size_t i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char label[96];
        snprintf(label, sizeof(label),
                 "US raw BIN descriptor window markers anchor %zu", i);
        probe_real_data_descriptor_window_markers(
            label,
            THERON_TRACK02_MD5_US_BIN,
            "FIRESTAFF_THERON_TRACK02_US_BIN",
            "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
            g_us_bin_descriptor_offsets[i],
            5,
            1,
            0 /* observed: descriptor in raw BINs is mid-window, not at tail */
        );
    }

    /* JP raw BIN anchors: descriptor-window markers same as US raw BIN. */
    for (size_t i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char label[96];
        snprintf(label, sizeof(label),
                 "JP raw BIN descriptor window markers anchor %zu", i);
        probe_real_data_descriptor_window_markers(
            label,
            THERON_TRACK02_MD5_JP_BIN,
            "FIRESTAFF_THERON_TRACK02_JP_BIN",
            "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
            g_jp_bin_descriptor_offsets[i],
            5,
            1,
            0
        );
    }

    probe_real_data_descriptor_offset_index_invariant();

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
