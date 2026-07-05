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
#define TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR 5u
#define TQR_US_ISO_BANK_BOUNDARY_OFFSET 0x3000u
#define TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES 16u
#define TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES 44u
#define TQR_RAW_SECTOR_BYTES 2352u
#define TQR_RAW_SECTOR_USER_DATA_OFFSET 0x10u
#define TQR_RAW_BIN_BANK_ANCHOR_COUNT 3u
#define TQR_RAW_INITIAL_LEVEL_WIDTH 32u
#define TQR_RAW_INITIAL_LEVEL_HEIGHT 27u
#define TQR_RAW_INITIAL_LEVEL_SEED 0x0108e938u
#define TQR_RAW_INITIAL_LEVEL_INDEX 0x0026u
#define TQR_RAW_INITIAL_LEVEL_DESCRIPTOR_DELTA \
    (TQR_US_ISO_BANK_STRIDE_OFFSET + 0x92ceu)

/* Audio-bank marker fingerprint (raw Track 02 BIN only).
 *
 * The 16-byte prefix immediately preceding each post-boundary span is
 * exactly:
 *   bytes 0..11   = 0x00, 0xff*10, 0x00  (12-byte sentinel)
 *   bytes 12..15  = a 4-byte little-endian audio-bank id word
 *
 * The sentinel prefix itself appears in many places (sector-index table),
 * but the (sentinel + 4-byte LE word + 44-byte post-boundary span) tuple
 * has been observed to occur exactly once per anchor in raw US and JP
 * Track 02 BINs at the offsets documented in
 * theron_v1_track02_source_evidence().  The 4-byte LE word therefore
 * acts as a per-anchor audio-bank-id marker.
 *
 * Source/evidence: docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 * §10.2 marks the ADPCM data block location as STUB; this marker is one
 * ADPCM-bank anchor candidate.  Bytes inspected locally from
 *   THERON_TRACK02_MD5_US_BIN  (f23601102138f87c33025877767ebf76)
 *   THERON_TRACK02_MD5_JP_BIN  (b7afb338ad31be1025b53f9aff12d73a)
 * via a 2352-byte CD-sector pointer scan.
 */
#define TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES 12u
#define TQR_RAW_BIN_AUDIO_BANK_ID_BYTES      4u

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

/* 12-byte sentinel that immediately precedes the 4-byte audio-bank id word
 * (and therefore the post-boundary span) at every audio-bank anchor in raw
 * US/JP Track 02 BINs.  See TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES. */
static const uint8_t g_audio_bank_prefix[TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES] = {
    0x00,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0x00
};

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

/* Read and validate the audio-bank marker at one raw-BIN anchor.
 *
 * anchor_index selects which of (descriptor_offsets, span_offsets) to use;
 * the function verifies that the 16-byte audio-bank prefix
 * (12-byte sentinel + 4-byte LE word) immediately precedes the
 * post-boundary span at that anchor.
 *
 * Returns 1 on success (out_* populated), 0 otherwise.  Out-args are
 * always zeroed on failure so callers can rely on default-zero state. */
