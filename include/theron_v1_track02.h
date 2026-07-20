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
#define THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES 3u
#define THERON_TRACK02_MAX_USER_DATA_WINDOWS 8u
#define THERON_TRACK02_MAX_STARTUP_TEXT_MARKERS 8u
#define THERON_TRACK02_MAX_STARTUP_ROSTER_NAMES 8u
#define THERON_TRACK02_MAX_STARTUP_BITMAP_SAMPLES 128u
#define THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY 16u
#define THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY 32u
#define THERON_TRACK02_RAW_SECTOR_BYTES 2352u
#define THERON_TRACK02_RAW_USER_DATA_OFFSET 0x10u
#define THERON_TRACK02_RAW_USER_DATA_BYTES 2048u
#define THERON_TRACK02_MOUNT_PATH_CAPACITY 1024u
#define THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES 32u
#define THERON_TRACK02_STARTUP_BITMAP_PIXELS 64u
#define THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX 4u
#define THERON_TRACK02_STARTUP_BITMAP_ATLAS_LEGACY_MAX_WIDTH 128u
#define THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH 256u
#define THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_HEIGHT 8u
#define THERON_TRACK02_STARTUP_BITMAP_ATLAS_PIXELS \
    (THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH * \
     THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_HEIGHT)
#define THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT 16u
#define THERON_TRACK02_4BPP_PALETTE_BYTES \
    (THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT * 2u)

#define THERON_TRACK01_CDDA_SECTOR_BYTES 2352u
#define THERON_TRACK01_CDDA_SAMPLE_RATE 44100
#define THERON_TRACK01_CDDA_CHANNELS 2
#define THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS 16u

enum {
    THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE = 1u << 0,
    THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE = 1u << 1,
    THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM = 1u << 2,
    THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD = 1u << 3
};

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

typedef enum {
    THERON_TRACK01_CDDA_AVAILABLE = 1,
    THERON_TRACK01_CDDA_UNAVAILABLE = 0,
    THERON_TRACK01_CDDA_BAD_INPUT = -1,
    THERON_TRACK01_CDDA_UNVERIFIED = -2
} Theron_Track01CddaStatus;

typedef struct {
    Theron_Track01CddaStatus status;
    Theron_Track02Variant track02_variant;
    char cue_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char audio_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char track02_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char unavailable_reason[160];
    unsigned int track_number;
    unsigned int index_minute;
    unsigned int index_second;
    unsigned int index_frame;
    unsigned int index_lba;
    size_t audio_start_byte;
    size_t audio_file_bytes;
    size_t audio_sector_count;
    int original_cdda;
    int playback_handoff_ready;
} Theron_Track01CddaHandoff;

typedef struct {
    void *audio_file;
    void *sdl_stream;
    size_t audio_start_byte;
    size_t audio_sector_count;
    size_t sectors_read;
    size_t sectors_queued;
    size_t loop_count;
    int output_started;
} Theron_Track01CddaStream;

Theron_Track01CddaStatus theron_v1_track01_cdda_handoff_from_verified_media(
    const char *media_path,
    const char *verified_track02_md5,
    Theron_Track01CddaHandoff *out_handoff);
int theron_v1_track01_cdda_stream_start(
    const Theron_Track01CddaHandoff *handoff,
    Theron_Track01CddaStream *out_stream);
int theron_v1_track01_cdda_stream_pump(Theron_Track01CddaStream *stream);
void theron_v1_track01_cdda_stream_stop(Theron_Track01CddaStream *stream);
int theron_v1_track01_cdda_lifecycle_update(
    const Theron_Track01CddaHandoff *handoff,
    int title_active,
    Theron_Track01CddaStream *stream);

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

/* Resolve an operator-supplied Track 02 path to the payload that the existing
 * hash-gated decoders consume.  A plain BIN/ISO path is returned unchanged.
 * A CUE path is accepted only when it declares exactly one `TRACK 02 MODE1/2352`
 * entry backed by a preceding `FILE "..." BINARY` declaration.  The returned
 * payload must be readable.  This is media mounting only: it does not inspect
 * or decode the payload, and callers must still hash-verify it before use. */
Theron_Track02SignalStatus theron_v1_track02_resolve_media_path(
    const char *media_path,
    char out_payload_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]);

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

Theron_Track02SignalStatus theron_v1_track02_copy_raw_user_data_range(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t raw_offset,
    size_t byte_count,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_user_data_offset);

typedef enum {
    THERON_TRACK02_USER_DATA_WINDOW_UNKNOWN = 0,
    THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE,
    THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN,
    THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE
} Theron_Track02UserDataWindowRole;

typedef struct {
    Theron_Track02UserDataWindowRole role;
    size_t raw_offset;
    size_t user_data_offset;
    size_t byte_count;
    size_t anchor_index;
    size_t candidate_index;
} Theron_Track02UserDataWindow;

typedef struct {
    Theron_Track02Variant variant;
    size_t entry_count;
    size_t overflow_count;
    Theron_Track02UserDataWindow
        entries[THERON_TRACK02_MAX_USER_DATA_WINDOWS];
} Theron_Track02UserDataWindowCatalog;

Theron_Track02SignalStatus theron_v1_track02_catalog_user_data_windows(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02UserDataWindowCatalog *out_catalog);

Theron_Track02SignalStatus theron_v1_track02_copy_user_data_window_by_role(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02UserDataWindowRole role,
    size_t occurrence_index,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_byte_count,
    Theron_Track02UserDataWindow *out_window);

const char *theron_v1_track02_user_data_window_role_name(
    Theron_Track02UserDataWindowRole role);

typedef enum {
    THERON_TRACK02_STARTUP_TEXT_UNKNOWN = 0,
    THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT,
    THERON_TRACK02_STARTUP_TEXT_JP_CHAMPION_ROSTER_CLUSTER
} Theron_Track02StartupTextMarkerKind;

typedef struct {
    Theron_Track02StartupTextMarkerKind kind;
    size_t raw_offset;
    size_t user_data_offset;
    size_t byte_count;
    size_t occurrence_index;
} Theron_Track02StartupTextMarker;

typedef struct {
    Theron_Track02Variant variant;
    size_t marker_count;
    size_t overflow_count;
    Theron_Track02StartupTextMarker
        markers[THERON_TRACK02_MAX_STARTUP_TEXT_MARKERS];
} Theron_Track02StartupTextMarkerCatalog;

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_text_markers(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupTextMarkerCatalog *out_catalog);

const char *theron_v1_track02_startup_text_marker_kind_name(
    Theron_Track02StartupTextMarkerKind kind);

Theron_Track02SignalStatus theron_v1_track02_copy_startup_text_marker(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupTextMarkerKind kind,
    size_t occurrence_index,
    char *out_text,
    size_t out_text_capacity,
    size_t *out_byte_count,
    Theron_Track02StartupTextMarker *out_marker);

