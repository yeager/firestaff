/*
 * theron_v1_track02.c -- narrow Track 02 evidence helpers.
 *
 * This is not a Theron's Quest dungeon loader.  It locks one small,
 * hash-gated Track 02 bank-descriptor signal so later dungeon-bank work
 * can build from bytes that are regression-proved against real data.
 *
 * Source/evidence:
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md sections 1.3,
 *   1.5, and 4.2 record Track 02 provenance and state that the dungeon
 *   format remains unknown.  The offsets below are from local byte
 *   inspection of the hash-verified JP/US raw Track 02 BINs and the derived
 *   US Track 02 ISO, not from ReDMCSB.
 */

#include "theron_v1_track02.h"

#include <string.h>

#define TQR_US_ISO_BANK_STRIDE_OFFSET 0x1584u
#define TQR_US_ISO_BANK_STRIDE_COUNT  9u
#define TQR_US_ISO_BANK_STRIDE_BYTES  (TQR_US_ISO_BANK_STRIDE_COUNT * 2u)
#define TQR_US_ISO_BANK_STRIDE_STEP   0x0400u
#define TQR_US_ISO_BANK_BOUNDARY_OFFSET 0x3000u
#define TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES 16u
#define TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES 44u
#define TQR_RAW_SECTOR_BYTES 2352u
#define TQR_RAW_SECTOR_USER_DATA_OFFSET 0x10u
#define TQR_RAW_BIN_BANK_ANCHOR_COUNT 3u

static const uint8_t g_us_iso_bank_stride_descriptor[TQR_US_ISO_BANK_STRIDE_BYTES] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static const size_t g_us_bin_descriptor_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};

static const size_t g_jp_bin_descriptor_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static const size_t g_us_bin_post_boundary_span_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x2d53e0u, 0x47d040u, 0x712840u
};

static const size_t g_jp_bin_post_boundary_span_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x2d4ab0u, 0x47c710u, 0x711f10u
};

static const uint8_t g_us_iso_bank_boundary_prefix[TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80
};

static const uint8_t g_us_iso_post_boundary_span[TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
    0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
    0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
    0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
    0x93, 0x80, 0x00, 0x3f
};

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

Theron_Track02Variant theron_v1_track02_variant_for_md5(const char *md5_hex) {
    if (!md5_hex) return THERON_TRACK02_VARIANT_UNKNOWN;
    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) == 0) {
        return THERON_TRACK02_VARIANT_US_BIN;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_JP_BIN) == 0) {
        return THERON_TRACK02_VARIANT_JP_BIN;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_ISO) == 0) {
        return THERON_TRACK02_VARIANT_US_ISO;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_JP_REV1_ISO) == 0) {
        return THERON_TRACK02_VARIANT_JP_REV1_ISO;
    }
    return THERON_TRACK02_VARIANT_UNKNOWN;
}

static int track_is_all_zero(const uint8_t *data, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        if (data[i] != 0) return 0;
    }
    return 1;
}

static int range_is_all_zero(const uint8_t *data, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        if (data[i] != 0) return 0;
    }
    return 1;
}

static void copy_offsets(size_t *dst, const size_t *src, size_t count) {
    size_t i;
    for (i = 0; i < count && i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        dst[i] = src[i];
    }
}

static int pattern_matches_at_offsets(const uint8_t *data,
                                      size_t size,
                                      const uint8_t *pattern,
                                      size_t pattern_size,
                                      const size_t *offsets,
                                      size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) {
        if (offsets[i] > size || pattern_size > size - offsets[i]) {
            return 0;
        }
        if (memcmp(data + offsets[i], pattern, pattern_size) != 0) {
            return 0;
        }
    }
    return 1;
}

static void fill_raw_sector_coordinates(Theron_Track02BankSignal *out_signal,
                                        const size_t *descriptor_offsets,
                                        const size_t *span_offsets,
                                        size_t count) {
    size_t i;

    out_signal->raw_sector_bytes = TQR_RAW_SECTOR_BYTES;
    out_signal->raw_sector_user_data_offset = TQR_RAW_SECTOR_USER_DATA_OFFSET;
    for (i = 0; i < count && i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        const size_t descriptor_remainder = descriptor_offsets[i] % TQR_RAW_SECTOR_BYTES;
        const size_t span_remainder = span_offsets[i] % TQR_RAW_SECTOR_BYTES;

        out_signal->descriptor_raw_sector_numbers[i] =
            descriptor_offsets[i] / TQR_RAW_SECTOR_BYTES;
        out_signal->post_boundary_span_raw_sector_numbers[i] =
            span_offsets[i] / TQR_RAW_SECTOR_BYTES;
        out_signal->descriptor_raw_sector_user_offsets[i] =
            descriptor_remainder >= TQR_RAW_SECTOR_USER_DATA_OFFSET
                ? descriptor_remainder - TQR_RAW_SECTOR_USER_DATA_OFFSET
                : descriptor_remainder;
        out_signal->post_boundary_span_raw_sector_user_offsets[i] =
            span_remainder >= TQR_RAW_SECTOR_USER_DATA_OFFSET
                ? span_remainder - TQR_RAW_SECTOR_USER_DATA_OFFSET
                : span_remainder;
    }
}

