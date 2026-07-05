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

#define THERON_TRACK02_MAX_LEVEL_CANDIDATES 32u
#define THERON_TRACK02_RAW_SECTOR_BYTES 2352u
#define THERON_TRACK02_RAW_USER_DATA_OFFSET 0x10u
#define THERON_TRACK02_RAW_USER_DATA_BYTES 2048u

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

/* Raw Track 02 BIN -> logical 2048-byte user-data stream helpers.
 *
 * JP/US raw Track 02 BIN images are MODE1/2352 sector dumps.  The startup
 * level, bank descriptors, and future menu/champion-art payload decoders need
 * stable conversion from raw-sector offsets to the logical 2048-byte
 * user-data stream before they can compare JP/US images or bind ISO-derived
 * offsets.  These helpers are intentionally hash-gated to the two raw BIN MD5s;
 * ISO variants and unknown MD5s return UNSUPPORTED_VARIANT.
 *
 * The helpers do not interpret user-data bytes as graphics, text, palettes,
 * levels, or audio.  They only expose the sector strip boundary:
 *   raw sector bytes 0..15     = sync/header
 *   raw sector bytes 16..2063  = 2048-byte user data
 *   raw sector bytes 2064..2351 = EDC/ECC/subheader area
 */
Theron_Track02SignalStatus theron_v1_track02_raw_user_data_size(
    size_t track02_size,
    const char *md5_hex,
    size_t *out_sector_count,
    size_t *out_user_data_size);

Theron_Track02SignalStatus theron_v1_track02_raw_offset_to_user_offset(
    size_t raw_offset,
    size_t track02_size,
    const char *md5_hex,
    size_t *out_user_offset);

Theron_Track02SignalStatus theron_v1_track02_copy_raw_user_data(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    uint8_t *out_user_data,
    size_t out_user_data_capacity,
    size_t *out_user_data_size);

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

/* Semantic role binding for descriptor table entries.
 *
 * This is a bounded byte-level role assignment derived only from
 * observable Track 02 evidence; it does NOT claim dungeon records,
 * map grids, object tables, palette payloads, text/font payloads, or
 * runtime loader handoff.  Roles are assigned deterministically from:
 *   - nonzero_byte_count (all-zero window vs data window)
 *   - contains_descriptor_table (which window holds the descriptor)
 *   - the byte at descriptor_offset - 1 (RTS marker when 0x60)
 *   - the byte at descriptor_offset + 18 (zero vs non-zero: code after
 *     descriptor within the descriptor-bearing window)
 *   - entry_index relative to the descriptor-window's index
 *     (pre-descriptor data, post-descriptor data)
 *
 * Source/evidence:
 *   - `0x60` is the documented HuC6280 (65C02-derivative) RTS opcode.
 *     When the byte at `descriptor_offset - 1` is `0x60`, the descriptor
 *     sits at the tail of an RTS-terminated executable code region.
 *     Source: HuC6280 datasheet + ReDMCSB 6502 reference (no ReDMCSB
 *     for Theron itself; PC Engine 65C02-derivative opcode set).
 *   - The 9 windows × 0x0400 stride = 0x2400 = 9216 bytes.  The PC
 *     Engine HuCard memory window at $4000-$7FFF spans 16KB and is
 *     bank-switched in 8KB pages via MPR (Memory Page Register).
 *     Source: HuC6280 datasheet MPR section.
 *   - The 0x0020 entry offset is the canonical first-window start.
 *     Source: src/theron/theron_v1_track02.c
 *     (g_us_iso_bank_stride_descriptor) and
 *     docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md. */
typedef enum {
    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_UNKNOWN = 0,
    /* All-zero 0x0400-byte window.  Padded/reserved sub-region with no
     * observable data bytes. */
    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL,
    /* Window's absolute byte range contains the 18-byte descriptor
     * table (descriptor_offset >= window.absolute_offset AND
     * descriptor_offset + 18 <= window.absolute_offset + 0x0400). */
    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE,
    /* Non-zero data window whose entry_index < descriptor-window
     * entry_index.  Bytes precede the descriptor in the descriptor
     * table's logical layout. */
    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA,
    /* Non-zero data window whose entry_index > descriptor-window
     * entry_index.  Bytes follow the descriptor in the descriptor
     * table's logical layout. */
    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA
} Theron_Track02DescriptorEntryRole;

