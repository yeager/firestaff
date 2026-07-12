/*
 * Theron's Quest Track 02 non-startup sector receipt probe.
 *
 * This probe records only MODE1/2352 container facts for structurally
 * post-descriptor windows.  It does not identify a bitmap, level, object,
 * palette, text payload, or a runtime route.
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

/* Whole MODE1/2352 sectors are required by the container index's logical
 * user-data coordinates. Keep the fixture just beyond the final anchor. */
#define RAW_FIXTURE_SIZE 0x713160u
#define DESCRIPTOR_OFFSET_DELTA 0x1584u
#define WINDOW_FIRST_RELATIVE_OFFSET 0x0020u
#define WINDOW_STRIDE 0x0400u

static int g_fail = 0;
static int g_skip = 0;

static const uint8_t g_descriptor[18] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static const uint8_t g_boundary_span[44] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
    0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
    0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
    0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
    0x93, 0x80, 0x00, 0x3f
};

static const size_t g_us_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};

static const size_t g_us_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d53e0u, 0x47d040u, 0x712840u
};

static const size_t g_jp_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static const size_t g_jp_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d4ab0u, 0x47c710u, 0x711f10u
};

static void check(int condition, const char *label) {
    if (!condition) {
        printf("FAIL %s\n", label);
        ++g_fail;
    }
}

static size_t window_offset(size_t descriptor_offset, size_t entry_index) {
    return descriptor_offset - DESCRIPTOR_OFFSET_DELTA +
        WINDOW_FIRST_RELATIVE_OFFSET + entry_index * WINDOW_STRIDE;
}

static void build_raw_fixture(uint8_t *bytes, size_t byte_count,
                              const size_t *descriptor_offsets,
                              const size_t *span_offsets,
                              uint8_t payload_tag) {
    size_t anchor_index;

    memset(bytes, 0, byte_count);
    for (anchor_index = 0u;
         anchor_index < THERON_TRACK02_MAX_BANK_ANCHORS;
         ++anchor_index) {
        size_t entry_index;

        memcpy(bytes + descriptor_offsets[anchor_index],
               g_descriptor, sizeof(g_descriptor));
        memcpy(bytes + span_offsets[anchor_index],
               g_boundary_span, sizeof(g_boundary_span));
        for (entry_index = 6u; entry_index < 9u; ++entry_index) {
            bytes[window_offset(descriptor_offsets[anchor_index],
                                entry_index)] =
                (uint8_t)(payload_tag + anchor_index * 3u + entry_index);
        }
    }
}

static void clear_entry7_windows(uint8_t *bytes,
                                 const size_t *descriptor_offsets) {
    size_t anchor_index;
    for (anchor_index = 0u;
         anchor_index < THERON_TRACK02_MAX_BANK_ANCHORS;
         ++anchor_index) {
        memset(bytes + window_offset(descriptor_offsets[anchor_index], 7u),
               0, WINDOW_STRIDE);
    }
}

static void compare_layouts(const Theron_Track02NonstartupSectorReceipt *jp,
                            const Theron_Track02NonstartupSectorReceipt *us,
                            const char *label) {
    Theron_Track02NonstartupSectorLayoutComparisonReceipt comparison;
    Theron_Track02NonstartupSectorLayoutComparisonStatus status =
        theron_v1_track02_compare_nonstartup_sector_layout_variants(
            jp, us, &comparison);
    unsigned int expected_mask =
        (1u << THERON_TRACK02_MAX_BANK_ANCHORS) - 1u;

    check(status == THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_OK &&
              comparison.valid,
          "JP/US opaque layout comparison is hash-gated and valid");
    check(comparison.opaque_only && comparison.promotion_blocked,
          "JP/US opaque layout comparison cannot promote a payload");
    check(comparison.comparable_anchor_mask == expected_mask &&
              comparison.layout_matching_anchor_mask == expected_mask,
          "JP/US descriptor-relative opaque geometry agrees at all anchors");
    check(comparison.comparison_hash != 0u,
          "JP/US opaque layout comparison has a reproducible hash");
    printf("RECEIPT %s comparable=0x%x layout=0x%x content-match=0x%x "
           "content-mismatch=0x%x hash=0x%08x opaque=%d blocked=%d\n",
           label, comparison.comparable_anchor_mask,
           comparison.layout_matching_anchor_mask,
           comparison.content_matching_anchor_mask,
           comparison.content_mismatch_anchor_mask,
           (unsigned)comparison.comparison_hash, comparison.opaque_only,
           comparison.promotion_blocked);
}

static void check_receipt(const Theron_Track02NonstartupSectorReceipt *receipt,
                          const char *label) {
    size_t anchor_index;
    int is_synthetic = strstr(label, "synthetic") != NULL;
    const size_t *descriptor_offsets = strstr(label, "-jp")
        ? g_jp_descriptor_offsets : g_us_descriptor_offsets;

    check(receipt->valid && receipt->verified_track02,
          "receipt valid and known-variant-gated");
    check(receipt->opaque_only && receipt->promotion_blocked,
          "receipt remains opaque and promotion-blocked");
    check(receipt->anchor_count == THERON_TRACK02_MAX_BANK_ANCHORS,
          "receipt retains all descriptor anchors");
    check(receipt->receipt_hash != 0u, "receipt has reproducible hash");

    for (anchor_index = 0u; anchor_index < receipt->anchor_count; ++anchor_index) {
        size_t window_index;

        if (is_synthetic) {
            check(receipt->window_count[anchor_index] == 3u,
                  "fixture has three post-descriptor windows per anchor");
        } else {
            check(receipt->window_count[anchor_index] > 0u,
                  "real anchor has opaque post-descriptor windows");
        }
        for (window_index = 0u;
             window_index < receipt->window_count[anchor_index];
             ++window_index) {
            const Theron_Track02NonstartupSectorWindowReceipt *window =
                &receipt->windows[anchor_index][window_index];
            size_t expected_entry = 6u + window_index;

            if (is_synthetic) {
                check(window->descriptor_entry_index == expected_entry,
                      "receipt preserves post-descriptor entry index");
                check(window->raw_offset ==
                          window_offset(descriptor_offsets[anchor_index],
                                        expected_entry),
                      "receipt preserves exact descriptor-derived raw boundary");
            }
            check(window->byte_count == WINDOW_STRIDE,
                  "receipt window remains exactly 0x400 bytes");
            check(window->raw_span_hash != 0u && window->opaque &&
                      window->promotion_blocked,
                  "receipt fingerprints bytes without semantic promotion");
            check(window->raw_span_contains_non_user_data ||
                      window->user_data_span_contiguous,
                  "receipt classifies the MODE1 container boundary");
            printf("RECEIPT %s anchor=%zu entry=%zu raw=0x%zx bytes=%zu "
                   "sectors=%zu..%zu user=0x%zx..0x%zx hash=0x%08x "
                   "opaque=%d blocked=%d\n",
                   label, anchor_index, window->descriptor_entry_index,
                   window->raw_offset, window->byte_count,
                   window->first_raw_sector, window->last_raw_sector,
                   window->user_data_offset, window->user_data_end_offset,
                   (unsigned)window->raw_span_hash, window->opaque,
                   window->promotion_blocked);
        }
    }
}

