#ifndef NEXUS_V1_BPK_ARCHIVE_H
#define NEXUS_V1_BPK_ARCHIVE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_BPK_MAGIC_BPPK 0x4250504BU
#define NEXUS_V1_BPK_MAGIC_BMPD 0x424D5044U
#define NEXUS_V1_BPK_MAGIC_PRS3 0x50525333U
#define NEXUS_V1_BPK_MAGIC_PALT 0x50414C54U
#define NEXUS_V1_BPK_ENTRY_PREFIX_BYTES 20U
#define NEXUS_V1_BPK_PALT_ENTRY_COUNT 256U

/* Real MENU.BPK byte inspection (pass1082).
 *
 * The observed entry prefix layout (20 bytes) for the 162 PRS3-bearing
 * entries is:
 *   bytes  0..4 : unknown u32 (likely CRC32-class hash of compressed data)
 *   bytes  4..8 : unknown u32 (likely CRC32-class hash of original data)
 *   bytes  8..12: unknown u32 (likely a secondary hash or trailer marker)
 *   bytes 12..14: width (BE uint16) - confirmed against PRS3+8 pixel count
 *   byte  14   : reserved (always 0x00 in observed MENU.BPK)
 *   byte  15   : height (BE uint8) - confirmed against PRS3+8 pixel count
 *   bytes 16..19: reserved/zero in observed MENU.BPK (byte 19 is the
 *                 opaque mode flags: 6 / 14 / 22 / 30 are the four observed
 *                 pixel-mode tags; the entry[0] directory trailer carries
 *                 the unique tag 10).
 *
 * The 21st..24th bytes are the `PRS3` magic, followed by a constant
 * 0x00000001 (BE) and a BE uint32 pixel count that always equals
 * width * height in the observed MENU.BPK. The DMWeb DecodePRS3 grammar is
 * implemented and now decodes the pinned retail payloads; Saturn palette,
 * VDP1 upload ownership, and screen placement remain separate gates.
 */
#define NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET 12U
#define NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET 15U
#define NEXUS_V1_BPK_PREFIX_MODE_OFFSET 19U
#define NEXUS_V1_BPK_PRS3_HEADER_BYTES 12U
#define NEXUS_V1_BPK_PRS3_VERSION 0x00000001U

/* Observed PRS3 mode flags (byte 19 of the 20-byte prefix). DMWeb's
 * Translation Kit states that every PRS3 bitmap decodes to 256-colour,
 * 8-bit indexed output. These values are not host pixel widths; the
 * mode-to-bpp helper is only for stored/non-PRS3 surface analysis.
 *  10  -> entry[0] directory-trailer tag (unique to the trailer entry)
 */
#define NEXUS_V1_BPK_MODE_8BPP  6U
#define NEXUS_V1_BPK_MODE_16BPP 14U
#define NEXUS_V1_BPK_MODE_24BPP 22U
#define NEXUS_V1_BPK_MODE_32BPP 30U
#define NEXUS_V1_BPK_MODE_TRAILER 10U

typedef struct {
    uint32_t outer_size;
    uint32_t bmpd_size;
    uint32_t entry_count_hint;
    uint32_t candidate_offset_count;
    uint32_t first_candidate_offset;
    uint32_t last_candidate_offset;
    uint32_t prs3_payload_count;
    uint32_t raw_payload_count;
} Nexus_V1_BpkArchiveInfo;

typedef struct {
    uint32_t offset;
    uint32_t next_offset;
    uint32_t stored_size;
    uint32_t payload_offset;
    uint32_t payload_size;
    int has_prs3;
} Nexus_V1_BpkEntry;

typedef struct {
    uint8_t raw[20];
    uint16_t width;
    uint8_t height;
    uint8_t mode;
    int prefix_complete;   /* 1 if the entry spans at least 20 bytes */
} Nexus_V1_BpkEntryPrefix;

typedef struct {
    int has_prs3;
    uint32_t prs3_version; /* BE uint32 at PRS3+4 */
    int prs3_version_matches; /* 1 if PRS3+4..PRS3+8 == 0x00000001 */
    uint32_t prs3_pixel_count; /* BE uint32 at PRS3+8 */
    uint32_t prefix_pixels;    /* width * height from the 20-byte prefix */
    int pixel_count_matches;   /* 1 if prefix_pixels == prs3_pixel_count */
    int payload_available;     /* 1 if entry spans past the 12 PRS3 hdr bytes */
    uint32_t compressed_size;  /* payload bytes available after PRS3 header */
} Nexus_V1_BpkPrs3Info;

typedef struct {
    uint32_t mode_count[256];
    uint32_t total_with_prefix;
    uint32_t trailer_index;   /* index whose prefix mode == MODE_TRAILER */
    int trailer_found;
} Nexus_V1_BpkModeDistribution;

/* The canonical retail MENU.BPK ends in a bounded PALT record: a BE record
 * byte count, a 256-entry count, then 512 opaque bytes. This parser only
 * retains that source framing. It does not assign colour format, CLUT use,
 * PRS3 ownership, or any renderer meaning to the entries. */
