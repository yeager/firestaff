/*
 * csb_v1_csbgraphics_dat_classify.c
 *
 * Read-only index classifier for the CSBWin "CSBgraphics.dat"
 * custom graphics override file. See
 * include/csb_v1_csbgraphics_dat_classify.h for scope and source
 * references.
 *
 * Implementation stays close to the CSBWin source:
 *
 *   CSBWin/Graphics.cpp:1918 ReadGraphicsIndex():
 *     - opens graphics.dat / CSBgraphics.dat
 *     - reads NumGraphic (uint16); the value 0x8001 means "the
 *       following uint16 is little-endian and is the real count"
 *     - reads NumGraphic * 2 bytes as compressed-size table
 *     - reads NumGraphic * 2 bytes as decompressed-size table
 *     - allocates parallel arrays, swaps bytes if little-endian
 *
 *   CSBWin/Graphics.cpp:1643 LocateNthGraphic(n):
 *     - returns (NumGraphic * 4 + 2) + sum(compressed_size[0..n-1])
 *
 *   The classifier below mirrors that layout byte-for-byte. It
 *   never tries to decompress a payload, never overrides a
 *   graphics entry, and never binds into the CSB runtime; those
 *   remain separate concerns tracked under
 *   docs/FIRESTAFF_GAP_LIST.md row C3 / A3.
 */

#include "csb_v1_csbgraphics_dat_classify.h"

#include <string.h>

/* ── Helpers ──────────────────────────────────────────────────────── */

/* Read a big-endian uint16 from `bytes` at `offset`. Caller is
 * responsible for bounds. */
static uint16_t read_be16(const uint8_t *bytes, size_t offset)
{
    return (uint16_t)(((uint16_t)bytes[offset] << 8) |
                      (uint16_t)bytes[offset + 1u]);
}

/* Read a little-endian uint16 from `bytes` at `offset`. Caller is
 * responsible for bounds. */
static uint16_t read_le16(const uint8_t *bytes, size_t offset)
{
    return (uint16_t)(((uint16_t)bytes[offset + 1u] << 8) |
                      (uint16_t)bytes[offset]);
}

/* ── Public API ───────────────────────────────────────────────────── */

