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
    NEXUS_V1_BPX_BPK_FORMAT_VERIFIED_MENU_BPK_MARKER = 2
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
    NEXUS_V1_BPX_BPK_METHOD_COMPRESSED_UNKNOWN = 1
} Nexus_V1_BpxBpkMethod;

typedef struct {
    char name[16];
    uint32_t offset;
    uint32_t packed_size;
    uint32_t unpacked_size;
    uint8_t method;
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