static void check_container_index(const uint8_t *bytes,
                                  size_t byte_count,
                                  const char *md5_hex,
                                  const char *label) {
    Theron_Track02NonstartupContainerIndex index;
    Theron_Track02SignalStatus status;
    size_t anchor;

    status = theron_v1_track02_build_nonstartup_container_index(
        bytes, byte_count, md5_hex, &index);
    check(status == THERON_TRACK02_SIGNAL_OK && index.valid,
          "non-startup container index is byte-validated");
    if (status != THERON_TRACK02_SIGNAL_OK) return;
    check(index.verified_track02 && index.opaque_only && index.promotion_blocked,
          "container index remains opaque and promotion-blocked");
    check(index.anchor_count == THERON_TRACK02_MAX_BANK_ANCHORS &&
              index.container_count == THERON_TRACK02_MAX_NONSTARTUP_CONTAINERS,
          "container index has entries 6 and 8 at every anchor");
    for (anchor = 0u; anchor < index.anchor_count; ++anchor) {
        const Theron_Track02NonstartupContainer *entry6 =
            theron_v1_track02_find_nonstartup_container(&index, anchor, 6u);
        const Theron_Track02NonstartupContainer *entry8 =
            theron_v1_track02_find_nonstartup_container(&index, anchor, 8u);
        check(entry6 && entry8 &&
                  !theron_v1_track02_find_nonstartup_container(&index, anchor, 7u),
              "container index has no zero-fill fallback entry");
        if (entry6 && entry8) {
            Theron_Track02NonstartupSectorDescriptor entry6_sectors;
            Theron_Track02NonstartupSectorDescriptor entry8_sectors;
            check(entry6->raw_byte_count == WINDOW_STRIDE &&
                      entry8->raw_byte_count == WINDOW_STRIDE &&
                      entry6->user_data_segment_count > 0u &&
                      entry8->user_data_segment_count > 0u &&
                      entry6->user_data_byte_count < entry6->raw_byte_count &&
                      entry8->user_data_byte_count < entry8->raw_byte_count,
                  "container index preserves raw and user-data boundaries");
            check(theron_v1_track02_describe_nonstartup_container_sectors(
                      &index, anchor, 6u, &entry6_sectors) ==
                      THERON_TRACK02_SIGNAL_OK &&
                      theron_v1_track02_describe_nonstartup_container_sectors(
                      &index, anchor, 8u, &entry8_sectors) ==
                      THERON_TRACK02_SIGNAL_OK,
                  "typed MODE1 sector descriptors are available");
            check(entry6_sectors.valid && entry8_sectors.valid &&
                      entry6_sectors.opaque_only && entry8_sectors.opaque_only &&
                      entry6_sectors.promotion_blocked &&
                      entry8_sectors.promotion_blocked &&
                      entry6_sectors.mode1_sync_header_byte_count +
                          entry6_sectors.mode1_user_data_byte_count +
                          entry6_sectors.mode1_sector_tail_byte_count == WINDOW_STRIDE &&
                      entry8_sectors.mode1_sync_header_byte_count +
                          entry8_sectors.mode1_user_data_byte_count +
                          entry8_sectors.mode1_sector_tail_byte_count == WINDOW_STRIDE,
                  "typed descriptors cover each raw window without promotion");
            check(entry6_sectors.span_count == (anchor == 2u ? 3u : 4u) &&
                      entry8_sectors.span_count == 4u &&
                      entry6_sectors.mode1_sync_header_byte_count == 16u &&
                      entry8_sectors.mode1_sync_header_byte_count == 16u &&
                      entry6_sectors.mode1_user_data_byte_count ==
                          (anchor == 2u ? 752u : 720u) &&
                      entry8_sectors.mode1_user_data_byte_count == 720u &&
                      entry6_sectors.mode1_sector_tail_byte_count ==
                          (anchor == 2u ? 256u : 288u) &&
                      entry8_sectors.mode1_sector_tail_byte_count == 288u,
                  "typed descriptors preserve header, tail-leading, and interleaved spans");
            printf("SECTORS %s anchor=%zu e6 header=%zu user=%zu tail=%zu spans=%zu "
                   "e8 header=%zu user=%zu tail=%zu spans=%zu\n",
                   label, anchor,
                   entry6_sectors.mode1_sync_header_byte_count,
                   entry6_sectors.mode1_user_data_byte_count,
                   entry6_sectors.mode1_sector_tail_byte_count,
                   entry6_sectors.span_count,
                   entry8_sectors.mode1_sync_header_byte_count,
                   entry8_sectors.mode1_user_data_byte_count,
                   entry8_sectors.mode1_sector_tail_byte_count,
                   entry8_sectors.span_count);
            printf("INDEX %s anchor=%zu entry6=0x%zx entry8=0x%zx hash=0x%08x\n",
                   label, anchor, entry6->raw_offset, entry8->raw_offset,
                   (unsigned)(entry6->raw_span_hash ^ entry8->raw_span_hash));
        }
    }
}