/* Semantic binding of one descriptor table entry.
 *
 * The descriptor window itself gets a deeper role claim based on the
 * bytes immediately before and after the descriptor within its 0x0400
 * window.  This is a bounded byte-level claim, not a dungeon/text/
 * palette/object meaning claim:
 *
 *   - descriptor_at_window_tail: descriptor_offset + 18 ==
 *     absolute_offset + byte_count (descriptor sits at the window's
 *     last 18 bytes).  Observed in the US Track 02 ISO.
 *   - byte_before_descriptor_is_rts: track02_data[descriptor_offset - 1]
 *     == 0x60 (HuC6280 RTS opcode).  When true, the descriptor sits
 *     immediately after an RTS-terminated code region.
 *   - descriptor_is_pure_window: descriptor_offset == absolute_offset
 *     AND absolute_offset + 18 == absolute_offset + byte_count
 *     (the window contains ONLY the descriptor bytes).  Useful
 *     synthetic-fixture sentinel.
 *
 * The non-claim boundary: this binding does not interpret the bytes as
 * code, graphics, palette, text, or any data-domain payload.  It only
 * pins observable byte-shape relationships around the descriptor. */
typedef struct {
    size_t entry_index;
    uint16_t relative_offset;
    size_t absolute_offset;
    size_t byte_count;
    Theron_Track02DescriptorEntryRole role;
    /* True iff role == CONTAINS_DESCRIPTOR_TABLE.  Duplicated for
     * easy single-entry checks without re-deriving role from the
     * window binding. */
    int is_descriptor_window;
    /* Byte at descriptor_offset - 1 if descriptor_offset > 0 and the
     * byte is inside track02_data.  0 otherwise.  When this is 0x60
     * and is_descriptor_window is true, the descriptor sits at the
     * tail of an RTS-terminated code region (observed in the US ISO
     * descriptor window: descriptor is preceded by `0x60`). */
    uint8_t byte_before_descriptor;
    int byte_before_descriptor_is_rts;
    /* First non-zero byte offset within this window after the
     * descriptor's last byte, or 0 if the bytes after the descriptor
     * within this window are all zero.  0 also when this is not the
     * descriptor window. */
    size_t first_nonzero_after_descriptor;
    /* True iff all bytes after the descriptor within this window are
     * zero.  When is_descriptor_window is true and this is true, the
     * descriptor is the last non-zero data in its window (US ISO
     * shape). */
    int all_zero_after_descriptor;
} Theron_Track02DescriptorEntrySemanticBinding;

/* Bind semantic roles to every entry of a decoded descriptor table.
 *
 * `descriptor_offset` is the absolute Track 02 offset of the 18-byte
 * descriptor.  The function reuses the byte-window classification from
 * theron_v1_track02_bind_descriptor_windows() (zero-fill vs data vs
 * descriptor window) and adds:
 *   - PRE_DESCRIPTOR_DATA / POST_DESCRIPTOR_DATA ordering relative to
 *     the descriptor window's entry index
 *   - descriptor-tail markers (descriptor_at_window_tail,
 *     byte_before_descriptor_is_rts, all_zero_after_descriptor) on the
 *     single entry that contains the descriptor table
 *
 * Returns THERON_TRACK02_TABLE_DECODE_OK on success.  All 9 entries of
 * `out_binding->entries` are populated in entry-index order.
 *
 * Bounded non-claim: this does not interpret the bytes as code,
 * graphics, palette, text, or any data-domain payload.  It only pins
 * byte-shape relationships around the descriptor.
 */
Theron_Track02TableDecodeStatus theron_v1_track02_bind_descriptor_entry_roles(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    const Theron_Track02DescriptorTable *table,
    Theron_Track02DescriptorEntrySemanticBinding *out_entries);

/* Look up the entry_index of the descriptor-window.  Returns -1 when no
 * descriptor-window entry exists in the supplied semantic binding
 * (should not happen if the binding was produced by the function above
 * on a successful window bind, but kept as a defensive helper). */
int theron_v1_track02_find_descriptor_window_entry_index(
    const Theron_Track02DescriptorEntrySemanticBinding *entries,
    size_t entry_count);

