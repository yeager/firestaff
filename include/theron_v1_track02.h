#ifndef THERON_V1_TRACK02_H
#define THERON_V1_TRACK02_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_world.h"

#define THERON_TRACK02_MAX_BANK_ANCHORS 3u
#define THERON_TRACK02_DUNGEON_COUNT 7u

/* Maximum number of entries the documented 9-word stride table can hold.
 * The 0x1584 descriptor observed in the hash-verified US Track 02 ISO and
 * the three replicated anchors in the JP/US raw Track 02 BINs all carry
 * exactly 9 little-endian uint16 words; this constant bounds the decoder
 * table size and the synthetic-fixture/negative-fixture tests. */
#define THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES 9u

#define THERON_TRACK02_MD5_JP_BIN      "b7afb338ad31be1025b53f9aff12d73a"
#define THERON_TRACK02_MD5_US_BIN      "f23601102138f87c33025877767ebf76"
#define THERON_TRACK02_MD5_JP_REV1_ISO "397039af02d50d15c70b74088eb8a1cb"
/* MyAbandonware's US dump splits this exact ISO into TQUS19.iso followed by
 * TQUS02End.iso.  The latter is only the tail, never a standalone Track 02.
 * TQUS02End.iso alone has MD5 THERON_TRACK02_MD5_US_ISO_TAIL; the full
 * concatenation (TQUS19 + TQUS02End) has THERON_TRACK02_MD5_US_ISO. */
#define THERON_TRACK02_MD5_US_ISO      "ceb02343868f80cec899e9b239aff2da"
#define THERON_TRACK02_MD5_US_ISO_TAIL "3d8b78571dcd0e6eb8eb4b01eeb7fbba"

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

typedef struct Theron_Track02ObjectTable {
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
     * which a level's rows begin/end, plus a hash that includes each ordinal
     * before its eight raw row bytes.  They do not assign semantics to the
     * row fields. */
    size_t level_first_record_indexes[THERON_TRACK02_DUNGEON_COUNT];
    size_t level_last_record_indexes[THERON_TRACK02_DUNGEON_COUNT];
    uint32_t level_position_hashes[THERON_TRACK02_DUNGEON_COUNT];
    Theron_Track02ObjectTableRecord
        records[THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS];
} Theron_Track02ObjectTable;

/* Object-table decoder receipt for the bytes following the initial level grid.
 *
 * The authentic JP/US Track 02 corpora expose a bounded 0x36c-byte level
 * envelope.  The 0x380-byte record tail is now parsed as a count-prefixed
 * compact object table; JP/US raw BINs both decode to an empty table (count 0,
 * all-zero tail), which is source-proven.  Non-empty tables are accepted when
 * every record maps to a level of the initial dungeon (level 0..2) and passes
 * the compact-row shape gate.  Callers must still gate runtime promotion on a
 * verified Track 02 MD5 and on the object's kind being supported by the live
 * world. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    size_t object_boundary_raw_offset;
    size_t object_boundary_user_data_offset;
    size_t following_user_data_bytes_in_record;
    uint32_t following_user_data_hash;
    int object_table_semantics_proven;
    int promotion_blocked;
    /* Decoded compact object table.  Populated only when
     * object_table_semantics_proven is true. */
    Theron_Track02ObjectTable object_table;
} Theron_Track02InitialLevelObjectTableReceipt;

/* Attempt to decode the object table that follows the initial level grid.
 *
 * The 0x380-byte record tail is parsed as a little-endian count-prefixed
 * compact object table (THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES bytes per
 * row).  JP/US raw Track 02 BINs both decode to an empty table (count 0,
 * all-zero tail), which is source-proven.  Non-empty tables are accepted when
 * every record maps to a level of the initial dungeon (level 0..2) and passes
 * the compact-row shape gate.  On success the parsed table is written to
 * out_receipt->object_table and promotion_blocked is cleared.
 *
 * Source-lock: THQUEST.ASM T900 (object database) for the API shape;
 * JP/US Track 02 raw BIN byte boundary (record 0x0b52, 0x36c-byte level
 * envelope, 0x380-byte tail) proven by
 * theron_v1_track02_capture_initial_level_object_boundary(). */
Theron_Track02SignalStatus theron_v1_track02_decode_initial_level_object_table(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelObjectTableReceipt *out_receipt);

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

/* Decode the object-table records that belong to one level of a validated
 * non-startup dungeon route.
 *
 * The route must already carry a complete level/object transaction from
 * authenticated Track 02 media (theron_v1_track02_build_dungeon_route() or
 * theron_v1_track02_load_verified_dungeon_route()).  The output table
 * contains only the records whose level_index matches the requested level;
 * the route's object transaction is the multi-level object tail for its
 * dungeon, so any level in range may be requested.  If the route is not
 * valid or the media evidence is missing, this returns NOT_FOUND and leaves
 * out_table zeroed.
 *
 * Source-lock: THQUEST.ASM T600/T900; authenticated non-startup dungeon route
 * with object-table transaction. */
Theron_Track02SignalStatus theron_v1_track02_decode_dungeon_level_object_table(
    const Theron_Track02DungeonRoute *route,
    int level_index,
    Theron_Track02ObjectTable *out_table);

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

/* Stage-two dispatch-machine closure: the remaining bounded bytes of
 * the L40B7 dispatch machine, verified against the hash-gated US media
 * and matched against the source-locked disassembly
 * (theron-us-stage2-huc6280.asm:183-235, 1590-1594, 2334-2337).  The
 * register-seed tail [0xb5..0xb7) closes the gap between the executed
 * entry path [0x00..0xb5) and the dispatcher body.  The seven dispatch
 * stubs [0xf1..0x10d) are the shared return tails that command
 * handlers jump to for selecting the stream-advance count (1..5, 7,
 * 9); each is an LDA #imm / BRA L40E4 pair.  The jump table
 * [0x10d..0x121) holds ten little-endian handler addresses, all inside
 * the loaded image ($41C5..$4253, strictly increasing).  The L4AF7
 * MPR-page body [0xaf7..0xb00) and the L4F5E selector body
 * [0xf5e..0xf66) are the dispatcher loop head's two direct callees
 * (call sites 0xcc and 0xc1 inside the bound dispatcher window).  With
 * the entry path and the round-12 callee bodies, the executed dispatch
 * machine [0x00..0x121) is contiguously bound.  The source-locked
 * documentation attests JP/US byte identity only for the $4090 CD_READ
 * window, so this is proven for the authenticated US stage-two body
 * only; the JP body remains covered by the variant-neutral windows
 * until staged JP media can verify the same streams.  None of this
 * assigns handler semantics to the ten jump-table targets, a System
 * Card base arithmetic, a record semantics, or any graphics role. */
#define THERON_TRACK02_IPL_STAGE2_SEED_TAIL_USER_OFFSET 0xb5u
#define THERON_TRACK02_IPL_STAGE2_SEED_TAIL_BYTES 0x02u
#define THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_USER_OFFSET 0xf1u
#define THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_BYTES 0x1cu
#define THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_USER_OFFSET 0x10du
#define THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_BYTES 0x14u
#define THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES 10u
#define THERON_TRACK02_IPL_STAGE2_MPR_PAGE_USER_OFFSET 0xaf7u
#define THERON_TRACK02_IPL_STAGE2_MPR_PAGE_BYTES 0x09u
#define THERON_TRACK02_IPL_STAGE2_SELECTOR_USER_OFFSET 0xf5eu
#define THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES 0x08u
#define THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES 0x43u
#define THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES 0x121u

/* L8000/L45A6 callee pair: L8000 is the entry path's first call (the
 * JSR $8000 at user offset 0x11 inside the bound prologue) and the
 * last unbound first-level callee.  Its body [0x4000..0x40bc) sits at
 * the head of image sector 8 (188 bytes: VDC register clears through
 * the $220C/$220D/$2210/$2211 STZ sequence and st0/st1/st2 pairs, the
 * L45A6 call, the zero-page result handoff through $4C/$4D, the
 * $47BF-$47D2/$3B6A-$3B6F stores, and the L4696/L48FC tail calls).
 * Three da65 decode-artifact spans in the source-locked disassembly
 * (the $2211 STZ at 0x400b split into .byte/ora, the ADC $00 at
 * 0x402a split into .byte/brk, and the STA $47CE/LDA #$00 at
 * 0x404a-0x404e split into .byte/dec/brk) are bound to the
 * authenticated media bytes; the disassembly also renders several
 * zero-page accesses as absolute labels, so the media bytes are
 * authoritative.  L45A6 [0x5a6..0x5ca) (36 bytes: the ($1C),y table
 * read, the $44E7-$44EA seed copy into $01-$05, and the PLA/LSR
 * branch into the dynamic-lane JSR $3AB7 or the JMP $4105 return) is
 * cleanly decodable, but its only call site is the JSR at L8000+0x1c,
 * so the two bind together to keep every call site of a bound window
 * inside a bound window.  The source-locked documentation attests
 * JP/US byte identity only for the $4090 CD_READ window, so this is
 * proven for the authenticated US stage-two body only.  None of this
 * assigns semantics to the L4696/L48FC callees, the dynamic-lane
 * $3AB7 target, a System Card base arithmetic, a record semantics, or
 * any graphics role. */
