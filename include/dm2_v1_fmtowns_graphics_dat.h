#ifndef DM2_V1_FMTOWNS_GRAPHICS_DAT_H
#define DM2_V1_FMTOWNS_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns DM2 GRAPHICS.DAT classifier and admission gate.
 *
 * The FM Towns release (Victor, 1994-01-28, HME-242) ships a GRAPHICS.DAT
 * that uses GDAT version 4 (container word 0x8004) instead of the DOS
 * version 5 (0x8005). The file is ~2.78 MB vs ~8.64 MB for DOS, with
 * 3407 raw entries vs 5624. The category/entry structure is identical;
 * the difference is fewer and smaller image assets (320x200 FM Towns
 * native resolution vs DOS VGA).  The format probe also requires the
 * HME-242 catalogue's 3,407 raw entries.  A version word and a broadly
 * plausible buffer size alone are not original-media evidence. */

#define DM2_FMTOWNS_GDAT_CONTAINER_WORD    0x8004u
#define DM2_FMTOWNS_GDAT_VERSION           4u
#define DM2_FMTOWNS_GRAPHICS_MIN_SIZE      (2U * 1024U * 1024U)
#define DM2_FMTOWNS_GRAPHICS_MAX_SIZE      (4U * 1024U * 1024U)
#define DM2_FMTOWNS_GRAPHICS_EXPECTED_SIZE 2783791U
#define DM2_FMTOWNS_GRAPHICS_RAW_COUNT     3407U

typedef struct {
    int      is_fmtowns;
    uint16_t gdat_version;
    uint16_t raw_data_count;
    uint32_t file_size;
    uint32_t nonzero_category_count;
    uint8_t  md5[16];
} DM2_V1_FmtownsGdatReceipt;

/* Probe whether a GRAPHICS.DAT buffer is the known FM Towns retail shape.
 * Returns 1 only for container word 0x8004, 3,407 raw entries and the
 * expected size interval.  This is still a bounded format gate, not a
 * substitute for the boot profile's full original-media identity check. */
int dm2_v1_fmtowns_gdat_probe(const uint8_t *data, size_t size);

/* Build a receipt describing the FM Towns GRAPHICS.DAT.
 * Returns 0 on success, -1 if the data is not a valid FM Towns GDAT. */
int dm2_v1_fmtowns_gdat_receipt(const uint8_t *data, size_t size,
                                DM2_V1_FmtownsGdatReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_FMTOWNS_GRAPHICS_DAT_H */
