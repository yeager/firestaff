/*
 * csb_v1_csbgraphics_dat_classify.h
 *
 * CSBWin custom graphics-file index classifier.
 *
 * CSBWin (Paul Stevens' PC port) reads an optional companion file
 * called "CSBgraphics.dat" alongside the standard CSB assets.
 * CSBGraffer + the CSBWin Viewport Compiler tool produce this
 * file to override specific graphics indices in the live engine.
 * The format mirrors the original DM/CSB graphics.dat table
 * layout — a count, a parallel pair of compressed-size and
 * decompressed-size tables, and the concatenated LZW-compressed
 * graphic payloads — and is identified by an MD5 of the entire
 * file split across two uint32 signature words.
 *
 * This module is the bounded read-only classifier Firestaff uses
 * to discover, header-validate, inventory, and locate compressed
 * entry byte spans in a CSBgraphics.dat without committing to a
 * full runtime override hook. It deliberately:
 *
 *   - reads the on-disk count + size tables
 *   - can return one entry's compressed payload span using the
 *     CSBWin LocateNthGraphic(n) offset rule
 *   - can decompress one entry's LZW payload through the existing
 *     ReDMCSB-compatible graphics LZW decoder
 *   - rejects truncated input, oversized counts, and table
 *     sums that overflow the file size
 *   - classifies the byte-order via the documented 0x8001
 *     little-endian sentinel that the original CSBWin Graphics.cpp
 *     uses to detect a Windows-produced CSBgraphics.dat
 *
 * The companion csb_v1_csbgraphics_dat_real_scan module adds
 * hash-based discovery in the user's ~/.firestaff/data tree so
 * future sessions can SKIP cleanly on hosts without a real
 * CSBgraphics.dat, just like the existing HCSB.HTC real-scan
 * module does for the Hint Oracle.
 *
 * Source references:
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count + sizes)
 *   - CSBWin/Graphics.cpp:1643 LocateNthGraphic (payload offset)
 *   - CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile (signature
 *     split into two uint32 words)
 *   - CSBWin/data.cpp:1936 Signature (MD5 file digest split as
 *     signature1/signature2)
 *   - ReDMCSB GRAPHICS.DAT format documented in
 *     greatstone d_items.html "Graphics.dat file format" +
 *     dmweb "Data Files" page.
 *
 * Non-claims:
 *   - No runtime override. No M11 / M12 wiring.
 *   - No overlay bitmap interpretation beyond LZW bytes.
 *   - No full CSBWin custom-dungeon support — that remains
 *     tracked under docs/FIRESTAFF_GAP_LIST.md row C3 / A3
 *     "CSBWin custom resource handling" (OPEN-LARGE).
 */

#ifndef FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_CLASSIFY_H
#define FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_CLASSIFY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Result codes. 0 = OK, negatives are bounded errors. */
typedef enum {
    CSB_V1_CSBGRAPHICS_CLASSIFY_OK = 0,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT = -1,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL = -2,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_COUNT = -3,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW = -4,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_MARKER = -5,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ENTRY_RANGE = -6,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OUTPUT_TOO_SMALL = -7,
    CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_LZW = -8
} CSB_V1_CSBGraphicsClassifyResult;

/* Hard caps. The original CSB graphics.dat caps at ~3000 graphics;
 * we round up to allow plenty of room for community overlays without
 * inviting garbage files through. */
#define CSB_V1_CSBGRAPHICS_MAX_COUNT          8192u
#define CSB_V1_CSBGRAPHICS_FILE_MIN_BYTES     4u     /* count(2) + 1 entry (comp+dec) */

/* The little-endian sentinel emitted by the CSBWin Viewport Compiler
 * before the real count when the producer ran on a little-endian
 * host. CSBWin/Graphics.cpp:1918-1934 explicitly checks for it. */
#define CSB_V1_CSBGRAPHICS_LITTLE_ENDIAN_MARKER   0x8001u

