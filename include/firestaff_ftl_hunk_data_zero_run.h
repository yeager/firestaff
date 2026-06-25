/*
 * firestaff_ftl_hunk_data_zero_run.h
 *
 * HUNK_DATA zero-run decompression for the FTL container format
 * used by Dungeon Master / Chaos Strikes Back / Dungeon Master II /
 * Dungeon Master Nexus on Amiga, X68000, MegaCD, and SegaCD.
 *
 * Source of the algorithm:
 *   greatstone d_ftl.html, "Note 7: How to decompress HUNK_DATA":
 *
 *     "The compression algorithm in very simple: it shrinks the
 *      consecutive 0x00.
 *      If 2 consecutive 0x00 are found, they must be copied in the
 *      uncompressed data and the next 2 bytes, converted as a word,
 *      is the number of additional 0x00 to write in the uncompressed
 *      data."
 *
 *     Pseudocode lifted from the spec:
 *       while (int i < size_area_1) {
 *         if (area_1[i] == 0x00 && area_1[i+1] == 0x00) {
 *           output += area_1[i]; output_idx++;
 *           output += area_1[i+1]; output_idx++;
 *           int additional = word(area_1[i+2], area_1[i+3]);
 *           for (int j = 0; j < additional; j++) {
 *             output += 0x00; output_idx++;
 *           }
 *           i += 4;
 *         } else {
 *           output += area_1[i]; output_idx++;
 *           output += area_1[i+1]; output_idx++;
 *           i += 2;
 *         }
 *       }
 *
 * Notes:
 *   - The algorithm works on 16-bit groups (the literal branch always
 *     copies two bytes, the run branch always emits two leading 0x00
 *     bytes plus the additional count). The compressed area_1 size is
 *     therefore always even in valid FTLs.
 *   - The output size is exactly the in-memory area_1 size stored in
 *     HUNK_BSS offset 4 (greatstone d_ftl.html, "Structure" /
 *     "HUNK_BSS" row at index 4). We expose it via
 *     uncompressed_size so callers can size the output buffer from the
 *     documented HUNK_BSS field, matching how the FTL loader / greatstone
 *     SCK extract the resource area before mapfile-driven item reads.
 *   - This decoder is intentionally bounded: it does NOT interpret the
 *     decoded bytes against any mapfile (greatstone d_mapfile.html), it
 *     does NOT touch HUNK_CODE (0x5223 + 1920-word table + nibble
 *     stream, owned by firestaff_pak_decode.c and the documented
 *     "Note 8" path), and it does NOT wire FTL containers into runtime
 *     loading. That keeps this module small, CTest-friendly, and
 *     source-locked to "Note 7" only.
 *
 * Scope summary:
 *   - Read-only decoder.
 *   - Validates the input bounds; rejects truncated runs and writes
 *     past out_size.
 *   - Compressed inputs of length 0 are valid no-ops.
 *   - Uncompressed-size == 0 is valid only when compressed-size == 0
 *     (the FTL loader treats area_1 as both compressed bytes and the
 *     in-memory resource pool).
 *
 * Provenance:
 *   greatstone d_ftl.html "Note 7" + "Note 4" (HUNK_DATA checksum) +
 * "Note 2" / "Note 1" (BSS / common header checksums). The decoder
 * does not compute checksums; that already lives in
 * firestaff_ftl_container.c (FirestaffFtlContainer_VerifyChecksums).
 */

#ifndef FIRESTAFF_FTL_HUNK_DATA_ZERO_RUN_H
#define FIRESTAFF_FTL_HUNK_DATA_ZERO_RUN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Result codes for FirestaffFtlHunkData_DecompressZeroRun. */
typedef enum {
    /* Decompression completed; out buffer holds exactly *out_written
     * bytes and matches the greatstone d_ftl.html "Note 7" output. */
    FIRESTAFF_FTL_HUNK_DATA_OK = 0,

    /* Null argument, or out_size == 0 with non-zero compressed input. */
    FIRESTAFF_FTL_HUNK_DATA_ERR_ARG = -1,

    /* Truncated input: the greatstone 4-byte run header or the 2-byte
     * literal pair fell off the end of the compressed buffer. */
    FIRESTAFF_FTL_HUNK_DATA_ERR_TRUNCATED = -2,

    /* A zero-run declared additional bytes that, when summed with the
     * two leading 0x00 bytes, would overflow the declared output size.
     * This means either the input is malformed, or out_size does not
     * match the FTL HUNK_BSS "size of area 1 of hunk 0x011 in memory"
     * field. We never silently truncate. */
    FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW = -3,

    /* The compressed input length is odd. The Note 7 pseudocode reads
     * 2 bytes per iteration; a real FTL area_1 size is always even. */
    FIRESTAFF_FTL_HUNK_DATA_ERR_ODD_INPUT = -4
} FirestaffFtlHunkDataStatus;

/*
 * Decode a compressed HUNK_DATA area_1 stream.
 *
 *   compressed       pointer to area_1 as stored on disk
 *   compressed_size  byte count of area_1 on disk
 *   uncompressed_size  in-memory area_1 size from HUNK_BSS offset 4
 *                      (the caller passes that explicitly; the decoder
 *                      does not consult the surrounding FTL container)
 *   out              output buffer; must hold uncompressed_size bytes
 *   out_written      on success, set to the number of bytes written
 *
 * On error, *out_written is set to 0. The decoder never writes
 * partial output past *out_written.
 *
 * The implementation is a direct port of greatstone d_ftl.html
 * "Note 7" pseudocode, with explicit bounds checks so that malformed
 * FTL inputs are rejected instead of overrunning the output buffer.
 */
FirestaffFtlHunkDataStatus FirestaffFtlHunkData_DecompressZeroRun(
    const uint8_t* compressed,
    size_t compressed_size,
    size_t uncompressed_size,
    uint8_t* out,
    size_t* out_written);

/*
 * Compute the maximum uncompressed size that
 * FirestaffFtlHunkData_DecompressZeroRun could possibly produce from a
 * compressed buffer of compressed_size bytes.
 *
 * Worst case for the Note 7 algorithm: every 2 input bytes are a run
 * header declaring 0xFFFF additional zeros, which would expand to
 * 2 + 0xFFFF bytes per 4-byte input group (2 byte pairs). The maximum
 * compressed_size we will accept is FIRESTAFF_FTL_HUNK_DATA_MAX_INPUT
 * (so we can sum without overflow in size_t). Returns 0 if the
 * calculation overflows.
 *
 * Callers can use this as a sizing hint when they do not have the
 * HUNK_BSS-derived uncompressed_size at hand, or as a sanity check
 * that the HUNK_BSS field is internally consistent with the
 * HUNK_DATA compressed-size field.
 */
size_t FirestaffFtlHunkData_MaxUncompressedSize(size_t compressed_size);

/* Bounded self-test covering the documented Note 7 cases:
 *   - all-zero input stream
 *   - no zero runs (pure literal stream)
 *   - mixed literal + zero runs
 *   - short compressed input that ends mid-run-header (must reject)
 *   - declared uncompressed_size too small for the actual output
 *     (must reject)
 *   - odd compressed input length (must reject)
 *   - compressed-size 0 / uncompressed-size 0 round trip
 *
 * Returns 0 on success and writes a short PASS/FAIL summary to stderr
 * when one or more cases fails.
 */
int FirestaffFtlHunkData_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FTL_HUNK_DATA_ZERO_RUN_H */
