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

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_BPK_ARCHIVE_H */