static int read_audio_bank_marker(const uint8_t *track02_data,
                                  size_t track02_size,
                                  const size_t *span_offsets,
                                  size_t anchor_index,
                                  uint32_t *out_audio_bank_id,
                                  size_t *out_audio_bank_id_offset,
                                  size_t *out_audio_bank_prefix_offset) {
    const size_t prefix_total_bytes =
        TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES + TQR_RAW_BIN_AUDIO_BANK_ID_BYTES;
    size_t span_offset;
    size_t prefix_offset;
    size_t id_offset;

    if (out_audio_bank_id) *out_audio_bank_id = 0u;
    if (out_audio_bank_id_offset) *out_audio_bank_id_offset = 0u;
    if (out_audio_bank_prefix_offset) *out_audio_bank_prefix_offset = 0u;

    if (!track02_data || !span_offsets ||
        anchor_index >= TQR_RAW_BIN_BANK_ANCHOR_COUNT ||
        prefix_total_bytes > track02_size) {
        return 0;
    }

    span_offset = span_offsets[anchor_index];
    if (span_offset < prefix_total_bytes ||
        span_offset > track02_size ||
        TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES >
            track02_size - span_offset) {
        return 0;
    }

    id_offset = span_offset - TQR_RAW_BIN_AUDIO_BANK_ID_BYTES;
    prefix_offset = id_offset - TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES;

    if (memcmp(track02_data + prefix_offset,
               g_audio_bank_prefix,
               TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES) != 0) {
        return 0;
    }

    if (out_audio_bank_id) *out_audio_bank_id = rd32le(track02_data + id_offset);
    if (out_audio_bank_id_offset) *out_audio_bank_id_offset = id_offset;
    if (out_audio_bank_prefix_offset) *out_audio_bank_prefix_offset = prefix_offset;
    return 1;
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

    /* Per-anchor audio-bank marker: 4-byte LE word that immediately
     * precedes the post-boundary span.  Read independently for each
     * anchor; failure on any one anchor is recorded in the per-anchor
     * recognized[] flag rather than failing the overall signal. */
    for (size_t i = 0; i < TQR_RAW_BIN_BANK_ANCHOR_COUNT; ++i) {
        out_signal->audio_bank_id_recognized[i] = read_audio_bank_marker(
            track02_data,
            track02_size,
            span_offsets,
            i,
            &out_signal->audio_bank_id[i],
            &out_signal->audio_bank_id_offsets[i],
            &out_signal->audio_bank_prefix_offsets[i]);
    }

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

Theron_Track02SignalStatus theron_v1_track02_find_audio_bank_marker(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t anchor_index,
    uint32_t *out_audio_bank_id,
    size_t *out_audio_bank_id_offset,
    size_t *out_audio_bank_prefix_offset) {
    Theron_Track02Variant variant;

    if (out_audio_bank_id) *out_audio_bank_id = 0u;
    if (out_audio_bank_id_offset) *out_audio_bank_id_offset = 0u;
    if (out_audio_bank_prefix_offset) *out_audio_bank_prefix_offset = 0u;

    if (!track02_data || track02_size == 0 || !md5_hex) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (anchor_index >= TQR_RAW_BIN_BANK_ANCHOR_COUNT) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        return read_audio_bank_marker(track02_data,
                                      track02_size,
                                      g_us_bin_post_boundary_span_offsets,
                                      anchor_index,
                                      out_audio_bank_id,
                                      out_audio_bank_id_offset,
                                      out_audio_bank_prefix_offset)
            ? THERON_TRACK02_SIGNAL_OK
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        return read_audio_bank_marker(track02_data,
                                      track02_size,
                                      g_jp_bin_post_boundary_span_offsets,
                                      anchor_index,
                                      out_audio_bank_id,
                                      out_audio_bank_id_offset,
                                      out_audio_bank_prefix_offset)
            ? THERON_TRACK02_SIGNAL_OK
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* US ISO is a partial extract and JP Rev 1 ISO is zero-filled; both
     * lack the post-boundary span audio-bank anchors, so the marker is
     * unsupported on those variants rather than missing. */
    return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
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
           "JP Rev 1 ISO dungeon-bank offset is claimed.  Audio-bank marker: "
           "raw US/JP BINs each carry a 12-byte `00 ff*10 00` sentinel "
           "immediately preceding the 4-byte little-endian audio-bank id word "
           "at every post-boundary span anchor; US ids are 0x01725800, "
           "0x01600801, 0x01122401 at offsets 0x2d53dc, 0x47d03c, "
           "0x71283c; JP ids are 0x01530301, 0x01411301, 0x01682801 at "
           "offsets 0x2d4aac, 0x47c70c, 0x711f0c.  Initial level candidate: "
           "a hash-gated Track 02 scan finds exactly one loader-compatible "
           "32x27 startup payload with seed 0x0108e938 and level index 0x0026 "
           "in each raw image: US offset 0x7015b4 and JP offset 0x700c84.  "
           "This is a bounded initial-level handoff, not a full dungeon-record "
           "decoder, object-table decoder, ADPCM decode, CD-DA decode, or "
           "runtime playback proof.";
}

/* ── Semantic dungeon-descriptor table decoder ──────────────────── */

/* Decode the documented 9-word little-endian stride table at the supplied
 * bytes.  Returns THERON_TRACK02_TABLE_DECODE_OK on success.
 *
 * Shape locked (see theron_v1_track02.h for the source citation):
 *   - 9 little-endian uint16 entries
 *   - strictly ascending (entries[i+1] > entries[i])
 *   - constant stride 0x0400 between adjacent entries
 *   - all entries + stride land in the closed range [0x0020, 0x2020 + 0x0400)
 *
 * The decoder is independent of any single offset: callers pass the raw
 * bytes, not an offset into Track 02.  This keeps the function
 * regression-testable from synthetic fixtures and from real Track 02 data
 * via theron_v1_track02_find_bank_signal(). */
Theron_Track02TableDecodeStatus theron_v1_track02_decode_descriptor_table(
    const uint8_t *descriptor_bytes,
    size_t descriptor_size,
    uint16_t expected_stride,
    Theron_Track02DescriptorTable *out_table) {

    const size_t required_bytes = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES * 2u;
    size_t i;

    if (out_table) {
        memset(out_table, 0, sizeof(*out_table));
    }
    if (!descriptor_bytes || !out_table || descriptor_size < required_bytes) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (expected_stride == 0u) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        out_table->entries[i] = rd16le(descriptor_bytes + (i * 2u));
    }
    out_table->entry_count = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES;
    out_table->first_value = out_table->entries[0];
    out_table->last_value =
        out_table->entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES - 1u];
    out_table->stride = expected_stride;
    out_table->exclusive_upper_bound =
        (uint16_t)(out_table->last_value + expected_stride);

    /* Strictly ascending: every adjacent pair must increase. */
    for (i = 0; i + 1u < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        if (out_table->entries[i + 1u] <= out_table->entries[i]) {
            return THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING;
        }
    }

    /* Constant stride: every adjacent difference must equal expected_stride. */
    for (i = 0; i + 1u < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        const uint16_t diff =
            (uint16_t)(out_table->entries[i + 1u] - out_table->entries[i]);
        if (diff != expected_stride) {
            return THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE;
        }
    }

    /* Range sanity: documented shape is [0x0020, 0x2020 + 0x0400) inclusive
     * on both ends (the last entry is 0x2020; the stride window ends at
     * 0x2020 + 0x0400 == 0x2420, exclusive).  We also accept any strictly
     * ascending 9-word stride sequence whose first value, last value, and
     * exclusive upper bound all fit in 16 bits -- but require the range to
     * stay sane (exclusive_upper_bound > last_value; first_value > 0) so
     * empty-range and zero-origin decoders are rejected. */
    if (out_table->first_value == 0u) {
        return THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING;
    }
    if (out_table->exclusive_upper_bound <= out_table->last_value) {
        return THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE;
    }

    /* The documented 0x0020..0x2020 inclusive + 0x0400 stride window. */
    {
        const uint16_t lo = 0x0020u;
        const uint16_t hi_inclusive = (uint16_t)(0x2020u + expected_stride);
        out_table->range_inclusive =
            (out_table->first_value >= lo) &&
            (out_table->exclusive_upper_bound <= hi_inclusive) ? 1 : 0;
    }

    return THERON_TRACK02_TABLE_DECODE_OK;
}

