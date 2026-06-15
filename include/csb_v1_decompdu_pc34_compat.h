/*
 * csb_v1_decompdu_pc34_compat.h
 *
 * CSB V1 Compressed Dungeon Support (Dungeon GAP 4,
 * DECOMPDU.C).  Source-locked per ReDMCSB DECOMPDU.C
 * F0455_FLOPPY_DecompressDungeon (the portable C variant,
 * MEDIA481_P20JB_I34E_I34M_F31E_F31J_P31J path, lines
 * ~250-290).
 *
 * The ACTUAL CSB on-floppy dungeon compression is NOT a
 * simple RLE.  The original F0455 routine is a static
 * bit-packed scheme:
 *
 *   - The first 20 bytes of the compressed buffer are two
 *     code tables: 4 "most common" bytes followed by 16
 *     "less common" bytes.
 *   - The remaining bytes are an MSB-first bit stream.
 *   - Each output byte is encoded with a variable-length
 *     prefix code:
 *       0bb      (3 bits)  -> mostCommon[bb]     (bb = 0..3)
 *       10bbbb   (6 bits)  -> lessCommon[bbbb]   (bbbb = 0..15)
 *       11bbbbbbbb (10 bits) -> literal byte bbbbbbbb
 *
 * v2 (2026-06-16, Dungeon GAP 4): bounded but source-faithful
 * port of F0455 with the bounds checks the original (which
 * trusted the stream) lacked, an encoder for round-trip
 * testing, and a grid wrapper that decompresses a level pack
 * (up to 24 levels of up to 64x64 tiles) into a heap struct.
 */
#ifndef REDMCSB_CSB_V1_DECOMPDU_PC34_COMPAT_H
#define REDMCSB_CSB_V1_DECOMPDU_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Detection (v1, retained) ─────────────────────────────── */

/* Compressed-dungeon container detection.  Returns 1 when
 * the file looks like a packed CSB dungeon (the Firestaff
 * "CDU\0" container magic at offset 0 + a known sub-format
 * byte), 0 otherwise. */
int csb_v1_decompdu_detect(const unsigned char* header, int headerLen);

/* Legacy v1 result codes (retained for the stub test). */
typedef enum {
    CSB_V1_DECOMPDU_OK = 0,             /* successful decompress */
    CSB_V1_DECOMPDU_NOT_COMPRESSED = 1, /* not a CDU file */
    CSB_V1_DECOMPDU_NOT_IMPLEMENTED = 2, /* OPEN-OMFATTANDE */
    CSB_V1_DECOMPDU_BAD_HEADER = 3,    /* magic mismatch */
    CSB_V1_DECOMPDU_IO_ERROR = 4        /* read/write failure */
} CSB_V1_DecompduResult;

/* Status: now that the F0455 core decompressor lands, this
 * returns 1.  (The 24-level >64x64 super-packs and the
 * floppy-side disk streaming remain out of scope, but the
 * core CSB compression scheme is implemented.) */
int csb_v1_decompdu_implemented(void);

/* ── F0455 core decompressor (Dungeon GAP 4) ──────────────── */

/* Error codes for the F0455 bit-stream decompressor.  0 is
 * success; all failures are negative so callers can test
 * `< 0`. */
typedef enum {
    CSB_DECOMPDU_ERR_OK            =  0,
    CSB_DECOMPDU_ERR_NULL         = -1, /* NULL pointer argument */
    CSB_DECOMPDU_ERR_BAD_SUBFORMAT = -2, /* sub-format byte unsupported */
    CSB_DECOMPDU_ERR_OOM           = -3, /* heap allocation failed */
    CSB_DECOMPDU_ERR_CORRUPT       = -4, /* stream shorter than 20-byte table */
    CSB_DECOMPDU_ERR_TRUNCATED     = -5, /* ran out of bits before outCount */
    CSB_DECOMPDU_ERR_BOUNDS        = -6  /* dimension/level out of bounds */
} CSB_DecompduError;

/* Decompress an F0455 bit-packed stream.
 *
 *   compressed     : input buffer (20-byte table + bit stream)
 *   compressedLen  : length of `compressed` in bytes
 *   out            : caller-provided output buffer
 *   outByteCount   : number of bytes to produce (== expected
 *                    decompressed size)
 *
 * Returns CSB_DECOMPDU_ERR_OK (0) on success, or a negative
 * CSB_DecompduError on failure.  Unlike the original 68k
 * routine, this version never reads past `compressedLen`
 * and reports truncation instead of running off the buffer.
 *
 * Source: ReDMCSB DECOMPDU.C F0455_FLOPPY_DecompressDungeon
 *         (MEDIA481 portable C path). */
