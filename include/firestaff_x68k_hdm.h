/*
 * Read-only reader for the documented DM1 X68000 2DHD image layout.
 *
 * This is an import boundary only. It never writes the image or extracted
 * members to disk; returned file bytes are copied into caller-owned memory.
 */
#ifndef FIRESTAFF_X68K_HDM_H
#define FIRESTAFF_X68K_HDM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_X68K_HDM_IMAGE_BYTES 1261568u
#define FIRESTAFF_X68K_HDM_ROOT_ENTRY_BYTES 32u
#define FIRESTAFF_X68K_HDM_CLUSTER_BYTES 1024u

typedef struct {
    const uint8_t *image;
    size_t image_size;
} FirestaffX68kHdm;

typedef struct {
    char name[13];
    uint8_t attributes;
    uint16_t first_cluster;
    uint32_t size;
    uint16_t directory_slot;
} FirestaffX68kHdmRootEntry;

/* Returns 0 on success, -1 for an unsupported or malformed image. */
int FirestaffX68kHdm_Open(const uint8_t *image,
                          size_t image_size,
                          FirestaffX68kHdm *out);

/*
 * Walk root-directory slots in order. Returns 0 with an entry, 1 at the end,
 * and -1 for malformed metadata. Deleted, long-name, and volume-label slots
 * are skipped. The input cursor must start at zero.
 */
int FirestaffX68kHdm_NextRootEntry(const FirestaffX68kHdm *hdm,
                                   uint16_t *cursor,
                                   FirestaffX68kHdmRootEntry *out);

/*
 * Copy one regular root file into caller-owned memory. Returns 0 on success
 * and -1 for directories, invalid cluster chains, or an insufficient buffer.
 */
int FirestaffX68kHdm_ReadRootFile(const FirestaffX68kHdm *hdm,
                                  const FirestaffX68kHdmRootEntry *entry,
                                  uint8_t *out,
                                  size_t out_capacity);

#ifdef __cplusplus
}
#endif

#endif