static void check_container_format(const uint8_t *bytes,
                                   size_t byte_count,
                                   const char *md5_hex,
                                   const char *label,
                                   Theron_Track02OpaqueContainerFormatReceipt *out_format) {
    Theron_Track02SignalStatus status = theron_v1_track02_capture_opaque_container_format(
        bytes, byte_count, md5_hex, out_format);
    size_t i;

    check(status == THERON_TRACK02_SIGNAL_OK && out_format->valid,
          "opaque container transport format captures");
    if (status != THERON_TRACK02_SIGNAL_OK) return;
    for (i = 0u; i < out_format->container_count; ++i) {
        const Theron_Track02OpaqueContainerLocalFormat *format =
            &out_format->containers[i];
        check(format->raw_byte_count == WINDOW_STRIDE &&
                  format->logical_reassembly_required &&
                  format->user_data_byte_count < format->raw_byte_count &&
                  format->header_state == THERON_TRACK02_OPAQUE_CONTAINER_HEADER_NOT_IDENTIFIED &&
                  format->count_state == THERON_TRACK02_OPAQUE_CONTAINER_COUNT_NOT_IDENTIFIED &&
                  format->compression_state == THERON_TRACK02_OPAQUE_CONTAINER_COMPRESSION_NOT_IDENTIFIED &&
                  format->opaque_only && format->promotion_blocked,
              "format stays transport-only with no header count or compression claim");
        printf("FORMAT %s anchor=%zu entry=%zu first-user=+0x%zx user=%zu segments=%zu hash=0x%08x\n",
               label, format->anchor_index, format->descriptor_entry_index,
               format->first_user_data_container_offset,
               format->user_data_byte_count, format->user_data_segment_count,
               (unsigned)format->transport_shape_hash);
    }
}

static void compare_container_formats(
    const Theron_Track02OpaqueContainerFormatReceipt *jp,
    const Theron_Track02OpaqueContainerFormatReceipt *us,
    const char *label) {
    Theron_Track02OpaqueContainerFormatComparisonReceipt comparison;
    const unsigned int expected = (1u << THERON_TRACK02_MAX_NONSTARTUP_CONTAINERS) - 1u;

    check(theron_v1_track02_compare_opaque_container_formats(jp, us, &comparison) ==
              THERON_TRACK02_SIGNAL_OK && comparison.valid &&
              comparison.comparable_container_mask == expected &&
              comparison.transport_matching_container_mask == expected &&
              comparison.logical_reassembly_required_mask == expected &&
              comparison.opaque_only && comparison.promotion_blocked,
          "JP/US agree on all six transport-only container formats");
    printf("FORMAT-COMPARE %s comparable=0x%x transport=0x%x reassembly=0x%x hash=0x%08x\n",
           label, comparison.comparable_container_mask,
           comparison.transport_matching_container_mask,
           comparison.logical_reassembly_required_mask,
           (unsigned)comparison.comparison_hash);
}

static void check_reassembly_boundary(
    const uint8_t *bytes, size_t byte_count, const char *md5_hex,
    const char *label, Theron_Track02OpaqueContainerReassemblyReceipt *out_receipt) {
    Theron_Track02SignalStatus status =
        theron_v1_track02_capture_opaque_container_reassembly_boundary(
            bytes, byte_count, md5_hex, out_receipt);
    size_t i;

    check(status == THERON_TRACK02_SIGNAL_OK && out_receipt->valid,
          "zero-filled logical reassembly boundary captures");
    if (status != THERON_TRACK02_SIGNAL_OK) return;
    for (i = 0u; i < out_receipt->container_count; ++i) {
        const Theron_Track02OpaqueContainerReassemblyBoundary *boundary =
            &out_receipt->containers[i];
        check(boundary->logical_bytes_all_zero &&
                  boundary->header_signature_absent &&
                  boundary->count_signature_absent &&
                  boundary->stride_signature_absent &&
                  boundary->compression_signature_absent,
              "logical reassembly proves only an empty transport boundary");
        printf("REASSEMBLY %s anchor=%zu entry=%zu bytes=%zu segments=%zu hash=0x%08x\n",
               label, boundary->anchor_index, boundary->descriptor_entry_index,
               boundary->reassembled_byte_count, boundary->segment_count,
               (unsigned)boundary->reassembly_shape_hash);
    }
}

static void compare_reassembly_boundaries(
    const Theron_Track02OpaqueContainerReassemblyReceipt *jp,
    const Theron_Track02OpaqueContainerReassemblyReceipt *us, const char *label) {
    Theron_Track02OpaqueContainerReassemblyComparisonReceipt comparison;
    const unsigned int expected =
        (1u << THERON_TRACK02_MAX_NONSTARTUP_CONTAINERS) - 1u;

    check(theron_v1_track02_compare_opaque_container_reassembly_boundaries(
              jp, us, &comparison) == THERON_TRACK02_SIGNAL_OK && comparison.valid &&
              comparison.comparable_container_mask == expected &&
              comparison.zero_filled_container_mask == expected &&
              comparison.matching_reassembly_shape_mask == expected &&
              comparison.opaque_only && comparison.promotion_blocked,
          "JP/US and repeat regions share only zero-filled reassembly evidence");
    printf("REASSEMBLY-COMPARE %s comparable=0x%x zero=0x%x shape=0x%x hash=0x%08x\n",
           label, comparison.comparable_container_mask,
           comparison.zero_filled_container_mask,
           comparison.matching_reassembly_shape_mask,
           (unsigned)comparison.comparison_hash);
}

static int read_file(const char *path, uint8_t **out_bytes, size_t *out_size) {
    FILE *file;
    long size;
    uint8_t *bytes;

    *out_bytes = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)size;
    return 1;
}

