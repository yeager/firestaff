/*
 * firestaff_ftl_decode.h — FTL container decoder
 *
 * FTL is the proprietary resource container used by FTL Games for
 * Atari ST / Amiga / X68000 / MegaCD / SegaCD versions of Dungeon
 * Master, Chaos Strikes Back, Dungeon Master II, and Dungeon Master
 * Nexus. It carries the FTL logo/swoosh animation, language-selection
 * screens, the main program code, the disk-error screen, the utility
 * disk menu, the hint oracle, and (on CSB) the champion editor and
 * hint oracle resources.
 *
 * The container layout is:
 *
 *   Offset  Size  Field
 *   ------  ----  --------------------------------------------------
 *   0x00    2     Magic             = 0x6160 (big-endian)
 *   0x02    2     Checksum          (16-bit checksum of HUNK_DATA area)
 *   0x04    2     Unknown1          (must be 0x0002 for the loader to read it)
 *   0x06    1     c_6               (must be 0x01)
 *   0x07    1     c_7               (must be 0x00)
 *   0x08    2     i_8               (always 0x0007, unreferenced)
 *   0x0a    1     c_0a              (always 0x00, unreferenced)
 *   0x0b    1     c_0b              (always 0x01, unreferenced)
 *   0x0c    1     c_0c              (always 0x04, unreferenced)
 *   0x0d    1     c_0d              (always 0x01, unreferenced)
 *   0x0e    2     Date1             (FTL disk date, packed)
 *   0x10    2     Date2             (same value as Date1)
 *   0x12    2     SegmentCount      (number of HUNK_* segments)
 *   0x14    12*N  SegmentHeaders    (one per HUNK_BSS / DATA / CODE)
 *
 *   Followed by:
 *
 *   HUNK_BSS payload (zero-run length table; payload is just metadata)
 *   HUNK_DATA payload (resource area; may use zero-run compression)
 *   HUNK_CODE payload (Amiga-style 68k code; may use 0x5223 magic +
 *                      1920-word frequency table + nibble-coded stream,
 *                      same algorithm as the Atari ST PAK format)
 *
 *   All multi-byte fields are big-endian (68000 / Atari ST order).
 *
 * Source of the layout:
 *   - ReDMCSB Toolchains/Common/Source/FTL.H (HEADER + SEGMENTHEADER)
 *   - ReDMCSB Toolchains/Common/Source/MAINLIB.C (loader)
 *   - Greatstone SCK docs (FTL format page)
 *   - ReDMCSB Toolchains/Common/Source/DECOMPCO.C F0913_DecompressPAK
 *     (the 0x5223 magic + 1920-word table + nibble-coded stream that
 *      PAK and FTL HUNK_CODE share)
 *
 * This decoder is read-only: it parses the container and exposes
 * pointers into the caller's input buffer (no allocation, no copy).
 * The caller is responsible for keeping the input buffer alive.
 * HUNK_CODE decompression delegates to FirestaffPak_Decode.
 *
 * HUNK_DATA is exposed in two forms:
 *   - raw_data:    pointer to the compressed bytes as stored on disk
 *                  (zero-run encoded; may need further FTL resource
 *                  decoding depending on which area the caller wants)
 *   - decoded:     best-effort zero-run decoded bytes (when raw_data
 *                  happens to be uncompressed, this is identical;
 *                  otherwise the zero-run scheme is reversed here)
 *
 * HUNK_BSS is exposed as raw_data only (BSS is metadata for the loader;
 * there is no payload to decompress).
 */
#ifndef FIRESTAFF_FTL_DECODE_H
#define FIRESTAFF_FTL_DECODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HUNK segment types as recorded in SEGMENTHEADER.Type. */
#define FIRESTAFF_HUNK_BSS  0x0010
#define FIRESTAFF_HUNK_DATA 0x0011
#define FIRESTAFF_HUNK_CODE 0x0012

/* FTL common-header magic (big-endian). Matches ReDMCSB FTL.H
 * HEADER.Magic and greatstone d_ftl.html. */
