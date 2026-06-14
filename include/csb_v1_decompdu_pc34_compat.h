/*
 * csb_v1_decompdu_pc34_compat.h
 *
 * CSB V1 Compressed Dungeon Support (Dungeon GAP 4,
 * DECOMPDU.C).  Source-locked per ReDMCSB DECOMPDU.C
 * (the "decomp du" tool that decompresses a compressed
 * CSB dungeon).  CSB stores some dungeons in a packed
 * format (per-M550 cell = 5 bytes instead of 8 in DM1)
 * to save disk space.
 *
 * v1 (2026-06-14): bounded header-detector that identifies
 * compressed dungeons and reports the format.  The full
 * decompressor is OPEN-OMFATTANDE; this helper is a
 * graceful-fail shim that the launcher can use to show
 * "this CSB dungeon is compressed; please decompress with
 * DECOMPDU.EXE" instead of crashing.
 */
#ifndef REDMCSB_CSB_V1_DECOMPDU_PC34_COMPAT_H
#define REDMCSB_CSB_V1_DECOMPDU_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Compressed-dungeon format detection.  Returns 1 when
 * the file looks like a packed CSB dungeon (the magic
 * "CDU\0" header at offset 0), 0 otherwise. */
int csb_v1_decompdu_detect(const unsigned char* header, int headerLen);

/* Decompression result codes. */
typedef enum {
    CSB_V1_DECOMPDU_OK = 0,             /* successful decompress */
    CSB_V1_DECOMPDU_NOT_COMPRESSED = 1, /* not a CDU file */
    CSB_V1_DECOMPDU_NOT_IMPLEMENTED = 2, /* OPEN-OMFATTANDE */
    CSB_V1_DECOMPDU_BAD_HEADER = 3,    /* magic mismatch */
    CSB_V1_DECOMPDU_IO_ERROR = 4        /* read/write failure */
} CSB_V1_DecompduResult;

/* Status: is DECOMPDU implemented in v1?  Returns 0
 * (not implemented) until the full decompressor lands. */
int csb_v1_decompdu_implemented(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_DECOMPDU_PC34_COMPAT_H */