const char *theron_v1_track02_descriptor_entry_role_name(
    Theron_Track02DescriptorEntryRole role);

typedef enum {
    THERON_TRACK02_LEVEL_HANDOFF_OK = 1,
    THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL = 0,
    THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT = -1,
    THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND = -2,
    THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA = -3,
    THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED = -4,
    THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES = -5
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
    int32_t binding_status;
    size_t candidate_count;
    size_t expected_offset;
    size_t descriptor_delta;
    int matches_initial_anchor;
    int loaded;
} Theron_Track02LevelHandoff;

typedef struct {
    size_t absolute_offset;
    size_t byte_count;
    uint16_t header_width;
    uint16_t header_height;
    uint32_t header_seed;
    uint16_t header_level_index;
    Theron_MapLoadResult map_status;
    int start_x;
    int start_y;
    int start_dir;
    size_t descriptor_delta;
    int matches_initial_anchor;
    int loaded;
} Theron_Track02LevelCandidate;

typedef struct {
    size_t candidate_count;
    size_t overflow_count;
    size_t scanned_bytes;
    Theron_Track02LevelCandidate
        candidates[THERON_TRACK02_MAX_LEVEL_CANDIDATES];
} Theron_Track02LevelCandidateCatalog;

typedef struct {
    Theron_Track02LevelHandoffStatus status;
    size_t descriptor_offset;
    size_t candidate_count;
    size_t candidate_index;
    size_t expected_offset;
    int expected_offset_valid;
    int matches_initial_anchor;
    Theron_Track02LevelCandidate candidate;
} Theron_Track02InitialCandidateBinding;

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

/* Hash/anchor-gated initial-level candidate handoff.
 *
 * The raw JP/US Track 02 BINs both carry a loader-compatible 32x27 level-like
 * payload with seed 0x0108e938 and level index 0x0026.  The handoff first
 * validates the descriptor table at `descriptor_offset`, then scans Track 02
 * for exactly one matching startup candidate before handing those bytes to
 * theron_v1_level_load().
 *
 * This is narrower than a full dungeon parser: it promotes one real startup
 * candidate and keeps all other Track 02 map/object semantics unclaimed.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_load_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02LevelHandoff *out_handoff);

/* Bounded raw Track 02 level-like payload catalog.
 *
 * This scanner finds the known loader-compatible startup payload in a
 * hash-verified Track 02 byte image.  It is intentionally conservative:
 * candidate headers must match the current 32x27 startup shape, seed, and
 * level index, and theron_v1_level_load() must accept the payload.  The
 * catalog is evidence and future routing infrastructure; it does not yet
 * assign candidates to named dungeons, levels, object tables, palettes, text,
 * or music.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_scan_level_candidates(
    const uint8_t *track02_data,
    size_t track02_size,
    Theron_Track02LevelCandidateCatalog *out_catalog);

/* Annotate a candidate catalog with the candidate->descriptor distance.
 *
 * The scanner intentionally does not require a descriptor offset because it
 * can be used on raw byte fixtures.  Call this after scanning when the
 * hash-verified descriptor anchor is known.  It fills
 * descriptor_delta for every candidate whose offset precedes the descriptor
 * and marks matches_initial_anchor only for the current source-locked raw
 * Track 02 startup relation used by
 * theron_v1_track02_load_initial_level_candidate().
 */
int theron_v1_track02_bind_level_candidate_anchor(
    size_t descriptor_offset,
    Theron_Track02LevelCandidateCatalog *catalog);

/* Bind the hash/anchor-gated initial startup candidate without loading it
 * into a Theron_V1_Level.
 *
 * This is the shared evidence contract used by the runtime loader and receipt
 * surfaces: the descriptor table must decode, the level-like scanner must find
 * exactly one candidate, and that candidate must sit at the source-locked
 * offset relation to the descriptor anchor.  It does not claim other Track 02
 * windows or candidates as dungeon records.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_bind_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    Theron_Track02InitialCandidateBinding *out_binding);

/* Expected raw Track 02 startup candidate offset for one descriptor anchor.
 *
 * The JP/US raw Track 02 BIN startup payload accepted by
 * theron_v1_track02_load_initial_level_candidate() sits at a fixed byte
 * distance before the 9-word descriptor table anchor.  This helper exposes
 * that relation for tests and M11 diagnostics without exposing the private
 * descriptor-base constants.
 *
 * Returns 1 when descriptor_offset is large enough to derive the offset,
 * otherwise 0. */