#define THERON_TRACK02_IPL_STAGE2_L8000_USER_OFFSET 0x4000u
#define THERON_TRACK02_IPL_STAGE2_L8000_BYTES 0xbcu
#define THERON_TRACK02_IPL_STAGE2_L8000_CALL_SITE_USER_OFFSET 0x11u
#define THERON_TRACK02_IPL_STAGE2_L45A6_CALL_SITE_L8000_OFFSET 0x1cu
#define THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_L8000_OFFSET 0x6au
#define THERON_TRACK02_IPL_STAGE2_L48FC_CALL_SITE_L8000_OFFSET 0xb8u
#define THERON_TRACK02_IPL_STAGE2_L45A6_USER_OFFSET 0x5a6u
#define THERON_TRACK02_IPL_STAGE2_L45A6_BYTES 0x24u
#define THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES 0xe0u

/* Jump-table handler bodies: the ten L410D table targets $41C5..$4253,
 * verified against the hash-gated US media and matched instruction by
 * instruction against the source-locked disassembly
 * (theron-us-stage2-huc6280.asm:344-430).  The bodies form one
 * contiguous span [0x1c5..0x254) (143 bytes): handler 1 [0x1c5..0x1cb)
 * (BSR L41B9 / CLA / JMP L40E4), handler 2 [0x1cb..0x1d8) (BSR L41F8 /
 * BNE L41D5 / BSR L41B9 / CLA / JMP L40E4 with the L41D5 JMP L4101
 * tail), handler 3 [0x1d8..0x1de) (BSR L41F8 / BNE L41CF / BRA L41D5),
 * handler 4 [0x1de..0x1e6) (BSR L41F8 / BCC L41D5 / BEQ L41D5 /
 * BRA L41CF), handler 5 [0x1e6..0x1ec) (BSR L41F8 / BCS L41D5 /
 * BRA L41CF), handler 6 [0x1ec..0x1f0) (BSR L4203 / BRA L41CD),
 * handler 7 [0x1f0..0x1f4) (BSR L4203 / BRA L41DA), handler 8
 * [0x1f4..0x214) (BSR L4203 / BRA L41E8 plus the shared L41F8 and L4203
 * operand-read sub bodies), handler 9 [0x214..0x253) (the L4215 operand
 * read, the L421C $4EC1/$4D7B store sub with the ADC $3008/STA $3009
 * pair, the L4F5E selector call, the L4233 carry path, and the
 * L4240/L424B sub ending in the dynamic-lane JSR $383E), and handler 10
 * [0x253..0x254) (a single RTS — the table's no-op/terminator entry,
 * reached by the dispatcher's indirect JMP).  Three da65
 * decode-artifact spans of the same class as the L8000 body (the BSR
 * L41F8 at 0x1d8 split into .byte/.byte, the LDA $2780,x at 0x1fc
 * split into .byte/bra, and the ADC $3008/STA $3009 at 0x225-0x22a
 * split into .byte/php/bmi/ora) are bound to the authenticated media
 * bytes; the disassembly also renders the 0x245 zero-page STA $20 as
 * the absolute label L0020, so the media bytes are authoritative.  The
 * table-read site (the JMP (L410D,x) at 0xe1) sits inside the bound
 * dispatcher window, so every reference to the table and its targets
 * stays inside bound windows.  The source-locked documentation attests
 * JP/US byte identity only for the $4090 CD_READ window, so this is
 * proven for the authenticated US stage-two body only.  None of this
 * assigns command or stream semantics to the ten handlers, semantics
 * to the L41B9/L43D6/L37D8 callees or the dynamic-lane $383E target, a
 * System Card base arithmetic, a record semantics, or any graphics
 * role. */
#define THERON_TRACK02_IPL_STAGE2_HANDLERS_USER_OFFSET 0x1c5u
#define THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES 0x8fu
#define THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT 10u
#define THERON_TRACK02_IPL_STAGE2_HANDLERS_FIRST_CPU_ADDRESS 0x41c5u
#define THERON_TRACK02_IPL_STAGE2_HANDLERS_LAST_CPU_ADDRESS 0x4253u
#define THERON_TRACK02_IPL_STAGE2_HANDLER_TABLE_READ_USER_OFFSET 0xe1u

/* L4696/L3114 far-callee pair.  L4696 is called from the bound L8000
 * window (the JSR $4696 at L8000+0x6a) and twice more from the unbound
 * $45xx tier.  The source-locked da65 dump labels image offset 0x696
 * as L4696, but its head byte there ($33) is not a HuC6280 opcode —
 * da65 emitted `.byte $33` (the flagged head-byte decode artifact of
 * its linear $4000-based map).  The authenticated body lives at image
 * offset 0x4696 (image bank 2, alongside its callers), where da65's
 * own decode at L8696 (theron-us-stage2-huc6280.asm:10212-10248)
 * matches the media bytes instruction by instruction: the 16-bit
 * shift-add multiply ($0E multiplier through $12, $10:$11
 * multiplicand, $0E:$0F product) — STZ $0F / STZ $11 / LDA $0E /
 * STA $12 / STZ $0E / LDX #$01, the BBS7..BBS0 bit-priority chain,
 * the INX staircase, and the LSR $12 / BCC / CLC / LDA $10 / ADC $0E /
 * STA $0E / LDA $11 / ADC $0F / STA $0F / ASL $10 / ROL $11 / DEX /
 * BNE / RTS loop.  The disassembly renders the $11 zero-page accesses
 * as the absolute label L0011, so the media bytes are authoritative.
 * L3114 is called from the bound L4F5E selector window (the JSR $3114
 * at L4F5E+4, selector bytes LDX #$C1 / LDY #$4E / JSR / RTS).  da65
 * declared L3114 := $3114 absolute without a body decode because CPU
 * $3114 lies below its $4000-based linear map; the CPU $3xxx window
 * shows image bank 0 at offset CPU-$2000 (L383E, by contrast, sits at
 * CPU $383E whose bank-0 offset 0x183e is clobbered at runtime by the
 * $3800 dynamic-payload CD_READ, which is why L383E belongs to the
 * dynamic-payload lane while L3114 stays in the stage-two image lane).
 * The authenticated body [0x1114..0x1172) decodes cleanly from the
 * media: PHA / BSR L3172 / LDA $4FDE / BNE / BSR $117D / BRA / LDA
 * #$1A / JSR $4F66 / DEC A / BNE / PLA / STA $4F8E / PLA / STA $4F8D /
 * the $4F9D-$4F9E -> $06/$07 and $4F93-$4F94 -> $4F8B-$4F8C copies /
 * JSR $526D / the $06:$07 +4 fix-up / LDX $4F8D / LDY $4F8E / the
 * PHX / $0E:$0F save / JSR $55E0 / restore / JSR $5213 / PLX / DEY /
 * BNE loop / RTS — the trailing RTS at 0x1171 sits immediately before
 * the da65-declared L3172 entry, confirming the span.  Its BSR/JSR
 * callees (L3172, $117D, $4F66, $526D, $55E0, $5213) remain unbound
 * future windows.  The source-locked documentation attests JP/US byte
 * identity only for the $4090 CD_READ window, so this is proven for
 * the authenticated US stage-two body only.  None of this assigns
 * multiplier or queue semantics to the bodies, semantics to their
 * callees, a System Card base arithmetic, a bank-mapping arithmetic, a
 * record semantics, or any graphics role. */
#define THERON_TRACK02_IPL_STAGE2_L4696_USER_OFFSET 0x4696u
#define THERON_TRACK02_IPL_STAGE2_L4696_BYTES 0x45u
#define THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS 0x4696u
#define THERON_TRACK02_IPL_STAGE2_L4696_LINEAR_ARTIFACT_USER_OFFSET 0x696u
#define THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET 0x1114u
#define THERON_TRACK02_IPL_STAGE2_L3114_BYTES 0x5eu
#define THERON_TRACK02_IPL_STAGE2_L3114_CPU_ADDRESS 0x3114u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_SELECTOR_OFFSET 0x04u
#define THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES 0xa3u

/* L3114 near callees.  L3172 and the $117D far-helper trampoline sit
 * directly after the bound L3114 body in the low-image region (below
 * $3800, never clobbered by the stage-two $3800 dynamic-payload
 * CD_READ), so they stay in the stage-two image lane.  da65 declared
 * L3172 := $3172 absolute without a body decode (CPU $3172 lies below
 * its $4000-based linear map); the authenticated bodies decode cleanly
 * from the media.  L3172: LDA #$01 / STA $5C22 / LDA #$01 / STA $5C23 /
 * RTS.  $117D: JSR $5BF5 / JSR $5C8C / JSR $5CB0 / JSR $5C25 / RTS (its
 * own callees remain unbound future windows).  Their CPU entry
 * addresses are not pinned, so the receipt carries 0 for them. */
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_USER_OFFSET 0x1172u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_BYTES 0x0bu
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET 0x117du
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES 0x0du