Theron_Track02TableDecodeStatus theron_v1_track02_bind_descriptor_windows(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    const Theron_Track02DescriptorTable *table,
    Theron_Track02DescriptorWindowBinding *out_binding) {

    size_t base_offset;
    size_t i;

    if (out_binding) {
        memset(out_binding, 0, sizeof(*out_binding));
    }
    if (!track02_data || track02_size == 0 || !table || !out_binding) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (table->entry_count != THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES ||
        table->stride == 0u) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (descriptor_offset < TQR_US_ISO_BANK_STRIDE_OFFSET ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size ||
        descriptor_offset > track02_size - TQR_US_ISO_BANK_STRIDE_BYTES) {
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    /* The source-locked descriptor offset is 0x1584 bytes into the
     * descriptor region.  ReDMCSB has no Theron's Quest Track 02 loader;
     * docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md records
     * this byte anchor for the US ISO and JP/US raw BIN replicas. */
    base_offset = descriptor_offset - TQR_US_ISO_BANK_STRIDE_OFFSET;

    out_binding->entry_count = table->entry_count;
    out_binding->base_offset = base_offset;
    out_binding->descriptor_offset = descriptor_offset;
    out_binding->window_size = table->stride;

    for (i = 0; i < table->entry_count; ++i) {
        Theron_Track02DescriptorWindow *window = &out_binding->windows[i];
        const size_t absolute_offset = base_offset + (size_t)table->entries[i];
        const size_t window_size = (size_t)table->stride;
        size_t j;
        int saw_nonzero = 0;

        if (absolute_offset < base_offset ||
            absolute_offset > track02_size ||
            window_size > track02_size - absolute_offset) {
            memset(out_binding, 0, sizeof(*out_binding));
            return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
        }

        window->entry_index = i;
        window->relative_offset = table->entries[i];
        window->absolute_offset = absolute_offset;
        window->byte_count = window_size;

        for (j = 0; j < window_size; ++j) {
            if (track02_data[absolute_offset + j] != 0u) {
                if (!saw_nonzero) {
                    window->first_nonzero_offset = absolute_offset + j;
                    saw_nonzero = 1;
                }
                window->last_nonzero_offset = absolute_offset + j;
                ++window->nonzero_byte_count;
            }
        }

        window->contains_descriptor_table =
            descriptor_offset >= absolute_offset &&
            descriptor_offset <= (absolute_offset + window_size) &&
            TQR_US_ISO_BANK_STRIDE_BYTES <=
                (absolute_offset + window_size) - descriptor_offset;
        if (window->contains_descriptor_table) {
            window->kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE;
        } else if (window->nonzero_byte_count == 0u) {
            window->kind = THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL;
        } else {
            window->kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
        }
    }

    if (!out_binding
             ->windows[TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR]
             .contains_descriptor_table) {
        memset(out_binding, 0, sizeof(*out_binding));
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    return THERON_TRACK02_TABLE_DECODE_OK;
}

/* ── Semantic role binding for descriptor table entries ──────────────── */

/* Look up the entry_index of the descriptor-window in a semantic-binding
 * array.  Returns -1 when no entry has is_descriptor_window set. */
int theron_v1_track02_find_descriptor_window_entry_index(
    const Theron_Track02DescriptorEntrySemanticBinding *entries,
    size_t entry_count) {
    size_t i;

    if (!entries || entry_count == 0u) return -1;
    for (i = 0; i < entry_count; ++i) {
        if (entries[i].is_descriptor_window) {
            return (int)i;
        }
    }
    return -1;
}

const char *theron_v1_track02_descriptor_entry_role_name(
    Theron_Track02DescriptorEntryRole role) {
    switch (role) {
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL:
        return "reserved-zero-fill";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE:
        return "contains-descriptor-table";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA:
        return "pre-descriptor-data";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA:
        return "post-descriptor-data";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_UNKNOWN:
    default:
        return "unknown";
    }
}

/* Bind semantic roles to every entry of a decoded descriptor table.
 *
 * This is a deterministic byte-level binding derived from the existing
 * window classification plus three descriptor-window markers:
 *   - descriptor_at_window_tail: descriptor_offset + 18 ==
 *     absolute_offset + byte_count (descriptor occupies the last 18
 *     bytes of its window).  Observed in the US Track 02 ISO and
 *     hash-verified JP raw BIN anchor 0; the descriptor sits at the
 *     tail of an RTS-terminated code region in both cases.
 *   - byte_before_descriptor_is_rts: track02_data[descriptor_offset - 1]
 *     == 0x60 (HuC6280 / 65C02-derivative RTS opcode).
 *   - all_zero_after_descriptor: every byte after the descriptor within
 *     its 0x0400-byte window is zero.
 *
 * The function is shape-driven, not magic-number-driven: it does not
 * look up specific byte sequences, only derives positions and counts
 * from the supplied descriptor_offset and the windows already classified
 * by theron_v1_track02_bind_descriptor_windows().
 *
 * Bounded non-claim: this binding does not interpret the bytes as code,
 * graphics, palette, text, or any data-domain payload.  It only pins
 * byte-shape relationships around the descriptor.
 */
Theron_Track02TableDecodeStatus theron_v1_track02_bind_descriptor_entry_roles(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    const Theron_Track02DescriptorTable *table,
    Theron_Track02DescriptorEntrySemanticBinding *out_entries) {

    Theron_Track02DescriptorWindowBinding windows;
    Theron_Track02TableDecodeStatus status;
    int descriptor_window_index = -1;
    size_t i;

    if (out_entries) {
        memset(out_entries, 0,
               sizeof(*out_entries) * THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    }
    if (!track02_data || track02_size == 0 || !table || !out_entries) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (table->entry_count != THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES ||
        table->stride == 0u) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    /* Reuse the existing window classification.  This keeps the role
     * binding consistent with the byte-level kind flags already shipped
     * by theron_v1_track02_bind_descriptor_windows(). */
    status = theron_v1_track02_bind_descriptor_windows(
        track02_data,
        track02_size,
        descriptor_offset,
        table,
        &windows);
    if (status != THERON_TRACK02_TABLE_DECODE_OK) {
        return status;
    }

    /* Locate the descriptor-window entry index.  This index drives
     * PRE_DESCRIPTOR_DATA / POST_DESCRIPTOR_DATA classification for
     * the other 8 entries. */
    for (i = 0; i < windows.entry_count; ++i) {
        if (windows.windows[i].contains_descriptor_table) {
            descriptor_window_index = (int)i;
            break;
        }
    }
    if (descriptor_window_index < 0) {
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    for (i = 0; i < table->entry_count; ++i) {
        const Theron_Track02DescriptorWindow *window = &windows.windows[i];
        Theron_Track02DescriptorEntrySemanticBinding *entry = &out_entries[i];

        entry->entry_index = i;
        entry->relative_offset = window->relative_offset;
        entry->absolute_offset = window->absolute_offset;
        entry->byte_count = window->byte_count;
        entry->is_descriptor_window = window->contains_descriptor_table ? 1 : 0;

        /* Order the role assignment deterministically.  Descriptor-window
         * check must come first because the descriptor-window is the
         * reference point for PRE/POST classification. */
        if (entry->is_descriptor_window) {
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE;
        } else if (window->nonzero_byte_count == 0u) {
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL;
        } else if ((int)i < descriptor_window_index) {
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA;
        } else {
            /* i > descriptor_window_index is the only remaining case.
             * Equal-to is excluded by the is_descriptor_window branch. */
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA;
        }

        if (entry->is_descriptor_window) {
            /* Bounded byte-tail markers for the descriptor-window:
             *   - byte_before_descriptor_is_rts
             *   - all_zero_after_descriptor
             *   - first_nonzero_after_descriptor (0 when all-zero)
             *
             * These markers do not interpret the bytes; they only
             * describe their relative position around the descriptor. */
            if (descriptor_offset > 0u &&
                descriptor_offset <= track02_size) {
                entry->byte_before_descriptor =
                    track02_data[descriptor_offset - 1u];
                entry->byte_before_descriptor_is_rts =
                    (entry->byte_before_descriptor == 0x60u) ? 1 : 0;
            }
            {
                const size_t after_offset =
                    descriptor_offset + TQR_US_ISO_BANK_STRIDE_BYTES;
                size_t j;
                int saw_nonzero_after = 0;
                if (after_offset <= track02_size &&
                    (entry->absolute_offset + entry->byte_count) <=
                        track02_size) {
                    const size_t after_end =
                        entry->absolute_offset + entry->byte_count;
                    for (j = after_offset; j < after_end; ++j) {
                        if (track02_data[j] != 0u) {
                            entry->first_nonzero_after_descriptor = j;
                            saw_nonzero_after = 1;
                            break;
                        }
                    }
                }
                entry->all_zero_after_descriptor = saw_nonzero_after ? 0 : 1;
            }
        }
    }

    return THERON_TRACK02_TABLE_DECODE_OK;
}

static uint16_t rd16be(const uint8_t *p) {
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

static uint32_t rd32be(const uint8_t *p) {
    return ((uint32_t)rd16be(p) << 16) | rd16be(p + 2);
}

static int tqr_square_is_passable(uint8_t square) {
    return square != THERON_SQUARE_WALL && square != THERON_SQUARE_SECRET;
}

static void choose_initial_level_start_pose(Theron_V1_Level *level) {
    static const int dirs[4] = {1, 2, 3, 0}; /* prefer E/S/W/N for visible corridor entry */
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int y;
    int x;

    if (!level || level->width <= 2 || level->height <= 2) {
        return;
    }

    for (y = 1; y + 1 < level->height; ++y) {
        for (x = 1; x + 1 < level->width; ++x) {
            int di;
            if (level->squares[y][x] != THERON_SQUARE_FLOOR) {
                continue;
            }
            for (di = 0; di < 4; ++di) {
                int dir = dirs[di];
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                if (nx >= 0 && nx < level->width &&
                    ny >= 0 && ny < level->height &&
                    tqr_square_is_passable(level->squares[ny][nx])) {
                    level->start_x = (int16_t)x;
                    level->start_y = (int16_t)y;
                    level->start_dir = dir;
                    return;
                }
            }
        }
    }
}

static int tqr_level_candidate_header_matches(const uint8_t *bytes,
                                               size_t available,
                                               uint16_t *out_width,
                                               uint16_t *out_height,
                                               size_t *out_payload_size) {
    uint16_t width;
    uint16_t height;
    size_t grid_size;

    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (out_payload_size) *out_payload_size = 0u;
    if (!bytes || available < 12u) {
        return 0;
    }

    width = rd16be(bytes + 0);
    height = rd16be(bytes + 2);
    if (width != TQR_RAW_INITIAL_LEVEL_WIDTH ||
        height != TQR_RAW_INITIAL_LEVEL_HEIGHT ||
        rd32be(bytes + 4) != TQR_RAW_INITIAL_LEVEL_SEED ||
        rd16be(bytes + 8) != TQR_RAW_INITIAL_LEVEL_INDEX) {
        return 0;
    }

    grid_size = (size_t)width * (size_t)height;
    if (grid_size > available - 12u) {
        return 0;
    }

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    if (out_payload_size) *out_payload_size = 12u + grid_size;
    return 1;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_scan_level_candidates(
    const uint8_t *track02_data,
    size_t track02_size,
    Theron_Track02LevelCandidateCatalog *out_catalog) {

    size_t offset;

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
    }
    if (!track02_data || track02_size < 12u || !out_catalog) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    out_catalog->scanned_bytes = track02_size;
    for (offset = 0; offset + 12u <= track02_size; ++offset) {
        uint16_t width = 0u;
        uint16_t height = 0u;
        size_t payload_size = 0u;
        Theron_V1_Level level;
        Theron_MapLoadResult map_status;
        Theron_Track02LevelCandidate *candidate;

        if (!tqr_level_candidate_header_matches(track02_data + offset,
                                                track02_size - offset,
                                                &width,
                                                &height,
                                                &payload_size)) {
            continue;
        }

        map_status = theron_v1_level_load(&level,
                                          track02_data + offset,
                                          (int)payload_size,
                                          THERON_DUNGEON_1_HALL_OF_RECORDS,
                                          (int)out_catalog->candidate_count);
        if (map_status != THERON_MAP_OK) {
            continue;
        }

        if (out_catalog->candidate_count >=
            THERON_TRACK02_MAX_LEVEL_CANDIDATES) {
            ++out_catalog->overflow_count;
            continue;
        }

        candidate =
            &out_catalog->candidates[out_catalog->candidate_count++];
        candidate->absolute_offset = offset;
        candidate->byte_count = payload_size;
        candidate->header_width = width;
        candidate->header_height = height;
        candidate->header_seed = rd32be(track02_data + offset + 4u);
        candidate->header_level_index = rd16be(track02_data + offset + 8u);
        candidate->map_status = map_status;
        candidate->start_x = level.start_x;
        candidate->start_y = level.start_y;
        candidate->start_dir = level.start_dir;
        candidate->loaded = 1;
    }

    return out_catalog->candidate_count > 0u
        ? THERON_TRACK02_LEVEL_HANDOFF_OK
        : THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
}

int theron_v1_track02_bind_level_candidate_anchor(
    size_t descriptor_offset,
    Theron_Track02LevelCandidateCatalog *catalog) {

    size_t expected_offset = 0u;
    int expected_ok;

    if (!catalog) {
        return 0;
    }

    expected_ok = theron_v1_track02_initial_candidate_expected_offset(
        descriptor_offset,
        &expected_offset);
    for (size_t i = 0; i < catalog->candidate_count; ++i) {
        Theron_Track02LevelCandidate *candidate = &catalog->candidates[i];
        candidate->descriptor_delta = 0u;
        candidate->matches_initial_anchor = 0;
        if (descriptor_offset >= candidate->absolute_offset) {
            candidate->descriptor_delta =
                descriptor_offset - candidate->absolute_offset;
        }
        if (expected_ok && candidate->absolute_offset == expected_offset) {
            candidate->matches_initial_anchor = 1;
        }
    }
    return 1;
}

int theron_v1_track02_initial_candidate_expected_offset(
    size_t descriptor_offset,
    size_t *out_candidate_offset) {

    if (out_candidate_offset) {
        *out_candidate_offset = 0u;
    }
    if (descriptor_offset < TQR_RAW_INITIAL_LEVEL_DESCRIPTOR_DELTA) {
        return 0;
    }
    if (out_candidate_offset) {
        *out_candidate_offset =
            descriptor_offset - TQR_RAW_INITIAL_LEVEL_DESCRIPTOR_DELTA;
    }
    return 1;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_bind_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    Theron_Track02InitialCandidateBinding *out_binding) {

    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus table_status;
    Theron_Track02LevelCandidateCatalog catalog;
    size_t expected_candidate_offset = 0u;
    int expected_ok;
    Theron_Track02LevelHandoffStatus status =
        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;

    if (out_binding) {
        memset(out_binding, 0, sizeof(*out_binding));
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
        out_binding->candidate_index = (size_t)-1;
    }

    if (!track02_data || track02_size == 0u || !out_binding) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    out_binding->descriptor_offset = descriptor_offset;
    if (descriptor_offset < TQR_US_ISO_BANK_STRIDE_OFFSET ||
        descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
        return out_binding->status;
    }

    table_status = theron_v1_track02_decode_descriptor_table(
        track02_data + descriptor_offset,
        TQR_US_ISO_BANK_STRIDE_BYTES,
        TQR_US_ISO_BANK_STRIDE_STEP,
        &table);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
        return out_binding->status;
    }

    status = theron_v1_track02_scan_level_candidates(track02_data,
                                                     track02_size,
                                                     &catalog);
    out_binding->candidate_count = catalog.candidate_count;
    if (status != THERON_TRACK02_LEVEL_HANDOFF_OK) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }
    if (catalog.candidate_count > 1u) {
        out_binding->status =
            THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES;
        return out_binding->status;
    }
    if (catalog.candidate_count != 1u) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }

    theron_v1_track02_bind_level_candidate_anchor(descriptor_offset, &catalog);
    expected_ok = theron_v1_track02_initial_candidate_expected_offset(
        descriptor_offset,
        &expected_candidate_offset);
    out_binding->expected_offset_valid = expected_ok ? 1 : 0;
    out_binding->expected_offset = expected_candidate_offset;
    out_binding->candidate_index = 0u;
    out_binding->candidate = catalog.candidates[0];
    out_binding->matches_initial_anchor =
        catalog.candidates[0].matches_initial_anchor;

    if (!expected_ok ||
        catalog.candidates[0].absolute_offset != expected_candidate_offset ||
        !catalog.candidates[0].matches_initial_anchor) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }

    out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_OK;
    return out_binding->status;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_load_descriptor_window_level(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    size_t entry_index,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02LevelHandoff *out_handoff) {

    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorWindowBinding binding;
    const Theron_Track02DescriptorWindow *window;
    Theron_Track02TableDecodeStatus table_status;
    const uint8_t *level_bytes;
    Theron_MapLoadResult map_status;

    if (out_handoff) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        out_handoff->map_status = THERON_MAP_ERR_NULL;
    }
    if (out_level) {
        memset(out_level, 0, sizeof(*out_level));
    }

    if (!track02_data || track02_size == 0 || !out_level || !out_handoff) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }
    if (entry_index >= THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }
    if (descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
    }

    table_status = theron_v1_track02_decode_descriptor_table(
        track02_data + descriptor_offset,
        TQR_US_ISO_BANK_STRIDE_BYTES,
        TQR_US_ISO_BANK_STRIDE_STEP,
        &table);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
    }

    table_status = theron_v1_track02_bind_descriptor_windows(
        track02_data,
        track02_size,
        descriptor_offset,
        &table,
        &binding);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
    }

    window = &binding.windows[entry_index];
    out_handoff->entry_index = entry_index;
    out_handoff->absolute_offset = window->absolute_offset;
    out_handoff->byte_count = window->byte_count;
    out_handoff->window_kind = window->kind;

    if (window->kind != THERON_TRACK02_DESCRIPTOR_WINDOW_DATA) {
        return THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA;
    }
    if (window->byte_count < 12u ||
        window->absolute_offset > track02_size ||
        window->byte_count > track02_size - window->absolute_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    level_bytes = track02_data + window->absolute_offset;
    out_handoff->header_width = rd16be(level_bytes + 0);
    out_handoff->header_height = rd16be(level_bytes + 2);
    out_handoff->header_seed = rd32be(level_bytes + 4);
    out_handoff->header_level_index = rd16be(level_bytes + 8);

    /* Handoff target: theron_v1_world.c T560-shaped 12-byte header + grid
     * loader.  This keeps Track 02 selection separate from world parsing
     * until real dungeon-window semantics are known. */
    map_status = theron_v1_level_load(out_level,
                                      level_bytes,
                                      (int)window->byte_count,
                                      dungeon_id,
                                      sub_level_index);
    out_handoff->map_status = map_status;
    if (map_status != THERON_MAP_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED;
    }

    out_handoff->loaded = 1;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_load_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02LevelHandoff *out_handoff) {

    Theron_Track02LevelHandoffStatus binding_status;
    Theron_Track02InitialCandidateBinding binding;
    const Theron_Track02LevelCandidate *candidate;
    const uint8_t *level_bytes;
    Theron_MapLoadResult map_status;

    if (out_handoff) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        out_handoff->map_status = THERON_MAP_ERR_NULL;
    }
    if (out_level) {
        memset(out_level, 0, sizeof(*out_level));
    }

    if (!track02_data || track02_size == 0u || !out_level || !out_handoff) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }
    binding_status = theron_v1_track02_bind_initial_level_candidate(
        track02_data,
        track02_size,
        descriptor_offset,
        &binding);
    out_handoff->binding_status = (int32_t)binding_status;
    out_handoff->candidate_count = binding.candidate_count;
    out_handoff->expected_offset = binding.expected_offset;
    out_handoff->matches_initial_anchor = binding.matches_initial_anchor;
    if (binding.candidate_count == 1u) {
        out_handoff->descriptor_delta = binding.candidate.descriptor_delta;
    }
    if (binding_status != THERON_TRACK02_LEVEL_HANDOFF_OK) {
        return binding_status;
    }

    candidate = &binding.candidate;
    if (candidate->absolute_offset > track02_size ||
        candidate->byte_count > track02_size - candidate->absolute_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    level_bytes = track02_data + candidate->absolute_offset;
    out_handoff->entry_index = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES;
    out_handoff->absolute_offset = candidate->absolute_offset;
    out_handoff->byte_count = candidate->byte_count;
    out_handoff->window_kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
    out_handoff->header_width = rd16be(level_bytes + 0);
    out_handoff->header_height = rd16be(level_bytes + 2);
    out_handoff->header_seed = rd32be(level_bytes + 4);
    out_handoff->header_level_index = rd16be(level_bytes + 8);

    /* Real raw JP/US Track 02 candidate gate.  These four header fields
     * are identical in the hash-verified JP and US raw BINs at
     * descriptor_base - 0x92ce.  They deliberately keep this handoff
     * narrower than a broad scan, avoiding tiny false-positive headers
     * that also exist near the same bank. */
    if (out_handoff->header_width != TQR_RAW_INITIAL_LEVEL_WIDTH ||
        out_handoff->header_height != TQR_RAW_INITIAL_LEVEL_HEIGHT ||
        out_handoff->header_seed != TQR_RAW_INITIAL_LEVEL_SEED ||
        out_handoff->header_level_index != TQR_RAW_INITIAL_LEVEL_INDEX) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    map_status = theron_v1_level_load(out_level,
                                      level_bytes,
                                      (int)candidate->byte_count,
                                      dungeon_id,
                                      sub_level_index);
    out_handoff->map_status = map_status;
    if (map_status != THERON_MAP_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED;
    }

    choose_initial_level_start_pose(out_level);
    out_handoff->loaded = 1;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}