int csb_v1_decompdu_f0455(const unsigned char* compressed,
                          long compressedLen,
                          unsigned char* out,
                          long outByteCount);

/* Encode a raw byte buffer into the F0455 bit-packed format.
 * Builds the 20-byte code table by frequency analysis, then
 * emits the variable-length bit stream.  This is the inverse
 * of csb_v1_decompdu_f0455 and exists so the decompressor can
 * be round-trip tested against real-format data (the original
 * encoder lived in the DECOMPDU.EXE tool, not in the engine).
 *
 *   raw          : input bytes
 *   rawLen       : number of input bytes
 *   out          : output buffer for the compressed stream
 *   outCapacity  : capacity of `out` in bytes
 *   outLen       : receives the number of bytes written
 *
 * Returns CSB_DECOMPDU_ERR_OK (0) on success or a negative
 * error code (OOM / OOM-equivalent when outCapacity is too
 * small -> CSB_DECOMPDU_ERR_OOM). */
int csb_v1_decompdu_f0455_encode(const unsigned char* raw,
                                 long rawLen,
                                 unsigned char* out,
                                 long outCapacity,
                                 long* outLen);

/* ── Grid wrapper (bounded level-pack decompression) ──────── */

#define CSB_CDU_MAX_DIM     64
#define CSB_CDU_MAX_LEVELS  24   /* CSB DUNGEON.DAT NumLevel() = 24 */

/* A decompressed CSB compressed-dungeon level pack.  `tiles`
 * is a single heap allocation of levelCount*width*height bytes
 * laid out level-major then row-major:
 *   tile(level,x,y) = tiles[(level*height + y)*width + x]
 * Free with csb_v1_cdu_dungeon_free(). */
typedef struct {
    int            width;
    int            height;
    int            levelCount;
    unsigned char* tiles;
} CSB_CDUDungeon;

/* Firestaff CDU container header layout (little-endian):
 *   [0..3]  magic 'C','D','U','\0'
 *   [4]     sub-format byte (1 = F0455 bit-packed; the real
 *           CSB on-floppy scheme.  Other values reserved.)
 *   [5]     width   (1..64)
 *   [6]     height  (1..64)
 *   [7]     levelCount (1..24)
 *   [8..11] decompressed payload byte count (uint32 LE)
 *   [12..]  F0455 compressed payload
 * The decompressed payload must equal width*height*levelCount.
 */
#define CSB_CDU_HEADER_SIZE   12
#define CSB_CDU_SUBFORMAT_F0455  1

/* Decompress a full CDU container into a heap CSB_CDUDungeon.
 *
 *   container     : the bytes of the CDU file (header + payload)
 *   containerLen  : length of `container`
 *   outDungeon    : receives width/height/levelCount and an
 *                   allocated `tiles` buffer (caller frees)
 *
 * Returns CSB_DECOMPDU_ERR_OK (0) on success or a negative
 * CSB_DecompduError.  On any error `outDungeon->tiles` is
 * left NULL. */
int csb_v1_decompdu_decompress_grid(const unsigned char* container,
                                    long containerLen,
                                    CSB_CDUDungeon* outDungeon);

/* Build a CDU container (header + F0455 payload) from a raw
 * level pack.  Inverse of csb_v1_decompdu_decompress_grid;
 * used for round-trip tests and for re-packing.
 *
 *   tiles        : levelCount*width*height raw tile bytes
 *   width,height,levelCount : dimensions (bounded as above)
 *   out          : output buffer for the container
 *   outCapacity  : capacity of `out`
 *   outLen       : receives container length written
 *
 * Returns CSB_DECOMPDU_ERR_OK (0) or a negative error. */
int csb_v1_decompdu_build_grid(const unsigned char* tiles,
                               int width, int height, int levelCount,
                               unsigned char* out,
                               long outCapacity,
                               long* outLen);

/* Release the heap buffer owned by a CSB_CDUDungeon and zero
 * the struct.  Safe to call on a zeroed/failed struct. */
void csb_v1_cdu_dungeon_free(CSB_CDUDungeon* dungeon);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_DECOMPDU_PC34_COMPAT_H */