/* L3114 $4xxx/$5xxx callees.  da65 lists each body inline under its
 * linear map (CPU = image + $4000), so they live on image banks 0/1
 * like L4696/L3114 and stay in the stage-two image lane.  L4F66 is the
 * shared delay loop (PHA/PHX/PHY, the LDA #$03 CLX CLY DEY/DEX/DEC A
 * BNE nest, PLY/PLX/PLA/RTS — matching the L3114 LDA #$1A loop); it
 * sits at bank-0 offset 0x0f66, directly after the bound L4F5E
 * selector window (CPU $4F66 shows bank 0 at CPU-$4000 here, as da65's
 * linear decode attests).  L5213: CLC / LDA $0E / ADC #$40 / STA $0E /
 * BCC / INC $0F / RTS.  L526D: JSR L51F9 / LDX #$04 / the LSR $07 /
 * ROR $06 / DEX / BNE shift loop / LDA $07 / ORA #$F0 / STA $07 / RTS
 * (its L51F9 callee remains an unbound future window).  L55E0: BSR
 * L55F6 / BSR L55E8 / DEX / BNE L55E0 / RTS (its L55F6/L55E8 callees
 * remain unbound future windows). */
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_USER_OFFSET 0x0f66u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_CPU_ADDRESS 0x4f66u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES 0x14u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_USER_OFFSET 0x1213u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_CPU_ADDRESS 0x5213u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_BYTES 0x0cu
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_USER_OFFSET 0x126du
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_CPU_ADDRESS 0x526du
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES 0x13u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET 0x15e0u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_CPU_ADDRESS 0x55e0u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES 0x08u

/* L3114 call-site invariants: byte offset of each JSR/BSR opcode within
 * the bound L3114 body (call_site offset + 3 <= L3114_BYTES is
 * compile-time-asserted).  Binding the six callee bodies above pins
 * these call targets. */
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L3172_OFF 0x01u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_FAR117D_OFF 0x08u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L4F66_OFF 0x0eu
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L526D_OFF 0x32u
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L55E0_OFF 0x4du
#define THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L5213_OFF 0x56u

/* $45xx-tier L4696 call sites: two JSR $4696 opcodes at image offsets
 * 0x45ba/0x45cb inside the otherwise unbound $45xx routine (da65
 * asm:10107/10115, both preceded by LDA #$08 / STA $0E).  Only the
 * 3-byte JSR windows are bound; the target is compile-time asserted to
 * be the bound L4696 body. */
#define THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_A_USER_OFFSET 0x45bau
#define THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_B_USER_OFFSET 0x45cbu
#define THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES 0x03u

/* Same-image bytes bound by the stage-two L3114-callees verifier. */
#define THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES 0x59u

/* L3114 tier-2 callees: the callees of the bound $117D trampoline,
 * L526D, and L55E0.  da65 lists every body inline under its linear map
 * (CPU = image + $4000, image banks 0/1), so all seven stay in the
 * stage-two image lane (all image offsets lie below $3800, never
 * clobbered by the dynamic-payload CD_READ).  L51F9 carries a da65
 * mid-instruction label artifact: the declared L5200 label splits the
 * `ror $0E` at 0x11ff-0x1200 (da65 emitted `.byte $66` plus the
 * garbage `asl $664A` / `asl $0F85` renderings), so the media bytes
 * are authoritative; the body [0x11f9..0x1213) ends exactly at the
 * bound L5213 entry.  L55E8 [0x15e8..0x15ef) sits directly after the
 * bound L55E0 body.  L5C25's L5C2C alternate entry, the LE063 far
 * calls inside L5C8C/L5CB0, and every callee-of-callee (L5C06, L5C9F,
 * L536E, L5439, L5C69, L54A0, L5600) remain unbound future windows. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_USER_OFFSET 0x11f9u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_CPU_ADDRESS 0x51f9u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES 0x1au
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_USER_OFFSET 0x15e8u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_CPU_ADDRESS 0x55e8u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_BYTES 0x07u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_USER_OFFSET 0x15f6u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_CPU_ADDRESS 0x55f6u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES 0x0au
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_USER_OFFSET 0x1bf5u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_CPU_ADDRESS 0x5bf5u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_BYTES 0x11u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET 0x1c25u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_CPU_ADDRESS 0x5c25u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES 0x44u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_USER_OFFSET 0x1c8cu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_CPU_ADDRESS 0x5c8cu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES 0x13u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_USER_OFFSET 0x1cb0u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_CPU_ADDRESS 0x5cb0u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_BYTES 0x0fu

/* Tier-2 call-site invariants: byte offset of each JSR/BSR opcode
 * within its bound caller body (call_site offset + 3 <= caller BYTES
 * is compile-time-asserted): four JSR opcodes inside the bound $117D
 * trampoline, the JSR L51F9 head of the bound L526D body, and the two
 * BSR opcodes at the head of the bound L55E0 body.  Binding the seven
 * callee bodies above pins these call targets. */
#define THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5BF5_OFF 0x00u
#define THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5C8C_OFF 0x03u
#define THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5CB0_OFF 0x06u
#define THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5C25_OFF 0x09u
#define THERON_TRACK02_IPL_STAGE2_L526D_CALL_SITE_L51F9_OFF 0x00u
#define THERON_TRACK02_IPL_STAGE2_L55E0_CALL_SITE_L55F6_OFF 0x00u
#define THERON_TRACK02_IPL_STAGE2_L55E0_CALL_SITE_L55E8_OFF 0x02u

/* Same-image bytes bound by the stage-two L3114 tier-2-callees
 * verifier. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES 0xa2u

/* L3114 tier-3 callees: the callees of the bound L5C8C, L5C25, and
 * L55F6 bodies, plus the L5C20 table and L5C25's L5C2C alternate
 * entry.  da65 lists every body inline under its linear map (CPU =
 * image + $4000, image banks 0/1), so all stay in the stage-two image
 * lane (all image offsets lie below $3800).  L54A0 and L5600 store
 * through da65's `a:$02`/`a:$03` absolute renderings — the media
 * confirms the 3-byte absolute form ($8D $02 $00), ending the bodies
 * exactly at the next da65 labels L54AF/L560B.  L5C69 is
 * self-modifying (STA L5C7E rewrites the ORA/AND opcode at 0x1c7e);
 * the media bytes are the as-loaded image.  The L5C20 table is the
 * 5-byte data window read by the bound L5BF5 copy loop (LDA $5C20,x)
 * and the L5C24 mask byte written by L5C25/L5C2C.  L5C25's L5C2C
 * alternate entry sits at +0x07 inside the already bound L5C25
 * window; only its offset is asserted, no new bytes.  Tier-4 windows
 * (L4F7A, L542D, L5482, L5492, L535E, L5455, L53C4, L54AF, L560B, the
 * LE063 targets) remain unbound future windows. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_USER_OFFSET 0x1c06u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_CPU_ADDRESS 0x5c06u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_BYTES 0x1au
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_USER_OFFSET 0x1c20u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_CPU_ADDRESS 0x5c20u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_BYTES 0x05u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_USER_OFFSET 0x1c69u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_CPU_ADDRESS 0x5c69u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES 0x23u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_USER_OFFSET 0x1c9fu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_CPU_ADDRESS 0x5c9fu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES 0x11u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_USER_OFFSET 0x136eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_CPU_ADDRESS 0x536eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_BYTES 0x56u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_USER_OFFSET 0x1439u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_CPU_ADDRESS 0x5439u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_BYTES 0x1cu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_USER_OFFSET 0x14a0u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_CPU_ADDRESS 0x54a0u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_BYTES 0x0fu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_USER_OFFSET 0x1600u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_CPU_ADDRESS 0x5600u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_BYTES 0x0bu

/* L5C25's L5C2C alternate entry: byte offset inside the bound L5C25
 * window (the LDA #$EF / STA $5C24 head, no new bytes bound). */
#define THERON_TRACK02_IPL_STAGE2_L5C25_L5C2C_ENTRY_OFF 0x07u

/* Tier-3 call-site invariants: byte offset of each JSR/BSR opcode
 * within its bound caller body (call_site offset + 3 <= caller BYTES
 * is compile-time-asserted), plus the L5BF5 data-site invariant (the
 * LDA $5C20,x at +0x03 of the bound L5BF5 body, whose target is the
 * bound L5C20 table). */
#define THERON_TRACK02_IPL_STAGE2_L5C8C_CALL_SITE_L5C06_OFF 0x00u
#define THERON_TRACK02_IPL_STAGE2_L5C8C_CALL_SITE_L5C9F_OFF 0x05u
#define THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L536E_OFF 0x0cu
#define THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L5C69_OFF 0x2cu
#define THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L5439_OFF 0x3cu
#define THERON_TRACK02_IPL_STAGE2_L55F6_CALL_SITE_L54A0_OFF 0x02u
#define THERON_TRACK02_IPL_STAGE2_L55F6_CALL_SITE_L5600_OFF 0x05u
#define THERON_TRACK02_IPL_STAGE2_L5BF5_DATA_SITE_L5C20_OFF 0x03u

