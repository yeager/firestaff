#ifndef THERON_V1_TRACK02_H
#define THERON_V1_TRACK02_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_MAX_BANK_ANCHORS 3u

/* Maximum number of entries the documented 9-word stride table can hold.
 * The 0x1584 descriptor observed in the hash-verified US Track 02 ISO and
 * the three replicated anchors in the JP/US raw Track 02 BINs all carry
 * exactly 9 little-endian uint16 words; this constant bounds the decoder
 * table size and the synthetic-fixture/negative-fixture tests. */
#define THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES 9u

#define THERON_TRACK02_MD5_JP_BIN      "b7afb338ad31be1025b53f9aff12d73a"
#define THERON_TRACK02_MD5_US_BIN      "f23601102138f87c33025877767ebf76"
#define THERON_TRACK02_MD5_JP_REV1_ISO "397039af02d50d15c70b74088eb8a1cb"
#define THERON_TRACK02_MD5_US_ISO      "3d8b78571dcd0e6eb8eb4b01eeb7fbba"

typedef enum {
    THERON_TRACK02_VARIANT_UNKNOWN = 0,
    THERON_TRACK02_VARIANT_JP_BIN,
    THERON_TRACK02_VARIANT_US_BIN,
    THERON_TRACK02_VARIANT_JP_REV1_ISO,
    THERON_TRACK02_VARIANT_US_ISO
} Theron_Track02Variant;

typedef enum {
    THERON_TRACK02_SIGNAL_OK = 1,
    THERON_TRACK02_SIGNAL_NOT_FOUND = 0,
    THERON_TRACK02_SIGNAL_BAD_INPUT = -1,
    THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT = -2,
    THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE = -3
} Theron_Track02SignalStatus;

