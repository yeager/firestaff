#ifndef FIRESTAFF_AMIGA_MFM_H
#define FIRESTAFF_AMIGA_MFM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_AMIGA_MFM_SECTOR_BYTES 512u
#define FIRESTAFF_AMIGA_MFM_MAX_TRACK_BYTES 32768u

typedef struct FirestaffAmigaMfmSector {
    uint8_t track;       /* Amiga physical track number: cylinder * 2 + head. */
    uint8_t cylinder;
    uint8_t head;
    uint8_t sector;      /* Standard DD sector numbers are 0 through 10. */
    uint8_t sectors_to_gap;
    uint8_t payload[FIRESTAFF_AMIGA_MFM_SECTOR_BYTES];
} FirestaffAmigaMfmSector;

/* Return nonzero to stop scanning after this sector. Payload is valid only
 * during the callback; copy it if it must outlive the call. */
typedef int (*FirestaffAmigaMfmVisitor)(
    const FirestaffAmigaMfmSector *sector,
    void *context);

/* Scan one packed bit-cell revolution, MSB first within each input byte.
 * bit_count may be less than byte_count * 8; matching and sector reads wrap
 * at bit_count so sectors crossing the revolution seam are recognized.
 * expected_track is the Amiga physical track number (0..159), and
 * sectors_per_track must be 11 (DD) or 22 (HD). Returns delivered sectors,
 * or -1 for invalid arguments. Input is bounded to 32 KiB.
 */
int firestaff_amiga_mfm_scan_track(
    const uint8_t *bit_cells,
    size_t byte_count,
    size_t bit_count,
    unsigned expected_track,
    unsigned sectors_per_track,
    FirestaffAmigaMfmVisitor visitor,
    void *context);

#ifdef __cplusplus
}
#endif

#endif