#define FIRESTAFF_FTL_CONTAINER_MAGIC 0x6160u

typedef struct {
    uint16_t magic;             /* must be 0x6160 */
    uint16_t checksum;
    uint16_t unknown1;          /* must be 0x0002 for the loader to accept it */
    uint8_t  c_6;               /* must be 0x01 */
    uint8_t  c_7;               /* must be 0x00 */
    uint16_t i_8;               /* always 0x0007 */
    uint8_t  c_0a;              /* always 0x00 */
    uint8_t  c_0b;              /* always 0x01 */
    uint8_t  c_0c;              /* always 0x04 */
    uint8_t  c_0d;              /* always 0x01 */
    uint16_t date1;             /* packed date */
    uint16_t date2;             /* same value as date1 */
    uint16_t segment_count;
} FirestaffFtlHeader;

typedef struct {
    uint16_t type;              /* FIRESTAFF_HUNK_BSS / DATA / CODE */
    uint16_t id;                /* unique within same type */
    uint32_t offset;            /* absolute offset in the file */
    uint32_t size;              /* payload size in bytes */
} FirestaffFtlSegmentHeader;

typedef struct {
    const uint8_t* data;        /* pointer into caller's input buffer */
    size_t         size;
} FirestaffFtlSlice;

/* Result of parsing an FTL container.
 *
 * All pointers reference the caller's input buffer (no copies are made).
 * Callers must keep the input buffer alive for the lifetime of this
 * struct and any inner slices.
 */
typedef struct {
    FirestaffFtlHeader        header;
    FirestaffFtlSegmentHeader segments[8];   /* FTL spec allows up to 8 segments */
    uint16_t                  segment_count_parsed;

    /* Convenience slices indexed by segment type. NULL if the segment
     * is absent. hunk_bss, hunk_data_raw, and hunk_code_raw are
     * zero-copy views into the caller's input buffer (must stay alive
     * for the lifetime of this struct and any decoded buffers).
     * hunk_data_decoded is the zero-run-decoded HUNK_DATA payload;
     * hunk_code is the 0x5223-decoded HUNK_CODE payload (or a verbatim
     * copy for uncompressed CODE hunks). Both are owned by this struct
     * and released by FirestaffFtl_Free. */
    FirestaffFtlSlice hunk_bss;
    FirestaffFtlSlice hunk_data_raw;
    uint8_t*          hunk_data_decoded;
    size_t            hunk_data_decoded_size;
    FirestaffFtlSlice hunk_code_raw;
    uint8_t*          hunk_code;
    size_t            hunk_code_size;
} FirestaffFtl;

/* Parse the FTL header + segment table.
 *
 * Validates magic, header invariants (Unknown1 == 2, c_6 == 1, c_7 == 0)
 * and segment type/id uniqueness.
 *
 * Returns 0 on success, < 0 on failure:
 *   -1: null arg or too-small input (< 0x14 bytes)
 *   -2: magic mismatch
 *   -3: Unknown1 != 0x0002 / c_6 != 0x01 / c_7 != 0x00
 *   -4: SegmentCount out of range (> 8) or segment header truncated
 *   -5: segment type/id collision
 *   -6: segment offset+size falls outside the input buffer
 */
int FirestaffFtl_Parse(const uint8_t* data, size_t data_size,
                       FirestaffFtl* out);

/* Decode HUNK_DATA (zero-run compression) and HUNK_CODE (0x5223 +
 * 1920-word table + nibble stream, delegated to FirestaffPak_Decode).
 *
 * Returns 0 on success. On failure, partially-decoded buffers may have
 * been allocated; call FirestaffFtl_Free either way.
 */
int FirestaffFtl_Decode(FirestaffFtl* ftl);

/* Free decoded buffers allocated by FirestaffFtl_Decode. */
void FirestaffFtl_Free(FirestaffFtl* ftl);

/* Self-test using synthetic FTL containers. Always returns 0. */
int FirestaffFtl_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FTL_DECODE_H */