int theron_v1_track02_initial_candidate_expected_offset(
    size_t descriptor_offset,
    size_t *out_candidate_offset);

const char *theron_v1_track02_level_handoff_status_name(
    Theron_Track02LevelHandoffStatus status);

const char *theron_v1_track02_table_decode_status_name(
    Theron_Track02TableDecodeStatus status);

const char *theron_v1_track02_descriptor_window_kind_name(
    Theron_Track02DescriptorWindowKind kind);

/* Semantic dungeon-descriptor binding.
 *
 * The 0x1584 descriptor table and its three replicated raw-BIN anchors
 * enumerate 9 windows of stride 0x0400.  Each window is structurally
 * classified as zero-fill / data / descriptor-table, but a *semantic*
 * role needs the working hypothesis that one specific entry maps to a
 * named in-game concept.
 *
 * This release binds ONE descriptor entry to ONE semantic role: the
 * first entry (descriptor.entry_index == 0, window relative offset
 * 0x0020) is bound to the role THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE,
 * which reads the documented 7 × uint32 little-endian dungeon_seeds from
 * tqr_v1_phase2_data_formats_H2339.md §9.1 (a.k.a. the THQUEST.ASM T560
 * `dungeon_seed` table, sourced from theron_v1_boot.c:318-345 /
 * theron_v1_dungeon_progression.c:38-110).  No other entry has a semantic
 * role yet; it remains UNKNOWN.  Real-data promotion is still no-claim
 * until the Track 02 dungeon block offset is verified.
 *
 * Source-locks:
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §9.1
 *     `Offset 10-37: dungeon_seeds (7 × 4 bytes = 28 bytes, uint32 LE)`.
 *   src/theron/theron_v1_dungeon_progression.c:38-110 (in-game default
 *     seeds 313/414/527/632/749/856/967 for the 7 mini-dungeons).
 *   src/theron/theron_v1_boot.c:318-345 (boot-time seed extraction).
 *   THQUEST.ASM T560 (header parsing + dungeon_seed extraction).
 *   docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md (descriptor
 *     region offsets and the 0x1584 / raw-BIN anchor byte anchors). */

#define THERON_TRACK02_DUNGEON_COUNT 7u
#define THERON_TRACK02_DUNGEON_SEED_BYTES_PER_ENTRY 4u
#define THERON_TRACK02_DUNGEON_SEED_TABLE_BYTES \
    (THERON_TRACK02_DUNGEON_COUNT * THERON_TRACK02_DUNGEON_SEED_BYTES_PER_ENTRY)

/* Default seeds documented in theron_v1_dungeon_progression.c:38-110.
 * Used as the value-driven hash fingerprint for the synthetic semantic
 * binding probe.  Real-data promotion uses these values only as a
 * reference list, never as a decoder input. */
typedef struct {
    uint32_t seeds[THERON_TRACK02_DUNGEON_COUNT];
    /* Set to 1 when the decoder finds 7 strictly nonzero, non-decreasing,
     * 32-bit-aligned seeds within the bound window.  Otherwise 0.  This is
     * a shape gate only; it does NOT validate that the real Track 02
     * seed values match the working-hypothesis list. */
    int shape_ok;
} Theron_Track02DungeonSeedTable;

typedef enum {
    THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN = 0,
    THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE,
    THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE,
    THERON_TRACK02_SEMANTIC_OBJECT_TABLE,
    THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE,
    THERON_TRACK02_SEMANTIC_TEXT_TABLE,
    THERON_TRACK02_SEMANTIC_PALETTE_TABLE
} Theron_Track02SemanticRole;

typedef enum {
    THERON_TRACK02_SEMANTIC_BINDING_OK = 1,
    THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND = 0,
    THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT = -1,
    THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL = -2,
    THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE = -3,
    THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL = -4
} Theron_Track02SemanticBindingStatus;

