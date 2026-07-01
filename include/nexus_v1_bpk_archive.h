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
#define NEXUS_V1_BPK_ENTRY_PREFIX_BYTES 20U

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
 *                 bpp/mode tag: 6 / 14 / 22 / 30 are the four observed
 *                 pixel-mode tags; the entry[0] directory trailer carries
 *                 the unique tag 10).
 *
 * The 21st..24th bytes are the `PRS3` magic, followed by a constant
 * 0x00000001 (BE) and a BE uint32 pixel count that always equals
 * width * height in the observed MENU.BPK. The actual compression
 * algorithm of PRS3 is still unknown and intentionally unsupported.
 */
#define NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET 12U
#define NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET 15U
#define NEXUS_V1_BPK_PREFIX_MODE_OFFSET 19U
#define NEXUS_V1_BPK_PRS3_HEADER_BYTES 12U
#define NEXUS_V1_BPK_PRS3_VERSION 0x00000001U

/* Observed PRS3 pixel-mode tags (byte 19 of the 20-byte prefix).
 * The numeric ordering matches: tag = (bytes_per_pixel * 8) - 2.
 *   6  -> 1 byte/pixel  (8 bpp Saturn palette/indexed mode)
 *  14  -> 2 bytes/pixel (16 bpp RGB565 Saturn mode)
 *  22  -> 3 bytes/pixel (24 bpp RGB888 mode)
 *  30  -> 4 bytes/pixel (32 bpp RGBA mode)
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

/* Per-entry surface layout (pass1083). For every entry whose 20-byte
 * prefix is complete AND whose prefix mode is one of the four PRS3
 * pixel-mode tags (6/14/22/30), the surface-class helpers below compute
 * the bytes-per-pixel, rowstride (no alignment padding), and expected
 * unpacked surface byte count. For the unique directory trailer (mode
 * 10) and any unknown mode, the helpers return 0 and the surface_estimate
 * walker skips the entry. This is the strongest no-decode surface-class
 * contract the engine can offer until a real PRS3 implementation lands.
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

/*
 * Parse the DM Nexus MENU.BPK BPPK/BMPD directory shape.
 *
 * Scope: this is an archive/directory validator only. It records candidate
 * payload spans and detects PRS3-tagged payloads, but it deliberately does
 * not decompress PRS3 yet. ReDMCSB has no Saturn/Nexus implementation; this
 * is source-locked to the observed Nexus MENU.BPK file and the documented
 * "MENU.BPK packed graphics not analyzed" gap in
 * docs/nexus_v1_phase2_data_formats_H2321.md.
 */
int nexus_v1_bpk_archive_parse(const uint8_t *data,
                               size_t data_size,
                               Nexus_V1_BpkArchiveInfo *out_info);

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
} Nexus_V1_BpkPrs3PayloadEvidenceSummary;

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

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_BPK_ARCHIVE_H */
