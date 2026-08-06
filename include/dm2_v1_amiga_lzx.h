#ifndef FIRESTAFF_DM2_V1_AMIGA_LZX_H
#define FIRESTAFF_DM2_V1_AMIGA_LZX_H

/*
 * DM2's original Amiga installer joins dm2_arcsplit1 through
 * dm2_arcsplit6 into DM2_archive.LZX, then asks unlzx to extract it.
 * These declarations intentionally cover only the join and archive index:
 * callers keep every byte in memory and may not treat an indexed archive as
 * playable until the requested files have been decoded and hash-verified.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_AMIGA_LZX_PART_COUNT 6u
#define DM2_V1_AMIGA_LZX_ENTRY_MAX  64u
#define DM2_V1_AMIGA_LZX_NAME_MAX   64u

typedef struct {
    const uint8_t *bytes;
    size_t size;
} DM2_V1_AmigaLzxPart;

typedef struct {
    char name[DM2_V1_AMIGA_LZX_NAME_MAX];
    uint32_t uncompressed_size;
    uint32_t compressed_size;
    uint32_t data_crc32;
    size_t data_offset;
    uint16_t attributes;
    uint16_t flags;
    uint8_t os;
    uint8_t method;
    uint8_t version;
} DM2_V1_AmigaLzxEntry;

typedef struct {
    int valid;
    size_t size;
    unsigned int entry_count;
    DM2_V1_AmigaLzxEntry entries[DM2_V1_AMIGA_LZX_ENTRY_MAX];
} DM2_V1_AmigaLzxArchive;

/* Joins precisely the six original installer parts in RAM.  The caller owns
 * the returned allocation and releases it with dm2_v1_amiga_lzx_free(). */
int dm2_v1_amiga_lzx_join_parts(const DM2_V1_AmigaLzxPart parts[DM2_V1_AMIGA_LZX_PART_COUNT],
                                uint8_t **out_bytes, size_t *out_size);
void dm2_v1_amiga_lzx_free(uint8_t *bytes);

/* Reads the bounded original LZX file index without decompressing a payload. */
int dm2_v1_amiga_lzx_parse(DM2_V1_AmigaLzxArchive *out,
                           const uint8_t *bytes, size_t size);
const DM2_V1_AmigaLzxEntry *dm2_v1_amiga_lzx_find(
    const DM2_V1_AmigaLzxArchive *archive, const char *name);

/* Confirms that the index declares the minimum DM2 install payload.  This is
 * not a launch gate: each compressed file still needs in-memory decoding and
 * original-hash verification. */
int dm2_v1_amiga_lzx_has_install_payload(const DM2_V1_AmigaLzxArchive *archive);

/* Decodes one declared file entirely in RAM.  The archive's LZX CRC is
 * verified before success.  The caller owns the returned bytes and releases
 * them with dm2_v1_amiga_lzx_free(). */
int dm2_v1_amiga_lzx_extract_entry(const DM2_V1_AmigaLzxArchive *archive,
                                   const uint8_t *archive_bytes,
                                   const DM2_V1_AmigaLzxEntry *entry,
                                   uint8_t **out_bytes, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_AMIGA_LZX_H */