typedef enum {
    CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN = 0,
    CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER = 1
} CSB_V1_CSBGraphicsByteOrder;

/* Parsed index view. All sizes are bytes. */
typedef struct {
    CSB_V1_CSBGraphicsByteOrder byte_order;
    uint32_t count;                 /* number of graphics entries            */
    uint64_t total_compressed;      /* sum of compressed-size entries        */
    uint64_t total_decompressed;    /* sum of decompressed-size entries      */
    uint64_t payload_offset;        /* file offset where payload begins      */
    uint64_t payload_bytes_avail;   /* bytes available for payload           */
    /* Largest single entry — useful diagnostic for overlay budgets. */
    uint32_t max_compressed;
    uint32_t max_decompressed;
} CSB_V1_CSBGraphicsIndex;

/* Bounded view of one compressed payload entry inside a
 * CSBgraphics.dat. This is the handoff shape a future LZW decoder
 * or M11 override hook can consume without re-deriving offsets.
 * `payload_offset` points to the compressed byte span for
 * `entry_index`; `compressed_size` may be zero for an empty
 * override slot. */
typedef struct {
    uint32_t entry_index;
    uint64_t payload_offset;
    uint32_t compressed_size;
    uint32_t decompressed_size;
} CSB_V1_CSBGraphicsEntrySpan;

/* Parse the on-disk count + parallel size tables from `bytes` of
 * `size` bytes. Returns CSB_V1_CSBGRAPHICS_CLASSIFY_OK on success.
 *
 * The parser is read-only: it never owns the input buffer and the
 * classifier itself does not decompress a payload. Callers that
 * want bounded raw bytes for a single entry can use
 * csb_v1_csbgraphics_dat_decode_entry(); callers that need bitmap
 * interpretation or runtime override behavior still need the later
 * CSBgraphics.dat importer path.
 *
 * Bounds:
 *   - rejects size < CSB_V1_CSBGRAPHICS_FILE_MIN_BYTES
 *   - rejects count > CSB_V1_CSBGRAPHICS_MAX_COUNT
 *   - rejects total_compressed > size-payload_offset
 *   - rejects any uint16 entry that overflows uint32 max
 *     (defensive — DM/CSB original caps well below 65535)
 */
int csb_v1_csbgraphics_dat_classify(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBGraphicsIndex *out_index);

/* Locate the compressed payload byte span for one entry. This
 * mirrors CSBWin/Graphics.cpp LocateNthGraphic(n): payload base
 * plus the sum of all preceding compressed-size entries. The
 * helper does not decompress the entry and does not interpret the
 * graphic payload; it only returns a bounds-checked span.
 */
int csb_v1_csbgraphics_dat_entry_span(
    const uint8_t *bytes, size_t size, uint32_t entry_index,
    CSB_V1_CSBGraphicsEntrySpan *out_span);

/* Decode one CSBgraphics.dat entry into caller-owned memory. The
 * helper first uses csb_v1_csbgraphics_dat_entry_span(), then feeds
 * exactly that compressed byte range to m11_gfx_lzw_decompress().
 *
 * Returns OK only when the decoded byte count exactly matches the
 * entry's declared decompressed size. Empty entries (compressed=0,
 * decompressed=0) are accepted and write 0 bytes. The output buffer
 * must be at least entry.decompressed_size bytes for non-empty entries.
 */
int csb_v1_csbgraphics_dat_decode_entry(
    const uint8_t *bytes, size_t size, uint32_t entry_index,
    uint8_t *out_bytes, size_t out_capacity,
    size_t *out_written);

/* Human-readable label for a result code. */
const char *csb_v1_csbgraphics_dat_result_name(int result);

/* Human-readable label for the byte-order enum. */
const char *csb_v1_csbgraphics_dat_byte_order_name(
    CSB_V1_CSBGraphicsByteOrder byte_order);

/* Source-evidence string for tests + docs. */
const char *csb_v1_csbgraphics_dat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_CLASSIFY_H */
