#ifndef THERON_V1_TRACK02_H
#define THERON_V1_TRACK02_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_world.h"

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
 * The descriptor-window binding below gives each entry a bounded byte-level
 * role (zero-fill, payload data, or the descriptor-table-bearing window).
 * It still does NOT claim a per-dungeon level-table binding, map-grid
 * decoding, object-table decoding, or runtime loader handoff. */

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

typedef enum {
    THERON_TRACK02_DESCRIPTOR_WINDOW_UNKNOWN = 0,
    THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL,
    THERON_TRACK02_DESCRIPTOR_WINDOW_DATA,
    THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE
} Theron_Track02DescriptorWindowKind;

typedef struct {
    size_t entry_index;
    uint16_t relative_offset;
    size_t absolute_offset;
    size_t byte_count;
    size_t nonzero_byte_count;
    size_t first_nonzero_offset;
    size_t last_nonzero_offset;
    int contains_descriptor_table;
    Theron_Track02DescriptorWindowKind kind;
} Theron_Track02DescriptorWindow;

typedef struct {
    size_t entry_count;
    size_t base_offset;
    size_t descriptor_offset;
    uint16_t window_size;
    Theron_Track02DescriptorWindow windows[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
} Theron_Track02DescriptorWindowBinding;

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

/* Bind a decoded descriptor table back to Track 02 byte windows.
 *
 * `descriptor_offset` is the absolute Track 02 offset where the 18-byte
 * descriptor table was found (0x1584 in the US ISO, or one of the raw BIN
 * anchor offsets).  The function derives the descriptor-region base using
 * the source-locked 0x1584 anchor, then classifies each 0x0400-byte entry
 * window as zero-fill, payload data, or the window that contains the
 * descriptor table.
 *
 * This is still a bounded byte-level semantic binding. It does not claim
 * dungeon level records, map grids, object tables, or runtime loader handoff.
 */
Theron_Track02TableDecodeStatus theron_v1_track02_bind_descriptor_windows(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    const Theron_Track02DescriptorTable *table,
    Theron_Track02DescriptorWindowBinding *out_binding);

typedef enum {
    THERON_TRACK02_LEVEL_HANDOFF_OK = 1,
    THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL = 0,
    THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT = -1,
    THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND = -2,
    THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA = -3,
    THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED = -4
} Theron_Track02LevelHandoffStatus;

typedef struct {
    size_t entry_index;
    size_t absolute_offset;
    size_t byte_count;
    Theron_Track02DescriptorWindowKind window_kind;
    Theron_MapLoadResult map_status;
    uint16_t header_width;
    uint16_t header_height;
    uint32_t header_seed;
    uint16_t header_level_index;
    int loaded;
} Theron_Track02LevelHandoff;

/* Bounded Track 02 -> V1 level-loader handoff.
 *
 * Decodes the 9-word descriptor table at `descriptor_offset`, binds the
 * resulting 0x0400-byte windows, and attempts to pass one DATA window to
 * theron_v1_level_load().  This is a handoff contract only: it proves that
 * bytes selected by the descriptor table can reach the existing Theron V1
 * level loader under a bounded API.  It does not claim that real Track 02
 * windows are decoded dungeon records yet.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_load_descriptor_window_level(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    size_t entry_index,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02LevelHandoff *out_handoff);

const char *theron_v1_track02_level_handoff_status_name(
    Theron_Track02LevelHandoffStatus status);

const char *theron_v1_track02_table_decode_status_name(
    Theron_Track02TableDecodeStatus status);

const char *theron_v1_track02_descriptor_window_kind_name(
    Theron_Track02DescriptorWindowKind kind);

#endif /* THERON_V1_TRACK02_H */