const char *theron_v1_track02_level_handoff_status_name(
    Theron_Track02LevelHandoffStatus status) {
    switch (status) {
    case THERON_TRACK02_LEVEL_HANDOFF_OK:
        return "ok";
    case THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL:
        return "no-level";
    case THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND:
        return "table-not-found";
    case THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA:
        return "window-not-data";
    case THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED:
        return "level-load-failed";
    case THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES:
        return "ambiguous-candidates";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_table_decode_status_name(
    Theron_Track02TableDecodeStatus status) {
    switch (status) {
    case THERON_TRACK02_TABLE_DECODE_OK:
        return "ok";
    case THERON_TRACK02_TABLE_DECODE_NOT_FOUND:
        return "not-found";
    case THERON_TRACK02_TABLE_DECODE_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_TABLE_DECODE_WRONG_ENTRY_COUNT:
        return "wrong-entry-count";
    case THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING:
        return "not-strictly-ascending";
    case THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE:
        return "wrong-stride";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_descriptor_window_kind_name(
    Theron_Track02DescriptorWindowKind kind) {
    switch (kind) {
    case THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL:
        return "zero-fill";
    case THERON_TRACK02_DESCRIPTOR_WINDOW_DATA:
        return "data";
    case THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE:
        return "descriptor-table";
    case THERON_TRACK02_DESCRIPTOR_WINDOW_UNKNOWN:
    default:
        return "unknown";
    }
}

/* ── Semantic dungeon-descriptor binding ─────────────────────────── */

/* Documented working hypothesis (see header source-locks section):
 *   entry 0 -> DUNGEON_SEED_TABLE
 *   entry 5 -> DESCRIPTOR_TABLE  (already classified structurally;
 *                                  semantic role is a re-statement of
 *                                  the existing contains_descriptor_table
 *                                  flag for the middle entry)
 *   every other entry -> UNKNOWN
 *
 * This is one bound entry per the lane task.  Other entries keep their
 * structural classification but no semantic role until real Track 02
 * decoding promotes them. */
Theron_Track02SemanticRole theron_v1_track02_semantic_role_for_entry(
    size_t entry_index) {
    if (entry_index == 0u) return THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE;
    if (entry_index == (size_t)TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR) {
        return THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE;
    }
    return THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN;
}

const char *theron_v1_track02_semantic_role_name(
    Theron_Track02SemanticRole role) {
    switch (role) {
    case THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE:
        return "dungeon-seed-table";
    case THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE:
        return "descriptor-table";
    case THERON_TRACK02_SEMANTIC_OBJECT_TABLE:
        return "object-table";
    case THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE:
        return "level-grid-table";
    case THERON_TRACK02_SEMANTIC_TEXT_TABLE:
        return "text-table";
    case THERON_TRACK02_SEMANTIC_PALETTE_TABLE:
        return "palette-table";
    case THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_semantic_binding_status_name(
    Theron_Track02SemanticBindingStatus status) {
    switch (status) {
    case THERON_TRACK02_SEMANTIC_BINDING_OK:
        return "ok";
    case THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND:
        return "not-bound";
    case THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL:
        return "window-too-small";
    case THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE:
        return "bad-shape";
    case THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL:
        return "zero-fill";
    default:
        return "unknown";
    }
}

Theron_Track02SemanticBindingStatus theron_v1_track02_read_dungeon_seed_table(
    const uint8_t *seed_bytes,
    size_t seed_size,
    Theron_Track02DungeonSeedTable *out_table) {
    size_t i;
    int shape_ok;

    if (out_table) {
        memset(out_table, 0, sizeof(*out_table));
    }
    if (!seed_bytes || !out_table) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (seed_size < THERON_TRACK02_DUNGEON_SEED_TABLE_BYTES) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    for (i = 0; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        out_table->seeds[i] = rd32le(seed_bytes + (i * THERON_TRACK02_DUNGEON_SEED_BYTES_PER_ENTRY));
    }

    /* Strictly nonzero: every seed must be non-zero.  This rejects an
     * uninitialized/zero-filled window even if it otherwise fits. */
    shape_ok = 1;
    for (i = 0; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        if (out_table->seeds[i] == 0u) {
            shape_ok = 0;
            break;
        }
    }

    /* Non-decreasing: seeds[i+1] >= seeds[i].  This mirrors the documented
     * working-hypothesis placeholder list (313/414/527/632/749/856/967,
     * strictly ascending in the current build) while still accepting equal
     * adjacent seeds, which would be the natural shape if two dungeons
     * shared a starting seed. */
    if (shape_ok) {
        for (i = 0; i + 1u < THERON_TRACK02_DUNGEON_COUNT; ++i) {
            if (out_table->seeds[i + 1u] < out_table->seeds[i]) {
                shape_ok = 0;
                break;
            }
        }
    }

    out_table->shape_ok = shape_ok;
    return shape_ok ? THERON_TRACK02_SEMANTIC_BINDING_OK
                    : THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
}

Theron_Track02SemanticBindingStatus theron_v1_track02_bind_semantic_descriptor(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    size_t entry_index,
    Theron_Track02SemanticBinding *out_binding) {

    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorWindowBinding binding;
    const Theron_Track02DescriptorWindow *window;
    Theron_Track02TableDecodeStatus table_status;
    Theron_Track02SemanticBindingStatus status = THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;

    if (out_binding) {
        memset(out_binding, 0, sizeof(*out_binding));
    }
    if (!track02_data || track02_size == 0 || !out_binding) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (entry_index >= THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    out_binding->entry_index = entry_index;
    out_binding->role = theron_v1_track02_semantic_role_for_entry(entry_index);
    if (out_binding->role == THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN) {
        out_binding->status = THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
        return THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
    }

    /* Decode the table once so the entry window absolute offsets can be
     * derived from a verified-shape descriptor. */
    table_status = theron_v1_track02_decode_descriptor_table(
        track02_data + descriptor_offset,
        TQR_US_ISO_BANK_STRIDE_BYTES,
        TQR_US_ISO_BANK_STRIDE_STEP,
        &table);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        out_binding->status = THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    table_status = theron_v1_track02_bind_descriptor_windows(
        track02_data,
        track02_size,
        descriptor_offset,
        &table,
        &binding);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        out_binding->status = THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    window = &binding.windows[entry_index];
    out_binding->absolute_offset = window->absolute_offset;
    out_binding->byte_count = window->byte_count;
    out_binding->window_kind = window->kind;

    /* Role-specific decode.  Today this only covers DUNGEON_SEED_TABLE;
     * every other bound role (DESCRIPTOR_TABLE) returns NOT_BOUND at the
     * role-classification step above and never reaches here.  When new
     * roles are added, they get a dedicated branch. */
    if (out_binding->role == THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE) {
        if (window->kind == THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL) {
            status = THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL;
        } else if (window->byte_count <
                   THERON_TRACK02_DUNGEON_SEED_TABLE_BYTES) {
            status = THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL;
        } else {
            status = theron_v1_track02_read_dungeon_seed_table(
                track02_data + window->absolute_offset,
                window->byte_count,
                &out_binding->dungeon_seed_table);
        }
    } else {
        /* DESCRIPTOR_TABLE role reaches here only by explicit future
         * extension; for now classify as not-bound so callers don't get a
         * silent OK on a role we have not yet given semantic content. */
        status = THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
    }

    out_binding->status = status;
    return status;
}