static void probe_real_media(const char *env_name,
                             const char *default_file,
                             const char *expected_md5,
                             const char *label,
                             Theron_Track02NonstartupSectorReceipt *out_receipt,
                             int *out_have_receipt,
                             Theron_Track02OpaqueContainerFormatReceipt *out_format,
                             Theron_Track02OpaqueContainerReassemblyReceipt *out_reassembly) {
    char default_path[1024];
    const char *path = getenv(env_name);
    uint8_t *bytes;
    size_t byte_count;
    char md5[33];
    Theron_Track02NonstartupSectorReceipt receipt;
    Theron_Track02SignalStatus status;

    if (!path || !path[0]) {
        const char *home = getenv("HOME");
        snprintf(default_path, sizeof(default_path), "%s%s.firestaff%sdata%s%s",
                 (home && home[0]) ? home : ".", PATH_SEP, PATH_SEP,
                 PATH_SEP, default_file);
        path = default_path;
    }
    if (!read_file(path, &bytes, &byte_count)) {
        printf("SKIP %s media absent: %s\n", label, path);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path, md5) || strcmp(md5, expected_md5) != 0) {
        printf("SKIP %s media MD5 is not the verified Track 02 variant\n", label);
        ++g_skip;
        free(bytes);
        return;
    }
    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        bytes, byte_count, expected_md5, &receipt);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        printf("FAIL %s sector receipt status=%s\n", label,
               theron_v1_track02_signal_status_name(status));
    }
    check(status == THERON_TRACK02_SIGNAL_OK,
          "real media sector receipt captures");
    if (receipt.valid) {
        check_receipt(&receipt, label);
        check_container_index(bytes, byte_count, expected_md5, label);
        check_container_format(bytes, byte_count, expected_md5, label, out_format);
        check_reassembly_boundary(bytes, byte_count, expected_md5, label,
                                  out_reassembly);
    }
    if (receipt.valid && out_receipt && out_have_receipt) {
        *out_receipt = receipt;
        *out_have_receipt = 1;
    }
    free(bytes);
}

static const char *resolve_real_path(const char *env_name,
                                     const char *default_file,
                                     char out_path[1024]) {
    const char *path = getenv(env_name);
    if (path && path[0]) return path;
    {
        const char *home = getenv("HOME");
        snprintf(out_path, 1024, "%s%s.firestaff%sdata%s%s",
                 (home && home[0]) ? home : ".", PATH_SEP, PATH_SEP,
                 PATH_SEP, default_file);
    }
    return out_path;
}