typedef struct {
    Theron_Track02Variant variant;
    size_t anchor_count;
    size_t descriptor_offset;
    size_t descriptor_size;
    size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t occurrence_count;
    uint16_t first_value;
    uint16_t last_value;
    uint16_t stride;
    size_t value_count;
    size_t post_descriptor_zero_offset;
    size_t post_descriptor_zero_bytes;
    size_t next_nonzero_offset;
    size_t boundary_prefix_size;
    size_t boundary_prefix_occurrence_count;
    size_t post_boundary_span_size;
    size_t post_boundary_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t post_boundary_span_occurrence_count;
    uint16_t post_boundary_span_first_word;
    uint16_t post_boundary_span_last_word;
    size_t raw_sector_bytes;
    size_t raw_sector_user_data_offset;
    size_t descriptor_raw_sector_numbers[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t descriptor_raw_sector_user_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t post_boundary_span_raw_sector_numbers[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t post_boundary_span_raw_sector_user_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    /* Audio-bank marker: 4-byte little-endian word that immediately precedes
     * the post-boundary span at each anchor in raw Track 02 BINs.
     * Audio-bank prefix is 12 bytes of `00 ff*10 00` followed by this word,
     * which we have observed to encode a 2352-byte CD sector pointer at all
     * three anchors in both US and JP raw Track 02 BINs.  Populated only for
     * raw BIN variants (THERON_TRACK02_VARIANT_US_BIN / JP_BIN); zeroed for
     * the US Track 02 ISO (partial extract, no anchors present) and for the
     * JP Rev 1 ISO (zero-filled image).
     *
     * Source/evidence:
     *   src/theron/theron_v1_track02.c (this module, post-boundary span
     *   fingerprinting); docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
     *   §10.2 (ADPCM audio data block location is STUB; this marker is one
     *   ADPCM-bank-table anchor candidate). */
    uint32_t audio_bank_id[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t audio_bank_id_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t audio_bank_prefix_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int audio_bank_id_recognized[THERON_TRACK02_MAX_BANK_ANCHORS];
} Theron_Track02BankSignal;

Theron_Track02Variant theron_v1_track02_variant_for_md5(const char *md5_hex);

Theron_Track02SignalStatus theron_v1_track02_find_bank_signal(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02BankSignal *out_signal);

/* One-shot audio-bank marker reader for a single anchor index.
 *
 * Hash-gated to raw BIN variants only (US_BIN, JP_BIN).  Validates the
 * 12-byte `00 ff*10 00` prefix and the 44-byte post-boundary span at the
 * known anchor offset for (variant, anchor_index), then returns the 4-byte
 * little-endian audio-bank id word immediately preceding the span.
 *
 * Returns:
 *   THERON_TRACK02_SIGNAL_OK on success (out_* populated).
 *   THERON_TRACK02_SIGNAL_NOT_FOUND if the prefix or span is missing.
 *   THERON_TRACK02_SIGNAL_BAD_INPUT for NULL/zero-size inputs.
 *   THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT for non-raw-BIN variants.
 *
 * Source/evidence: src/theron/theron_v1_track02.c (audio-bank prefix
 * fingerprint); see theron_v1_track02_source_evidence() for full citation. */
Theron_Track02SignalStatus theron_v1_track02_find_audio_bank_marker(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t anchor_index,
    uint32_t *out_audio_bank_id,
    size_t *out_audio_bank_id_offset,
    size_t *out_audio_bank_prefix_offset);

const char *theron_v1_track02_signal_status_name(Theron_Track02SignalStatus status);
const char *theron_v1_track02_variant_name(Theron_Track02Variant variant);
const char *theron_v1_track02_source_evidence(void);

/* Semantic dungeon-descriptor table decoding.
 *
 * The 0x1584 descriptor block (US Track 02 ISO) and the three replicated
 * raw BIN anchors in JP/US Track 02 BINs each carry the same nine
 * little-endian uint16 words `0x0020, 0x0420, 0x0820, 0x0c20, 0x1020,
 * 0x1420, 0x1820, 0x1c20, 0x2020` with stride `0x0400`.  This struct
 * and decoder lock the shape that has been observed:
 *
 *   - entry_count == 9
 *   - entries are strictly ascending uint16 words
 *   - stride (entries[i+1] - entries[i]) == 0x0400
 *   - all entries land in the half-open range [0x0020, 0x2020 + 0x0400)
 *
 * The values are documented offsets relative to the start of the data
 * region (a 0x2000-byte descriptor block plus the 0x400 stride window).
 * The decoder does NOT claim a semantic per-entry type, a per-dungeon
 * level-table binding, or runtime loader handoff; it only validates the
 * shape so a future level-descriptor reader can rely on it. */

typedef enum {
    THERON_TRACK02_TABLE_DECODE_OK = 1,
    THERON_TRACK02_TABLE_DECODE_NOT_FOUND = 0,
    THERON_TRACK02_TABLE_DECODE_BAD_INPUT = -1,
    THERON_TRACK02_TABLE_DECODE_WRONG_ENTRY_COUNT = -2,
    THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING = -3,
    THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE = -4
} Theron_Track02TableDecodeStatus;

typedef struct {
    size_t entry_count;
    uint16_t entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    uint16_t stride;
    uint16_t first_value;
    uint16_t last_value;
    /* exclusive upper bound == entries[entry_count - 1] + stride.
     * Mirrors the half-open [first, exclusive) window the descriptor
     * describes. */
    uint16_t exclusive_upper_bound;
    /* Sanity flag: 1 when all entries + stride land in the closed range
     * [0x0020, 0x2020 + 0x0400). */
    int range_inclusive;
} Theron_Track02DescriptorTable;

/* Decode a 9-word little-endian stride table starting at `descriptor_bytes`.
 *
 * `descriptor_bytes` must be at least 18 bytes; the decoder reads 9 little-
 * endian uint16 words and validates the documented shape.  On success the
 * out struct is filled and THERON_TRACK02_TABLE_DECODE_OK is returned.
 *
 * Negative fixtures:
 *   - descriptor_bytes == NULL or descriptor_size < 18 -> BAD_INPUT
 *   - wrong entry count encoded in the first 2 bytes ->
 *     NOT_FOUND (only the canonical 9-word table is accepted)
 *   - not strictly ascending (entries[i+1] <= entries[i]) ->
 *     NOT_STRICTLY_ASCENDING
 *   - stride mismatch (entries[i+1] - entries[i] != expected_stride) ->
 *     WRONG_STRIDE
 *
 * Note: `expected_stride` is the value the caller assumes (the documented
 * 0x0400).  The decoder computes the actual stride from entries and
 * compares it.  A zero expected_stride is rejected as BAD_INPUT. */
Theron_Track02TableDecodeStatus theron_v1_track02_decode_descriptor_table(
    const uint8_t *descriptor_bytes,
    size_t descriptor_size,
    uint16_t expected_stride,
    Theron_Track02DescriptorTable *out_table);

const char *theron_v1_track02_table_decode_status_name(
    Theron_Track02TableDecodeStatus status);

#endif /* THERON_V1_TRACK02_H */
