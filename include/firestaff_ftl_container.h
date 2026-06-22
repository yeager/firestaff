/*
 * firestaff_ftl_container.h
 *
 * Read-only parser for the proprietary FTL resource container used by
 * Dungeon Master / Chaos Strikes Back / Dungeon Master II variants on
 * Amiga, X68000, SegaCD, and MegaCD.
 *
 * Source:
 *   greatstone d_ftl.html, "Technical documentation - FTL file format"
 *   (Pierre Monnot's Swoosh Construction Kit documentation):
 *     - common header: 20 bytes, magic 0x6160, checksum at offset 2,
 *       hunk count at offset 18
 *     - hunk headers: 12 bytes each, type/unknown/offset/size
 *     - known required hunk types: 0x0010 BSS, 0x0011 DATA, 0x0012 CODE
 *     - checksum notes 1, 2, 4, and 5
 *
 * Scope:
 *   This module parses headers and verifies checksums only. It does not
 *   decompress HUNK_DATA or HUNK_CODE, does not interpret mapfile items,
 *   and does not wire FTL containers into runtime loading.
 */

#ifndef FIRESTAFF_FTL_CONTAINER_H
#define FIRESTAFF_FTL_CONTAINER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_FTL_CONTAINER_MAGIC 0x6160u
#define FIRESTAFF_FTL_HUNK_BSS        0x0010u
#define FIRESTAFF_FTL_HUNK_DATA       0x0011u
#define FIRESTAFF_FTL_HUNK_CODE       0x0012u
#define FIRESTAFF_FTL_MAX_HUNKS       16u

typedef struct {
    uint16_t magic;
    uint16_t common_checksum;
    uint16_t unknown_word;
    uint8_t  unknown_bytes[12];
    uint16_t hunk_count;
} FirestaffFtlContainerHeader;

typedef struct {
    uint16_t type;
    uint16_t unknown;
    uint32_t offset;
    uint32_t size;
    const uint8_t* payload;
} FirestaffFtlHunkHeader;

typedef struct {
    uint32_t bss_and_jump_memory_size;
    uint32_t data_area1_memory_size;
    uint32_t jump_table_memory_size;
    uint32_t bss_memory_size;
    uint32_t unknown_16;
    uint32_t code_memory_size;
    uint32_t data_file_size;
    uint16_t unknown_28;
    uint16_t unknown_30;
    uint16_t unknown_32;
    uint16_t bss_checksum;
    uint16_t code_checksum;
    uint16_t data_checksum;
} FirestaffFtlBssMetadata;

typedef struct {
    FirestaffFtlContainerHeader header;
    FirestaffFtlHunkHeader hunks[FIRESTAFF_FTL_MAX_HUNKS];
    size_t hunk_count;
    int bss_index;
    int data_index;
    int code_index;
    FirestaffFtlBssMetadata bss;
    int has_bss_metadata;
} FirestaffFtlContainer;

typedef enum {
    FIRESTAFF_FTL_CHECK_NOT_AVAILABLE = 0,
    FIRESTAFF_FTL_CHECK_MATCH = 1,
    FIRESTAFF_FTL_CHECK_MISMATCH = 2,
    FIRESTAFF_FTL_CHECK_UNSUPPORTED_COMPRESSED_CODE = 3
} FirestaffFtlChecksumStatus;

typedef struct {
    FirestaffFtlChecksumStatus status;
    uint16_t expected;
    uint16_t actual;
} FirestaffFtlChecksumResult;

typedef struct {
    FirestaffFtlChecksumResult common_header;
    FirestaffFtlChecksumResult bss_hunk;
    FirestaffFtlChecksumResult data_hunk;
    FirestaffFtlChecksumResult code_hunk;
    int has_required_bss_data_code;
} FirestaffFtlChecksumReport;

/*
 * Parse an FTL common header and hunk table.
 *
 * Returns 0 on success, -1 on malformed input. The output only contains
 * pointers into the caller-owned data buffer.
 */
int FirestaffFtlContainer_Parse(const uint8_t* data,
                                size_t data_size,
                                FirestaffFtlContainer* out);

/*
 * Verify the checksums documented by greatstone d_ftl.html.
 *
 * Common header, BSS hunk, and DATA hunk can be verified directly from
 * bytes on disk. CODE hunk checksum can be verified only for uncompressed
 * CODE hunks; compressed 0x5223 CODE hunks are explicitly reported as
 * FIRESTAFF_FTL_CHECK_UNSUPPORTED_COMPRESSED_CODE.
 */
int FirestaffFtlContainer_VerifyChecksums(const uint8_t* data,
                                          size_t data_size,
                                          const FirestaffFtlContainer* ftl,
                                          FirestaffFtlChecksumReport* out);

uint16_t FirestaffFtlContainer_CommonChecksum(const uint8_t* data,
                                              size_t data_size,
                                              size_t hunk_count);
uint16_t FirestaffFtlContainer_BssChecksum(const uint8_t* hunk,
                                           size_t hunk_size);
uint16_t FirestaffFtlContainer_ByteChecksum(const uint8_t* data,
                                            size_t data_size);

int FirestaffFtlContainer_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FTL_CONTAINER_H */
