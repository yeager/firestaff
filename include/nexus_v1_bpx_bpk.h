#ifndef NEXUS_V1_BPX_BPK_H
#define NEXUS_V1_BPX_BPK_H

#include <stddef.h>
#include <stdint.h>

/* Nexus BPX/BPK status, source-cited and intentionally bounded.
 *
 * Source evidence:
 * - docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md:291-306
 *   identifies *.BPK / MENU.BPK as packed, game-specific, and notes that no
 *   formal compression analysis has been performed.
 * - docs/VERIFIED_HASHES.md:103 records MENU.BPK size/hash.
 *
 * This module does not claim to decode the proprietary Saturn BPK stream.
 * It provides a real-file marker plus a synthetic BPX0 archive contract used
 * to lock parser bounds and stored-entry extraction until MENU.BPK bytes are
 * available for reverse engineering.
 */

#define NEXUS_V1_MENU_BPK_SIZE 89060u
#define NEXUS_V1_MENU_BPK_SHA256 \
    "740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636"

#define NEXUS_V1_BPX_BPK_MAX_ENTRIES 64
#define NEXUS_V1_BPX0_HEADER_SIZE 16u
#define NEXUS_V1_BPX0_ENTRY_SIZE 32u

typedef enum {
    NEXUS_V1_BPX_BPK_FORMAT_UNKNOWN = 0,
    NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_BPX0 = 1,
    NEXUS_V1_BPX_BPK_FORMAT_VERIFIED_MENU_BPK_MARKER = 2,
    /* Synthetic PRS3-tagged BPK stream contract (pass1082). Locks the
     * 20-byte MENU.BPK-shaped prefix (width/height/mode tag at bytes
     * 12..14 / 15 / 19) plus the PRS3 magic + 0x00000001 version + pixel
     * count sub-header without claiming PRS3 decompression. */
    NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_PRS3 = 3
} Nexus_V1_BpxBpkFormat;

typedef enum {
    NEXUS_V1_BPX_BPK_OK = 0,
    NEXUS_V1_BPX_BPK_ERR_NULL = -1,
    NEXUS_V1_BPX_BPK_ERR_TOO_SMALL = -2,
    NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC = -3,
    NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED = -4,
    NEXUS_V1_BPX_BPK_ERR_COUNT = -5,
    NEXUS_V1_BPX_BPK_ERR_BOUNDS = -6,
    NEXUS_V1_BPX_BPK_ERR_NOT_FOUND = -7,
    NEXUS_V1_BPX_BPK_ERR_OUTPUT_TOO_SMALL = -8,
    NEXUS_V1_BPX_BPK_ERR_METHOD = -9
} Nexus_V1_BpxBpkStatus;

typedef enum {
    NEXUS_V1_BPX_BPK_METHOD_STORED = 0,
    NEXUS_V1_BPX_BPK_METHOD_COMPRESSED_UNKNOWN = 1,
    /* Synthetic PRS3 stream contract (pass1082): the entry carries a
     * PRS3 magic + 0x00000001 version sub-header, but the actual
     * compression payload is still intentionally unsupported. */
    NEXUS_V1_BPX_BPK_METHOD_PRS3_UNKNOWN = 2,
    /* Synthetic directory-trailer entry (pass1083): a BPX3 entry whose
     * mode tag equals NEXUS_V1_BPK_MODE_TRAILER (10) and whose first 8
     * bytes are the BE uint32 offsets of the last two picture entries
     * (matching entry[0] in the observed real MENU.BPK). No PRS3 magic,
     * no payload, no decoded surface. */
    NEXUS_V1_BPX_BPK_METHOD_DIRECTORY_TRAILER = 3
} Nexus_V1_BpxBpkMethod;

typedef struct {
    char name[16];
    uint32_t offset;
    uint32_t packed_size;
    uint32_t unpacked_size;
    uint8_t method;
    /* PRS3 sub-header fields (synthetic PRS3 stream contract only;
     * zero on BPX0 stored entries). */
    uint16_t width;
    uint8_t height;
    uint8_t mode;
    uint32_t pixel_count;
    int has_prs3_magic;
} Nexus_V1_BpxBpkEntry;

typedef struct {
    Nexus_V1_BpxBpkFormat format;
    uint16_t entry_count;
    uint32_t table_offset;
    uint32_t data_offset;
    Nexus_V1_BpxBpkEntry entries[NEXUS_V1_BPX_BPK_MAX_ENTRIES];
} Nexus_V1_BpxBpkArchive;

int nexus_v1_bpx_bpk_identify_marker(const char *path,
                                     uint32_t size,
                                     const char *sha256,
                                     Nexus_V1_BpxBpkFormat *out_format);

int nexus_v1_bpx0_parse(const uint8_t *data,
                        size_t size,
                        Nexus_V1_BpxBpkArchive *out_archive);

/* Synthetic PRS3 stream contract parser (pass1082). The byte layout is:
 *   bytes  0..16 : entry name (zero-padded, no NUL terminator required)
 *   bytes 16..20 : width (BE uint16) and mode tag (byte 19)
 *   byte  20   : height (BE uint8)
 *   byte  21   : reserved (0x00 in observed MENU.BPK prefix)
 *   bytes 22..24 : reserved (0x00 in observed MENU.BPK prefix)
 *   bytes 24..28 : PRS3 magic
 *   bytes 28..32 : version (BE uint32, must be 0x00000001)
 *   bytes 32..36 : pixel count (BE uint32, must equal width * height)
 *   bytes 36..36+payload_size : compressed payload (unsupported)
 *
 * Does NOT decompress the payload. Stores width/height/mode/pixel_count
 * on the entry so probe/UI code can show what shape it WOULD decode to
 * once a PRS3 implementation lands. */
int nexus_v1_bpx_prs3_parse(const uint8_t *data,
                            size_t size,
                            Nexus_V1_BpxBpkArchive *out_archive);

#define NEXUS_V1_BPX_PRS3_HEADER_BYTES 36U

const Nexus_V1_BpxBpkEntry *nexus_v1_bpx_bpk_find_entry(
    const Nexus_V1_BpxBpkArchive *archive,
    const char *name);

int nexus_v1_bpx_bpk_extract_stored(const uint8_t *archive_data,
                                    size_t archive_size,
                                    const Nexus_V1_BpxBpkEntry *entry,
                                    uint8_t *out,
                                    size_t out_size);

const char *nexus_v1_bpx_bpk_status_string(int status);

#endif