typedef struct {
    char name[THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    char title[THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
    size_t raw_offset;
    size_t user_data_offset;
    size_t title_raw_offset;
    size_t title_user_data_offset;
    int title_offset_valid;
} Theron_Track02StartupRosterName;

typedef struct {
    Theron_Track02Variant variant;
    size_t name_count;
    size_t overflow_count;
    Theron_Track02StartupRosterName
        names[THERON_TRACK02_MAX_STARTUP_ROSTER_NAMES];
} Theron_Track02StartupRosterNameCatalog;

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_roster_names(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupRosterNameCatalog *out_catalog);

typedef struct {
    unsigned int route_bit;
    size_t raw_offset;
    size_t user_data_offset;
    size_t byte_count;
    uint8_t width;
    uint8_t height;
    uint8_t bpp;
    size_t nonzero_pixel_count;
    uint32_t checksum;
    uint8_t pixels[THERON_TRACK02_STARTUP_BITMAP_PIXELS];
} Theron_Track02StartupBitmapSample;

typedef struct {
    Theron_Track02Variant variant;
    size_t sample_count;
    size_t overflow_count;
    unsigned int route_mask;
    Theron_Track02StartupBitmapSample
        samples[THERON_TRACK02_MAX_STARTUP_BITMAP_SAMPLES];
} Theron_Track02StartupBitmapCatalog;

typedef struct {
    unsigned int route_bit;
    size_t tile_count;
    uint16_t width;
    uint16_t height;
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    size_t raw_offsets[THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH / 8u];
    size_t user_data_offsets[THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH / 8u];
    size_t nonzero_pixel_count;
    uint32_t checksum;
    uint8_t pixels[THERON_TRACK02_STARTUP_BITMAP_ATLAS_PIXELS];
} Theron_Track02StartupBitmapAtlasRoute;

typedef struct {
    Theron_Track02Variant variant;
    size_t route_count;
    size_t overflow_count;
    unsigned int route_mask;
    size_t promoted_wide_tile_count;
    unsigned int promoted_wide_route_mask;
    size_t total_tile_count;
    size_t total_nonzero_pixel_count;
    uint32_t checksum;
    Theron_Track02StartupBitmapAtlasRoute
        routes[THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX];
} Theron_Track02StartupBitmapAtlas;

Theron_Track02SignalStatus theron_v1_track02_decode_4bpp_tile(
    const uint8_t *tile_bytes,
    size_t tile_size,
    uint8_t out_pixels[THERON_TRACK02_STARTUP_BITMAP_PIXELS],
    size_t *out_nonzero_pixel_count,
    uint32_t *out_checksum);

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_bitmap_samples(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupBitmapCatalog *out_catalog);

Theron_Track02SignalStatus theron_v1_track02_build_startup_bitmap_atlas(
    const Theron_Track02StartupBitmapCatalog *catalog,
    Theron_Track02StartupBitmapAtlas *out_atlas);

Theron_Track02SignalStatus theron_v1_track02_build_startup_bitmap_atlas_wide(
    const Theron_Track02StartupBitmapCatalog *catalog,
    Theron_Track02StartupBitmapAtlas *out_atlas);

/* HuC6260/VCE palette payload used by one 4bpp tile bank: sixteen
 * little-endian 9-bit words. Bits 0..2 are blue, 3..5 red, and 6..8 green;
 * bits 9..15 must be clear. See
 * docs/source-lock/tqr_v1_huc6260_palette_word_format_2026-07-11.md.
 * Track 02 image
 * discovery deliberately does not guess a palette address: callers must
 * provide the exact, separately verified 32-byte payload. */
typedef struct {
    uint16_t raw_word;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Theron_Track02PaletteEntry;

typedef struct {
    Theron_Track02PaletteEntry entries[THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT];
    size_t nonblack_entry_count;
    uint32_t checksum;
    int valid;
} Theron_Track02Palette4Bpp;

typedef struct {
    int valid;
    unsigned int route_bit;
    uint16_t width;
    uint16_t height;
    uint32_t checksum;
    uint8_t rgba[THERON_TRACK02_STARTUP_BITMAP_ATLAS_PIXELS * 4u];
} Theron_Track02StartupBitmapRgbaRoute;

/* Evidence for a palette window whose offset was supplied by an external
 * source analysis.  This records only byte provenance and HuC6270 syntax.
 * It deliberately has no route binding: a palette-shaped 32-byte span is
 * not proof that the game loads it for title, stage, Soul Room, or
 * forcefield.  `promotion_allowed` therefore remains zero until a future,
 * source-locked loader binding can populate it. */
typedef struct {
    Theron_Track02Variant variant;
    size_t raw_offset;
    size_t user_data_offset;
    size_t byte_count;
    uint32_t payload_checksum;
    int raw_offset_is_user_data;
    int format_valid;
    int semantic_binding_verified;
    int promotion_allowed;
    Theron_Track02Palette4Bpp palette;
} Theron_Track02PaletteWindowEvidence;

/* These routines form the palette half of the Track 02 bitmap route.  A
 * malformed palette or an incomplete indexed route produces no RGBA output;
 * callers must not promote a fallback palette through this API. */
Theron_Track02SignalStatus theron_v1_track02_decode_4bpp_palette(
    const uint8_t *palette_bytes,
    size_t palette_size,
    Theron_Track02Palette4Bpp *out_palette);

Theron_Track02SignalStatus theron_v1_track02_colorize_startup_bitmap_route(
    const Theron_Track02StartupBitmapAtlasRoute *indexed_route,
    const Theron_Track02Palette4Bpp *palette,
    Theron_Track02StartupBitmapRgbaRoute *out_route);

/* Inspect one explicitly supplied palette payload offset.  The MD5 must name
 * a known Track 02 variant.  Raw BIN offsets are copied only through MODE1
 * user-data bytes; the US ISO has a plain 2048-byte user-data stream.  No
 * scanner calls this function and no successful result is a semantic or
 * render-promotion claim. */
Theron_Track02SignalStatus theron_v1_track02_inspect_4bpp_palette_window(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t raw_offset,
    Theron_Track02PaletteWindowEvidence *out_evidence);

int theron_v1_track02_palette_window_evidence_can_promote(
    const Theron_Track02PaletteWindowEvidence *evidence);

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

/* Raw-sector receipt for descriptor-derived, non-startup windows.
 *
 * The raw JP/US Track 02 images are MODE1/2352 dumps.  The replicated
 * descriptor table supplies exact 0x0400-byte windows, but independent
 * Theron's Quest loader evidence does not identify any post-descriptor
 * window as a bitmap, level, object, palette, or text payload.  This
 * receipt therefore records only container facts for windows structurally
 * classified as POST_DESCRIPTOR_DATA: descriptor entry index, physical
 * sector/user-data bounds, logical 2048-byte-stream bounds, and a raw-byte
 * fingerprint.  It deliberately cannot produce bytes for a runtime route.
 */
#define THERON_TRACK02_MAX_NONSTARTUP_SECTOR_WINDOWS \
    THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES

typedef struct {
    size_t descriptor_entry_index;
    size_t raw_offset;
    size_t byte_count;
    size_t first_raw_sector;
    size_t last_raw_sector;
    size_t first_sector_user_data_offset;
    size_t last_sector_user_data_offset;
    size_t user_data_offset;
    size_t user_data_end_offset;
    int first_raw_byte_is_user_data;
    int crosses_raw_sector_boundary;
    int raw_span_contains_non_user_data;
    int user_data_span_contiguous;
    uint32_t raw_span_hash;
    int opaque;
    int promotion_blocked;
} Theron_Track02NonstartupSectorWindowReceipt;

typedef struct {
    Theron_Track02Variant variant;
    size_t anchor_count;
    /* Exact raw descriptor anchors are retained solely so a JP/US receipt
     * comparison can prove descriptor-relative window geometry.  They do
     * not identify any payload semantics. */
    size_t descriptor_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t window_count[THERON_TRACK02_MAX_BANK_ANCHORS];
    Theron_Track02NonstartupSectorWindowReceipt windows
        [THERON_TRACK02_MAX_BANK_ANCHORS]
        [THERON_TRACK02_MAX_NONSTARTUP_SECTOR_WINDOWS];
    int valid;
    int verified_track02;
    int opaque_only;
    int promotion_blocked;
    uint32_t receipt_hash;
} Theron_Track02NonstartupSectorReceipt;

#define THERON_TRACK02_MAX_NONSTARTUP_CONTAINERS \
    (THERON_TRACK02_MAX_BANK_ANCHORS * \
     THERON_TRACK02_MAX_NONSTARTUP_SECTOR_WINDOWS)

typedef struct {
    size_t user_data_offset;
    size_t byte_count;
    uint32_t hash;
} Theron_Track02NonstartupContainerSegment;

typedef struct {
    size_t descriptor_entry_index;
    size_t raw_offset;
    size_t user_data_offset;
    size_t user_data_byte_count;
    uint32_t user_data_hash;
    size_t user_data_segment_count;
    Theron_Track02NonstartupContainerSegment user_data_segments
        [THERON_TRACK02_MAX_USER_DATA_WINDOWS];
    int opaque;
    int promotion_blocked;
} Theron_Track02NonstartupContainer;

typedef struct {
    Theron_Track02Variant variant;
    size_t container_count;
    Theron_Track02NonstartupContainer containers
        [THERON_TRACK02_MAX_NONSTARTUP_CONTAINERS];
    int valid;
    int verified_track02;
    int opaque_only;
    int promotion_blocked;
    uint32_t index_hash;
} Theron_Track02NonstartupContainerIndex;

/* Cross-region comparison of opaque post-descriptor windows.
 *
 * This is deliberately a layout observation, not a decoder.  A matching
 * layout means only that hash-verified JP and US raw BINs expose the same
 * descriptor entry, byte length, descriptor-relative raw offset, and MODE1
 * container shape.  Byte equality is reported independently because region
 * variants can legitimately differ.  Neither result identifies a bitmap,
 * palette, level, object, text, or runtime payload.
 */
typedef enum {
    THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_OK = 1,
    THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_BAD_INPUT = -1,
    THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT = -2,
    THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR = -3
} Theron_Track02NonstartupSectorLayoutComparisonStatus;

typedef struct {
    int valid;
    int opaque_only;
    int promotion_blocked;
    Theron_Track02NonstartupSectorLayoutComparisonStatus status;
    Theron_Track02Variant jp_variant;
    Theron_Track02Variant us_variant;
    unsigned int comparable_anchor_mask;
    unsigned int layout_matching_anchor_mask;
    unsigned int content_matching_anchor_mask;
    unsigned int content_mismatch_anchor_mask;
    size_t window_counts[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t layout_fingerprint[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t comparison_hash;
} Theron_Track02NonstartupSectorLayoutComparisonReceipt;

/* Capture physical and logical container boundaries for the post-descriptor
 * windows at every known raw-BIN descriptor anchor.  `md5_hex` must name one
 * of the two known raw Track 02 BIN variants; callers are responsible for
 * independently checking the supplied file MD5 before calling, as with the
 * rest of this module's real-media helpers.  No result assigns payload
 * semantics or enables a level/bitmap/object/palette/text route. */
Theron_Track02SignalStatus theron_v1_track02_capture_nonstartup_sector_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02NonstartupSectorReceipt *out_receipt);

/* Build a conservative opaque container index from the real raw-BIN
 * non-startup sector receipt. Only already verified, contiguous user-data
 * windows are indexed. The index does not decode object, level, bitmap,
 * palette, text, or runtime semantics and keeps promotion blocked. */
Theron_Track02SignalStatus theron_v1_track02_build_nonstartup_container_index(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02NonstartupContainerIndex *out_index);

const char *theron_v1_track02_nonstartup_sector_layout_comparison_status_name(
    Theron_Track02NonstartupSectorLayoutComparisonStatus status);

Theron_Track02NonstartupSectorLayoutComparisonStatus
theron_v1_track02_compare_nonstartup_sector_layout_variants(
    const Theron_Track02NonstartupSectorReceipt *first,
    const Theron_Track02NonstartupSectorReceipt *second,
    Theron_Track02NonstartupSectorLayoutComparisonReceipt *out_receipt);

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
    size_t user_data_offset;
    int user_data_offset_valid;
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
    size_t user_data_offset;
    int user_data_offset_valid;
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

/* Physical CD-record boundary for the one source-locked startup level
 * payload.  It joins the IPL's authenticated Track 02 INDEX 01 coordinate
 * with the existing descriptor-relative level receipt.  `object_*` names
 * the byte boundary immediately following the 32x27 level payload only; it
 * does not assert that the following bytes form an object table.  They remain
 * opaque until an original loader read establishes that semantic role.
 *
 * The known JP and US raw corpora agree on record 0x0b52, user-data offset
 * 0x114, a 0x36c-byte level envelope, and the opaque 0x0103 two-byte header
 * extension at offsets 10..11. No meaning is assigned to that extension, and
 * no tiles, objects, or runtime world state are returned by this receipt. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t data_track_index01_raw_sector;
    size_t level_first_raw_sector;
    size_t level_raw_offset;
    size_t level_user_data_offset;
    size_t level_user_data_offset_in_record;
    size_t level_byte_count;
    size_t object_boundary_raw_offset;
    size_t object_boundary_user_data_offset;
    size_t object_boundary_user_data_offset_in_record;
    size_t following_user_data_bytes_in_record;
    /* Exact raw-media witness for the following opaque bytes. This proves
     * continuity only; it does not identify an object-table format. */
    uint32_t following_user_data_hash;
    uint16_t level_width;
    uint16_t level_height;
    uint32_t level_seed;
    uint16_t level_index;
    uint16_t level_header_extension_be;
    uint32_t level_payload_hash;
    int object_table_parsed;
    int object_table_semantics_proven;
    int promotion_blocked;
    uint32_t receipt_hash;
} Theron_Track02InitialLevelObjectBoundaryReceipt;

/* Exact MODE1/2048 projection of a complete canonical raw Track 02 image.
 * This admits a full extracted ISO only when every user-data sector matches
 * the selected hash-verified BIN from INDEX 01 through end of track. It maps
 * the known first-level envelope and its opaque continuation into ISO byte
 * coordinates, but does not assign object-record grammar to that tail. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    size_t data_track_index01_raw_sector;
    size_t iso_sector_count;
    size_t iso_byte_count;
    uint32_t first_level_track02_record;
    size_t first_level_iso_user_data_offset;
    size_t first_level_byte_count;
    uint32_t first_level_hash;
    size_t post_envelope_iso_user_data_offset;
    size_t post_envelope_byte_count;
    uint32_t post_envelope_hash;
    int object_semantics_proven;
    int fallback_allowed;
    uint32_t receipt_hash;
} Theron_Track02CanonicalIsoProjectionReceipt;

/* Real-media decode of the source-locked initial level envelope.
 *
 * This receipt promotes only the level envelope facts established by the
 * hash-gated JP/US corpus and the descriptor-relative loader candidate:
 * the four decoded header fields and the exact following grid byte span.
 * Grid bytes remain byte-faithful data; no square, object, pose, trigger,
 * palette, or visual semantics are inferred here.  In particular, the
 * 0x0103 extension is retained as an opaque fingerprint, and the bytes after
 * the grid remain an unparsed tail. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t level_raw_offset;
    size_t level_user_data_offset;
    size_t level_byte_count;
    uint16_t width;
    uint16_t height;
    uint32_t seed;
    uint16_t level_index;
    uint16_t header_extension_be;
    size_t grid_offset_in_envelope;
    size_t grid_byte_count;
    uint32_t grid_hash;
    int header_semantics_proven;
    int grid_semantics_proven;
    int header_extension_semantics_proven;
    int object_tail_semantics_proven;
    int fallback_visuals_allowed;
    uint32_t receipt_hash;
} Theron_Track02InitialLevelEnvelopeReceipt;

/* Loader-backed semantic projection of the authenticated startup grid.
 *
 * This is intentionally a receipt, not a world route: it runs the exact
 * envelope bytes through theron_v1_level_load() and retains only the
 * loader's bounded 5-bit cell classification and selected entrance pose.
 * `raw_high_bit_cell_count` records cells whose source byte carried bits that
 * the existing loader masks away, so consumers cannot mistake this projection
 * for a claim about every raw-bit meaning.  No object records, transitions,
 * artwork, or runtime level are published.  The header extension and the
 * bytes following the grid remain opaque and blocked.
 */
#define THERON_TRACK02_LOADER_TILE_VALUE_COUNT 32u

typedef struct {
    int valid;
    Theron_Track02InitialLevelEnvelopeReceipt envelope;
    Theron_MapLoadResult map_status;
    size_t grid_cell_count;
    size_t raw_high_bit_cell_count;
    unsigned int loader_tile_value_mask;
    size_t loader_tile_value_counts[THERON_TRACK02_LOADER_TILE_VALUE_COUNT];
    int start_x;
    int start_y;
    int start_dir;
    int loader_grid_semantics_proven;
    int header_extension_semantics_proven;
    int object_tail_semantics_proven;
    int fallback_visuals_allowed;
    uint32_t receipt_hash;
} Theron_Track02InitialLevelLoaderSemanticReceipt;

/* The first positive Track 02 dungeon route. It owns only the authenticated
 * Hall of Records level-0 grid accepted by the existing level loader. */
typedef struct {
    int valid;
    int dungeon_id;
    int sub_level_index;
    Theron_V1_Level level;
    Theron_Track02InitialLevelLoaderSemanticReceipt semantics;
    int object_tail_semantics_proven;
    int fallback_visuals_allowed;
    uint32_t route_hash;
} Theron_Track02InitialLevelLoaderRoute;

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
 * theron_v1_level_load().  JP/US raw BINs are MODE1/2352 images, so an
 * anchor-verified candidate that crosses a sector user-data boundary is
 * reconstructed through the logical MODE1/2048 stream rather than rejected
 * for containing sector framing bytes.
 *
 * This is narrower than a full dungeon parser: it promotes one real startup
 * candidate and keeps all other Track 02 map/object semantics unclaimed.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_load_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
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

/* Annotate a raw Track 02 candidate catalog with logical 2048-byte user-data
 * offsets.
 *
 * `theron_v1_track02_scan_level_candidates()` reports raw byte offsets because
 * it scans the caller-supplied Track 02 byte image directly.  For JP/US raw
 * BINs, downstream menu/art/dungeon decoders usually need the matching offset
 * in the stripped MODE1/2048 user-data stream.  This helper preserves the raw
 * offset while filling `candidate.user_data_offset` and
 * `candidate.user_data_offset_valid` for candidates that begin inside a
 * sector's 2048-byte user-data area.
 *
 * This is an offset-binding helper only. It does not reinterpret level
 * candidates as graphics, menu art, text, palettes, objects, or broader dungeon
 * records.
 */
Theron_Track02SignalStatus theron_v1_track02_bind_level_candidate_user_offsets(
    size_t track02_size,
    const char *md5_hex,
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
    const char *md5_hex,
    size_t descriptor_offset,
    Theron_Track02InitialCandidateBinding *out_binding);

/* Parse the CD-record-relative boundary of the known initial level payload.
 * This is a real-media receipt: it accepts only the known raw JP/US hashes,
 * validates the IPL/INDEX 01 record coordinate, and reuses the strict
 * descriptor-relative candidate binder.  It never decodes or exposes the
 * opaque bytes after the level envelope as an object table. */
Theron_Track02SignalStatus
theron_v1_track02_capture_initial_level_object_boundary(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelObjectBoundaryReceipt *out_receipt);

/* Verify a complete ISO's bytes against the original raw Track 02 user-data
 * lane and return only the established first-level byte intervals. A short,
 * shifted, altered, or non-canonical ISO is rejected. */
Theron_Track02SignalStatus theron_v1_track02_verify_canonical_iso_projection(
    const uint8_t *raw_track02_data,
    size_t raw_track02_size,
    const char *raw_track02_md5,
    const uint8_t *iso_data,
    size_t iso_size,
    Theron_Track02CanonicalIsoProjectionReceipt *out_receipt);

/* Decode the corroborated initial-level envelope from authenticated raw JP/US
 * Track 02 media.  This is deliberately a no-visual, no-object receipt: it
 * has no synthetic-media path and never exposes or interprets the tail after
 * the grid. */
Theron_Track02SignalStatus theron_v1_track02_decode_initial_level_envelope(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelEnvelopeReceipt *out_receipt);

/* Historical compatibility entry point for the former startup-grid decoder.
 *
 * The source-locked Stage 2 disassembly now proves only the dynamic `$e009`
 * transfer and its control handoff to banked local RAM at `$3800`. It does
 * not prove that the selected sector, including its level-shaped bytes, is a
 * dungeon grid. This API therefore fail-closes for authentic media until a
 * game-owned post-$3800 consumer establishes the record grammar. The
 * byte-level envelope and loader-transfer receipts remain available to
 * capture tooling; neither may be promoted through this entry point. */
Theron_Track02SignalStatus
theron_v1_track02_decode_initial_level_loader_semantics(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelLoaderSemanticReceipt *out_receipt);

/* Historical compatibility entry point for the former Hall of Records route.
 * It remains fail-closed until the original game-owned post-$3800 consumer
 * proves a dungeon-record grammar and ownership relation. */
Theron_Track02SignalStatus theron_v1_track02_load_initial_level_loader_route(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    int dungeon_id,
    int sub_level_index,
    Theron_Track02InitialLevelLoaderRoute *out_route);

/* Copy the hash/anchor-gated initial startup candidate through the logical
 * MODE1/2048 user-data address space.
 *
 * The returned bytes are the same bounded 32x27 startup payload accepted by
 * theron_v1_track02_load_initial_level_candidate(), but the source address is
 * validated against the stripped user-data stream and `out_user_data_offset`
 * reports the logical offset.  This is decoder infrastructure only: it does
 * not claim the bytes are menu art, champion art, text, palettes, objects, or
 * any broader dungeon record.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_copy_initial_level_user_data_window(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_byte_count,
    size_t *out_user_data_offset);

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
 * This release binds two descriptor entries to bounded semantic roles: the
 * first entry (descriptor.entry_index == 0, window relative offset
 * 0x0020) is bound to the role THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE,
 * which reads the documented 7 × uint32 little-endian dungeon_seeds from
 * tqr_v1_phase2_data_formats_H2339.md §9.1 (a.k.a. the THQUEST.ASM T560
 * `dungeon_seed` table, sourced from theron_v1_boot.c:318-345 /
 * theron_v1_dungeon_progression.c:38-110); entry 5 restates the
 * descriptor-table-bearing window. The former entry-6 compact-row claim was
 * disproven by the hash-verified JP/US raw Track 02 receipts and is not a
 * semantic binding or decoder route.
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

#define THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS 64u
#define THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES 8u

typedef struct {
    uint8_t object_id;
    uint8_t kind;
    uint8_t x;
    uint8_t y;
    uint8_t level_index;
    uint8_t flags;
    uint16_t argument;
} Theron_Track02ObjectTableRecord;

typedef enum {
    THERON_TRACK02_OBJECT_TABLE_REJECT_NONE = 0,
    THERON_TRACK02_OBJECT_TABLE_REJECT_ZERO_COUNT,
    THERON_TRACK02_OBJECT_TABLE_REJECT_DECLARED_OVERFLOW,
    THERON_TRACK02_OBJECT_TABLE_REJECT_WINDOW_TOO_SMALL,
    THERON_TRACK02_OBJECT_TABLE_REJECT_ZERO_OBJECT_ID,
    THERON_TRACK02_OBJECT_TABLE_REJECT_ZERO_KIND,
    THERON_TRACK02_OBJECT_TABLE_REJECT_X_OUT_OF_RANGE,
    THERON_TRACK02_OBJECT_TABLE_REJECT_Y_OUT_OF_RANGE,
    THERON_TRACK02_OBJECT_TABLE_REJECT_LEVEL_OUT_OF_RANGE
} Theron_Track02ObjectTableRejectReason;

typedef struct {
    size_t declared_record_count;
    size_t record_count;
    size_t overflow_count;
    size_t required_byte_count;
    size_t byte_count;
    size_t nonzero_byte_count;
    uint32_t checksum;
    int shape_ok;
    size_t first_bad_record_index;
    Theron_Track02ObjectTableRejectReason reject_reason;
    /* Observation only: accepted compact rows are bucketed by their existing
     * level byte so real-media analysis can prove multi-level coverage without
     * assigning those rows to gameplay objects or a dungeon route. */
    unsigned int level_mask;
    size_t level_record_counts[THERON_TRACK02_DUNGEON_COUNT];
    uint32_t level_record_hashes[THERON_TRACK02_DUNGEON_COUNT];
    /* Layout evidence only.  These retain the compact-table ordinal at
     * which a level's accepted rows begin/end, plus a hash that includes
     * each ordinal before its eight raw row bytes.  They do not assign
     * semantics to the row fields. */
    size_t level_first_record_indexes[THERON_TRACK02_DUNGEON_COUNT];
    size_t level_last_record_indexes[THERON_TRACK02_DUNGEON_COUNT];
    uint32_t level_position_hashes[THERON_TRACK02_DUNGEON_COUNT];
    Theron_Track02ObjectTableRecord
        records[THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS];
} Theron_Track02ObjectTable;

/* A decoded Track 02 dungeon route is deliberately a single transaction:
 * bitmap evidence, a loader-accepted level, and one bounded object table
 * must all be present before a caller can publish it to the live world.
 * This avoids the older split route where a real map could be paired with
 * synthetic objects (or vice versa). */
typedef enum {
    THERON_TRACK02_DUNGEON_ROUTE_OK = 1,
    THERON_TRACK02_DUNGEON_ROUTE_NOT_FOUND = 0,
    THERON_TRACK02_DUNGEON_ROUTE_BAD_INPUT = -1,
    THERON_TRACK02_DUNGEON_ROUTE_LEVEL_REJECTED = -2,
    THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED = -3,
    THERON_TRACK02_DUNGEON_ROUTE_BITMAP_REJECTED = -4
} Theron_Track02DungeonRouteStatus;

typedef struct {
    int valid;
    Theron_Track02DungeonRouteStatus status;
    size_t descriptor_offset;
    int dungeon_id;
    int level_index;
    size_t level_entry_index;
    size_t level_raw_offset;
    size_t level_byte_count;
    size_t object_entry_index;
    size_t object_raw_offset;
    Theron_V1_Level level;
    Theron_Track02ObjectTable objects;
    Theron_Track02StartupBitmapAtlas bitmap_atlas;
    uint32_t checksum;
} Theron_Track02DungeonRoute;

/* A route catalog is a bounded, all-or-nothing selection surface for the
 * decoded level/object transactions above.  Its entries may arrive in any
 * order, but they must form one contiguous level sequence starting at 0 for
 * one dungeon.  This prevents a caller from skipping an unverified middle
 * level or silently choosing the first of duplicate candidates. */
typedef enum {
    THERON_TRACK02_ROUTE_CATALOG_OK = 1,
    THERON_TRACK02_ROUTE_CATALOG_NOT_FOUND = 0,
    THERON_TRACK02_ROUTE_CATALOG_BAD_INPUT = -1,
    THERON_TRACK02_ROUTE_CATALOG_ROUTE_REJECTED = -2,
    THERON_TRACK02_ROUTE_CATALOG_DUNGEON_MISMATCH = -3,
    THERON_TRACK02_ROUTE_CATALOG_DUPLICATE_LEVEL = -4,
    THERON_TRACK02_ROUTE_CATALOG_NONCONTIGUOUS = -5
} Theron_Track02RouteCatalogStatus;

typedef struct {
    Theron_Track02RouteCatalogStatus status;
    int dungeon_id;
    int requested_level_index;
    size_t route_count;
    unsigned int level_mask;
    size_t matching_route_count;
    uint32_t catalog_checksum;
    int selected;
} Theron_Track02RouteCatalogReceipt;

/* A Track 02 level transition is permitted only after the target has been
 * assembled as one complete, validated dungeon route.  In particular, a
 * level record cannot be promoted on its own while its object-table or
 * bitmap evidence is missing. */
typedef enum {
    THERON_TRACK02_LEVEL_TRANSITION_OK = 1,
    THERON_TRACK02_LEVEL_TRANSITION_NOT_PENDING = 0,
    THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT = -1,
    THERON_TRACK02_LEVEL_TRANSITION_SOURCE_REJECTED = -2,
    THERON_TRACK02_LEVEL_TRANSITION_TARGET_REJECTED = -3,
    THERON_TRACK02_LEVEL_TRANSITION_TARGET_MISMATCH = -4
} Theron_Track02LevelTransitionStatus;

typedef struct {
    int applied;
    Theron_Track02LevelTransitionStatus status;
    int dungeon_id;
    int source_level_index;
    int target_level_index;
    size_t target_object_record_count;
    uint32_t source_route_checksum;
    uint32_t target_route_checksum;
} Theron_Track02LevelTransitionReceipt;

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
    /* Populated when role == OBJECT_TABLE.  This is a bounded compact-row
     * shape claim only; it does not assign gameplay behavior to objects. */
    Theron_Track02ObjectTable object_table;
    Theron_Track02SemanticBindingStatus status;
} Theron_Track02SemanticBinding;

/* Map a descriptor entry index to a semantic role.
 *
 * The mapping is the documented working hypothesis from this header's
 * source-locks section: entry 0 is the dungeon_seed table, entry 5 is
 * the descriptor-table-bearing window, and every other entry is UNKNOWN.
 * Returning UNKNOWN is not an error; it simply means no semantic role is
 * currently bound. */
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

/* Read a compact count-prefixed object table.
 *
 * Shape gate:
 *   - first uint16 little-endian word is the record count
 *   - count is 1..THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS
 *   - each 8-byte row has nonzero object_id and kind
 *   - x < 32, y < 27, level_index < THERON_TRACK02_DUNGEON_COUNT
 *
 * The decoder records coordinates, flags, and a stable checksum only.  It
 * does not claim item type semantics, monster behavior, triggers, or runtime
 * object spawning. */
Theron_Track02SemanticBindingStatus theron_v1_track02_read_object_table(
    const uint8_t *object_bytes,
    size_t object_size,
    Theron_Track02ObjectTable *out_table);

/* Build one descriptor-local dungeon route.
 *
 * No descriptor-local level/object layout is currently supported by original
 * Track 02 evidence. This API returns OBJECT_REJECTED after input and atlas
 * validation; it must not infer a route from arbitrary data windows. The
 * separately hash-gated startup candidate remains the only supported level
 * decoder route. */
Theron_Track02DungeonRouteStatus theron_v1_track02_build_dungeon_route(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    const Theron_Track02StartupBitmapAtlas *bitmap_atlas,
    Theron_Track02DungeonRoute *out_route);

/* Hash/anchor-gated Track 02 route constructor. It remains blocked with
 * OBJECT_REJECTED until original media identifies a genuine non-startup
 * level/object handoff, and never combines the startup level with a guessed
 * descriptor window. */
Theron_Track02DungeonRouteStatus theron_v1_track02_load_verified_dungeon_route(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    int dungeon_id,
    const Theron_Track02StartupBitmapAtlas *bitmap_atlas,
    Theron_Track02DungeonRoute *out_route);

const char *theron_v1_track02_dungeon_route_status_name(
    Theron_Track02DungeonRouteStatus status);

const char *theron_v1_track02_route_catalog_status_name(
    Theron_Track02RouteCatalogStatus status);

/* Select one exact route from a fully validated contiguous catalog.
 *
 * Every supplied route must be a complete THERON_TRACK02_DUNGEON_ROUTE_OK
 * transaction for `dungeon_id`; levels must be unique and cover 0 through
 * route_count - 1 without a hole.  A missing, duplicate, mismatched, or
 * rejected route returns a non-OK status and leaves `out_route` NULL.  The
 * function never substitutes a neighbouring level or a fallback route. */
Theron_Track02RouteCatalogStatus theron_v1_track02_select_dungeon_route(
    const Theron_Track02DungeonRoute *routes,
    size_t route_count,
    int dungeon_id,
    int level_index,
    const Theron_Track02DungeonRoute **out_route,
    Theron_Track02RouteCatalogReceipt *out_receipt);

const char *theron_v1_track02_level_transition_status_name(
    Theron_Track02LevelTransitionStatus status);

/* Atomically install a validated Track 02 target route for a queued stairs
 * transition.  The caller supplies the already accepted source route so the
 * transition cannot pair a validated target level with an unchecked current
 * level.  Both routes must carry complete level/object/bitmap transactions;
 * a rejected transaction never produces alternate data for the world.
 *
 * The current Track 02 object record format is retained by the route receipt
 * rather than projected into the generic world-object database.  That
 * projection remains blocked until its real-media semantics are known. */
Theron_Track02LevelTransitionStatus theron_v1_track02_apply_level_transition(
    Theron_V1_World *world,
    const Theron_Track02DungeonRoute *source_route,
    const Theron_Track02DungeonRoute *target_route,
    Theron_Track02LevelTransitionReceipt *out_receipt);

/* Resolve both ends of a queued stairs transition from one catalog, then
 * atomically install the exact target through theron_v1_track02_apply_level_transition.
 * A catalog failure is converted to the corresponding source/target rejection
 * and leaves the world and queued transition untouched. */
Theron_Track02LevelTransitionStatus
theron_v1_track02_apply_level_transition_from_catalog(
    Theron_V1_World *world,
    const Theron_Track02DungeonRoute *routes,
    size_t route_count,
    Theron_Track02LevelTransitionReceipt *out_receipt,
    Theron_Track02RouteCatalogReceipt *out_source_receipt,
    Theron_Track02RouteCatalogReceipt *out_target_receipt);

const char *theron_v1_track02_semantic_binding_status_name(
    Theron_Track02SemanticBindingStatus status);

typedef struct {
    Theron_Track02LevelHandoffStatus status;
    size_t descriptor_offset;
    Theron_Track02SemanticBindingStatus seed_table_status;
    Theron_Track02SemanticBinding seed_table_binding;
    Theron_Track02InitialCandidateBinding initial_candidate;
    uint32_t startup_seed;
    uint16_t startup_level_index;
    int startup_seed_in_seed_table;
    size_t startup_seed_table_index;
    size_t user_data_offset;
    int user_data_offset_valid;
    int ready_for_runtime;
} Theron_Track02StartupSemanticHandoff;

typedef struct {
    int valid;
    Theron_Track02LevelHandoffStatus status;
    Theron_Track02SemanticBindingStatus seed_table_status;
    size_t descriptor_offset;
    size_t raw_offset;
    size_t byte_count;
    size_t user_data_offset;
    int user_data_offset_valid;
    uint16_t header_width;
    uint16_t header_height;
    uint32_t header_seed;
    uint16_t header_level_index;
    uint32_t progression_seed0;
    int ready_for_runtime;
    int fallback_visuals_allowed;
} Theron_Track02StartupRuntimeReceipt;

typedef struct {
    int valid;
    int verified_track02;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02Variant variant;
    int descriptor_route_ready;
    size_t descriptor_anchor_count;
    unsigned int descriptor_anchor_mask;
    size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t descriptor_entries_bound;
    unsigned int semantic_role_mask;
    size_t descriptor_table_semantic_count;
    size_t descriptor_table_semantic_anchor_count;
    unsigned int descriptor_table_semantic_anchor_mask;
    int object_table_role_mapped;
    size_t object_table_candidate_count;
    unsigned int object_table_candidate_anchor_mask;
    size_t object_table_candidate_entry_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_candidate_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_candidate_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int object_table_candidate_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_candidate_byte_counts[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_candidate_nonzero_byte_counts[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t object_table_candidate_header_width[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t object_table_candidate_header_height[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_candidate_header_seed[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t object_table_candidate_header_level_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    int object_table_candidate_header_matches_startup_shape[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_candidate_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_candidate_descriptor_delta[THERON_TRACK02_MAX_BANK_ANCHORS];
    int object_table_candidate_after_descriptor[THERON_TRACK02_MAX_BANK_ANCHORS];
    Theron_Track02DescriptorEntryRole object_table_candidate_entry_role[THERON_TRACK02_MAX_BANK_ANCHORS];
    Theron_Track02DescriptorWindowKind object_table_candidate_window_kind[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_blocked_anchor_count;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_anchor_binding_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_anchor_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_declared_record_count[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_record_count[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_required_byte_count[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_overflow_count[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_first_bad_record_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    Theron_Track02ObjectTableRejectReason
        object_table_reject_reason[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_record_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    unsigned int object_table_level_mask[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t object_table_level_record_counts[THERON_TRACK02_MAX_BANK_ANCHORS]
                                         [THERON_TRACK02_DUNGEON_COUNT];
    uint32_t object_table_level_record_hashes[THERON_TRACK02_MAX_BANK_ANCHORS]
                                             [THERON_TRACK02_DUNGEON_COUNT];
    size_t object_table_level_first_record_indexes[THERON_TRACK02_MAX_BANK_ANCHORS]
                                                  [THERON_TRACK02_DUNGEON_COUNT];
    size_t object_table_level_last_record_indexes[THERON_TRACK02_MAX_BANK_ANCHORS]
                                                 [THERON_TRACK02_DUNGEON_COUNT];
    uint32_t object_table_level_position_hashes[THERON_TRACK02_MAX_BANK_ANCHORS]
                                               [THERON_TRACK02_DUNGEON_COUNT];
    /* Cross-anchor observation for accepted compact rows.  A bit in
     * object_table_level_consensus_mask means every descriptor anchor carried
     * the same existing level byte, row count, row-byte FNV-1a hash, and
     * row ordinals.  This is evidence only: it neither identifies object
     * fields nor promotes a candidate into a route or runtime object. */
    unsigned int object_table_level_consensus_mask;
    unsigned int object_table_level_consensus_anchor_masks
        [THERON_TRACK02_DUNGEON_COUNT];
    size_t object_table_level_consensus_record_counts
        [THERON_TRACK02_DUNGEON_COUNT];
    uint32_t object_table_level_consensus_record_hashes
        [THERON_TRACK02_DUNGEON_COUNT];
    size_t object_table_level_consensus_first_record_indexes
        [THERON_TRACK02_DUNGEON_COUNT];
    size_t object_table_level_consensus_last_record_indexes
        [THERON_TRACK02_DUNGEON_COUNT];
    uint32_t object_table_level_consensus_position_hashes
        [THERON_TRACK02_DUNGEON_COUNT];
    int object_table_decode_ready;
    int blocked_for_missing_real_object_evidence;
    int fallback_visuals_allowed;
    uint32_t route_hash;
} Theron_Track02ObjectTableRouteReceipt;

/* Cross-variant comparison of the compact-row layout already accepted by
 * two independent raw Track 02 receipts.  This is deliberately narrower
 * than object decoding: it compares only per-level presence, row counts,
 * raw-row hashes and ordinal-bound hashes.  A matching bit is evidence that
 * the two media variants share a layout observation, not a claim about row
 * field meaning, a route, runtime objects, Continue, menus, or palettes. */
typedef enum {
    THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_OK = 1,
    THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_BAD_INPUT = -1,
    THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT = -2,
    THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR = -3
} Theron_Track02ObjectLayoutComparisonStatus;

typedef struct {
    int valid;
    Theron_Track02ObjectLayoutComparisonStatus status;
    Theron_Track02Variant jp_variant;
    Theron_Track02Variant us_variant;
    unsigned int jp_level_mask;
    unsigned int us_level_mask;
    unsigned int comparable_level_mask;
    unsigned int matching_level_mask;
    unsigned int mismatch_level_mask;
    size_t matching_record_counts[THERON_TRACK02_DUNGEON_COUNT];
    uint32_t matching_record_hashes[THERON_TRACK02_DUNGEON_COUNT];
    size_t matching_first_record_indexes[THERON_TRACK02_DUNGEON_COUNT];
    size_t matching_last_record_indexes[THERON_TRACK02_DUNGEON_COUNT];
    uint32_t matching_position_hashes[THERON_TRACK02_DUNGEON_COUNT];
    uint32_t comparison_hash;
} Theron_Track02ObjectLayoutComparisonReceipt;

typedef struct {
    int valid;
    int verified_track02;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02Variant variant;
    int descriptor_route_ready;
    size_t descriptor_anchor_count;
    unsigned int descriptor_anchor_mask;
    size_t startup_level_route_count;
    unsigned int startup_level_route_mask;
    size_t startup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_anchor_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t startup_level_anchor_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t startup_level_anchor_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_width[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_height[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t startup_level_anchor_seed[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_level_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_route_ready;
    size_t startup_descriptor_offset;
    size_t startup_raw_offset;
    size_t startup_user_data_offset;
    int startup_user_data_offset_valid;
    uint16_t startup_header_width;
    uint16_t startup_header_height;
    uint32_t startup_header_seed;
    uint16_t startup_header_level_index;
    unsigned int semantic_role_mask;
    int startup_level_grid_record_ready;
    size_t startup_level_grid_record_count;
    size_t startup_level_grid_descriptor_offset;
    size_t startup_level_grid_raw_offset;
    size_t startup_level_grid_user_data_offset;
    int startup_level_grid_user_data_offset_valid;
    int level_grid_role_mapped;
    size_t nonstartup_level_candidate_count;
    unsigned int nonstartup_level_candidate_anchor_mask;
    size_t
        nonstartup_level_candidate_sample_count
            [THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t
        nonstartup_level_candidate_sample_entry_index
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t
        nonstartup_level_candidate_sample_raw_offsets
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t
        nonstartup_level_candidate_sample_user_data_offsets
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    int
        nonstartup_level_candidate_sample_user_data_valid
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t
        nonstartup_level_candidate_sample_byte_counts
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t
        nonstartup_level_candidate_sample_descriptor_delta
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    uint32_t
        nonstartup_level_candidate_sample_hash
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    /* Raw-media layout evidence for every sampled post-descriptor data
     * window.  These fields intentionally do not bind a window to an object
     * table or a dungeon record: they preserve what the existing bounded
     * compact-row reader and big-endian header view observed so that staged
     * original JP/US Track 02 media can establish a layout first. */
    size_t
        nonstartup_level_candidate_sample_nonzero_byte_counts
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    uint16_t
        nonstartup_level_candidate_sample_header_width
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    uint16_t
        nonstartup_level_candidate_sample_header_height
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    uint32_t
        nonstartup_level_candidate_sample_header_seed
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    uint16_t
        nonstartup_level_candidate_sample_header_level_index
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    int
        nonstartup_level_candidate_sample_object_table_status
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t
        nonstartup_level_candidate_sample_object_table_declared_record_counts
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t
        nonstartup_level_candidate_sample_object_table_record_counts
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    unsigned int
        nonstartup_level_candidate_sample_object_table_level_masks
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    Theron_Track02ObjectTableRejectReason
        nonstartup_level_candidate_sample_object_table_reject_reasons
            [THERON_TRACK02_MAX_BANK_ANCHORS]
            [THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES];
    size_t nonstartup_level_candidate_entry_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t nonstartup_level_candidate_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t nonstartup_level_candidate_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int nonstartup_level_candidate_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t nonstartup_level_candidate_byte_counts[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t nonstartup_level_candidate_nonzero_byte_counts[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t nonstartup_level_candidate_header_width[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t nonstartup_level_candidate_header_height[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t nonstartup_level_candidate_header_seed[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t nonstartup_level_candidate_header_level_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    int nonstartup_level_candidate_header_matches_startup_shape[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t nonstartup_level_candidate_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t nonstartup_level_candidate_descriptor_delta[THERON_TRACK02_MAX_BANK_ANCHORS];
    int nonstartup_level_candidate_after_descriptor[THERON_TRACK02_MAX_BANK_ANCHORS];
    Theron_Track02DescriptorEntryRole nonstartup_level_candidate_entry_role[THERON_TRACK02_MAX_BANK_ANCHORS];
    Theron_Track02DescriptorWindowKind nonstartup_level_candidate_window_kind[THERON_TRACK02_MAX_BANK_ANCHORS];
    size_t nonstartup_level_blocked_anchor_count;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_decode_ready;
    int blocked_for_missing_nonstartup_level_evidence;
    int fallback_visuals_allowed;
    uint32_t route_hash;
} Theron_Track02LevelRouteReceipt;

/* Cross-variant comparison of opaque post-descriptor level candidates.
 * It compares only receipts captured from hash-verified JP and US raw BINs:
 * candidate count plus each sampled entry index, byte count,
 * descriptor-relative position and byte hash. Neither outcome identifies a
 * level or enables a route, runtime, Continue, palette, or fallback. */
typedef enum {
    THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_OK = 1,
    THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_BAD_INPUT = -1,
    THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT = -2,
    THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR = -3
} Theron_Track02NonstartupLevelLayoutComparisonStatus;

typedef struct {
    int valid;
    Theron_Track02NonstartupLevelLayoutComparisonStatus status;
    Theron_Track02Variant jp_variant;
    Theron_Track02Variant us_variant;
    unsigned int comparable_anchor_mask;
    unsigned int matching_anchor_mask;
    unsigned int mismatch_anchor_mask;
    size_t candidate_counts[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t matching_sample_hashes[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t comparison_hash;
} Theron_Track02NonstartupLevelLayoutComparisonReceipt;

/* Compose the bounded Track 02 startup evidence into one runtime-facing
 * handoff summary.
 *
 * This does not broaden the decoder claim: the only semantic descriptor role
 * consumed here is entry 0's DUNGEON_SEED_TABLE, and the only level payload is
 * the hash/anchor-gated 32x27 initial startup candidate.  ready_for_runtime is
 * true only when both gates are OK and the candidate has a logical MODE1/2048
 * user-data offset.  startup_seed_in_seed_table is reported as diagnostic
 * evidence only because the source-locked startup payload seed is distinct
 * from the seven progression dungeon seeds.  Real Track 02 images may still
 * report NO_LEVEL while the descriptor seed-window semantics are hardened.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_bind_startup_semantic_handoff(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    Theron_Track02StartupSemanticHandoff *out_handoff);

int theron_v1_track02_startup_runtime_receipt_from_handoff(
    const Theron_Track02StartupSemanticHandoff *handoff,
    Theron_Track02StartupRuntimeReceipt *out_receipt);

void theron_v1_track02_object_table_route_receipt_init(
    Theron_Track02ObjectTableRouteReceipt *receipt);

/* Descriptor-anchored object-table route evidence.
 *
 * This receipt is deliberately a no-fallback blocker unless entry 6's compact
 * object-table row candidate passes theron_v1_track02_read_object_table().
 * For verified Track 02 media, valid==1 with object_table_decode_ready==0
 * means callers have enough evidence to block synthetic object-table/runtime
 * visuals instead of silently falling back.
 */
int theron_v1_track02_capture_object_table_route_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02ObjectTableRouteReceipt *out_receipt);

const char *theron_v1_track02_object_layout_comparison_status_name(
    Theron_Track02ObjectLayoutComparisonStatus status);

/* Compare hash-gated JP and US raw-BIN compact-row observations.  Argument
 * order does not matter.  JP Rev 1/US ISO and same-region pairs are rejected
 * because this receipt is specifically a cross-region real-media comparison.
 * No failure path mutates either source receipt. */
Theron_Track02ObjectLayoutComparisonStatus
theron_v1_track02_compare_object_table_layout_variants(
    const Theron_Track02ObjectTableRouteReceipt *first,
    const Theron_Track02ObjectTableRouteReceipt *second,
    Theron_Track02ObjectLayoutComparisonReceipt *out_receipt);

void theron_v1_track02_level_route_receipt_init(
    Theron_Track02LevelRouteReceipt *receipt);

/* Descriptor-anchored level-route evidence.
 *
 * This receipt promotes the already source-locked startup level handoff as the
 * first real Track 02 level-grid/dungeon-record route and keeps broader/
 * non-startup level decoding blocked until real Track 02 evidence identifies
 * additional dungeon records.  Verified Track 02 media never falls back to
 * synthetic level visuals through this route.
 */
int theron_v1_track02_capture_level_route_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02LevelRouteReceipt *out_receipt);

const char *theron_v1_track02_nonstartup_level_layout_comparison_status_name(
    Theron_Track02NonstartupLevelLayoutComparisonStatus status);

Theron_Track02NonstartupLevelLayoutComparisonStatus
theron_v1_track02_compare_nonstartup_level_layout_variants(
    const Theron_Track02LevelRouteReceipt *first,
    const Theron_Track02LevelRouteReceipt *second,
    Theron_Track02NonstartupLevelLayoutComparisonReceipt *out_receipt);

/* Runtime-facing semantic startup level load.
 *
 * This is the narrowest promoted Track 02 startup path: it first composes the
 * semantic startup handoff above, then loads only that handoff's
 * hash/anchor-gated Hall of Records level-0 32x27 candidate through
 * theron_v1_level_load().
 * The caller gets both the semantic handoff (seed table + user-data offsets)
 * and the level-load handoff (header + map loader status).  It does not scan
 * descriptor DATA windows as levels and it does not claim broader dungeon,
 * object, menu-art, font, palette, or audio semantics.  Other dungeon ids
 * and nonzero level indexes are rejected: the same envelope cannot stand in
 * for an uncorrelated later Track 02 record.
 */
Theron_Track02LevelHandoffStatus theron_v1_track02_load_startup_semantic_level(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02StartupSemanticHandoff *out_semantic_handoff,
    Theron_Track02LevelHandoff *out_level_handoff);

/* Cross-variant graphics-format reconnaissance.
 *
 * This is intentionally narrower than the older non-startup transport
 * receipts.  It scans only JP/US MODE1 user-data sectors that agree exactly
 * at the documented JP n / US n+1 physical-sector displacement, looking for
 * two hardware-shaped byte forms:
 *
 *   - 16 HuC6260 9-bit palette words (32 bytes, first word black, at least
 *     eight distinct nonblack colours); and
 *   - eight LE uint16 values with a positive constant 0x20-aligned stride.
 *
 * Neither byte shape establishes ownership, a loader, compression, a tile
 * bank, or a display route.  The catalog is a strict next-evidence gate: no
 * candidate can enable a decoder until an original-loader reference binds
 * its exact user-data offset and intended payload length.
 */
#define THERON_TRACK02_MAX_GRAPHICS_FORMAT_CANDIDATES 64u

typedef enum {
    THERON_TRACK02_GRAPHICS_FORMAT_UNKNOWN = 0,
    THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP,
    THERON_TRACK02_GRAPHICS_FORMAT_LE16_STRIDE_TABLE
} Theron_Track02GraphicsFormat;

typedef struct {
    Theron_Track02GraphicsFormat format;
    size_t jp_raw_offset;
    size_t us_raw_offset;
    size_t jp_user_data_offset;
    size_t us_user_data_offset;
    size_t byte_count;
    uint32_t payload_checksum;
    uint16_t first_word;
    uint16_t stride;
    size_t value_count;
    size_t nonblack_or_distinct_count;
} Theron_Track02GraphicsFormatCandidate;

typedef struct {
    int valid;
    Theron_Track02Variant jp_variant;
    Theron_Track02Variant us_variant;
    size_t compared_sector_count;
    size_t matching_nonzero_sector_count;
    /* Total strict shapes observed before the bounded detail list.  These
     * counts make an overflowed real-media receipt auditable without turning
     * any repeated byte pattern into a decode or ownership claim. */
    size_t huc6260_palette_candidate_count;
    size_t le16_stride_table_candidate_count;
    size_t candidate_count;
    size_t overflow_count;
    int compression_signature_detected;
    int source_loader_binding_verified;
    int decoder_blocked;
    Theron_Track02GraphicsFormatCandidate
        candidates[THERON_TRACK02_MAX_GRAPHICS_FORMAT_CANDIDATES];
} Theron_Track02GraphicsFormatCatalog;

const char *theron_v1_track02_graphics_format_name(
    Theron_Track02GraphicsFormat format);

Theron_Track02SignalStatus theron_v1_track02_catalog_graphics_format_candidates(
    const uint8_t *jp_track02_data,
    size_t jp_track02_size,
    const char *jp_md5_hex,
    const uint8_t *us_track02_data,
    size_t us_track02_size,
    const char *us_md5_hex,
    Theron_Track02GraphicsFormatCatalog *out_catalog);

int theron_v1_track02_graphics_format_catalog_can_decode(
    const Theron_Track02GraphicsFormatCatalog *catalog);

/* Original PC Engine CD-ROM IPL loader receipt.
 *
 * The CUE-declared Track 01 is CD-DA narration; the bootstrap lives in the
 * MODE1 Track 02 data stream.  The second logical Track 02 sector is the
 * standard IPL information block.  On both known raw variants it selects
 * record 0x0003a3, local load/entry address 0x4000, and a 3-sector (JP) or
 * 4-sector (US) initial executable.  The executable's first verified
 * System-Card CD_READ call is at CPU address 0x40cd and asks for a local-RAM
 * destination at 0x3000.  It is not a VRAM transfer proof.
 *
 * The bootstrap then CD_EXECs record 0x0003e7 (17 sectors) into and enters
 * at $4000.  That
 * second-stage body has a literal one-sector CD_READ into local RAM $3800,
 * but its record registers are dynamic and intentionally remain unbound.
 * None of these fixed calls use the System Card VRAM destination modes.
 *
 * This receipt intentionally carries only media/bootstrap provenance.  It
 * does not classify dynamic CD records, derive graphics roles, or enable
 * palette/tile/VRAM rendering.  See
 * docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md.
 */
#define THERON_TRACK02_IPL_RECORD 0x0003a3u
#define THERON_TRACK02_IPL_LOAD_ADDRESS 0x4000u
#define THERON_TRACK02_IPL_CD_READ_CPU_ADDRESS 0x40cdu
#define THERON_TRACK02_IPL_CD_READ_SYSTEM_CARD_ADDRESS 0xe009u
#define THERON_TRACK02_IPL_CD_READ_LOCAL_DESTINATION 0x3000u
#define THERON_TRACK02_IPL_CD_EXEC_CPU_ADDRESS 0x40a4u
#define THERON_TRACK02_IPL_CD_EXEC_SYSTEM_CARD_ADDRESS 0xe00fu
#define THERON_TRACK02_IPL_STAGE2_RECORD 0x0003e7u
#define THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT 17u
#define THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS 0x4000u
#define THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR 224u
#define THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR 225u
#define THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS 0x4090u
#define THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION 0x3800u
/* Mednafen PCE debugger/CD traces against the authenticated original CUEs:
 * the live CL/DL/CH register triplet at $4090 resolves to these Track 02
 * records.  JP and US differ by the executable's one-sector size delta. */
#define THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP 0x0004dfu
#define THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US 0x0004e0u
#define THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES 2048u
#define THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES 0x520u
#define THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_BYTES 6u
#define THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT 218u
/* The verified $4090 setup writes AL, DH, and BX only.  CL/DL/CH remain
 * live across this boundary and jointly form the CD_READ record number. */
#define THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_CL 0x01u
#define THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_DL 0x02u
#define THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_CH 0x04u
#define THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_MASK \
    (THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_CL | \
     THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_DL | \
     THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_CH)

/* Static read-window completeness: the remaining bounded bytes of both
 * original loader read windows, verified against the hash-gated media.  The
 * stage-one CD_EXEC retry branch at user offset 0xa7 returns to the $4080
 * table-reader loop head.  The stage-one CD_READ table-load window at 0xa9
 * reads the preload table at $40dc into the same CL=$fc/DL=$fe/CH=$fd/AL=$f8
 * zero-page map the CD_EXEC setup uses for its own table, so the bound
 * preload table bytes and their reader code are joined.  In the stage-two
 * body, the JSR at 0x29 and the BSR at 0x7e are the two static invocations
 * of the $40ae register-seed subroutine; the BSR displacement 0x2e lands
 * exactly at the seed body, so the retry path's seed origin is byte-bound.
 * None of this assigns a System Card base arithmetic, record semantics, or
 * any graphics role; the live record values remain trace-bound only. */
#define THERON_TRACK02_IPL_CD_EXEC_RETRY_USER_OFFSET 0xa7u
#define THERON_TRACK02_IPL_CD_READ_TABLE_LOAD_USER_OFFSET 0xa9u
#define THERON_TRACK02_IPL_STAGE2_SEED_CALL_USER_OFFSET 0x29u
#define THERON_TRACK02_IPL_STAGE2_SEED_BSR_USER_OFFSET 0x7eu

/* Stage-two executed entry-path contiguity.  The stage-two body's executed
 * boot path enters at $4000 and runs linearly through the already-bound
 * seed call (0x29), the retry head seed BSR (0x7e), the CD_READ setup
 * (0x80), the post-read block (0x93), and the register seed (0xae..0xb4).
 * Two previously unbound gaps complete the linear stream: the entry
 * prologue [0x00..0x29) (SEI/stack/BIT-flag MPR paging through the L8000
 * and L40B7 calls up to the seed JSR) and the main path [0x2c..0x7e)
 * (post-seed init, interrupt-mask setup, and the bounded TII clears up to
 * the retry head).  Both windows match the source-locked US disassembly
 * (theron-us-stage2-huc6280.asm:107-181) byte for byte, so the executed
 * entry path [0x00..0xb5) is contiguous.  The source-locked documentation
 * attests JP/US byte identity only for the $4090 CD_READ window, so this
 * contiguity is proven for the authenticated US stage-two body only; the
 * JP body remains covered by the variant-neutral windows until staged JP
 * media can verify the same stream.  None of this assigns a System Card
 * base arithmetic, a record semantics, or any graphics role to the
 * stream. */
#define THERON_TRACK02_IPL_STAGE2_ENTRY_PROLOGUE_USER_OFFSET 0x00u
#define THERON_TRACK02_IPL_STAGE2_ENTRY_PROLOGUE_BYTES 0x29u
#define THERON_TRACK02_IPL_STAGE2_MAIN_PATH_USER_OFFSET 0x2cu
#define THERON_TRACK02_IPL_STAGE2_MAIN_PATH_BYTES 0x52u
#define THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES 0xb5u

/* Stage-two call-graph continuations of the executed entry path.  Four
 * callee bodies invoked from the contiguously bound entry stream
 * [0x00..0xb5) match the source-locked US disassembly
 * (theron-us-stage2-huc6280.asm:189-219, 1207-1232, 1622-1632,
 * 1661-1680) byte for byte: the L40B7 command-dispatch loop called at
 * user offset 0x1e (its own L4814 call at 0xb9 sits inside its body),
 * the L4B2D count-down delay called at 0x52, the L4B73 st0/st1/st2
 * port clear called at 0x55, and the L4814 zero-page pointer setup
 * called from the dispatcher.  The L4814 window's two da65
 * decode-artifact bytes at 0x81f-0x820 (the `.byte $A5` / `sxy` pair)
 * are bound to the authenticated media bytes.  The source-locked
 * documentation attests JP/US byte identity only for the $4090 CD_READ
 * window, so this is proven for the authenticated US stage-two body
 * only; the JP body remains covered by the variant-neutral windows
 * until staged JP media can verify the same streams.  None of this
 * assigns a System Card base arithmetic, a record semantics, a command
 * meaning to the L410D dispatch table, or any graphics role to the
 * st0/st1/st2 writes. */
#define THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET 0xb7u
#define THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES 0x3au
#define THERON_TRACK02_IPL_STAGE2_DELAY_USER_OFFSET 0xb2du
#define THERON_TRACK02_IPL_STAGE2_DELAY_BYTES 0x0fu
#define THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_USER_OFFSET 0xb73u
#define THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_BYTES 0x23u
#define THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_USER_OFFSET 0x814u
#define THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_BYTES 0x2eu
#define THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES 0x9au

typedef enum {
    THERON_TRACK02_IPL_DESTINATION_UNKNOWN = 0,
    THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM = 1
} Theron_Track02IplDestination;

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    size_t data_track_index01_raw_sector;
    size_t information_raw_sector;
    size_t executable_raw_sector;
    size_t executable_sector_count;
    size_t executable_user_data_bytes;
    uint32_t executable_user_data_hash;
    uint32_t record;
    uint16_t load_address;
    uint16_t entry_address;
    size_t cd_read_user_data_offset;
    uint16_t cd_read_cpu_address;
    uint16_t cd_read_system_card_address;
    Theron_Track02IplDestination cd_read_destination;
    uint16_t cd_read_local_destination;
    uint16_t cd_exec_cpu_address;
    uint16_t cd_exec_system_card_address;
    uint32_t stage2_record;
    uint8_t stage2_sector_count;
    Theron_Track02IplDestination stage2_destination;
    uint16_t stage2_load_address;
    /* CD_EXEC loads and transfers control to this same local-RAM address.
     * It is a bootstrap handoff fact only, not a claim about stage-two
     * program semantics after entry. */
    uint16_t stage2_entry_address;
    size_t stage2_raw_sector;
    size_t stage2_user_data_bytes;
    uint32_t stage2_user_data_hash;
    uint16_t stage2_cd_read_cpu_address;
    uint8_t stage2_cd_read_sector_count;
    Theron_Track02IplDestination stage2_cd_read_destination;
    uint16_t stage2_cd_read_local_destination;
    uint32_t stage2_cd_read_record;
    size_t stage2_cd_read_raw_sector;
    int stage2_cd_read_record_proven;
    /* The static code leaves CL/DL/CH live; their values are bound by the
     * original CUE runtime trace rather than inferred from the instruction
     * bytes alone. */
    int stage2_cd_read_dynamic_boundary_valid;
    uint8_t stage2_cd_read_live_record_register_mask;
    /* Static read-window completeness proofs.  Each binds only instruction
     * bytes at the stated original-media offsets: the CD_EXEC retry branch
     * back to its table-reader loop head, the CD_READ preload-table load
     * into the shared zero-page argument map, and the two static call sites
     * of the stage-two register-seed subroutine (the BSR displacement lands
     * exactly on the seed body).  No System Card base arithmetic, record
     * semantics, or graphics role is claimed. */
    int cd_exec_retry_branch_proven;
    int cd_read_table_load_proven;
    int stage2_seed_call_sites_proven;
    int vram_transfer_proven;
} Theron_Track02IplLoaderReceipt;

/* Byte-level receipt for the first dynamically addressed stage-two payload.
 * It proves the original one-sector payload's shared manifest shape only;
 * entry meanings remain unclassified until loader control flow establishes
 * them. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t raw_sector;
    size_t raw_offset;
    size_t user_data_offset;
    size_t user_data_bytes;
    uint16_t header_word0;
    uint16_t header_word1;
    size_t manifest_bytes;
    size_t manifest_entry_count;
    size_t nonzero_byte_count;
    uint32_t user_data_hash;
} Theron_Track02Stage2DynamicPayloadReceipt;

/* Receipt for the stage-two executed entry-path contiguity proof.  It binds
 * only instruction bytes of the authenticated US stage-two body: the entry
 * prologue [0x00..0x29) and the main path [0x2c..0x7e), which complete the
 * linear executed stream [0x00..0xb5) together with the already-bound seed
 * call (0x29), seed BSR (0x7e), CD_READ setup (0x80), post-read block
 * (0x93), and register seed (0xae..0xb4).  Proven for the US body only
 * (the source-locked JP/US identity attestation covers the $4090 window,
 * not this stream); no System Card base arithmetic, record semantics, or
 * graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t entry_path_prologue_bytes;
    size_t entry_path_main_path_bytes;
    size_t entry_path_bound_bytes;
    int entry_prologue_proven;
    int main_path_proven;
    int entry_path_contiguous_proven;
} Theron_Track02Stage2EntryPathReceipt;

/* Receipt for the stage-two call-graph continuation proof.  It binds
 * only instruction bytes of the authenticated US stage-two body: the
 * L40B7 dispatcher [0xb7..0xf1), the L4B2D delay [0xb2d..0xb3c), the
 * L4B73 port clear [0xb73..0xb96), and the L4814 pointer setup
 * [0x814..0x842), each invoked from the contiguously bound executed
 * entry path (call sites 0x1e, 0x52, and 0x55 inside [0x00..0xb5); the
 * L4814 call site 0xb9 inside the dispatcher body).  Proven for the US
 * body only (the source-locked JP/US identity attestation covers the
 * $4090 window, not these streams); no System Card base arithmetic,
 * record semantics, dispatch-table command meanings, or graphics role
 * follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t dispatcher_bytes;
    size_t delay_bytes;
    size_t port_clear_bytes;
    size_t pointer_setup_bytes;
    size_t call_graph_bound_bytes;
    int dispatcher_proven;
    int delay_proven;
    int port_clear_proven;
    int pointer_setup_proven;
} Theron_Track02Stage2CallGraphReceipt;

/* Scanner-to-M11 launch contract for an original CUE-mounted Track 02.
 * It binds the hash-verified MODE1/2352 payload to the IPL bootstrap and
 * stage-two receipt; it never materializes a replacement/cache payload. */
typedef struct {
    int valid;
    int cue_backed;
    int track02_md5_verified;
    int mode1_2352;
    int no_synthetic_cache;
    char cue_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char track02_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char track02_md5[33];
    Theron_Track02IplLoaderReceipt ipl_loader;
} Theron_Track02StartupLoaderReceipt;

/* Validates the actual IPL information block and its initial executable's
 * local-RAM CD_READ setup for one known raw JP/US Track 02 image.  Callers
 * must still authenticate the supplied file MD5 before this byte-level API is
 * used.  Non-raw and unknown variants reject; no fallback scan is attempted.
 */
Theron_Track02SignalStatus theron_v1_track02_find_ipl_loader(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02IplLoaderReceipt *out_receipt);

Theron_Track02SignalStatus theron_v1_track02_inspect_stage2_dynamic_payload(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2DynamicPayloadReceipt *out_receipt);

/* Verifies the stage-two executed entry-path contiguity against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader proof,
 * then requires the exact entry prologue and main path bytes at their
 * original user offsets inside the proven stage-two sector.  The JP variant
 * rejects (its entry stream is not attested byte-identical); any changed
 * byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_entry_path(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2EntryPathReceipt *out_receipt);

/* Verifies the stage-two call-graph continuation bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L40B7 dispatcher, L4B2D delay, L4B73
 * port clear, and L4814 pointer setup bytes at their original user
 * offsets inside the proven stage-two image.  The JP variant rejects
 * (these streams are not attested byte-identical); any changed byte
 * fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_call_graph(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2CallGraphReceipt *out_receipt);

#endif /* THERON_V1_TRACK02_H */