/* Same-image bytes bound by the stage-two L3114 tier-3-callees
 * verifier. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES 0xdfu

/* L3114 tier-4 callees: the remaining callees of the bound tier-2/3
 * bodies.  da65 lists every body inline under its linear map (CPU =
 * image + $4000, image banks 0/1), so all nine stay in the stage-two
 * image lane (all image offsets lie below $3800).  The bound windows
 * chain contiguously: L542D [0x142d..0x1439) ends at the bound L5439,
 * L5439 ends at L5455 [0x1455..0x1482), which ends at L5482
 * [0x1482..0x1492), which ends at L5492 [0x1492..0x14a0), which ends
 * at the bound L54A0, which ends at L54AF [0x14af..0x14c5); L535E
 * [0x135e..0x136e) ends at the bound L536E; L4F7A [0x0f7a..0x0f89)
 * starts where the bound L4F66 ends; L560B [0x160b..0x1657) starts
 * where the bound L5600 ends.  L560B carries a da65 mid-instruction
 * label artifact (the round-18 class): the declared L563D label sits
 * on the BCC operand byte at 0x163d (da65 emitted `.byte $90` plus
 * the garbage `st0 #$EE` / `cld` / `.byte $4F` renderings), so the
 * media bytes are authoritative — the real flow is BCC L5641 /
 * INC $4FD8.  L560B's L0000 renderings are da65's zero-page-as-
 * absolute form, superseded by the media bytes.  L535E's stores
 * confirm da65's `a:$02`/`a:$03` absolute form.  Tier-5 windows
 * (L5403, L541E, L52A2, L52C8, L5657, L54C5, the LE063 targets, the
 * L5657-tail data) remain unbound future windows. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_USER_OFFSET 0x0f7au
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_CPU_ADDRESS 0x4f7au
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_BYTES 0x0fu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_USER_OFFSET 0x135eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_CPU_ADDRESS 0x535eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_BYTES 0x10u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_USER_OFFSET 0x13c4u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_CPU_ADDRESS 0x53c4u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_BYTES 0x3fu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_USER_OFFSET 0x142du
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_CPU_ADDRESS 0x542du
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_BYTES 0x0cu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_USER_OFFSET 0x1455u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_CPU_ADDRESS 0x5455u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_BYTES 0x2du
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_USER_OFFSET 0x1482u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_CPU_ADDRESS 0x5482u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_BYTES 0x10u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_USER_OFFSET 0x1492u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_CPU_ADDRESS 0x5492u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_BYTES 0x0eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_USER_OFFSET 0x14afu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_CPU_ADDRESS 0x54afu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_BYTES 0x16u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_USER_OFFSET 0x160bu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_CPU_ADDRESS 0x560bu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES 0x4cu

/* Tier-4 call-site invariants in previously bound caller bodies (the
 * whole-body exact matches of the newly bound bodies cover their own
 * internal call sites): the JSR L4F7A at +0x09 inside the bound L5C9F
 * body and the JSR L5492 at +0x1f inside the bound L5C69 body. */
#define THERON_TRACK02_IPL_STAGE2_L5C9F_CALL_SITE_L4F7A_OFF 0x09u
#define THERON_TRACK02_IPL_STAGE2_L5C69_CALL_SITE_L5492_OFF 0x1fu

/* Same-image bytes bound by the stage-two L3114 tier-4-callees
 * verifier. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES 0x117u

/* Enclosing $45xx routine: the body that holds the two bound $45xx-tier
 * JSR $4696 windows (bound in round 17) at +0x09/+0x1a.  da65 gives the
 * routine no entry label (its decode starts mid-stream after the
 * preceding RTS at asm:10100); the authenticated body [0x45b1..0x466a)
 * decodes cleanly from the media and matches da65's inline listing
 * (theron-us-stage2-huc6280.asm:10102-10191) instruction by
 * instruction: the LDA $12 / PHA / LDY $11 prologue, the two LDA #$08 /
 * STA $0E / JSR L4696 multiply calls with their $54:$55 and $52:$53
 * result saves, JSR L4552, the $13/accumulator ASL pair into
 * L47B8/L47B9, the DEC $5A / JSR L4932 / STZ $5A window, the $14:$15 ->
 * $0E:$0F / JSR L458E call, the $02:$03 -> $06:$07 and $56-$58 ->
 * L466A/$16/$17 saves, and the L47B9/2 X-trip-counted main loop (da65's
 * L8610 head at +0x5f): the L466A/$16/$17 -> $56-$58 restore, JSR
 * L424B, the $47E0 -> $00:$01 / $06:$07 -> $02:$03 / L47B8 -> $0E:$0F
 * setup, JSR L466B, the $06:$07 +$40 advance, the $17 EOR #$02 toggle
 * with its $16 row counter (the STZ $16 plus L47BE/L47C4 accumulate and
 * JSR L43D6 every $10 rows), DEX / BNE L8610, RTS — every relative
 * (BCC 90 02 -> +0x95, BNE d0 17/d0 0f -> +0xb4, BNE d0 a7 -> +0x5f)
 * resolves inside the body, and the trailing RTS at 0x4669 sits
 * immediately before the next stream's BRK byte (da65 asm:10193),
 * confirming the span.  da65's L0011/L0000 renderings are its
 * zero-page-as-absolute artifact class — the media confirms the
 * zero-page forms (A4 11 at +0x03, 85 00 at +0x72); its L466A is a data
 * label (the STA/LDA $466A absolute operands at +0x4f/+0x60), not code
 * inside the span.  The entry CPU address is not pinned (no bound
 * caller; the runtime bank mapping stays out of scope), so the receipt
 * carries 0 for it.  The callees L4552, L4932, L458E, L424B, L466B, and
 * L43D6 remain unbound future windows.  Proven for the authenticated US
 * stage-two body only; no semantics, System Card base or bank-mapping
 * arithmetic, record semantics, or graphics role follows. */
#define THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_USER_OFFSET 0x45b1u
#define THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES 0xb9u

/* $45xx-routine internal invariants: byte offsets of the two bound
 * JSR $4696 windows and of da65's L8610 main-loop head inside the
 * body (each compile-time asserted against the round-17 call-site
 * image offsets and the body span). */
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4696_A_OFF 0x09u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4696_B_OFF 0x1au
#define THERON_TRACK02_IPL_STAGE2_45XX_LOOP_HEAD_OFF 0x5fu

/* Enclosing-$45xx callees: the six JSR targets of the bound $45xx
 * routine.  All six live on image bank 2 alongside their caller (the
 * runtime bank mapping that the bound JSR $4696 operands pin for the
 * $4xxx window keeps the $3800 dynamic-payload CD_READ away from
 * them), so they stay in the stage-two image lane; da65 decodes each
 * body inline under its linear map (rendered at $824B/$83D6/$8552/
 * $858E/$866B/$8932 — its L424B/L43D6/L4552/L458E/L466B/L4932 labels
 * instead sit on unrelated bank-0 streams, the L4696-label class).
 * The bound $45xx body encodes each target's JSR operand ($424B,
 * $43D6, $4552, $458E, $466B, $4932), which pins each CPU entry
 * address to its image offset exactly like the round-16 L4696 pinning.
 * L424B [0x424b..0x42bf) includes its BSR-local subroutine at +0x5b
 * (da65's L82A6: the ($00),y -> L47E0,x pair copy); da65 mis-splits
 * the LDA #$02 / STA $10 / CLX head into `.byte $85` / `bpl $821C`
 * (the round-18 mid-instruction class) and renders DEC $11 / LDA
 * ($00),y as L0011/L0000 zero-page-as-absolute, so the media bytes are
 * authoritative.  L466B is self-modifying (STA $468D/$468E/$4691
 * rewrite the TIA source/length operands — the L5C69 class); the media
 * bytes are the as-loaded image, and da65's `a:$02`/`a:$03` renderings
 * confirm the 3-byte absolute VDC-data stores.  L4552 ends exactly at
 * L458E; L466B ends exactly at the bound L4696 body; L458E ends at the
 * unbound STZ L47B8 / TII gap routine [0x45a6..0x45b1); L424B ends at
 * the unbound L42BF; L43D6 ends before the unbound $4417 stream; L4932
 * ends before the unbound TMA/PHA stream.  L424B's own callees (L43A1,
 * L42BF) and every other stream remain unbound future windows.  No
 * semantics, System Card base or bank-mapping arithmetic, record
 * semantics, or graphics role follows. */
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_USER_OFFSET 0x424bu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_CPU_ADDRESS 0x424bu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES 0x74u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_USER_OFFSET 0x43d6u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_CPU_ADDRESS 0x43d6u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_BYTES 0x41u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_USER_OFFSET 0x4552u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_CPU_ADDRESS 0x4552u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_BYTES 0x3cu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_USER_OFFSET 0x458eu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_CPU_ADDRESS 0x458eu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_BYTES 0x18u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_USER_OFFSET 0x466bu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_CPU_ADDRESS 0x466bu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_BYTES 0x2bu
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_USER_OFFSET 0x4932u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_CPU_ADDRESS 0x4932u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_BYTES 0x11u

/* $45xx-callee call-site invariants: byte offset of each JSR opcode
 * within the bound $45xx routine body (call_site offset + 3 <=
 * 45XX_ROUTINE_BYTES is compile-time-asserted), plus the internal
 * JSR L43D6 at +0x02 inside the bound L424B body. */
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4552_OFF 0x25u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4932_OFF 0x35u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L458E_OFF 0x42u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L424B_OFF 0x6du
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L466B_OFF 0x87u
#define THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L43D6_OFF 0xb1u
#define THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L43D6_OFF 0x02u

/* Same-image bytes bound by the stage-two enclosing-$45xx-callees
 * verifier. */
#define THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES 0x145u