typedef struct {
    size_t entry_index;
    Theron_Track02SemanticRole role;
    size_t absolute_offset;
    size_t byte_count;
    Theron_Track02DescriptorWindowKind window_kind;
    /* Populated when role == DUNGEON_SEED_TABLE.  shape_ok mirrors the
     * decoder gate; seeds[] is byte-faithful regardless of shape_ok. */
    Theron_Track02DungeonSeedTable dungeon_seed_table;
    Theron_Track02SemanticBindingStatus status;
} Theron_Track02SemanticBinding;

/* Map a descriptor entry index to a semantic role.
 *
 * The mapping is the documented working hypothesis from this header's
 * source-locks section: entry 0 is the dungeon_seed table, entry 5 is
 * the descriptor-table-bearing window (already classified structurally),
 * every other entry is UNKNOWN.  Returning UNKNOWN is not an error; it
 * simply means no semantic role is currently bound. */
Theron_Track02SemanticRole theron_v1_track02_semantic_role_for_entry(
    size_t entry_index);

/* String name for a semantic role.  Always returns a non-NULL string. */
const char *theron_v1_track02_semantic_role_name(
    Theron_Track02SemanticRole role);

/* Bind one descriptor entry to its semantic role.
 *
 * `track02_data` / `track02_size` describe the full Track 02 image
 * (or a synthetic fixture); `descriptor_offset` is the byte offset
 * of the 18-byte descriptor table within that buffer; `entry_index`
 * selects one of the 9 descriptor entries.  The function decodes the
 * table, binds all 9 windows, and classifies the requested entry
 * according to its semantic role.
 *
 * Status codes:
 *   OK                 -> role == DUNGEON_SEED_TABLE and shape_ok == 1
 *   NOT_BOUND          -> role == UNKNOWN for this entry_index
 *   BAD_INPUT          -> NULL/zero-size/invalid entry_index
 *   WINDOW_TOO_SMALL   -> window byte_count < dungeon_seed_table bytes
 *   BAD_SHAPE          -> dungeon_seed_table byte shape fails the gate
 *                         (e.g. seeds not strictly nonzero, not
 *                         non-decreasing, or out of 32-bit range)
 *   ZERO_FILL          -> window is all zero bytes (kind ==
 *                         THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL),
 *                         which is an honest signal that the seeded
 *                         region is not present on this image
 *
 * Real Track 02 data MUST be allowed to return ZERO_FILL or NOT_BOUND
 * honestly.  Only synthetic Track 02 fixtures (and the documented US ISO
 * at descriptor_offset 0x1584 if and when its dungeon_seed window is
 * identified) are expected to return OK today. */
Theron_Track02SemanticBindingStatus theron_v1_track02_bind_semantic_descriptor(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    size_t entry_index,
    Theron_Track02SemanticBinding *out_binding);

/* Lower-level: read a 7 × uint32 little-endian dungeon_seed table from
 * `seed_bytes`.  Used both by theron_v1_track02_bind_semantic_descriptor
 * (when entry 0 is requested) and by the synthetic probe (so the probe
 * can pin the shape contract without depending on a full descriptor
 * decode).
 *
 * Shape gate:
 *   - seed_bytes != NULL && seed_size >= 28 (THERON_TRACK02_DUNGEON_SEED_TABLE_BYTES)
 *   - seeds are read byte-faithfully regardless of shape_ok
 *   - shape_ok == 1 requires every seed != 0, and seeds are non-decreasing
 *     (seeds[i+1] >= seeds[i]).  The non-decreasing rule mirrors the
 *     placeholder seed list 313/414/527/632/749/856/967 (strictly
 *     ascending in this build) but accepts equal-adjacent seeds too,
 *     because real Track 02 seeds may be tied.
 *   - shape_ok == 0 leaves the caller's out_table populated with the
 *     byte-faithful seeds[], so callers can inspect the actual values.
 *
 * Returns THERON_TRACK02_SEMANTIC_BINDING_OK on shape_ok == 1,
 * THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT on NULL/short input,
 * THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE on shape failure. */
Theron_Track02SemanticBindingStatus theron_v1_track02_read_dungeon_seed_table(
    const uint8_t *seed_bytes,
    size_t seed_size,
    Theron_Track02DungeonSeedTable *out_table);

const char *theron_v1_track02_semantic_binding_status_name(
    Theron_Track02SemanticBindingStatus status);

#endif /* THERON_V1_TRACK02_H */
