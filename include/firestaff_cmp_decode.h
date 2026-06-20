/*
 * firestaff_cmp_decode.h
 *
 * CSB utility-disk .CMP file decoder.
 *
 * The .CMP (Champion Portrait) format is the on-disk format
 * used by Chaos Strikes Back's Utility Disk Champion
 * Editor. Each .CMP file contains the metadata for one
 * champion plus the bitmap data for their portrait.
 *
 * Format (per ReDMCSB DEFS.H + PORTRAIT.C):
 *
 *   offset 0   int16_t cmp_i_C  - reserved/unused (0)
 *   offset 2   int16_t cmp_i_E  - reserved/unused (0)
 *   offset 4   char Name[8]     - 8-byte champion name
 *                                (uppercase ASCII,
 *                                 null-padded)
 *   offset 12  char Title[20]   - 20-byte champion title
 *                                (uppercase ASCII,
 *                                 null-padded)
 *   offset 32  char Portrait[464] - 32x29 pixel 4bpp
 *                                  portrait bitmap, total
 *                                  = 32 * 29 * 4 / 8
 *                                  = 464 bytes
 *
 * Total file size = 496 bytes.
 *
 * The portrait is stored in the Amiga interleave layout
 * (the layout ReDMCSB PORTRAIT.C calls "Amiga" in
 * F0515_CHAMPION_ConvertPortraitsToAtariSTPlanar). Atari
 * ST binaries need a different layout, achieved by the
 * conversion function in PORTRAIT.C; we do not perform
 * that conversion here because (a) it is platform-specific
 * and (b) Firestaff renders portraits through the source-
 * locked ReDMCSB path when running CSB V1, so the
 * conversion happens upstream of any CMP ingestion.
 *
 * What this decoder does:
 *   - Validates file size is exactly 496 bytes.
 *   - Parses cmp_i_C / cmp_i_E (must both be 0; if not,
 *     returns -2 so the caller can distinguish "not a
 *     CMP" from "malformed").
 *   - Validates Name and Title are uppercase ASCII or
 *     null bytes (control characters are rejected).
 *   - Returns a pointer to the raw 464-byte portrait
 *     bitmap in the caller's buffer; the bitmap is NOT
 *     copied, the caller owns the input buffer for the
 *     lifetime of the returned pointer.
 *
 * Provenance:
 *   - ReDMCSB DEFS.H: CMP typedef (size 466 in ReDMCSB's
 *     own counting because it counts the trailing
 *     Portrait[464] as a separate array; the actual struct
 *     is 32 + 464 = 496 bytes on disk).
 *   - ReDMCSB PORTRAIT.C: F0515_CHAMPION_ConvertPortraits
 *     ToAtariSTPlanar / FromAtariSTPlanar (used to lay
 *     out the 4 bitplanes for either Amiga or Atari ST).
 *   - The 32x29 portrait dimensions are from
 *     G2078_C32_PortraitWidth / G2079_C29_PortraitHeight
 *     (CEDT001.C globals).
 *
 * Scope:
 *   - Tier 2 read-only decoder.
 *   - Bitplan conversions for rendering are Tier 3 work
 *     that should live alongside future portrait rendering.
 *   - No write-side support: we never emit .CMP files.
 *     That is the Champion Editor's job.
 */

#ifndef FIRESTAFF_CMP_DECODE_H
#define FIRESTAFF_CMP_DECODE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CMP file size in bytes. */
#define FIRESTAFF_CMP_FILE_SIZE  496u

/* Portrait dimensions. */
#define FIRESTAFF_CMP_PORTRAIT_WIDTH   32u
#define FIRESTAFF_CMP_PORTRAIT_HEIGHT  29u
#define FIRESTAFF_CMP_PORTRAIT_BYTES   464u

/* Field sizes in the CMP header. */
#define FIRESTAFF_CMP_NAME_SIZE   8u
#define FIRESTAFF_CMP_TITLE_SIZE  20u

/* Decoded CMP structure (header fields only; the portrait
 * bitmap remains in the caller's input buffer). */
typedef struct {
    uint16_t cmp_i_C;                                /* reserved, must be 0 */
    uint16_t cmp_i_E;                                /* reserved, must be 0 */
    char     name[FIRESTAFF_CMP_NAME_SIZE];          /* uppercase ASCII, null-padded */
    char     title[FIRESTAFF_CMP_TITLE_SIZE];        /* uppercase ASCII, null-padded */
    const uint8_t* portrait;                         /* 464 bytes, owned by caller */
    size_t   portrait_size;                          /* always 464 on success */
} FirestaffCmp;

/*
 * Decode a CMP file.
 *
 * Returns:
 *   0  on success; out->portrait points into *data.
 *  -1  on invalid arguments (NULL pointer, data_size < 496).
 *  -2  on invalid magic (cmp_i_C or cmp_i_E != 0).
 *  -3  on invalid Name/Title characters (not uppercase ASCII
 *      or null).
 *
 * The function does not copy the portrait bitmap. The caller
 * must keep *data alive for as long as *out is used.
 */
int FirestaffCmp_Decode(const uint8_t* data, size_t data_size,
                         FirestaffCmp* out);

/*
 * Run a series of self-tests covering valid CMP, wrong size,
 * bad magic, and bad name/title characters.
 *
 * Returns 0 on success, -1 on first failure.
 */
int FirestaffCmp_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CMP_DECODE_H */