/* L3114 tier-5 callees: the remaining callees of the bound tier-4
 * bodies (L53C4's BSR L5403; L560B's BSR L5657 / JSR L52A2 / JSR
 * L52C8), L5403's own BSR L541E, the L5667 data table read through the
 * bound L560B -> L5657 copy path, and L54C5 (called only from the
 * unbound L54DB stream — its bytes still bind).  da65 lists every body
 * inline under its linear map (CPU = image + $4000, image banks 0/1),
 * so all stay in the stage-two image lane (all image offsets lie below
 * $1800).  L5403's `a:$02`/`a:$03` absolute loads confirm da65's
 * 3-byte form (AD 02 00 / AD 03 00); L52C8's STA L0000 is da65's
 * zero-page-as-absolute artifact (media 85 00).  The windows chain
 * contiguously: L5403 [0x1403..0x141e) ends at L541E [0x141e..0x142d),
 * which ends at the bound L542D; L52A2 [0x12a2..0x12c8) ends at L52C8
 * [0x12c8..0x12da), which ends at the unbound L52DA; L5657
 * [0x1657..0x1667) ends at the L5667 data table [0x1667..0x16af)
 * (9 rows x 8 bytes — da65 garbage-decodes it as bbs7/cpy/brk/st0
 * code, so the media bytes are authoritative; the table resumes code
 * at da65's L56AF); L54C5 [0x14c5..0x14db) ends at the unbound L54DB.
 * The L5667 table is the data window the bound L560B body loads into
 * $00:$01 (LDA #$67 / STA $00 / LDA #$56 / STA $01 at L560B+0x0e) for
 * the L5657 ($00),y -> ($02),y copy loop — the L5C20-table class.
 * L52C8's L52FD callee, L52DA, L54DB, and every other stream remain
 * unbound future windows. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_USER_OFFSET 0x1403u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_CPU_ADDRESS 0x5403u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_BYTES 0x1bu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_USER_OFFSET 0x141eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_CPU_ADDRESS 0x541eu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_BYTES 0x0fu
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_USER_OFFSET 0x12a2u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_CPU_ADDRESS 0x52a2u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_BYTES 0x26u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_USER_OFFSET 0x12c8u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_CPU_ADDRESS 0x52c8u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_BYTES 0x12u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_USER_OFFSET 0x1657u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_CPU_ADDRESS 0x5657u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_BYTES 0x10u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_USER_OFFSET 0x14c5u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_CPU_ADDRESS 0x54c5u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_BYTES 0x16u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_USER_OFFSET 0x1667u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_CPU_ADDRESS 0x5667u
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_BYTES 0x48u

/* Tier-5 call-site invariants in previously bound caller bodies (the
 * whole-body exact matches of the newly bound bodies cover their own
 * internal call sites): the BSR L5403 at +0x2c inside the bound L53C4
 * body, the BSR L5657 at +0x1f / JSR L52A2 at +0x21 / JSR L52C8 at
 * +0x25 inside the bound L560B body, and the L560B data site (the
 * LDA #$67 / STA $00 / LDA #$56 / STA $01 setup at +0x0e whose $5667
 * target is the bound L5667 table). */
#define THERON_TRACK02_IPL_STAGE2_L53C4_CALL_SITE_L5403_OFF 0x2cu
#define THERON_TRACK02_IPL_STAGE2_L560B_CALL_SITE_L5657_OFF 0x1fu
#define THERON_TRACK02_IPL_STAGE2_L560B_CALL_SITE_L52A2_OFF 0x21u
#define THERON_TRACK02_IPL_STAGE2_L560B_CALL_SITE_L52C8_OFF 0x25u
#define THERON_TRACK02_IPL_STAGE2_L560B_DATA_SITE_L5667_OFF 0x0eu

/* Same-image bytes bound by the stage-two L3114 tier-5-callees
 * verifier. */
#define THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES 0xd0u

/* $45xx-lane tier-2 windows: the two callees of the bound L424B body
 * (L43A1, L42BF) plus the adjacent $45A6 TII gap stream.  All three
 * live on image bank 2 alongside the bound $45xx-routine lane, so they
 * stay in the stage-two image lane; da65 decodes each body inline
 * under its linear map ($82BF/$83A1/$85A6 renderings — its L42BF/L43A1
 * labels sit on unrelated bank-0 streams, the L4696-label class).  The
 * bound L424B body encodes each callee's JSR operand ($43A1 at +0x19
 * and +0x45, $42BF at +0x2d and +0x55), which pins each CPU entry
 * address to its image offset exactly like the round-16 L4696 pinning;
 * L42BF's own JSR $43D6 at +0x14 targets the round-21 L43D6 body.
 * L43A1 [0x43a1..0x43d6) ends exactly at the bound L43D6 body.  L42BF
 * [0x42bf..0x42db) ends at the unbound $3B75 stream.  The $45A6 TII
 * gap stream [0x45a6..0x45b1) (STZ L47B8 / TII $47B8,$47B9,$00A7 /
 * RTS) sits between the bound L458E and the bound $45xx routine; it is
 * called only from the unbound $401C stream (JSR $45A6 at image
 * 0x401c), so its entry CPU address is not pinned (the $45xx-routine
 * precedent) and the receipt carries 0 for it; its span ends exactly
 * at the bound $45xx routine (adjacency compile-time-asserted).  The
 * $3B75 stream, the $4417 stream, and the $4943 stream remain unbound
 * future windows.  No semantics, System Card base or bank-mapping
 * arithmetic, record semantics, or graphics role follows. */
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_USER_OFFSET 0x43a1u
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_CPU_ADDRESS 0x43a1u
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_BYTES 0x35u
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_USER_OFFSET 0x42bfu
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_CPU_ADDRESS 0x42bfu
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_BYTES 0x1cu
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_USER_OFFSET 0x45a6u
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES 0x0bu

/* $45xx-tier-2 call-site invariants: byte offset of each JSR opcode
 * within the bound L424B body (call_site offset + 3 <=
 * 45XX_CALLEE_L424B_BYTES is compile-time-asserted), plus the internal
 * JSR L43D6 at +0x14 inside the bound L42BF body. */
#define THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L43A1_A_OFF 0x19u
#define THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L43A1_B_OFF 0x45u
#define THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L42BF_A_OFF 0x2du
#define THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L42BF_B_OFF 0x55u
#define THERON_TRACK02_IPL_STAGE2_L42BF_CALL_SITE_L43D6_OFF 0x14u

/* Same-image bytes bound by the stage-two $45xx-lane tier-2
 * verifier. */
#define THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES 0x5cu

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