static size_t count_pattern_occurrences(const uint8_t *data,
                                        size_t size,
                                        const uint8_t *pattern,
                                        size_t pattern_size) {
    size_t count = 0;
    size_t i;

    if (!data || !pattern || pattern_size == 0 || size < pattern_size) return 0;
    for (i = 0; i <= size - pattern_size; ++i) {
        if (memcmp(data + i, pattern, pattern_size) == 0) {
            ++count;
        }
    }
    return count;
}

static Theron_Track02SignalStatus find_raw_bin_bank_signal(
    const uint8_t *track02_data,
    size_t track02_size,
    Theron_Track02BankSignal *out_signal,
    const size_t *descriptor_offsets,
    const size_t *span_offsets) {

    const size_t occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_bank_stride_descriptor,
                                  TQR_US_ISO_BANK_STRIDE_BYTES);
    const size_t boundary_prefix_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_bank_boundary_prefix,
                                  TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES);
    const size_t post_boundary_span_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_post_boundary_span,
                                  TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES);

    if (!pattern_matches_at_offsets(track02_data,
                                    track02_size,
                                    g_us_iso_bank_stride_descriptor,
                                    TQR_US_ISO_BANK_STRIDE_BYTES,
                                    descriptor_offsets,
                                    TQR_RAW_BIN_BANK_ANCHOR_COUNT)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (!pattern_matches_at_offsets(track02_data,
                                    track02_size,
                                    g_us_iso_post_boundary_span,
                                    TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES,
                                    span_offsets,
                                    TQR_RAW_BIN_BANK_ANCHOR_COUNT)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    out_signal->anchor_count = TQR_RAW_BIN_BANK_ANCHOR_COUNT;
    out_signal->descriptor_offset = descriptor_offsets[0];
    out_signal->descriptor_size = TQR_US_ISO_BANK_STRIDE_BYTES;
    copy_offsets(out_signal->descriptor_offsets,
                 descriptor_offsets,
                 TQR_RAW_BIN_BANK_ANCHOR_COUNT);
    out_signal->occurrence_count = occurrence_count;
    out_signal->first_value = rd16le(g_us_iso_bank_stride_descriptor);
    out_signal->last_value =
        rd16le(g_us_iso_bank_stride_descriptor + TQR_US_ISO_BANK_STRIDE_BYTES - 2u);
    out_signal->stride = TQR_US_ISO_BANK_STRIDE_STEP;
    out_signal->value_count = TQR_US_ISO_BANK_STRIDE_COUNT;
    out_signal->next_nonzero_offset = span_offsets[0];
    out_signal->boundary_prefix_size = TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES;
    out_signal->boundary_prefix_occurrence_count = boundary_prefix_occurrence_count;
    out_signal->post_boundary_span_size = TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES;
    copy_offsets(out_signal->post_boundary_span_offsets,
                 span_offsets,
                 TQR_RAW_BIN_BANK_ANCHOR_COUNT);
    out_signal->post_boundary_span_occurrence_count = post_boundary_span_occurrence_count;
    out_signal->post_boundary_span_first_word = rd16le(g_us_iso_post_boundary_span);
    out_signal->post_boundary_span_last_word =
        rd16le(g_us_iso_post_boundary_span + TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES - 2u);
    fill_raw_sector_coordinates(out_signal,
                                descriptor_offsets,
                                span_offsets,
                                TQR_RAW_BIN_BANK_ANCHOR_COUNT);

    return occurrence_count == TQR_RAW_BIN_BANK_ANCHOR_COUNT &&
           boundary_prefix_occurrence_count == TQR_RAW_BIN_BANK_ANCHOR_COUNT &&
           post_boundary_span_occurrence_count == TQR_RAW_BIN_BANK_ANCHOR_COUNT
        ? THERON_TRACK02_SIGNAL_OK
        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_find_bank_signal(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02BankSignal *out_signal) {

    Theron_Track02Variant variant;
    size_t occurrence_count;
    size_t boundary_prefix_occurrence_count;
    size_t post_boundary_span_occurrence_count;
    const size_t zero_offset = TQR_US_ISO_BANK_STRIDE_OFFSET + TQR_US_ISO_BANK_STRIDE_BYTES;
    const size_t zero_bytes = TQR_US_ISO_BANK_BOUNDARY_OFFSET - zero_offset;

    if (out_signal) {
        memset(out_signal, 0, sizeof(*out_signal));
    }
    if (!track02_data || track02_size == 0 || !md5_hex || !out_signal) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_signal->variant = variant;

    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        return track_is_all_zero(track02_data, track02_size)
            ? THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        return find_raw_bin_bank_signal(track02_data,
                                        track02_size,
                                        out_signal,
                                        g_us_bin_descriptor_offsets,
                                        g_us_bin_post_boundary_span_offsets);
    }
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        return find_raw_bin_bank_signal(track02_data,
                                        track02_size,
                                        out_signal,
                                        g_jp_bin_descriptor_offsets,
                                        g_jp_bin_post_boundary_span_offsets);
    }
    if (variant != THERON_TRACK02_VARIANT_US_ISO) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    if (track02_size < TQR_US_ISO_BANK_BOUNDARY_OFFSET + TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_STRIDE_OFFSET,
               g_us_iso_bank_stride_descriptor,
               TQR_US_ISO_BANK_STRIDE_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (!range_is_all_zero(track02_data + zero_offset, zero_bytes)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_BOUNDARY_OFFSET,
               g_us_iso_bank_boundary_prefix,
               TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_BOUNDARY_OFFSET,
               g_us_iso_post_boundary_span,
               TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    occurrence_count = count_pattern_occurrences(track02_data,
                                                 track02_size,
                                                 g_us_iso_bank_stride_descriptor,
                                                 TQR_US_ISO_BANK_STRIDE_BYTES);
    boundary_prefix_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_bank_boundary_prefix,
                                  TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES);
    post_boundary_span_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_post_boundary_span,
                                  TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES);
    out_signal->descriptor_offset = TQR_US_ISO_BANK_STRIDE_OFFSET;
    out_signal->descriptor_size = TQR_US_ISO_BANK_STRIDE_BYTES;
    out_signal->anchor_count = 1u;
    out_signal->descriptor_offsets[0] = TQR_US_ISO_BANK_STRIDE_OFFSET;
    out_signal->occurrence_count = occurrence_count;
    out_signal->first_value = rd16le(g_us_iso_bank_stride_descriptor);
    out_signal->last_value =
        rd16le(g_us_iso_bank_stride_descriptor + TQR_US_ISO_BANK_STRIDE_BYTES - 2u);
    out_signal->stride = TQR_US_ISO_BANK_STRIDE_STEP;
    out_signal->value_count = TQR_US_ISO_BANK_STRIDE_COUNT;
    out_signal->post_descriptor_zero_offset = zero_offset;
    out_signal->post_descriptor_zero_bytes = zero_bytes;
    out_signal->next_nonzero_offset = TQR_US_ISO_BANK_BOUNDARY_OFFSET;
    out_signal->boundary_prefix_size = TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES;
    out_signal->boundary_prefix_occurrence_count = boundary_prefix_occurrence_count;
    out_signal->post_boundary_span_size = TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES;
    out_signal->post_boundary_span_offsets[0] = TQR_US_ISO_BANK_BOUNDARY_OFFSET;
    out_signal->post_boundary_span_occurrence_count = post_boundary_span_occurrence_count;
    out_signal->post_boundary_span_first_word = rd16le(g_us_iso_post_boundary_span);
    out_signal->post_boundary_span_last_word =
        rd16le(g_us_iso_post_boundary_span + TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES - 2u);

    return occurrence_count == 1u &&
           boundary_prefix_occurrence_count == 1u &&
           post_boundary_span_occurrence_count == 1u
        ? THERON_TRACK02_SIGNAL_OK
        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

const char *theron_v1_track02_signal_status_name(Theron_Track02SignalStatus status) {
    switch (status) {
    case THERON_TRACK02_SIGNAL_OK:
        return "ok";
    case THERON_TRACK02_SIGNAL_NOT_FOUND:
        return "not-found";
    case THERON_TRACK02_SIGNAL_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT:
        return "unsupported-variant";
    case THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE:
        return "insufficient-zero-image";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_variant_name(Theron_Track02Variant variant) {
    switch (variant) {
    case THERON_TRACK02_VARIANT_JP_BIN:
        return "jp-bin";
    case THERON_TRACK02_VARIANT_US_BIN:
        return "us-bin";
    case THERON_TRACK02_VARIANT_JP_REV1_ISO:
        return "jp-rev1-iso";
    case THERON_TRACK02_VARIANT_US_ISO:
        return "us-iso";
    case THERON_TRACK02_VARIANT_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_source_evidence(void) {
    return "theron_v1_track02.c: US Track 02 ISO MD5 "
           THERON_TRACK02_MD5_US_ISO
           " has a unique little-endian bank-stride descriptor at offset "
           "0x1584 (9 words, 0x0020..0x2020, stride 0x0400), followed by "
           "zero-fill through a unique opaque 44-byte boundary span at offset "
           "0x3000; raw US Track 02 BIN "
           THERON_TRACK02_MD5_US_BIN
           " carries the same descriptor at offsets 0x70be06, 0x70e2c6, "
           "0x710904 and the same opaque span at offsets 0x2d53e0, "
           "0x47d040, 0x712840; raw JP Track 02 BIN "
           THERON_TRACK02_MD5_JP_BIN
           " carries the same anchors exactly one 2352-byte raw CD sector "
           "earlier at descriptor offsets 0x70b4d6, 0x70d996, 0x70ffd4 "
           "and span offsets 0x2d4ab0, 0x47c710, 0x711f10; JP Rev 1 ISO "
           THERON_TRACK02_MD5_JP_REV1_ISO
           " is hash-verified but zero-filled in the available image, so no "
           "JP Rev 1 ISO dungeon-bank offset is claimed.";
}
