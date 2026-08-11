#ifndef FIRESTAFF_CSB_V1_X68K_HDM_H
#define FIRESTAFF_CSB_V1_X68K_HDM_H

#include <stddef.h>
#include <stdint.h>

/* Read-only root-file access for raw 1,232 KiB Chaos Strikes Back X68000
 * HDM images. Human68k stores its root directory and FAT12 allocation chain
 * in the normal PC-compatible little-endian FAT byte order, even though the
 * X68000 executable payloads are 68000 big-endian data.
 *
 * This deliberately does not interpret any extracted game asset. It is the
 * media boundary that supplies source-faithful byte spans to the existing
 * DMCSB2/FTL readers. */

#define CSB_V1_X68K_HDM_BYTES_PER_SECTOR 1024u
#define CSB_V1_X68K_HDM_BYTES_PER_DISK 1261568u

typedef struct {
    uint16_t sectors_per_cluster;
    uint16_t fat_count;
    uint16_t sectors_per_fat;
    uint16_t root_entry_count;
    uint16_t root_file_count;
    uint32_t data_offset;
} CSB_V1_X68kHdmReceipt;

typedef struct {
    char name[13];
    uint8_t attributes;
    uint16_t first_cluster;
    uint32_t byte_count;
} CSB_V1_X68kHdmRootEntry;

/* Validate the fixed 2DHD Human68k layout without allocating or mutating
 * input. A successful result only establishes a structurally readable HDM;
 * it is not an authenticity claim. */
int csb_v1_x68k_hdm_probe(const uint8_t *hdm, size_t hdm_size,
                          CSB_V1_X68kHdmReceipt *out_receipt);

/* Extract one non-directory 8.3 root entry. Names are ASCII and matched
 * case-insensitively (for example, "graphics.dat"). Pass out_bytes as NULL
 * to query its exact size. Malformed FAT chains, subdirectories and files
 * outside the HDM are rejected. */
int csb_v1_x68k_hdm_extract_root_file(const uint8_t *hdm, size_t hdm_size,
                                      const char *name, uint8_t *out_bytes,
                                      size_t out_capacity, size_t *out_size,
                                      CSB_V1_X68kHdmReceipt *out_receipt);

/* Return the zero-based non-directory root entry. The directory order is
 * preserved because Human68k startup scripts name files directly from this
 * root. Metadata is copied only; game bytes remain in the caller's HDM. */
int csb_v1_x68k_hdm_root_entry(const uint8_t *hdm, size_t hdm_size,
                               uint16_t entry_index,
                               CSB_V1_X68kHdmRootEntry *out_entry);

#endif /* FIRESTAFF_CSB_V1_X68K_HDM_H */