static void probe_repeatable_real_regions(void) {
    char jp_default[1024];
    char us_default[1024];
    const char *jp_path = resolve_real_path(
        "FIRESTAFF_THERON_TRACK02_JP_BIN",
        "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
        jp_default);
    const char *us_path = resolve_real_path(
        "FIRESTAFF_THERON_TRACK02_US_BIN",
        "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
        us_default);
    uint8_t *jp_bytes = NULL;
    uint8_t *us_bytes = NULL;
    size_t jp_size = 0u;
    size_t us_size = 0u;
    char jp_md5[33];
    char us_md5[33];
    Theron_Track02RepeatableRegionCatalog catalog;
    Theron_Track02RepeatableRegionStructuralClusterReceipt cluster;
    Theron_Track02KnownAnchorRegionCrossReferenceReceipt cross_reference;
    Theron_Track02Anchor2Region5FragmentReceipt fragment;
    Theron_Track02Anchor2RepeatCorrelationReceipt repeat_correlation;
    Theron_Track02AnchorRepeatSectorNeighborReceipt neighbor_receipt;
    size_t i;

    if (!read_file(jp_path, &jp_bytes, &jp_size) ||
        !read_file(us_path, &us_bytes, &us_size) ||
        !m12_file_md5_hex(jp_path, jp_md5) ||
        !m12_file_md5_hex(us_path, us_md5) ||
        strcmp(jp_md5, THERON_TRACK02_MD5_JP_BIN) != 0 ||
        strcmp(us_md5, THERON_TRACK02_MD5_US_BIN) != 0) {
        printf("SKIP repeatable region catalog needs both verified raw BINs\n");
        ++g_skip;
        free(jp_bytes);
        free(us_bytes);
        return;
    }
    check(theron_v1_track02_catalog_repeatable_nonstartup_regions(
              jp_bytes, jp_size, jp_md5, us_bytes, us_size, us_md5,
              &catalog) == THERON_TRACK02_SIGNAL_OK && catalog.valid,
          "real JP/US nonstartup region catalog captures");
    check(catalog.verified_track02 && catalog.opaque_only &&
              catalog.promotion_blocked && catalog.region_count == 11u &&
              catalog.rejected_nonrepeatable_run_count > 0u &&
              catalog.catalog_hash != 0u,
          "catalog retains only repeatable regions and negative evidence");
    for (i = 0u; i < catalog.region_count; ++i) {
        const Theron_Track02RepeatableRegion *region = &catalog.regions[i];
        check(region->sector_count >= THERON_TRACK02_MIN_REPEATABLE_REGION_SECTORS &&
                  region->us_first_raw_sector == region->jp_first_raw_sector + 1u &&
                  region->user_data_byte_count == region->sector_count *
                      THERON_TRACK02_RAW_USER_DATA_BYTES &&
                  region->nonzero_user_data_byte_count > 0u &&
                  region->user_data_hash != 0u &&
                  region->excludes_indexed_empty_containers &&
                  region->opaque_only && region->promotion_blocked,
              "repeatable region is bounded, hashed, empty-container-excluded, and opaque");
        printf("REGION jp=%zu..%zu raw=0x%zx us=%zu..%zu raw=0x%zx sectors=%zu "
               "user=%zu nonzero=%zu hash=0x%08x\n",
               region->jp_first_raw_sector, region->jp_last_raw_sector,
               region->jp_raw_offset, region->us_first_raw_sector,
               region->us_last_raw_sector, region->us_raw_offset,
               region->sector_count, region->user_data_byte_count,
               region->nonzero_user_data_byte_count,
               (unsigned)region->user_data_hash);
    }
    check(theron_v1_track02_catalog_repeatable_nonstartup_regions(
              jp_bytes, jp_size, jp_md5, jp_bytes, jp_size, jp_md5,
              &catalog) == THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
              "repeatable catalog rejects same-variant fallback input");
    check(theron_v1_track02_cluster_repeatable_nonstartup_regions(
              jp_bytes, jp_size, jp_md5, us_bytes, us_size, us_md5,
              &cluster) == THERON_TRACK02_SIGNAL_OK && cluster.valid,
          "real JP/US consensus structural cluster captures");
    check(cluster.verified_track02 && cluster.opaque_only &&
              cluster.promotion_blocked &&
              cluster.signature_count ==
                  THERON_TRACK02_CONSENSUS_NONSTARTUP_REGION_COUNT &&
              cluster.correlation_count ==
                  (THERON_TRACK02_CONSENSUS_NONSTARTUP_REGION_COUNT *
                   (THERON_TRACK02_CONSENSUS_NONSTARTUP_REGION_COUNT - 1u)) / 2u &&
              cluster.receipt_hash != 0u,
          "structural cluster is consensus-bound, complete, and opaque");
    for (i = 0u; i < cluster.signature_count; ++i) {
        const Theron_Track02RepeatableRegionStructuralSignature *signature =
            &cluster.signatures[i];
        check(signature->region_index == i &&
                  signature->sector_count >=
                      THERON_TRACK02_MIN_REPEATABLE_REGION_SECTORS &&
                  signature->user_data_byte_count == signature->sector_count *
                      THERON_TRACK02_RAW_USER_DATA_BYTES &&
                  signature->prefix_signature != 0u &&
                  signature->suffix_signature != 0u &&
                  signature->first_sector_signature != 0u &&
                  signature->last_sector_signature != 0u &&
                  signature->leading_zero_sector_boundary &&
                  signature->trailing_zero_sector_boundary,
              "structural signature records byte fingerprints and zero boundaries");
        printf("CLUSTER region=%zu prefix=0x%08x suffix=0x%08x first-sector=0x%08x "
               "last-sector=0x%08x prefix-mask=0x%x sector-mask=0x%x\n",
               signature->region_index, (unsigned)signature->prefix_signature,
               (unsigned)signature->suffix_signature,
               (unsigned)signature->first_sector_signature,
               (unsigned)signature->last_sector_signature,
               signature->matching_prefix_region_mask,
               signature->matching_first_sector_region_mask);
    }
    check(theron_v1_track02_cluster_repeatable_nonstartup_regions(
              jp_bytes, jp_size, jp_md5, jp_bytes, jp_size, jp_md5,
              &cluster) == THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "structural cluster rejects same-variant fallback input");
    check(theron_v1_track02_cross_reference_known_anchors_to_repeatable_regions(
              jp_bytes, jp_size, jp_md5, us_bytes, us_size, us_md5,
              &cross_reference) == THERON_TRACK02_SIGNAL_OK &&
              cross_reference.valid && cross_reference.verified_track02 &&
              cross_reference.opaque_only && cross_reference.promotion_blocked &&
              cross_reference.anchor_count == THERON_TRACK02_MAX_BANK_ANCHORS &&
              cross_reference.post_boundary_region_mask == (1u << 5u) &&
              cross_reference.startup_candidate_region_mask == 0u &&
              cross_reference.receipt_hash != 0u,
          "known anchors cross-reference exact regional bytes without boot promotion");
    check(theron_v1_track02_capture_anchor2_region5_first_user_data_fragment(
              jp_bytes, jp_size, jp_md5, us_bytes, us_size, us_md5,
              &fragment) == THERON_TRACK02_SIGNAL_OK && fragment.valid &&
              fragment.verified_track02 && fragment.opaque_only &&
              fragment.promotion_blocked && fragment.anchor_index == 2u &&
              fragment.region_index == 5u && fragment.byte_count == 44u &&
              fragment.jp_raw_offset == 0x711f10u &&
              fragment.us_raw_offset == 0x712840u &&
              fragment.jp_raw_sector == 3152u && fragment.us_raw_sector == 3153u &&
              fragment.jp_user_data_stream_offset == 0x628000u &&
              fragment.us_user_data_stream_offset == 0x628800u &&
              fragment.region_first_sector_user_data_offset == 0u &&
              fragment.nonzero_byte_count == 43u && fragment.zero_byte_count == 1u &&
              fragment.first_le_word == 0x80beu && fragment.last_le_word == 0x3f00u &&
              fragment.first_16_byte_hash == 0x1d234a41u &&
              fragment.fragment_hash == 0x4cab6ed1u && fragment.exact_jp_signature &&
              fragment.exact_us_signature && fragment.variants_match &&
              fragment.receipt_hash != 0u,
          "anchor-2 region-5 first-user fragment is exact and opaque");
    check(theron_v1_track02_capture_anchor2_repeat_correlation(
              jp_bytes, jp_size, jp_md5, us_bytes, us_size, us_md5,
              &repeat_correlation) == THERON_TRACK02_SIGNAL_OK &&
              repeat_correlation.valid && repeat_correlation.verified_track02 &&
              repeat_correlation.opaque_only &&
              repeat_correlation.promotion_blocked &&
              repeat_correlation.anchor_count == 3u &&
              repeat_correlation.block_byte_count == 2048u &&
              repeat_correlation.half_block_byte_count == 1024u &&
              repeat_correlation.first_nonmatching_byte_offset == 2048u &&
              repeat_correlation.fragment_prefix_byte_count == 4u &&
              repeat_correlation.pair_count == 3u &&
              repeat_correlation.jp_fragment_prefix_match_count == 3u &&
              repeat_correlation.us_fragment_prefix_match_count == 3u &&
              repeat_correlation.jp_raw_offsets[0] == 0x2d4ab0u &&
              repeat_correlation.jp_raw_offsets[1] == 0x47c710u &&
              repeat_correlation.jp_raw_offsets[2] == 0x711f10u &&
              repeat_correlation.us_raw_offsets[0] == 0x2d53e0u &&
              repeat_correlation.us_raw_offsets[1] == 0x47d040u &&
              repeat_correlation.us_raw_offsets[2] == 0x712840u &&
              repeat_correlation.jp_raw_sectors[0] == 1262u &&
              repeat_correlation.jp_raw_sectors[1] == 2000u &&
              repeat_correlation.jp_raw_sectors[2] == 3152u &&
              repeat_correlation.us_raw_sectors[0] == 1263u &&
              repeat_correlation.us_raw_sectors[1] == 2001u &&
              repeat_correlation.us_raw_sectors[2] == 3153u &&
              repeat_correlation.region5_anchor_mask == (1u << 2u) &&
              repeat_correlation.all_offsets_start_at_mode1_user_data &&
              repeat_correlation.all_blocks_within_one_mode1_user_data_sector &&
              repeat_correlation.variants_match &&
              repeat_correlation.jp_post_block_first_mismatch_offsets[0] == 0x800u &&
              repeat_correlation.jp_post_block_first_mismatch_offsets[1] == 0x800u &&
              repeat_correlation.jp_post_block_first_mismatch_offsets[2] == 0x800u &&
              repeat_correlation.us_post_block_first_mismatch_offsets[0] == 0x800u &&
              repeat_correlation.us_post_block_first_mismatch_offsets[1] == 0x800u &&
              repeat_correlation.us_post_block_first_mismatch_offsets[2] == 0x800u &&
              repeat_correlation.jp_first_half_matching_pair_mask == 0x7u &&
              repeat_correlation.us_first_half_matching_pair_mask == 0x7u &&
              repeat_correlation.jp_second_half_matching_pair_mask == 0x7u &&
              repeat_correlation.us_second_half_matching_pair_mask == 0x7u &&
              repeat_correlation.jp_full_block_matching_pair_mask == 0x7u &&
              repeat_correlation.us_full_block_matching_pair_mask == 0x7u &&
              repeat_correlation.shared_first_half_hash != 0u &&
              repeat_correlation.shared_second_half_hash != 0u &&
              repeat_correlation.repeated_user_data_hash == 0xa58bead7u &&
              repeat_correlation.receipt_hash != 0u,
          "anchor-2 fragment half-block correlation is aligned and opaque");
    check(theron_v1_track02_capture_anchor_repeat_sector_neighbors(
              jp_bytes, jp_size, jp_md5, us_bytes, us_size, us_md5,
              &neighbor_receipt) == THERON_TRACK02_SIGNAL_OK &&
              neighbor_receipt.valid && neighbor_receipt.verified_track02 &&
              neighbor_receipt.opaque_only && neighbor_receipt.promotion_blocked &&
              neighbor_receipt.anchor_count == 3u &&
              neighbor_receipt.all_adjacent_sectors_available &&
              neighbor_receipt.jp_preceding_raw_sectors[0] == 1261u &&
              neighbor_receipt.jp_repeated_raw_sectors[1] == 2000u &&
              neighbor_receipt.jp_following_raw_sectors[2] == 3153u &&
              neighbor_receipt.us_preceding_raw_sectors[0] == 1262u &&
              neighbor_receipt.us_repeated_raw_sectors[1] == 2001u &&
              neighbor_receipt.us_following_raw_sectors[2] == 3154u &&
              neighbor_receipt.jp_sector_gaps[0] == 738u &&
              neighbor_receipt.jp_sector_gaps[1] == 1152u &&
              neighbor_receipt.us_sector_gaps[0] == 738u &&
              neighbor_receipt.us_sector_gaps[1] == 1152u &&
              neighbor_receipt.jp_us_sector_displacements[0] == 1u &&
              neighbor_receipt.jp_us_sector_displacements[1] == 1u &&
              neighbor_receipt.jp_us_sector_displacements[2] == 1u &&
              neighbor_receipt.jp_preceding_matches_repeat_mask == 0u &&
              neighbor_receipt.jp_following_matches_repeat_mask == 0u &&
              neighbor_receipt.us_preceding_matches_repeat_mask == 0u &&
              neighbor_receipt.us_following_matches_repeat_mask == 0u &&
              neighbor_receipt.receipt_hash != 0u,
          "anchor repeat neighbors retain only opaque sector geometry");
    printf("REPEAT-NEIGHBORS jp-gap=%zu/%zu us-gap=%zu/%zu displacement=%zu/%zu/%zu "
           "repeat-masks=0x%x/0x%x/0x%x/0x%x hash=0x%08x opaque=%d blocked=%d\n",
           neighbor_receipt.jp_sector_gaps[0], neighbor_receipt.jp_sector_gaps[1],
           neighbor_receipt.us_sector_gaps[0], neighbor_receipt.us_sector_gaps[1],
           neighbor_receipt.jp_us_sector_displacements[0],
           neighbor_receipt.jp_us_sector_displacements[1],
           neighbor_receipt.jp_us_sector_displacements[2],
           neighbor_receipt.jp_preceding_matches_repeat_mask,
           neighbor_receipt.jp_following_matches_repeat_mask,
           neighbor_receipt.us_preceding_matches_repeat_mask,
           neighbor_receipt.us_following_matches_repeat_mask,
           (unsigned)neighbor_receipt.receipt_hash,
           neighbor_receipt.opaque_only, neighbor_receipt.promotion_blocked);
    printf("REPEAT anchors=%zu block=%zu half=%zu first-diff=+0x%zx prefix=%zu "
           "jp-first-diff=+0x%zx/+0x%zx/+0x%zx us-first-diff=+0x%zx/+0x%zx/+0x%zx "
           "first-half=0x%x/0x%x second-half=0x%x/0x%x full=0x%x/0x%x region5=0x%x "
           "hash=0x%08x opaque=%d blocked=%d\n",
           repeat_correlation.anchor_count,
           repeat_correlation.block_byte_count,
           repeat_correlation.half_block_byte_count,
           repeat_correlation.first_nonmatching_byte_offset,
           repeat_correlation.fragment_prefix_byte_count,
           repeat_correlation.jp_post_block_first_mismatch_offsets[0],
           repeat_correlation.jp_post_block_first_mismatch_offsets[1],
           repeat_correlation.jp_post_block_first_mismatch_offsets[2],
           repeat_correlation.us_post_block_first_mismatch_offsets[0],
           repeat_correlation.us_post_block_first_mismatch_offsets[1],
           repeat_correlation.us_post_block_first_mismatch_offsets[2],
           repeat_correlation.jp_first_half_matching_pair_mask,
           repeat_correlation.us_first_half_matching_pair_mask,
           repeat_correlation.jp_second_half_matching_pair_mask,
           repeat_correlation.us_second_half_matching_pair_mask,
           repeat_correlation.jp_full_block_matching_pair_mask,
           repeat_correlation.us_full_block_matching_pair_mask,
           repeat_correlation.region5_anchor_mask,
           (unsigned)repeat_correlation.repeated_user_data_hash,
           repeat_correlation.opaque_only,
           repeat_correlation.promotion_blocked);
    for (i = 0u; i < cross_reference.anchor_count; ++i) {
        const Theron_Track02KnownAnchorRegionCrossReference *reference =
            &cross_reference.anchors[i];
        check(reference->anchor_index == i &&
                  reference->post_boundary_byte_count == 44u &&
                  reference->post_boundary_in_consensus_region == (i == 2u) &&
                  reference->post_boundary_region_index == (i == 2u ? 5u : 0u) &&
                  reference->post_boundary_region_first_raw_sector ==
                      (i == 2u ? 3152u : 0u) &&
                  reference->post_boundary_logical_user_data_offset == 0u &&
                  reference->post_boundary_starts_at_mode1_user_data == (i == 2u) &&
                  reference->post_boundary_within_first_mode1_user_data_sector ==
                      (i == 2u) &&
                  reference->post_boundary_first_sector_user_data_byte_count ==
                      (i == 2u ? 44u : 0u) &&
                  !reference->startup_candidate_in_consensus_region,
              "cross-reference retains only aligned MODE1 user-data geometry");
        printf("CROSSREF anchor=%zu jp-span=0x%zx us-span=0x%zx region=%s%zu "
               "first-sector=%zu user=+0x%zx first-user-bytes=%zu "
               "jp-startup=0x%zx us-startup=0x%zx startup-overlap=%d\n",
               reference->anchor_index,
               reference->jp_post_boundary_raw_offset,
               reference->us_post_boundary_raw_offset,
               reference->post_boundary_in_consensus_region ? "" : "none/",
               reference->post_boundary_region_index,
               reference->post_boundary_region_first_raw_sector,
               reference->post_boundary_logical_user_data_offset,
               reference->post_boundary_first_sector_user_data_byte_count,
               reference->jp_startup_candidate_raw_offset,
               reference->us_startup_candidate_raw_offset,
               reference->startup_candidate_in_consensus_region);
    }
    check(theron_v1_track02_cross_reference_known_anchors_to_repeatable_regions(
              jp_bytes, jp_size, jp_md5, jp_bytes, jp_size, jp_md5,
              &cross_reference) == THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "known-anchor cross-reference rejects same-variant fallback input");
    check(theron_v1_track02_capture_anchor2_region5_first_user_data_fragment(
              jp_bytes, jp_size, jp_md5, jp_bytes, jp_size, jp_md5,
              &fragment) == THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "anchor-2 region-5 fragment rejects same-variant fallback input");
    check(theron_v1_track02_capture_anchor2_repeat_correlation(
              jp_bytes, jp_size, jp_md5, jp_bytes, jp_size, jp_md5,
              &repeat_correlation) == THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "anchor-2 repeat correlation rejects same-variant fallback input");
    {
        uint8_t *jp_missing_first_sector = (uint8_t *)malloc(jp_size);
        uint8_t *us_missing_first_sector = (uint8_t *)malloc(us_size);

        check(jp_missing_first_sector != NULL && us_missing_first_sector != NULL,
              "source-adjacent rejection fixtures allocate");
        if (jp_missing_first_sector && us_missing_first_sector) {
            memcpy(jp_missing_first_sector, jp_bytes, jp_size);
            memcpy(us_missing_first_sector, us_bytes, us_size);
            /* Region 5 begins at JP sector 3152 / US sector 3153. Removing
             * that first logical sector makes the observed 8-sector alignment
             * unavailable; it must not become a different candidate. */
            memset(jp_missing_first_sector + 3152u * THERON_TRACK02_RAW_SECTOR_BYTES +
                       THERON_TRACK02_RAW_USER_DATA_OFFSET,
                   0,
                   THERON_TRACK02_RAW_USER_DATA_BYTES);
            memset(us_missing_first_sector + 3153u * THERON_TRACK02_RAW_SECTOR_BYTES +
                       THERON_TRACK02_RAW_USER_DATA_OFFSET,
                   0,
                   THERON_TRACK02_RAW_USER_DATA_BYTES);
            check(theron_v1_track02_cross_reference_known_anchors_to_repeatable_regions(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_missing_first_sector, us_size, us_md5,
                      &cross_reference) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "known-anchor cross-reference rejects missing source-adjacent MODE1 sector");
            check(theron_v1_track02_capture_anchor2_region5_first_user_data_fragment(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_missing_first_sector, us_size, us_md5,
                      &fragment) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor-2 region-5 fragment rejects missing first user-data sector");
            check(theron_v1_track02_capture_anchor2_repeat_correlation(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_missing_first_sector, us_size, us_md5,
                      &repeat_correlation) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor-2 repeat correlation rejects missing region-5 sector");

            memcpy(jp_missing_first_sector, jp_bytes, jp_size);
            memcpy(jp_missing_first_sector + 1261u * THERON_TRACK02_RAW_SECTOR_BYTES +
                       THERON_TRACK02_RAW_USER_DATA_OFFSET,
                   jp_missing_first_sector + 1262u * THERON_TRACK02_RAW_SECTOR_BYTES +
                       THERON_TRACK02_RAW_USER_DATA_OFFSET,
                   THERON_TRACK02_RAW_USER_DATA_BYTES);
            check(theron_v1_track02_capture_anchor_repeat_sector_neighbors(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_bytes, us_size, us_md5,
                      &neighbor_receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor repeat neighbors reject a preceding repeated sector");

            memcpy(jp_missing_first_sector, jp_bytes, jp_size);
            jp_missing_first_sector[0x711f10u + 21u] ^= 0x01u;
            check(theron_v1_track02_capture_anchor2_region5_first_user_data_fragment(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_bytes, us_size, us_md5,
                      &fragment) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor-2 region-5 fragment rejects one-byte signature mutation");
            check(theron_v1_track02_capture_anchor2_repeat_correlation(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_bytes, us_size, us_md5,
                      &repeat_correlation) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor-2 repeat correlation rejects one-byte mutation");

            memcpy(jp_missing_first_sector, jp_bytes, jp_size);
            memcpy(us_missing_first_sector, us_bytes, us_size);
            /* Change the same second-half byte in both variants. The JP/US
             * block pairing remains equal, so this isolates the required
             * three-anchor half-block correlation rejection. */
            jp_missing_first_sector[0x711f10u + 0x400u] ^= 0x01u;
            us_missing_first_sector[0x712840u + 0x400u] ^= 0x01u;
            check(theron_v1_track02_capture_anchor2_repeat_correlation(
                      jp_missing_first_sector, jp_size, jp_md5,
                      us_missing_first_sector, us_size, us_md5,
                      &repeat_correlation) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor-2 repeat correlation rejects paired second-half drift");
            check(theron_v1_track02_capture_anchor2_region5_first_user_data_fragment(
                      jp_bytes, 0x711f10u + 43u, jp_md5,
                      us_bytes, us_size, us_md5,
                      &fragment) == THERON_TRACK02_SIGNAL_NOT_FOUND,
                  "anchor-2 region-5 fragment rejects truncated final byte");
        }
        free(jp_missing_first_sector);
        free(us_missing_first_sector);
    }
    free(jp_bytes);
    free(us_bytes);
}

int main(void) {
    uint8_t *fixture = (uint8_t *)malloc(RAW_FIXTURE_SIZE);
    Theron_Track02NonstartupSectorReceipt receipt;
    Theron_Track02NonstartupSectorReceipt synthetic_us;
    Theron_Track02NonstartupSectorReceipt synthetic_jp;
    Theron_Track02NonstartupSectorReceipt real_us;
    Theron_Track02NonstartupSectorReceipt real_jp;
    Theron_Track02OpaqueContainerFormatReceipt synthetic_us_format;
    Theron_Track02OpaqueContainerFormatReceipt synthetic_jp_format;
    Theron_Track02OpaqueContainerFormatReceipt real_us_format;
    Theron_Track02OpaqueContainerFormatReceipt real_jp_format;
    Theron_Track02OpaqueContainerReassemblyReceipt real_us_reassembly;
    Theron_Track02OpaqueContainerReassemblyReceipt real_jp_reassembly;
    int have_real_us = 0;
    int have_real_jp = 0;
    Theron_Track02SignalStatus status;

    check(fixture != NULL, "fixture allocation");
    if (!fixture) return 1;
    build_raw_fixture(fixture, RAW_FIXTURE_SIZE, g_us_descriptor_offsets,
                      g_us_span_offsets, 0xa0u);
    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN, &receipt);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        printf("FAIL synthetic sector receipt status=%s\n",
               theron_v1_track02_signal_status_name(status));
    }
    check(status == THERON_TRACK02_SIGNAL_OK,
          "synthetic raw container receipt captures");
    synthetic_us = receipt;
    if (receipt.valid) {
        check_receipt(&receipt, "synthetic-us");
        check(theron_v1_track02_build_nonstartup_container_index(
                  fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN,
                  &(Theron_Track02NonstartupContainerIndex){0}) ==
                  THERON_TRACK02_SIGNAL_NOT_FOUND,
              "container index rejects synthetic nonzero fallback entry");
        clear_entry7_windows(fixture, g_us_descriptor_offsets);
        check_container_index(fixture, RAW_FIXTURE_SIZE,
                              THERON_TRACK02_MD5_US_BIN, "synthetic-us");
        check_container_format(fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN,
                               "synthetic-us", &synthetic_us_format);
        check(theron_v1_track02_capture_opaque_container_reassembly_boundary(
                  fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN,
                  &(Theron_Track02OpaqueContainerReassemblyReceipt){0}) ==
                  THERON_TRACK02_SIGNAL_NOT_FOUND,
              "nonzero logical synthetic payload rejects reassembly prerequisite");
    }
    build_raw_fixture(fixture, RAW_FIXTURE_SIZE, g_jp_descriptor_offsets,
                      g_jp_span_offsets, 0xb0u);
    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_JP_BIN, &receipt);
    check(status == THERON_TRACK02_SIGNAL_OK,
          "synthetic JP raw container receipt captures");
    synthetic_jp = receipt;
    if (receipt.valid) {
        check_receipt(&receipt, "synthetic-jp");
        check(theron_v1_track02_build_nonstartup_container_index(
                  fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_JP_BIN,
                  &(Theron_Track02NonstartupContainerIndex){0}) ==
                  THERON_TRACK02_SIGNAL_NOT_FOUND,
              "container index rejects JP synthetic nonzero fallback entry");
        clear_entry7_windows(fixture, g_jp_descriptor_offsets);
        check_container_index(fixture, RAW_FIXTURE_SIZE,
                              THERON_TRACK02_MD5_JP_BIN, "synthetic-jp");
        check_container_format(fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_JP_BIN,
                               "synthetic-jp", &synthetic_jp_format);
    }
    compare_layouts(&synthetic_jp, &synthetic_us, "synthetic-jp-us");
    compare_container_formats(&synthetic_jp_format, &synthetic_us_format,
                              "synthetic-jp-us");
    check(theron_v1_track02_capture_nonstartup_sector_receipt(
              fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_ISO, &receipt) ==
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "ISO cannot enter raw-sector receipt");
    check(theron_v1_track02_capture_nonstartup_sector_receipt(
              NULL, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN, &receipt) ==
              THERON_TRACK02_SIGNAL_BAD_INPUT,
          "NULL raw container rejects");
    free(fixture);

    probe_real_media("FIRESTAFF_THERON_TRACK02_US_BIN",
                     "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                     THERON_TRACK02_MD5_US_BIN, "real-us", &real_us,
                     &have_real_us, &real_us_format, &real_us_reassembly);
    probe_real_media("FIRESTAFF_THERON_TRACK02_JP_BIN",
                     "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                     THERON_TRACK02_MD5_JP_BIN, "real-jp", &real_jp,
                     &have_real_jp, &real_jp_format, &real_jp_reassembly);
    if (have_real_jp && have_real_us) {
        compare_layouts(&real_jp, &real_us, "real-jp-us");
        compare_container_formats(&real_jp_format, &real_us_format,
                                  "real-jp-us");
        compare_reassembly_boundaries(&real_jp_reassembly, &real_us_reassembly,
                                      "real-jp-us");
    } else {
        printf("SKIP real JP/US opaque layout comparison needs both staged raw Track 02 BINs\n");
        ++g_skip;
    }
    probe_repeatable_real_regions();

    printf("Theron Track 02 non-startup sector receipt probe: %s (%d skipped)\n",
           g_fail == 0 ? "PASS" : "FAIL", g_skip);
    return g_fail == 0 ? 0 : 1;
}