typedef struct {
    int valid;
    uint32_t record_offset;
    uint32_t record_bytes;
    uint32_t entry_count;
    uint32_t entry_bytes;
    uint64_t entry_bytes_fnv1a64;
    int raw_entries_are_be16;
    int palette_format_proven;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_BpkPaletteTrailerReceipt;

/* Per-entry surface layout (pass1083). PRS3 entries use DMWeb's 8-bit
 * indexed output regardless of their opaque prefix mode byte. Stored/non-
 * PRS3 entries retain their mode-derived layout until source-bound. The
 * helpers expose bytes-per-pixel, rowstride (no alignment padding), and
 * expected unpacked surface byte count. For the unique directory trailer
 * (mode 10) and any unknown mode, they return 0 and the surface_estimate
 * walker skips the entry.
 */
typedef enum {
    NEXUS_V1_BPK_SURFACE_UNKNOWN = 0,
    /* 8 bpp Saturn palette / indexed mode (1 byte per pixel). */
    NEXUS_V1_BPK_SURFACE_INDEXED_8BPP = 1,
    /* 16 bpp Saturn RGB565 mode (2 bytes per pixel). */
    NEXUS_V1_BPK_SURFACE_RGB565 = 2,
    /* 24 bpp RGB888 mode (3 bytes per pixel). */
    NEXUS_V1_BPK_SURFACE_RGB888 = 3,
    /* 32 bpp RGBA8888 mode (4 bytes per pixel). */
    NEXUS_V1_BPK_SURFACE_RGBA8888 = 4,
    /* Entry[0] directory trailer (mode 10). No decoded surface. */
    NEXUS_V1_BPK_SURFACE_DIRECTORY_TRAILER = 5
} Nexus_V1_BpkSurfaceClass;

typedef struct {
    uint32_t bpp;             /* bytes per pixel (0 for trailer / unknown) */
    uint32_t rowstride;       /* width * bpp (0 for trailer / unknown) */
    uint32_t surface_bytes;   /* width * height * bpp (0 for trailer) */
    Nexus_V1_BpkSurfaceClass surface_class;
} Nexus_V1_BpkSurfaceLayout;

typedef struct {
    uint32_t entry_index;
    uint8_t mode;
    uint16_t width;
    uint8_t height;
    uint32_t pixel_count;     /* width * height from the 20-byte prefix */
    Nexus_V1_BpkSurfaceLayout layout;
} Nexus_V1_BpkSurfaceEntry;

typedef struct {
    uint32_t total_with_surface;  /* entries with a non-zero surface layout */
    uint64_t total_surface_bytes; /* sum of layout.surface_bytes */
    uint32_t trailer_skipped;      /* entries skipped because mode == trailer */
    uint32_t unknown_skipped;      /* entries skipped because mode != any known tag */
    uint32_t capacity;             /* max entries the caller asked us to fill */
    uint32_t used;                 /* number of Nexus_V1_BpkSurfaceEntry rows we wrote */
    int truncated;                 /* 1 if any PRS3 entry was skipped because capacity was hit */
} Nexus_V1_BpkSurfaceEstimate;

typedef enum {
    NEXUS_V1_BPK_RUNTIME_ROUTE_INVALID = 0,
    NEXUS_V1_BPK_RUNTIME_ROUTE_NO_SURFACES = 1,
    NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_PRS3 = 2,
    NEXUS_V1_BPK_RUNTIME_ROUTE_READY_STORED = 3,
    NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_STORED_TRUNCATED = 4
} Nexus_V1_BpkRuntimeRenderRoute;

typedef struct {
    uint32_t archive_entries;
    uint32_t prs3_entries;
    uint32_t raw_entries;
    uint32_t surface_entries;
    uint32_t prs3_surface_entries;
    uint32_t stored_surface_entries;
    uint32_t trailer_entries;
    uint32_t unknown_mode_entries;
    uint64_t expected_surface_bytes;
    uint64_t packed_payload_bytes;
    uint64_t stored_surface_bytes_available;
    uint32_t stored_surface_short_entries;
    int directory_trailer_found;
    int all_prs3_versions_match;
    int all_prs3_pixel_counts_match;
    int all_stored_surface_payloads_fit;
    int requires_prs3_decoder;
    int fallback_visuals_permitted;
    int palette_trailer_observed;
    Nexus_V1_BpkPaletteTrailerReceipt palette_trailer;
    Nexus_V1_BpkRuntimeRenderRoute route;
} Nexus_V1_BpkRuntimeRenderReceipt;

typedef enum {
    NEXUS_V1_BPK_EXTRACT_OK = 0,
    NEXUS_V1_BPK_EXTRACT_ERR_NULL = -1,
    NEXUS_V1_BPK_EXTRACT_ERR_ARCHIVE = -2,
    NEXUS_V1_BPK_EXTRACT_ERR_PRS3 = -3,
    NEXUS_V1_BPK_EXTRACT_ERR_NOT_SURFACE = -4,
    NEXUS_V1_BPK_EXTRACT_ERR_OUTPUT_TOO_SMALL = -5,
    NEXUS_V1_BPK_EXTRACT_ERR_TRUNCATED = -6
} Nexus_V1_BpkSurfaceExtractStatus;

/* Decode a single renderable BPK entry to its declared unpacked surface.
 * Stored entries and the bounded PRS3 candidate decoder are supported for
 * format analysis, including the supplied retail MENU.BPK.  Decode success
 * alone does not authenticate Saturn opcode, palette, or VDP1 semantics;
 * the launcher therefore keeps the live MENU.BPK draw route blocked until
 * those independent provenance requirements are met. */
typedef enum {
    NEXUS_V1_BPK_DECODE_OK = 0,
    NEXUS_V1_BPK_DECODE_ERR_NULL = -1,
    NEXUS_V1_BPK_DECODE_ERR_ARCHIVE = -2,
    NEXUS_V1_BPK_DECODE_ERR_NOT_SURFACE = -3,
    NEXUS_V1_BPK_DECODE_ERR_OUTPUT_TOO_SMALL = -4,
    NEXUS_V1_BPK_DECODE_ERR_TRUNCATED = -5,
    NEXUS_V1_BPK_DECODE_ERR_STREAM = -6
} Nexus_V1_BpkSurfaceDecodeStatus;

typedef enum {
    NEXUS_V1_BPK_SURFACE_HANDOFF_INVALID = 0,
    NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED = 1,
    NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3 = 2,
    NEXUS_V1_BPK_SURFACE_HANDOFF_READY_DECODED = 3,
    NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_TRUNCATED = 4
} Nexus_V1_BpkSurfaceHandoffStatus;

typedef struct {
    uint32_t entry_index;
    Nexus_V1_BpkSurfaceHandoffStatus status;
    uint32_t payload_offset;
    uint32_t payload_size;
    int extractable;
    Nexus_V1_BpkSurfaceEntry surface;
} Nexus_V1_BpkRuntimeSurfaceHandoff;

typedef struct {
    uint32_t archive_entries;
    uint32_t surface_entries;
    uint32_t ready_stored_surfaces;
    uint32_t blocked_prs3_surfaces;
    uint32_t blocked_truncated_surfaces;
    uint32_t trailer_skipped;
    uint32_t unknown_skipped;
    uint64_t expected_surface_bytes;
    uint64_t extractable_surface_bytes;
    uint32_t capacity;
    uint32_t used;
    int requires_prs3_decoder;
    int truncated;
} Nexus_V1_BpkRuntimeSurfaceHandoffSummary;

/* Map a 20-byte prefix mode tag to a surface class. Returns
 * NEXUS_V1_BPK_SURFACE_UNKNOWN for any byte value that is not one of the
 * four observed pixel-mode tags (6/14/22/30) or the directory trailer
 * (10). Pure lookup; does not touch an archive buffer. */
Nexus_V1_BpkSurfaceClass nexus_v1_bpk_mode_to_surface_class(uint8_t mode);

/* Bytes-per-pixel for a 20-byte prefix mode tag. Returns 1/2/3/4 for the
 * four pixel-mode tags, 0 for the directory trailer, 0 for unknown modes.
 * Mirrors the documented (tag + 2) / 8 mapping for the 4 pixel modes. */
uint32_t nexus_v1_bpk_mode_to_bpp(uint8_t mode);

/* Walk every entry whose 20-byte prefix is complete AND whose prefix
 * mode is one of the four PRS3 pixel-mode tags. For each such entry,
 * record (entry_index, mode, width, height, pixel_count, surface layout)
 * into out_entries[0..out->used-1] up to entry_capacity rows. Returns 0
 * on success, negative on bad args / malformed archive. The directory
 * trailer (mode 10) and unknown mode tags are skipped and counted in
 * out_summary->trailer_skipped / out_summary->unknown_skipped so callers
 * can verify the 162 + 1 = 163 entry sum. If entry_capacity is hit, the
 * remaining PRS3 entries are still counted toward total_with_surface and
 * total_surface_bytes but NOT written to out_entries; out_summary->truncated
 * is set to 1 to flag the overflow. */
int nexus_v1_bpk_archive_surface_estimate(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkSurfaceEntry *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkSurfaceEstimate *out_summary);

int nexus_v1_bpk_archive_runtime_render_receipt(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeRenderReceipt *out_receipt);

const char *nexus_v1_bpk_runtime_render_route_name(
    Nexus_V1_BpkRuntimeRenderRoute route);

int nexus_v1_bpk_archive_extract_stored_surface(
    const uint8_t *data,
    size_t data_size,
    uint32_t index,
    uint8_t *out,
    size_t out_size,
    Nexus_V1_BpkSurfaceEntry *out_surface,
    size_t *out_written);

int nexus_v1_bpk_archive_decode_surface(
    const uint8_t *data,
    size_t data_size,
    uint32_t index,
    uint8_t *out,
    size_t out_size,
    Nexus_V1_BpkSurfaceEntry *out_surface,
    size_t *out_written);

const char *nexus_v1_bpk_surface_decode_status_name(int status);

const char *nexus_v1_bpk_surface_extract_status_name(int status);

int nexus_v1_bpk_archive_runtime_surface_handoff(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeSurfaceHandoff *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary *out_summary);

const char *nexus_v1_bpk_surface_handoff_status_name(
    Nexus_V1_BpkSurfaceHandoffStatus status);

/*
 * Parse the DM Nexus MENU.BPK BPPK/BMPD directory shape.
 *
 * Scope: this is an archive/directory validator only. It records candidate
 * payload spans and detects PRS3-tagged payloads, but it deliberately does
 * does not authorize Saturn presentation. ReDMCSB has no Saturn/Nexus
 * implementation; this is source-locked to the observed Nexus MENU.BPK file
 * and the documented presentation gap in
 * docs/nexus_v1_phase2_data_formats_H2321.md.
 */
int nexus_v1_bpk_archive_parse(const uint8_t *data,
                               size_t data_size,
                               Nexus_V1_BpkArchiveInfo *out_info);

/* Inspect only the exact end-of-file PALT record described above. Returns
 * zero for a structurally complete record, otherwise -1. */
int nexus_v1_bpk_archive_inspect_palette_trailer(
    const uint8_t *data, size_t data_size,
    Nexus_V1_BpkPaletteTrailerReceipt *out_receipt);

/* Copies the authenticated PALT payload as raw big-endian 16-bit words.
 * This is a source-byte operation only: it deliberately does not claim
 * BGR555/RGB555 ordering, CLUT ownership, palette bank, or VDP1 use. */
int nexus_v1_bpk_archive_copy_palette_words_be16(
    const uint8_t *data, size_t data_size,
    uint16_t out_words[NEXUS_V1_BPK_PALT_ENTRY_COUNT],
    uint64_t *out_words_fnv1a64);

int nexus_v1_bpk_archive_get_entry(const uint8_t *data,
                                   size_t data_size,
                                   uint32_t index,
                                   Nexus_V1_BpkEntry *out_entry);

/*
 * Read the 20-byte entry prefix (zero-filled if the entry is too small)
 * and decode width / height / mode tags. Bounds-checked against the
 * actual entry span, never against the full archive.
 */
int nexus_v1_bpk_archive_get_entry_prefix(const uint8_t *data,
                                          size_t data_size,
                                          uint32_t index,
                                          Nexus_V1_BpkEntryPrefix *out_prefix);

/*
 * Inspect the PRS3 sub-header of an entry (PRS3 magic + version + pixel
 * count). Does NOT attempt to decompress the payload. Returns 0 on a
 * clean read, negative on a malformed entry / bounds violation / missing
 * PRS3 marker. prs3_pixel_count is the BE uint32 right after the
 * constant version word; cross-validated against width * height.
 */
int nexus_v1_bpk_archive_inspect_prs3(const uint8_t *data,
                                      size_t data_size,
                                      uint32_t index,
                                      Nexus_V1_BpkPrs3Info *out_info);

/*
 * Walk every entry whose offset + 20 bytes fits inside the archive and
 * bucket them by their prefix mode tag (byte 19). Also detects the
 * unique entry whose mode == MODE_TRAILER (typically index 0).
 */
int nexus_v1_bpk_archive_mode_distribution(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkModeDistribution *out_dist);

/* PRS3 compression-algorithm evidence probe (pass1084).
 *
 * Bounded, source-locked evidence walker for the DM Nexus MENU.BPK PRS3
 * payload stream. The compression algorithm is still intentionally
 * unsupported; this function surfaces per-entry structural receipts that
 * a future decoder would have to account for, without claiming to
 * decode anything:
 *
 *   - The first 4 bytes of every PRS3 payload in the observed
 *     MENU.BPK are a big-endian uint32 that APPROXIMATELY tracks the
 *     payload byte count (header_first_u32 - payload_size is small and
 *     non-negative across every observed entry). The constant overhead
 *     is the strongest single receipt that there is a leading header
 *     word, even though the precise meaning is still unknown.
 *   - The first 8 bytes of every payload are surfaced verbatim so probe
 *     code can spot identical-prefix patterns between adjacent entries
 *     (e.g. multiple 16x15 14bpp frames share the same first 8 bytes).
 *   - A bounded byte-frequency receipt is computed over the first
 *     sample_size payload bytes (capped at 4 KiB) so the walker never
 *     spends unbounded time scanning large payloads.
 *   - Per-mode and aggregate compression-ratio distributions are
 *     reported so callers can compare how amenable each pixel-mode
 *     (8/16/24/32 bpp) is to the unknown stream format.
 *   - A bounded 4-quadrant byte-class receipt (pass1084b) buckets the
 *     same sampled bytes into [0x00..0x3F], [0x40..0x7F],
 *     [0x80..0xBF], [0xC0..0xFF] buckets. This is the cheapest
 *     structural fingerprint that distinguishes "random / uniformly
 *     distributed" bytes from "stream with length-class hints and
 *     repeated literal zeros". The observed real MENU.BPK distribution
 *     shows quadrant 0 (0x00..0x3F) as the dominant bucket, which is
 *     consistent with the small header_minus_payload values (4..7)
 *     and possible repeated-literal runs the stream uses to mark
 *     short back-references.
 *
 * This walker does NOT call any third-party decompression library, does
 * NOT attempt to decode literal/back-reference opcodes, and does NOT
 * advance any read cursor inside an opaque payload. Pass1084 is the
 * "first evidence ledger" for the unknown PRS3 algorithm; subsequent
 * passes are expected to deepen the analysis, never to backtrack on
 * these receipts.
 */

#define NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_FIRST_BYTES 8U
#define NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_SAMPLE_BYTES 4096U

typedef struct {
    uint32_t entry_index;
    uint8_t  mode;            /* prefix byte 19 */
    uint16_t width;           /* prefix bytes 12..14 (BE) */
    uint8_t  height;          /* prefix byte 15 */
    uint32_t pixel_count;     /* width * height */
    uint32_t bpp;             /* bytes per pixel for the recognised mode */
    uint32_t uncompressed_size; /* width * height * bpp */
    uint32_t payload_size;    /* bytes between the PRS3 sub-header end
                                 and the next entry's start */
    int payload_available;    /* 1 if payload_size > 0 */
    uint32_t header_first_u32; /* first 4 bytes of the payload read as
                                  BE uint32; 0 if the payload is < 4 bytes */
    int header_first_readable; /* 1 iff payload_size >= 4 */
    uint32_t header_minus_payload; /* sat(header_first_u32 - payload_size).
                                      Saturated to UINT32_MAX if the
                                      difference underflows. This is the
                                      strongest receipt we currently
                                      have that the leading word
                                      approximates the payload length. */
    double   compression_ratio; /* (double)payload_size / uncompressed_size.
                                   0.0 if uncompressed_size == 0. */
    /* First 8 bytes of the payload (algorithm-agnostic structural
     * receipt). Zero-padded if payload_size < 8. */
    uint8_t  first_payload[NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_FIRST_BYTES];
    /* Bounded byte-frequency receipt over the first sample_size bytes.
     * sample_size_used <= sample_size and <= payload_size. */
    uint32_t sample_size_used;
    uint8_t  most_common_byte; /* 0 if sample_size_used == 0 */
    uint32_t most_common_byte_count; /* 0 if sample_size_used == 0 */
    uint32_t distinct_byte_values;  /* 0..256, 0 if sample_size_used == 0 */
    /* Bounded 4-quadrant byte-class receipt over the same sample.
     * byte_class_count[0] = bytes in [0x00..0x3F],
     * byte_class_count[1] = bytes in [0x40..0x7F],
     * byte_class_count[2] = bytes in [0x80..0xBF],
     * byte_class_count[3] = bytes in [0xC0..0xFF].
     * Sum of byte_class_count[*] == sample_size_used. */
    uint32_t byte_class_count[4];
    int runtime_decode_status;
    int runtime_decode_blocked;
    int evidence_only;
    int renderer_handoff_blocked;
    uint32_t decoded_pixels_emitted;
    int fallback_visuals_permitted;
} Nexus_V1_BpkPrs3PayloadEvidence;

typedef struct {
    uint32_t entries_seen;        /* total entries walked */
    uint32_t mode_count[256];     /* entries whose prefix mode matched,
                                     bucketed by mode byte */
    uint32_t trailer_skipped;     /* entries skipped because mode == MODE_TRAILER */
    uint32_t unknown_skipped;     /* entries skipped because mode is none
                                     of the four PRS3 pixel-mode tags and
                                     not the trailer */
    uint32_t trailer_partial;     /* entries whose payload did not span
                                     past the PRS3 sub-header */
    uint32_t capacity;            /* max entries the caller asked us to fill */
    uint32_t used;                /* number of Nexus_V1_BpkPrs3PayloadEvidence
                                     rows we wrote */
    uint32_t smallest_payload;    /* minimum payload_size seen across used rows */
    uint32_t largest_payload;     /* maximum payload_size seen across used rows */
    uint64_t total_uncompressed;  /* sum of uncompressed_size across used rows */
    uint64_t total_payload;       /* sum of payload_size across used rows */
    double   mean_compression_ratio; /* mean of compression_ratio across used rows */
    double   min_compression_ratio;  /* min of compression_ratio across used rows */
    double   max_compression_ratio;  /* max of compression_ratio across used rows */
    int truncated;                /* 1 if any PRS3 entry was skipped
                                     because capacity was hit */
    int decoder_promoted;         /* Always zero: evidence never authorizes runtime. */
    int renderer_handoff_blocked; /* True when any PRS3 evidence row exists. */
    uint32_t decoded_pixels_emitted;
} Nexus_V1_BpkPrs3PayloadEvidenceSummary;

typedef enum {
    NEXUS_V1_BPK_PRS3_STREAM_OK = 0,
    NEXUS_V1_BPK_PRS3_STREAM_ERR_NULL = -1,
    NEXUS_V1_BPK_PRS3_STREAM_ERR_ARCHIVE = -2,
    NEXUS_V1_BPK_PRS3_STREAM_ERR_NOT_PRS3 = -3,
    NEXUS_V1_BPK_PRS3_STREAM_ERR_UNSUPPORTED_MODE = -4,
    NEXUS_V1_BPK_PRS3_STREAM_ERR_TRUNCATED = -5
} Nexus_V1_BpkPrs3StreamStatus;

typedef struct {
    uint32_t entry_index;
    uint8_t mode;
    uint16_t width;
    uint8_t height;
    uint32_t bpp;
    uint32_t pixel_count;
    uint32_t expected_output_bytes;
    uint32_t stream_offset;       /* after PRS3 magic/version/pixel-count */
    uint32_t stream_size;
    uint32_t body_offset;         /* after leading stream-size word */
    uint32_t body_size;
    uint32_t header_first_u32;
    uint64_t header_span_fnv1a64;
    uint64_t body_span_fnv1a64;
    uint32_t header_minus_payload;
    int header_first_readable;
    int header_underflow;
    int bounded_header_candidate; /* header_minus_payload <= 16 */
    int decode_blocked;           /* true until opcode decoder exists */
    int evidence_only;
    int renderer_handoff_blocked;
    int upload_blocked;
    int decoder_promoted;
    uint32_t decoded_pixels_emitted;
    int fallback_visuals_permitted;
} Nexus_V1_BpkPrs3StreamPlan;

/* Source-bound PRS3 compression framing only. `mode_flags` is the observed
 * prefix byte; it is not a codec claim. The compressed span starts after the
 * leading stream word and remains opaque until original decoder evidence
 * exists. */
typedef struct {
    int valid;
    uint32_t entry_index;
    uint8_t mode_flags;
    uint32_t declared_pixel_count;
    uint32_t declared_output_bytes;
    uint32_t compressed_offset;
    uint32_t compressed_length;
    uint64_t compressed_fnv1a64;
    int decoder_promoted;
    int pixels_exposed;
    int fallback_visuals_permitted;
} Nexus_V1_BpkPrs3CompressionDescriptorReceipt;

/* Bounded diagnostic evaluation of LSB-first and MSB-first traversals of one
 * literal/back-reference candidate against the declared byte target. This is
 * evidence tooling, not a PRS3 decoder: it never exposes pixels, never feeds
 * a renderer, and a complete candidate result does not change any runtime
 * route. The output cap prevents an untrusted prefix from allocating an
 * arbitrary surface. Provenance: the BPPK/PRS3 framing is the observed
 * MENU.BPK boundary in docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md
 * lines 291-306 and docs/VERIFIED_HASHES.md; neither source documents this
 * candidate opcode grammar. */
#define NEXUS_V1_BPK_PRS3_CANDIDATE_MAX_OUTPUT_BYTES (1024U * 1024U)

typedef enum {
    NEXUS_V1_BPK_PRS3_CANDIDATE_UNSUPPORTED = 0,
    NEXUS_V1_BPK_PRS3_CANDIDATE_OUTPUT_LIMIT = 1,
    NEXUS_V1_BPK_PRS3_CANDIDATE_STREAM_FAILURE = 2,
    NEXUS_V1_BPK_PRS3_CANDIDATE_COMPLETE_TRAILING = 3,
    NEXUS_V1_BPK_PRS3_CANDIDATE_COMPLETE_EXACT = 4
} Nexus_V1_BpkPrs3CandidateStatus;

/* PRS3's observed outer fields are big-endian, but no Saturn decoder has
 * established the order in which control bits are consumed. Keep trial
 * bit-order evaluations explicit and diagnostic-only. */
typedef enum {
    NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST = 0,
    NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_MSB_FIRST = 1
} Nexus_V1_BpkPrs3CandidateBitOrder;

typedef struct {
    uint32_t entry_index;
    uint8_t mode;
    uint32_t expected_output_bytes;
    uint32_t body_size;
    uint32_t body_bytes_consumed;
    int runtime_decode_status;
    int runtime_decode_blocked;
    int evidence_only;
    int renderer_handoff_blocked;
    uint32_t decoded_pixels_emitted;
    int fallback_visuals_permitted;
    Nexus_V1_BpkPrs3CandidateStatus status;
} Nexus_V1_BpkPrs3CandidateEvidence;

typedef struct {
    uint32_t archive_entries;
    uint32_t prs3_surfaces;
    uint32_t evaluated;
    uint32_t unsupported;
    uint32_t output_limited;
    uint32_t stream_failures;
    uint32_t complete_trailing;
    uint32_t complete_exact;
    uint32_t capacity;
    uint32_t used;
    int truncated;
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order;
    int decoder_promoted; /* Always zero: evidence never authorizes runtime. */
    int renderer_handoff_blocked;
    uint32_t decoded_pixels_emitted;
} Nexus_V1_BpkPrs3CandidateEvidenceSummary;

/* PRS3 post-header framing evidence. The first four bytes after the
 * observed PRS3 magic/version/pixel-count are not promoted to a codec
 * field. This receipt only compares their BE/LE interpretations to the
 * directory-bounded span ending at the next BPK entry. It exists because
 * the verified MENU.BPK corpus has one final PRS3 entry followed by data
 * that is not covered by its short BE declaration. */
typedef struct {
    uint32_t entry_index;
    uint32_t directory_stream_size; /* bytes after the 12-byte PRS3 header */
    uint32_t first_word_be;
    uint32_t first_word_le;
    int be_at_least_stream;
    int be_near_stream; /* BE word - directory span is in [4, 7]. */
    uint32_t be_trailing_bytes; /* directory span - BE word, if positive */
} Nexus_V1_BpkPrs3FramingEvidence;

typedef struct {
    uint32_t prs3_entries;
    uint32_t readable_first_words;
    uint32_t be_at_least_stream;
    uint32_t be_near_stream;
    uint32_t le_at_least_stream;
    uint32_t le_near_stream;
    uint32_t be_shorter_than_stream;
    uint32_t be_tail_bytes_total;
    uint32_t first_be_short_entry;
    uint32_t capacity;
    uint32_t used;
    int truncated;
    int decoder_promoted; /* Always zero: framing does not establish opcodes. */
} Nexus_V1_BpkPrs3FramingEvidenceSummary;

/* Exact bounded evaluation after the observed BE framing word.  A frame is
 * eligible only when its BE word covers the complete directory-bounded stream
 * plus its four-byte framing word, allowing at most three bytes of directory
 * padding.
 * The command grammar is still a trial (control bit: 1 = literal, 0 = the
 * existing two-byte back-reference form), so these receipts never promote a
 * decoder or expose decoded pixels to runtime code. */
typedef enum {
    NEXUS_V1_BPK_PRS3_FRAMED_EVAL_UNVALIDATED_FRAME = 0,
    NEXUS_V1_BPK_PRS3_FRAMED_EVAL_OUTPUT_LIMIT = 1,
    NEXUS_V1_BPK_PRS3_FRAMED_EVAL_COMMAND_FAILURE = 2,
    NEXUS_V1_BPK_PRS3_FRAMED_EVAL_COMPLETE_TRAILING = 3,
    NEXUS_V1_BPK_PRS3_FRAMED_EVAL_COMPLETE_EXACT = 4
} Nexus_V1_BpkPrs3FramedEvalStatus;

typedef struct {
    uint32_t entry_index;
    uint8_t mode;
    uint32_t expected_output_bytes;
    uint32_t directory_body_size;
    uint32_t frame_body_size;
    uint32_t body_bytes_consumed;
    uint32_t literal_commands;
    uint32_t backref_commands;
    int runtime_decode_status;
    int runtime_decode_blocked;
    int evidence_only;
    int renderer_handoff_blocked;
    uint32_t decoded_pixels_emitted;
    int fallback_visuals_permitted;
    Nexus_V1_BpkPrs3FramedEvalStatus status;
} Nexus_V1_BpkPrs3FramedEvalEvidence;

typedef struct {
    uint32_t archive_entries;
    uint32_t prs3_surfaces;
    uint32_t frame_validated;
    uint32_t evaluated;
    uint32_t unvalidated_frames;
    uint32_t output_limited;
    uint32_t command_failures;
    uint32_t complete_trailing;
    uint32_t complete_exact;
    uint32_t capacity;
    uint32_t used;
    int truncated;
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order;
    int decoder_promoted; /* Always zero: exact trial results are evidence. */
    int renderer_handoff_blocked;
    uint32_t decoded_pixels_emitted;
} Nexus_V1_BpkPrs3FramedEvalSummary;

/* Bounded opcode-prefix witness for the same trial command grammar used by
 * the diagnostic PRS3 evaluators. It records only which stream bytes an
 * opcode reader would consume under an explicit control-bit order. It never
 * allocates an output surface, never validates pixels, and cannot change a
 * runtime route. */
typedef enum {
    NEXUS_V1_BPK_PRS3_OPCODE_PREFIX_UNVALIDATED_FRAME = 0,
    NEXUS_V1_BPK_PRS3_OPCODE_PREFIX_COMMAND_LIMIT = 1,
    NEXUS_V1_BPK_PRS3_OPCODE_PREFIX_STREAM_END = 2,
    NEXUS_V1_BPK_PRS3_OPCODE_PREFIX_TRUNCATED_OPERAND = 3
} Nexus_V1_BpkPrs3OpcodePrefixStatus;

typedef struct {
    uint32_t entry_index;
    uint8_t mode;
    uint32_t expected_output_bytes;
    uint32_t body_offset;
    uint32_t body_size;
    uint32_t first_control_offset;
    uint8_t first_control_byte;
    uint32_t requested_command_limit;
    uint32_t commands_observed;
    uint32_t literal_commands;
    uint32_t backref_commands;
    uint32_t control_bytes_consumed;
    uint32_t operand_bytes_consumed;
    uint32_t body_bytes_consumed;
    uint32_t first_backref_raw_offset;
    uint32_t first_backref_length;
    int first_backref_observed;
    Nexus_V1_BpkPrs3OpcodePrefixStatus status;
} Nexus_V1_BpkPrs3OpcodePrefixEvidence;

typedef struct {
    uint32_t archive_entries;
    uint32_t prs3_surfaces;
    uint32_t frame_validated;
    uint32_t witnessed;
    uint32_t unvalidated_frames;
    uint32_t command_limit_reached;
    uint32_t stream_end;
    uint32_t truncated_operands;
    uint32_t total_commands_observed;
    uint32_t total_literal_commands;
    uint32_t total_backref_commands;
    uint32_t capacity;
    uint32_t used;
    int truncated;
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order;
    int decoder_promoted; /* Always zero: opcode witness is not decode. */
} Nexus_V1_BpkPrs3OpcodePrefixSummary;

typedef enum {
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_INVALID = 0,
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_STREAM_BLOCKED = 1,
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_OUTPUT_MISSING = 2,
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_SIZE_MISMATCH = 3,
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_HASH_MISMATCH = 4,
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_PROVENANCE_REQUIRED = 5,
    NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_SOURCE_BOUND_NO_RUNTIME = 6
} Nexus_V1_BpkPrs3DecodedOutputProofStatus;

typedef struct {
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint32_t expected_output_bytes;
    uint32_t observed_output_bytes;
    uint64_t expected_output_fnv1a64;
    uint64_t observed_output_fnv1a64;
    int length_matches;
    int hash_matches;
    int capture_source_bound;
    int decoded_output_sidecar_bound;
    int original_saturn_provenance_verified;
    int decoded_output_proof_ready;
    int opcode_grammar_proven;
    int decoder_promoted;
    int runtime_upload_permitted;
    int fallback_visuals_permitted;
    Nexus_V1_BpkPrs3DecodedOutputProofStatus status;
} Nexus_V1_BpkPrs3DecodedOutputProofReceipt;

typedef enum {
    NEXUS_V1_BPK_DECODE_ROUTE_INVALID = 0,
    NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED = 1,
    NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 = 2,
    NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED = 3,
    NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES = 4,
    NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED = 5
} Nexus_V1_BpkRuntimeDecodeRoute;

typedef struct {
    uint32_t archive_entries;
    uint32_t surface_entries;
    uint32_t ready_stored_surfaces;
    uint32_t blocked_prs3_surfaces;
    uint32_t blocked_truncated_surfaces;
    uint32_t prs3_stream_plans;
    uint32_t prs3_stream_plan_failures;
    uint32_t prs3_bounded_header_candidates;
    uint32_t prs3_header_underflows;
    uint32_t prs3_decode_attempts;
    uint32_t prs3_decode_successes;
    uint32_t prs3_decode_failures;
    uint64_t prs3_decoded_surface_bytes;
    uint64_t prs3_decoded_pixels_fnv1a64;
    uint32_t first_blocked_entry;
    uint32_t first_blocked_stream_offset;
    uint32_t first_blocked_stream_size;
    uint32_t first_blocked_expected_output_bytes;
    uint32_t first_blocked_header_first_u32;
    uint32_t first_blocked_header_minus_payload;
    int first_blocked_decode_status;
    int requires_prs3_decoder;
    int prs3_evidence_only;
    int prs3_decoder_promoted;
    uint32_t prs3_decoded_pixels_emitted;
    int renderer_handoff_blocked;
    int fallback_visuals_permitted;
    int decode_blocked;
    Nexus_V1_BpkRuntimeDecodeRoute route;
} Nexus_V1_BpkRuntimeDecodeReceipt;

#define NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS 8U

typedef enum {
    NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID = 0,
    NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED = 1,
    NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 = 2,
    NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED = 3,
    NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES = 4,
    NEXUS_V1_BPK_UPLOAD_ROUTE_READY_DECODED = 5,
    NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_CAPACITY = 6
} Nexus_V1_BpkRuntimeUploadRoute;

typedef struct {
    uint32_t entry_index;
    Nexus_V1_BpkSurfaceHandoffStatus status;
    Nexus_V1_BpkSurfaceClass surface_class;
    uint8_t mode;
    uint16_t width;
    uint8_t height;
    uint32_t bpp;
    uint32_t expected_output_bytes;
    uint32_t payload_offset;
    uint32_t payload_size;
    uint64_t payload_fnv1a64;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint32_t body_offset;
    uint32_t body_size;
    uint32_t header_first_u32;
    uint32_t header_minus_payload;
    uint32_t prs3_version;
    uint32_t prs3_pixel_count;
    int prs3_header_valid;
    Nexus_V1_BpkPrs3CompressionDescriptorReceipt compression;
    int decode_blocked;
    int evidence_only;
    int renderer_handoff_blocked;
    int upload_blocked;
    uint32_t decoded_pixels_emitted;
    int fallback_visuals_permitted;
    int upload_ready;
} Nexus_V1_BpkRuntimeUploadRow;

typedef struct {
    Nexus_V1_BpkRuntimeUploadRoute route;
    uint32_t archive_entries;
    uint32_t surface_entries;
    int directory_trailer_found;
    uint32_t directory_trailer_entries;
    int directory_trailer_at_entry_zero;
    int directory_trailer_valid;
    uint32_t ready_uploads;
    uint32_t blocked_prs3_uploads;
    uint32_t first_prs3_entry_index;
    uint32_t first_prs3_payload_offset;
    uint32_t first_prs3_payload_size;
    uint64_t first_prs3_payload_fnv1a64;
    uint32_t first_prs3_version;
    uint32_t first_prs3_pixel_count;
    uint32_t first_prs3_header_first_u32;
    uint32_t first_prs3_header_minus_payload;
    Nexus_V1_BpkPrs3CompressionDescriptorReceipt first_prs3_compression;
    uint32_t unknown_prs3_mode_entries;
    uint32_t prs3_decode_successes;
    uint32_t prs3_decode_failures;
    uint64_t prs3_decoded_surface_bytes;
    uint64_t prs3_decoded_pixels_fnv1a64;
    uint32_t blocked_truncated_uploads;
    uint32_t planned_rows;
    uint32_t capacity;
    int truncated;
    uint64_t expected_upload_bytes;
    uint64_t extractable_upload_bytes;
    int prs3_evidence_only;
    int prs3_decoder_promoted;
    uint32_t prs3_decoded_pixels_emitted;
    int prs3_upload_blocked;
    int blocks_real_menu_surface_render;
    int fallback_visuals_permitted;
    int palette_trailer_observed;
    Nexus_V1_BpkPaletteTrailerReceipt palette_trailer;
} Nexus_V1_BpkRuntimeUploadReceipt;

/* Walk every entry whose 20-byte prefix is complete AND whose prefix
 * mode is one of the four PRS3 pixel-mode tags (6/14/22/30). For each
 * such entry, record bounded compression evidence into
 * out_entries[0..out->used-1] up to entry_capacity rows.
 *
 * sample_size caps the number of payload bytes the walker inspects to
 * build the byte-frequency receipt per entry. The actual amount sampled
 * is min(sample_size, payload_size, NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_SAMPLE_BYTES),
 * so callers always get a fully-bounded walk regardless of input size.
 *
 * The walker never claims to decode PRS3; out entries are structural
 * receipts only. Returns 0 on success, negative on bad args / malformed
 * archive. */
int nexus_v1_bpk_archive_prs3_payload_evidence(
    const uint8_t *data,
    size_t data_size,
    uint32_t sample_size,
    Nexus_V1_BpkPrs3PayloadEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3PayloadEvidenceSummary *out_summary);

int nexus_v1_bpk_archive_prs3_stream_plan(
    const uint8_t *data,
    size_t data_size,
    uint32_t index,
    Nexus_V1_BpkPrs3StreamPlan *out_plan);

/* Builds one strict, source-bounded descriptor for a recognized PRS3 entry.
 * Unknown mode bytes, malformed headers, empty/overflowing spans, and any
 * declaration mismatch fail closed. This does not decode compressed bytes. */
int nexus_v1_bpk_archive_prs3_compression_descriptor(
    const uint8_t *data, size_t data_size, uint32_t index,
    Nexus_V1_BpkPrs3CompressionDescriptorReceipt *out_receipt);

const char *nexus_v1_bpk_prs3_stream_status_name(int status);

int nexus_v1_bpk_archive_prs3_candidate_evidence(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkPrs3CandidateEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3CandidateEvidenceSummary *out_summary);

/* Evaluate the bounded literal/back-reference trial grammar with an explicit
 * control-bit order. This is not a decoder and never changes runtime routes.
 * The legacy candidate-evidence entry point remains the retired LSB-first
 * comparison for stable callers. */
int nexus_v1_bpk_archive_prs3_candidate_evidence_with_bit_order(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order,
    Nexus_V1_BpkPrs3CandidateEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3CandidateEvidenceSummary *out_summary);

/* Compare the first post-header word with each entry's directory-bounded
 * PRS3 span. This is diagnostic-only and does not alter decode or upload
 * routing. */
int nexus_v1_bpk_archive_prs3_framing_evidence(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkPrs3FramingEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3FramingEvidenceSummary *out_summary);

/* Evaluate one explicitly selected control-bit order from immediately after
 * the BE framing word. Exact completion requires both the declared surface
 * byte target and complete consumption of the validated frame body. */
int nexus_v1_bpk_archive_prs3_framed_decode_evidence(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order,
    Nexus_V1_BpkPrs3FramedEvalEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3FramedEvalSummary *out_summary);

/* Walk at most command_limit trial opcodes from each validated PRS3 frame and
 * report the consumed control/operand bytes. This is diagnostic-only opcode
 * proof; it never materializes output bytes. */
int nexus_v1_bpk_archive_prs3_opcode_prefix_witness(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order,
    uint32_t command_limit,
    Nexus_V1_BpkPrs3OpcodePrefixEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3OpcodePrefixSummary *out_summary);

/* Bind a caller-supplied PRS3 output sidecar to one MENU.BPK stream plan by
 * exact byte count and FNV. Even a source-bound sidecar cannot authorize
 * runtime upload here: original Saturn provenance and opcode grammar review
 * remain explicit blockers. */
int nexus_v1_bpk_archive_prs3_decoded_output_proof_gate(
    const uint8_t *data,
    size_t data_size,
    uint32_t index,
    const uint8_t *decoded_output,
    size_t decoded_output_size,
    uint64_t expected_output_fnv1a64,
    int capture_source_bound,
    int original_saturn_provenance_verified,
    Nexus_V1_BpkPrs3DecodedOutputProofReceipt *out_receipt);

const char *nexus_v1_bpk_prs3_candidate_bit_order_name(
    Nexus_V1_BpkPrs3CandidateBitOrder bit_order);

const char *nexus_v1_bpk_prs3_candidate_status_name(
    Nexus_V1_BpkPrs3CandidateStatus status);

const char *nexus_v1_bpk_prs3_framed_eval_status_name(
    Nexus_V1_BpkPrs3FramedEvalStatus status);

const char *nexus_v1_bpk_prs3_opcode_prefix_status_name(
    Nexus_V1_BpkPrs3OpcodePrefixStatus status);

const char *nexus_v1_bpk_prs3_decoded_output_proof_status_name(
    Nexus_V1_BpkPrs3DecodedOutputProofStatus status);

int nexus_v1_bpk_archive_runtime_decode_receipt(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeDecodeReceipt *out_receipt);

const char *nexus_v1_bpk_runtime_decode_route_name(
    Nexus_V1_BpkRuntimeDecodeRoute route);

int nexus_v1_bpk_archive_runtime_upload_plan(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeUploadRow *out_rows,
    uint32_t row_capacity,
    Nexus_V1_BpkRuntimeUploadReceipt *out_receipt);

const char *nexus_v1_bpk_runtime_upload_route_name(
    Nexus_V1_BpkRuntimeUploadRoute route);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_BPK_ARCHIVE_H */