/* Receipt for the stage-two dispatch-machine closure proof.  It binds
 * only instruction and table bytes of the authenticated US stage-two
 * body: the register-seed tail [0xb5..0xb7), the seven dispatch stubs
 * [0xf1..0x10d), the ten-entry jump table [0x10d..0x121) (each entry
 * verified to point inside the loaded image), the L4AF7 MPR-page body
 * [0xaf7..0xb00), and the L4F5E selector body [0xf5e..0xf66).  With
 * the already-bound windows the executed dispatch machine [0x00..0x121)
 * is contiguously bound.  Proven for the US body only (the
 * source-locked JP/US identity attestation covers the $4090 window,
 * not these streams); no handler semantics for the ten jump-table
 * targets, no System Card base arithmetic, no record semantics, and no
 * graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t seed_tail_bytes;
    size_t dispatch_stubs_bytes;
    size_t jump_table_bytes;
    size_t jump_table_entries;
    size_t mpr_page_bytes;
    size_t selector_bytes;
    size_t loop_closure_bound_bytes;
    size_t dispatch_machine_bound_bytes;
    int seed_tail_proven;
    int dispatch_stubs_proven;
    int jump_table_proven;
    int mpr_page_proven;
    int selector_proven;
    int dispatch_machine_contiguous_proven;
} Theron_Track02Stage2DispatchMachineReceipt;

/* Receipt for the stage-two L8000/L45A6 callee-pair proof.  It binds
 * only instruction bytes of the authenticated US stage-two body: the
 * L8000 body [0x4000..0x40bc) at the head of image sector 8 (the
 * entry path's first call, at user offset 0x11 inside the bound
 * prologue; its three da65 decode-artifact spans bound to the
 * authenticated media bytes) and the L45A6 body [0x5a6..0x5ca) (whose
 * only call site is the JSR at L8000+0x1c inside the bound L8000
 * window).  Proven for the US body only (the source-locked JP/US
 * identity attestation covers the $4090 window, not these streams);
 * no semantics for the L4696/L48FC callees or the dynamic-lane $3AB7
 * target, no System Card base arithmetic, no record semantics, and no
 * graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l8000_bytes;
    size_t l45a6_bytes;
    size_t pair_bound_bytes;
    int l8000_proven;
    int l45a6_proven;
    int l8000_call_site_proven;
    int l45a6_single_caller_proven;
} Theron_Track02Stage2L8000PairReceipt;

/* Receipt for the stage-two jump-table handler-body proof.  It binds
 * only instruction bytes of the authenticated US stage-two body: the
 * ten L410D handler targets $41C5..$4253 as one contiguous span
 * [0x1c5..0x254) (their three da65 decode-artifact spans bound to the
 * authenticated media bytes), with the entry chain (strictly
 * increasing little-endian targets, first target at the span head,
 * last target's single byte closing the span) and the table-read site
 * inside the bound dispatcher window.  Proven for the US body only
 * (the source-locked JP/US identity attestation covers the $4090
 * window, not these streams); no command or stream semantics for the
 * handlers, no semantics for the L41B9/L43D6/L37D8 callees or the
 * dynamic-lane $383E target, no System Card base arithmetic, no record
 * semantics, and no graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t handlers_bytes;
    size_t handler_count;
    uint16_t first_handler_cpu_address;
    uint16_t last_handler_cpu_address;
    int handlers_proven;
    int handler_entry_chain_proven;
    int handlers_contiguous_proven;
} Theron_Track02Stage2JumpTableHandlersReceipt;

/* Receipt for the stage-two L4696/L3114 far-callee proof.  It binds
 * only instruction bytes of the authenticated US stage-two body: the
 * L4696 multiply body [0x4696..0x46db) (da65's linear L4696 label at
 * 0x696 carries the undecodable head byte $33 — the flagged head-byte
 * decode artifact; the authenticated body matches da65's L8696 decode
 * instruction by instruction, its L0011 zero-page-as-absolute
 * renderings superseded by the media bytes) and the L3114 body
 * [0x1114..0x1172) (declared absolute by da65 without a body decode;
 * its trailing RTS sits immediately before the da65-declared L3172
 * entry), with the call-site invariant (the L4696 JSR inside the bound
 * L8000 window; the L3114 JSR inside the bound L4F5E selector window).
 * Proven for the US body only (the source-locked JP/US identity
 * attestation covers the $4090 window, not these streams); no
 * multiplier or queue semantics, no callee semantics, no System Card
 * base or bank-mapping arithmetic, no record semantics, and no
 * graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l4696_bytes;
    size_t l3114_bytes;
    size_t l4696_l3114_bound_bytes;
    uint16_t l4696_cpu_address;
    uint16_t l3114_cpu_address;
    int l4696_proven;
    int l3114_proven;
    int l4696_call_site_proven;
    int l3114_call_site_proven;
} Theron_Track02Stage2L4696L3114Receipt;

/* Receipt for the stage-two L3114-callees proof.  It binds only
 * instruction bytes of the authenticated US stage-two body: the six
 * L3114 callee bodies (L3172 [0x1172..0x117d) and the $117D trampoline
 * [0x117d..0x118a) in the low-image region; L4F66 [0x0f66..0x0f7a),
 * L5213 [0x1213..0x121f), L526D [0x126d..0x1280), and L55E0
 * [0x15e0..0x15e8) on image banks 0/1 per da65's inline linear
 * decodes), the six L3114 call-site invariants (each JSR/BSR opcode at
 * its compile-time-asserted offset inside the bound L3114 body), and
 * the two $45xx-tier L4696 call sites (3-byte JSR windows at
 * 0x45ba/0x45cb targeting the bound L4696 body).  Proven for the US
 * body only (the source-locked JP/US identity attestation covers the
 * $4090 window, not these streams); no callee-of-callee, semantics,
 * System Card base or bank-mapping arithmetic, record semantics, or
 * graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l3172_bytes;
    size_t far117d_bytes;
    size_t l4f66_bytes;
    size_t l5213_bytes;
    size_t l526d_bytes;
    size_t l55e0_bytes;
    size_t l3114_callees_bound_bytes;
    uint16_t l4f66_cpu_address;
    uint16_t l5213_cpu_address;
    uint16_t l526d_cpu_address;
    uint16_t l55e0_cpu_address;
    int l3172_proven;
    int far117d_proven;
    int l4f66_proven;
    int l5213_proven;
    int l526d_proven;
    int l55e0_proven;
    int l3114_call_sites_proven;
    int l4696_call_sites_45xx_proven;
} Theron_Track02Stage2L3114CalleesReceipt;

/* Receipt for the stage-two L3114 tier-2-callees proof.  It binds only
 * instruction bytes of the authenticated US stage-two body: the seven
 * callees of the bound $117D trampoline, L526D, and L55E0 — L51F9
 * [0x11f9..0x1213) (da65's L5200 mid-instruction label artifact splits
 * the `ror $0E`; the media bytes are authoritative, and the body ends
 * exactly at the bound L5213 entry), L55E8 [0x15e8..0x15ef) and L55F6
 * [0x15f6..0x1600) (L55E8 directly after the bound L55E0 body), and
 * L5BF5 [0x1bf5..0x1c06), L5C25 [0x1c25..0x1c69), L5C8C
 * [0x1c8c..0x1c9f), and L5CB0 [0x1cb0..0x1cbf) — all listed inline by
 * da65 under its linear map (CPU = image + $4000, image banks 0/1).
 * The call-site invariants (four JSR opcodes inside the bound $117D
 * trampoline, the JSR L51F9 head of the bound L526D body, the two BSR
 * opcodes at the head of the bound L55E0 body) are compile-time
 * asserted.  Proven for the US body only (the source-locked JP/US
 * identity attestation covers the $4090 window, not these streams); no
 * callee-of-callee (L5C06/L5C9F/L536E/L5439/L5C69/L54A0/L5600), the
 * LE063 far-call targets, L5C25's L5C2C alternate entry, semantics,
 * System Card base or bank-mapping arithmetic, record semantics, or
 * graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l51f9_bytes;
    size_t l55e8_bytes;
    size_t l55f6_bytes;
    size_t l5bf5_bytes;
    size_t l5c25_bytes;
    size_t l5c8c_bytes;
    size_t l5cb0_bytes;
    size_t tier2_bound_bytes;
    uint16_t l51f9_cpu_address;
    uint16_t l55e8_cpu_address;
    uint16_t l55f6_cpu_address;
    uint16_t l5bf5_cpu_address;
    uint16_t l5c25_cpu_address;
    uint16_t l5c8c_cpu_address;
    uint16_t l5cb0_cpu_address;
    int l51f9_proven;
    int l55e8_proven;
    int l55f6_proven;
    int l5bf5_proven;
    int l5c25_proven;
    int l5c8c_proven;
    int l5cb0_proven;
    int far117d_call_sites_proven;
    int l526d_call_site_proven;
    int l55e0_call_sites_proven;
} Theron_Track02Stage2L3114Tier2CalleesReceipt;

/* Receipt for the stage-two L3114 tier-3-callees proof.  It binds only
 * instruction and table bytes of the authenticated US stage-two body:
 * the seven callees of the bound L5C8C/L5C25/L55F6 bodies — L5C06
 * [0x1c06..0x1c20) (the L4FD1/L4FD2 toggle with its JSR L5C25 /
 * JSR L5C2C pair), the L5C20 table [0x1c20..0x1c25) (5 zero bytes as
 * loaded), L5C69 [0x1c69..0x1c8c) (self-modifying: STA L5C7E rewrites
 * the ORA/AND opcode at 0x1c7e; the media bytes are the as-loaded
 * image), L5C9F [0x1c9f..0x1cb0) (ends exactly at the bound L5CB0
 * entry), L536E [0x136e..0x13c4), L5439 [0x1439..0x1455), L54A0
 * [0x14a0..0x14af) and L5600 [0x1600..0x160b) (the media confirms
 * da65's `a:$02`/`a:$03` absolute-store renderings; both bodies end
 * exactly at the next da65 labels L54AF/L560B) — all listed inline by
 * da65 under its linear map (CPU = image + $4000, image banks 0/1).
 * The call-site invariants (two JSR opcodes inside the bound L5C8C
 * body; the JSR L536E / BSR L5C69 / JSR L5439 inside the bound L5C25
 * body; the JSR L54A0 / BSR L5600 inside the bound L55F6 body; the
 * LDA $5C20,x data site inside the bound L5BF5 body) and the L5C2C
 * alternate-entry offset (+0x07 inside the bound L5C25 window) are
 * compile-time asserted.  Proven for the US body only (the
 * source-locked JP/US identity attestation covers the $4090 window,
 * not these streams); no tier-4 callee (L4F7A/L542D/L5482/L5492/
 * L535E/L5455/L53C4/L54AF/L560B), LE063-target, semantics, System
 * Card base or bank-mapping arithmetic, record semantics, or graphics
 * role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l5c06_bytes;
    size_t l5c20_bytes;
    size_t l5c69_bytes;
    size_t l5c9f_bytes;
    size_t l536e_bytes;
    size_t l5439_bytes;
    size_t l54a0_bytes;
    size_t l5600_bytes;
    size_t tier3_bound_bytes;
    uint16_t l5c06_cpu_address;
    uint16_t l5c20_cpu_address;
    uint16_t l5c69_cpu_address;
    uint16_t l5c9f_cpu_address;
    uint16_t l536e_cpu_address;
    uint16_t l5439_cpu_address;
    uint16_t l54a0_cpu_address;
    uint16_t l5600_cpu_address;
    int l5c06_proven;
    int l5c20_proven;
    int l5c69_proven;
    int l5c9f_proven;
    int l536e_proven;
    int l5439_proven;
    int l54a0_proven;
    int l5600_proven;
    int l5c2c_entry_proven;
    int l5c8c_call_sites_proven;
    int l5c25_call_sites_proven;
    int l55f6_call_sites_proven;
    int l5bf5_data_site_proven;
} Theron_Track02Stage2L3114Tier3CalleesReceipt;

/* Receipt for the stage-two L3114 tier-4-callees proof.  It binds
 * only instruction bytes of the authenticated US stage-two body: the
 * nine remaining callees of the bound tier-2/3 bodies — L4F7A
 * [0x0f7a..0x0f89) (the L4FD4-indexed X/Y delay nest, starting where
 * the bound L4F66 ends), L535E [0x135e..0x136e) (the ($04),y pair
 * loader with da65's `a:$02`/`a:$03` absolute stores confirmed by the
 * media, ending at the bound L536E), L53C4 [0x13c4..0x1403) (the
 * L51F9-driven record writer), L542D [0x142d..0x1439) (the $04:$05
 * +6 fix-up, ending at the bound L5439), L5455 [0x1455..0x1482),
 * L5482 [0x1482..0x1492), L5492 [0x1492..0x14a0) (the contiguous
 * chain ending at the bound L54A0), L54AF [0x14af..0x14c5) (the
 * L4F9F-indexed L4FA0,x pair store), and L560B [0x160b..0x1657) (the
 * 9-row $5667 copy setup — da65's L563D mid-instruction label
 * artifact splits the BCC operand at 0x163d into `.byte $90` plus
 * garbage `st0 #$EE`/`cld`/`.byte $4F` renderings, so the media bytes
 * are authoritative: BCC L5641 / INC $4FD8; its L0000 zero-page-as-
 * absolute renderings are likewise superseded) — all listed inline by
 * da65 under its linear map (CPU = image + $4000, image banks 0/1).
 * The call-site invariants (the JSR L4F7A at +0x09 inside the bound
 * L5C9F body; the JSR L5492 at +0x1f inside the bound L5C69 body)
 * are compile-time asserted.  Proven for the US body only (the
 * source-locked JP/US identity attestation covers the $4090 window,
 * not these streams); no tier-5 callee (L5403/L541E/L52A2/L52C8/
 * L5657/L54C5), LE063-target, semantics, System Card base or
 * bank-mapping arithmetic, record semantics, or graphics role
 * follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l4f7a_bytes;
    size_t l535e_bytes;
    size_t l53c4_bytes;
    size_t l542d_bytes;
    size_t l5455_bytes;
    size_t l5482_bytes;
    size_t l5492_bytes;
    size_t l54af_bytes;
    size_t l560b_bytes;
    size_t tier4_bound_bytes;
    uint16_t l4f7a_cpu_address;
    uint16_t l535e_cpu_address;
    uint16_t l53c4_cpu_address;
    uint16_t l542d_cpu_address;
    uint16_t l5455_cpu_address;
    uint16_t l5482_cpu_address;
    uint16_t l5492_cpu_address;
    uint16_t l54af_cpu_address;
    uint16_t l560b_cpu_address;
    int l4f7a_proven;
    int l535e_proven;
    int l53c4_proven;
    int l542d_proven;
    int l5455_proven;
    int l5482_proven;
    int l5492_proven;
    int l54af_proven;
    int l560b_proven;
    int l5c9f_call_site_proven;
    int l5c69_call_site_proven;
    int tier4_chain_contiguous_proven;
} Theron_Track02Stage2L3114Tier4CalleesReceipt;

/* Enclosing $45xx routine [0x45b1..0x466a): the otherwise unbound body
 * that holds the two round-17 $45xx-tier JSR $4696 windows at +0x09 and
 * +0x1a, decoded inline by da65 (asm:10102-10191, no entry label; its
 * L8610 main-loop head sits at +0x5f) and byte-matched against the
 * authenticated US stage-two image.  da65's L0011/L0000 zero-page-as-
 * absolute renderings and its L466A data label are flagged artifact
 * classes superseded by the media bytes.  The entry CPU address is not
 * pinned (no bound caller), so routine_cpu_address carries 0.  The
 * call-site offsets and the L8610 loop-head offset are compile-time
 * asserted against the round-17 image offsets and the body span.  The
 * six callees (L4552/L4932/L458E/L424B/L466B/L43D6) remain unbound
 * future windows.  Proven for the US body only (the source-locked JP/US
 * identity attestation covers the $4090 window, not this stream); no
 * callee semantics, System Card base or bank-mapping arithmetic, record
 * semantics, or graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t routine_bytes;
    uint16_t routine_cpu_address;
    int routine_proven;
    int l4696_call_sites_within_proven;
} Theron_Track02Stage2Enclosing45xxReceipt;

/* Enclosing-$45xx callees: the six JSR targets of the bound $45xx
 * routine — L424B [0x424b..0x42bf) (with its BSR-local subroutine at
 * +0x5b — da65's L82A6; da65's head mis-split `.byte $85`/`bpl $821C`
 * and its L0011/L0000 zero-page-as-absolute renderings superseded by
 * the media bytes), L43D6 [0x43d6..0x4417), L4552 [0x4552..0x458e)
 * (ends at L458E), L458E [0x458e..0x45a6), L466B [0x466b..0x4696)
 * (self-modifying TIA setup — the media bytes are the as-loaded image;
 * ends at the bound L4696 body), and L4932 [0x4932..0x4943) — all
 * listed inline by da65 under its linear map ($824B/$83D6/$8552/$858E/
 * $866B/$8932 renderings) and byte-matched against the authenticated
 * US stage-two image.  The CPU entry addresses are pinned by the JSR
 * operands inside the bound $45xx body (the round-16 L4696 class);
 * the call-site offsets inside the $45xx body, the internal JSR L43D6
 * at L424B+0x02, and the L4552->L458E and L466B->L4696 adjacencies are
 * compile-time asserted.  Proven for the US body only; L424B's L43A1/
 * L42BF callees and every other stream remain unbound future windows;
 * no semantics, System Card base or bank-mapping arithmetic, record
 * semantics, or graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l424b_bytes;
    size_t l43d6_bytes;
    size_t l4552_bytes;
    size_t l458e_bytes;
    size_t l466b_bytes;
    size_t l4932_bytes;
    size_t callees_bound_bytes;
    uint16_t l424b_cpu_address;
    uint16_t l43d6_cpu_address;
    uint16_t l4552_cpu_address;
    uint16_t l458e_cpu_address;
    uint16_t l466b_cpu_address;
    uint16_t l4932_cpu_address;
    int l424b_proven;
    int l43d6_proven;
    int l4552_proven;
    int l458e_proven;
    int l466b_proven;
    int l4932_proven;
    int l424b_call_site_proven;
    int adjacency_proven;
} Theron_Track02Stage2Enclosing45xxCalleesReceipt;

/* L3114 tier-5 callees: L5403 [0x1403..0x141e) (the CLY-counted
 * `a:$02`/`a:$03` absolute-load pair copy with its BSR L541E and BSR
 * L5492 — da65's 3-byte absolute form media-confirmed), L541E
 * [0x141e..0x142d) (the ST0 #$01 / $0E:$0F -> $0002:$0003 / ST0 #$02
 * VDC address setup), L52A2 [0x12a2..0x12c8) (the L4FD5-L4FD8 ->
 * $04:$07 / $14-$17 setup), L52C8 [0x12c8..0x12da) (the $14,x +
 * L4FD5/L4FD6 -> $00:$01 and JSR L52FD — da65's L0000 zero-page-as-
 * absolute rendering superseded by the media 85 00), L5657
 * [0x1657..0x1667) (the 8-byte ($00),y -> ($02),y SXY copy loop), the
 * L5667 data table [0x1667..0x16af) (9 rows x 8 bytes read through the
 * bound L560B $00:$01 setup — da65 garbage-decodes it as code, media
 * authoritative), and L54C5 [0x14c5..0x14db) (the L4F9F-indexed
 * L4FA0,x -> L4FD7/L4FD8 pair load — called only from the unbound
 * L54DB stream) — all listed inline by da65 under its linear map (CPU
 * = image + $4000, image banks 0/1) and byte-matched against the
 * authenticated US stage-two image.  The call-site invariants (the BSR
 * L5403 at +0x2c inside the bound L53C4 body; the BSR L5657 at +0x1f,
 * JSR L52A2 at +0x21, JSR L52C8 at +0x25, and the L5667 data site at
 * +0x0e inside the bound L560B body) are compile-time asserted, as are
 * the L5403->L541E->L542D, L52A2->L52C8, and L5657->L5667 adjacencies.
 * Proven for the US body only; L52FD, L52DA, L54DB, and every other
 * stream remain unbound future windows; no semantics, System Card base
 * or bank-mapping arithmetic, record semantics, or graphics role
 * follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l5403_bytes;
    size_t l541e_bytes;
    size_t l52a2_bytes;
    size_t l52c8_bytes;
    size_t l5657_bytes;
    size_t l54c5_bytes;
    size_t l5667_bytes;
    size_t tier5_bound_bytes;
    uint16_t l5403_cpu_address;
    uint16_t l541e_cpu_address;
    uint16_t l52a2_cpu_address;
    uint16_t l52c8_cpu_address;
    uint16_t l5657_cpu_address;
    uint16_t l54c5_cpu_address;
    uint16_t l5667_cpu_address;
    int l5403_proven;
    int l541e_proven;
    int l52a2_proven;
    int l52c8_proven;
    int l5657_proven;
    int l54c5_proven;
    int l5667_proven;
    int l53c4_call_site_proven;
    int l560b_call_sites_proven;
    int tier5_chain_contiguous_proven;
} Theron_Track02Stage2L3114Tier5CalleesReceipt;

/* $45xx-lane tier-2 windows: L43A1 [0x43a1..0x43d6) (the $14:$15 ->
 * $0E:$0F / three ASL/ROL pairs / L47CB/L47CC add / $58 ASL A add /
 * $0E:$0F -> $00:$01 finish — da65's L0000 zero-page-as-absolute
 * rendering superseded by the media 85 00; ends exactly at the bound
 * L43D6 body), L42BF [0x42bf..0x42db) (the $56 $10-counter with its
 * L47C4 save/increment/JSR L43D6/restore — the internal JSR $43D6 at
 * +0x14 targets the round-21 body; ends at the unbound $3B75 stream),
 * and the $45A6 TII gap stream [0x45a6..0x45b1) (STZ L47B8 /
 * TII $47B8,$47B9,$00A7 / RTS — called only from the unbound $401C
 * stream, so its entry CPU address is not pinned and the receipt
 * carries 0; ends exactly at the bound $45xx routine) — all listed
 * inline by da65 under its linear map ($83A1/$82BF/$85A6 renderings)
 * and byte-matched against the authenticated US stage-two image.  The
 * L424B call-site offsets (+0x19/+0x45 L43A1, +0x2d/+0x55 L42BF), the
 * internal L42BF JSR L43D6 at +0x14, and the L43A1->L43D6 and
 * $45A6->$45xx-routine adjacencies are compile-time asserted.  Proven
 * for the US body only; the $3B75/$4417/$4943 streams remain unbound
 * future windows; no semantics, System Card base or bank-mapping
 * arithmetic, record semantics, or graphics role follows. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage2_record;
    size_t stage2_raw_sector;
    size_t l43a1_bytes;
    size_t l42bf_bytes;
    size_t gap45a6_bytes;
    size_t tier2_bound_bytes;
    uint16_t l43a1_cpu_address;
    uint16_t l42bf_cpu_address;
    uint16_t gap45a6_cpu_address;
    int l43a1_proven;
    int l42bf_proven;
    int gap45a6_proven;
    int l424b_call_sites_proven;
    int adjacency_proven;
} Theron_Track02Stage245xxTier2CalleesReceipt;

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

/* Verifies the stage-two dispatch-machine closure against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact register-seed tail, dispatch stubs,
 * jump table, L4AF7 MPR-page, and L4F5E selector bytes at their
 * original user offsets inside the proven stage-two image, and checks
 * that each of the ten little-endian jump-table entries points inside
 * the loaded image.  The JP variant rejects (these streams are not
 * attested byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_dispatch_machine(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2DispatchMachineReceipt *out_receipt);

/* Verifies the stage-two L8000/L45A6 callee pair against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L8000 body bytes [0x4000..0x40bc) at
 * the head of image sector 8 and the exact L45A6 body bytes
 * [0x5a6..0x5ca) at their original user offsets inside the proven
 * stage-two image, and checks that the L8000 call site (0x11) sits
 * inside the bound entry path and that the L45A6 call site
 * (L8000+0x1c) and the L4696/L48FC tail call sites sit inside the
 * bound L8000 window.  The JP variant rejects (these streams are not
 * attested byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l8000_pair(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L8000PairReceipt *out_receipt);

/* Verifies the stage-two jump-table handler bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact 143 handler bytes [0x1c5..0x254) at
 * their original user offset inside the proven stage-two image, and
 * checks that the ten little-endian jump-table entries chain onto the
 * span (strictly increasing, first entry at the span head, last
 * entry's single byte closing the span) and that the JMP (L410D,x)
 * table-read site (0xe1) sits inside the bound dispatcher window.  The
 * JP variant rejects (these streams are not attested byte-identical);
 * any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_jump_table_handlers(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2JumpTableHandlersReceipt *out_receipt);

/* Verifies the stage-two L4696/L3114 far-callee bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L4696 multiply body bytes
 * [0x4696..0x46db) and the exact L3114 body bytes [0x1114..0x1172) at
 * their original user offsets inside the proven stage-two image, and
 * checks that the L4696 call site (L8000+0x6a) sits inside the bound
 * L8000 window and that the L3114 call site (L4F5E+4) sits inside the
 * bound selector window.  The JP variant rejects (these streams are
 * not attested byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l4696_l3114(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L4696L3114Receipt *out_receipt);

/* Verifies the stage-two L3114 callee bodies against the authenticated
 * US Track 02 body.  Chains the fail-closed IPL loader proof, then
 * requires the exact L3172, $117D, L4F66, L5213, L526D, and L55E0 body
 * bytes at their original user offsets inside the proven stage-two
 * image, checks that each of the six L3114 JSR/BSR call sites sits at
 * its compile-time-asserted offset inside the bound L3114 body, and
 * requires the two $45xx-tier 3-byte JSR $4696 call-site windows at
 * 0x45ba/0x45cb.  The JP variant rejects (these streams are not
 * attested byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114CalleesReceipt *out_receipt);

/* Verifies the stage-two L3114 tier-2 callee bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L51F9, L55E8, L55F6, L5BF5, L5C25,
 * L5C8C, and L5CB0 body bytes at their original user offsets inside
 * the proven stage-two image, and checks that each tier-2 call site
 * (four JSR opcodes inside the bound $117D trampoline, the JSR L51F9
 * head of the bound L526D body, the two BSR opcodes at the head of
 * the bound L55E0 body) sits at its compile-time-asserted offset.
 * The JP variant rejects (these streams are not attested
 * byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier2_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier2CalleesReceipt *out_receipt);

/* Verifies the stage-two L3114 tier-3 callee bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L5C06, L5C20 table, L5C69, L5C9F,
 * L536E, L5439, L54A0, and L5600 bytes at their original user offsets
 * inside the proven stage-two image, and checks that each tier-3 call
 * site (inside the bound L5C8C/L5C25/L55F6 bodies), the L5BF5 data
 * site (LDA $5C20,x), and the L5C2C alternate-entry offset sit at
 * their compile-time-asserted positions.  The JP variant rejects
 * (these streams are not attested byte-identical); any changed byte
 * fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier3_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier3CalleesReceipt *out_receipt);

/* Verifies the stage-two L3114 tier-4 callee bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L4F7A, L535E, L53C4, L542D, L5455,
 * L5482, L5492, L54AF, and L560B body bytes at their original user
 * offsets inside the proven stage-two image, checks that the tier-4
 * call sites (the JSR L4F7A inside the bound L5C9F body; the JSR
 * L5492 inside the bound L5C69 body) sit at their compile-time
 * asserted offsets, and asserts the contiguous L535E->L536E and
 * L542D->...->L54AF adjacency chain.  The JP variant rejects (these
 * streams are not attested byte-identical); any changed byte fails
 * closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier4_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier4CalleesReceipt *out_receipt);

/* Verifies the enclosing $45xx routine body against the authenticated
 * US Track 02 body.  Chains the fail-closed IPL loader proof, then
 * requires the exact [0x45b1..0x466a) body bytes at their original user
 * offset inside the proven stage-two image, and checks that the two
 * round-17 JSR $4696 windows sit at their compile-time-asserted offsets
 * inside the body.  The entry CPU address is not pinned (no bound
 * caller), so the receipt carries 0 for it.  The JP variant rejects
 * (this stream is not attested byte-identical); any changed byte fails
 * closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_enclosing_45xx(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2Enclosing45xxReceipt *out_receipt);

/* Verifies the six enclosing-$45xx callee bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L424B, L43D6, L4552, L458E, L466B,
 * and L4932 body bytes at their original user offsets inside the
 * proven stage-two image, and asserts the call-site offsets inside the
 * bound $45xx routine body, the internal JSR L43D6 at L424B+0x02, and
 * the L4552->L458E / L466B->L4696 adjacency chain.  The JP variant
 * rejects (these streams are not attested byte-identical); any changed
 * byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_enclosing_45xx_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2Enclosing45xxCalleesReceipt *out_receipt);

/* Verifies the stage-two L3114 tier-5 callee bodies against the
 * authenticated US Track 02 body.  Chains the fail-closed IPL loader
 * proof, then requires the exact L5403, L541E, L52A2, L52C8, L5657,
 * L54C5, and L5667 data-table bytes at their original user offsets
 * inside the proven stage-two image, checks that the tier-5 call
 * sites (the BSR L5403 inside the bound L53C4 body; the BSR L5657,
 * JSR L52A2, JSR L52C8, and the L5667 data site inside the bound
 * L560B body) sit at their compile-time-asserted offsets, and asserts
 * the L5403->L541E->L542D, L52A2->L52C8, and L5657->L5667 adjacency
 * chain.  The JP variant rejects (these streams are not attested
 * byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier5_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier5CalleesReceipt *out_receipt);

/* Verifies the $45xx-lane tier-2 windows against the authenticated US
 * Track 02 body.  Chains the fail-closed IPL loader proof, then
 * requires the exact L43A1, L42BF, and $45A6 TII gap-stream bytes at
 * their original user offsets inside the proven stage-two image, and
 * asserts the L424B call-site offsets, the internal L42BF JSR L43D6,
 * and the L43A1->L43D6 / $45A6->$45xx-routine adjacency chain.  The
 * $45A6 entry CPU address is not pinned (no bound caller), so the
 * receipt carries 0 for it.  The JP variant rejects (these streams are
 * not attested byte-identical); any changed byte fails closed. */
Theron_Track02SignalStatus theron_v1_track02_verify_stage2_45xx_tier2_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage245xxTier2CalleesReceipt *out_receipt);

#endif /* THERON_V1_TRACK02_H */