int csb_v1_csbgraphics_dat_classify(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBGraphicsIndex *out_index)
{
    uint32_t count = 0u;
    uint64_t table_bytes = 0u;
    uint64_t payload_offset = 0u;
    uint64_t payload_avail = 0u;
    uint64_t total_compressed = 0u;
    uint64_t total_decompressed = 0u;
    uint32_t max_compressed = 0u;
    uint32_t max_decompressed = 0u;
    CSB_V1_CSBGraphicsByteOrder byte_order =
        CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN;
    uint32_t i;

    if (!bytes || !out_index) {
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT;
    }
    if (size < CSB_V1_CSBGRAPHICS_FILE_MIN_BYTES) {
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL;
    }

    /* First two bytes are big-endian uint16. CSBWin treats 0x8001
     * as the little-endian marker; the actual count follows it. */
    {
        uint16_t first = read_be16(bytes, 0u);
        if (first == (uint16_t)CSB_V1_CSBGRAPHICS_LITTLE_ENDIAN_MARKER) {
            /* Sentinel present — the real count starts at offset 2
             * and is little-endian. Need at least 4 bytes total. */
            if (size < 4u) {
                return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL;
            }
            byte_order = CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER;
            count = (uint32_t)read_le16(bytes, 2u);
        } else {
            count = (uint32_t)first;
        }
    }

    if (count == 0u) {
        /* CSBWin treats count==0 as a failed read and dies(42) —
         * we report a bad-count error so callers can reject empty
         * headers cleanly without invoking the die() path. */
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_COUNT;
    }
    if (count > CSB_V1_CSBGRAPHICS_MAX_COUNT) {
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_COUNT;
    }

    /* Two parallel uint16 tables of length `count`. Overflow-safe
     * arithmetic: tables fit when count <= CSB_V1_CSBGRAPHICS_MAX_COUNT
     * (8192 * 2 * 2 = 32768 bytes, well within uint64). */
    if (count > (UINT32_MAX / 4u)) {
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW;
    }
    table_bytes = (uint64_t)count * 4u;
    /* payload offset:
     *   - big-endian: 2 bytes header + tables
     *   - little-endian marker: 4 bytes header + tables
     */
    payload_offset = (byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                         ? 4u + table_bytes
                         : 2u + table_bytes;
    if (payload_offset > size) {
        /* Declared count needs more header+table bytes than the file
         * holds — semantically "too small", not a numeric overflow. */
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL;
    }
    payload_avail = size - payload_offset;

    /* Tables start right after the count word (or right after the
     * little-endian marker). Both tables are uint16 per entry. */
    {
        size_t comp_off = (byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                              ? 4u
                              : 2u;
        size_t deco_off = comp_off + (size_t)count * 2u;
        if (deco_off + (size_t)count * 2u > size) {
            return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL;
        }
        for (i = 0u; i < count; ++i) {
            uint16_t comp;
            uint16_t deco;
            if (byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER) {
                comp = read_le16(bytes, comp_off + (size_t)i * 2u);
                deco = read_le16(bytes, deco_off + (size_t)i * 2u);
            } else {
                comp = read_be16(bytes, comp_off + (size_t)i * 2u);
                deco = read_be16(bytes, deco_off + (size_t)i * 2u);
            }
            total_compressed += (uint64_t)comp;
            total_decompressed += (uint64_t)deco;
            if (comp > max_compressed) {
                max_compressed = comp;
            }
            if (deco > max_decompressed) {
                max_decompressed = deco;
            }
            /* Defensive overflow check: total_compressed must fit
             * in the file's payload region. CSBWin's documented
             * checksum loop uses uint16 sums only, so we tolerate
             * up to UINT16_MAX per entry. */
            if (total_compressed > payload_avail) {
                return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW;
            }
            if (total_decompressed > (uint64_t)UINT32_MAX * (uint64_t)count) {
                return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW;
            }
        }
    }

    out_index->byte_order = byte_order;
    out_index->count = count;
    out_index->total_compressed = total_compressed;
    out_index->total_decompressed = total_decompressed;
    out_index->payload_offset = payload_offset;
    out_index->payload_bytes_avail = payload_avail;
    out_index->max_compressed = max_compressed;
    out_index->max_decompressed = max_decompressed;
    return CSB_V1_CSBGRAPHICS_CLASSIFY_OK;
}

int csb_v1_csbgraphics_dat_entry_span(
    const uint8_t *bytes, size_t size, uint32_t entry_index,
    CSB_V1_CSBGraphicsEntrySpan *out_span)
{
    CSB_V1_CSBGraphicsIndex index;
    size_t comp_off;
    size_t deco_off;
    uint64_t entry_payload_offset;
    uint64_t preceding_compressed = 0u;
    uint32_t i;
    int rc;

    if (!out_span) {
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT;
    }
    memset(out_span, 0, sizeof(*out_span));

    rc = csb_v1_csbgraphics_dat_classify(bytes, size, &index);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK) {
        return rc;
    }
    if (entry_index >= index.count) {
        return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ENTRY_RANGE;
    }

    comp_off = (index.byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                   ? 4u
                   : 2u;
    deco_off = comp_off + (size_t)index.count * 2u;

    /* CSBWin/Graphics.cpp:1643 LocateNthGraphic(n) walks the
     * compressed-size table for all entries before n, then seeks
     * to payload_base + that accumulated count. Keep this helper
     * bounded and byte-span-only; the later LZW decode remains a
     * separate gate. */
    for (i = 0u; i < entry_index; ++i) {
        uint16_t comp = (index.byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                            ? read_le16(bytes, comp_off + (size_t)i * 2u)
                            : read_be16(bytes, comp_off + (size_t)i * 2u);
        preceding_compressed += (uint64_t)comp;
    }

    {
        uint16_t comp = (index.byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                            ? read_le16(bytes, comp_off + (size_t)entry_index * 2u)
                            : read_be16(bytes, comp_off + (size_t)entry_index * 2u);
        uint16_t deco = (index.byte_order == CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                            ? read_le16(bytes, deco_off + (size_t)entry_index * 2u)
                            : read_be16(bytes, deco_off + (size_t)entry_index * 2u);
        entry_payload_offset = index.payload_offset + preceding_compressed;
        if (entry_payload_offset > (uint64_t)size ||
            (uint64_t)comp > (uint64_t)size - entry_payload_offset) {
            return CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW;
        }

        out_span->entry_index = entry_index;
        out_span->payload_offset = entry_payload_offset;
        out_span->compressed_size = (uint32_t)comp;
        out_span->decompressed_size = (uint32_t)deco;
    }

    return CSB_V1_CSBGRAPHICS_CLASSIFY_OK;
}

const char *csb_v1_csbgraphics_dat_result_name(int result)
{
    switch (result) {
    case CSB_V1_CSBGRAPHICS_CLASSIFY_OK: return "OK";
    case CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT: return "argument";
    case CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL: return "too-small";
    case CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_COUNT: return "bad-count";
    case CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW: return "overflow";
    case CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_MARKER: return "bad-marker";
    case CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ENTRY_RANGE: return "entry-range";
    default: return "unknown";
    }
}

const char *csb_v1_csbgraphics_dat_byte_order_name(
    CSB_V1_CSBGraphicsByteOrder byte_order)
{
    switch (byte_order) {
    case CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN:
        return "big-endian";
    case CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER:
        return "little-endian-marker";
    default:
        return "unknown";
    }
}

const char *csb_v1_csbgraphics_dat_source_evidence(void)
{
    return
        "CSBWin/Graphics.cpp:1918 ReadGraphicsIndex\n"
        "CSBWin/Graphics.cpp:1643 LocateNthGraphic\n"
        "CSBWin/Graphics.cpp:1717 ReadGraphic\n"
        "CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile\n"
        "CSBWin/Graphics.cpp:1851 OPEN(\"CSBgraphics.dat\",\"rb\")\n"
        "CSBWin/data.cpp:1936 Signature file MD5 split as uint32 words\n"
        "ReDMCSB F0200_DMMISC_ReadCompressedGraphic + GRAPH21.C F0914\n"
        "greatstone d_items.html \"Graphics.dat file format\"\n"
        "dmweb Data Files page (graphics.dat layout)";
}
