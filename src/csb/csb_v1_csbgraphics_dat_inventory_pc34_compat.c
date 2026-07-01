/*
 * csb_v1_csbgraphics_dat_inventory_pc34_compat.c
 *
 * Bounded on-disk CSBWin "CSBgraphics.dat" custom-graphics shape
 * classifier + inventory walker. See
 * include/csb_v1_csbgraphics_dat_inventory_pc34_compat.h for
 * scope and source references.
 *
 * Implementation notes:
 *   - The shape classifier walks at most the first 8 bytes of
 *     the buffer (magic detection); for the CSBgraphics.dat
 *     verdict it delegates to csb_v1_csbgraphics_dat_classify()
 *     which already owns the byte-precise contract.
 *   - The inventory walker uses only the parsed parallel size
 *     tables; it never reads payload bytes, never invokes an
 *     LZW / RLE decoder, never binds to M11 / M12.
 *   - end_aligned reports whether the concatenated payloads
 *     consume exactly the bytes available past `payload_offset`
 *     (the strict CSBWin contract). Files that reserve trailing
 *     bytes for the per-file MD5 signature documented at
 *     CSBWin/data.cpp:1936 surface as end_aligned=0 with
 *     payload_tail_bytes > 0; the walker does NOT try to read
 *     the signature itself — that lives in the real-scan module.
 */

#include "csb_v1_csbgraphics_dat_inventory_pc34_compat.h"

#include <string.h>

/* ── Magic constants ─────────────────────────────────────────────── */

#define CSB_V1_CSBGRAPHICS_MAGIC_LEN 8u
#define CSB_V1_CSBGRAPHICS_DM1_RAW_MAGIC  "RDMCSB15"
#define CSB_V1_CSBGRAPHICS_CSB_SAVE_MAGIC "CSBGAME\0"

/* ── Shape classification ────────────────────────────────────────── */

CSB_V1_CSBGraphicsShape csb_v1_csbgraphics_dat_shape_classify(
    const uint8_t *bytes, size_t size)
{
    if (!bytes || size < CSB_V1_CSBGRAPHICS_FILE_MIN_BYTES) {
        return CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL;
    }

    /* 8-byte magic match: DM1 raw save. */
    if (size >= CSB_V1_CSBGRAPHICS_MAGIC_LEN &&
        memcmp(bytes, CSB_V1_CSBGRAPHICS_DM1_RAW_MAGIC,
               CSB_V1_CSBGRAPHICS_MAGIC_LEN) == 0) {
        return CSB_V1_CSBGRAPHICS_SHAPE_DM1_RAW_RDMCSB15;
    }

    /* 8-byte magic match: CSB v2.0 / v2.1 save. */
    if (size >= CSB_V1_CSBGRAPHICS_MAGIC_LEN &&
        memcmp(bytes, CSB_V1_CSBGRAPHICS_CSB_SAVE_MAGIC,
               CSB_V1_CSBGRAPHICS_MAGIC_LEN) == 0) {
        return CSB_V1_CSBGRAPHICS_SHAPE_CSB_SAVE_CSBGAME;
    }

    /* CSBWin 512-byte XOR-pad header: 4-byte magic literals
     * documented at CSBWin/SaveGame.cpp:927 / CSBWin/Data.h:590. */
    if (size >= 4u) {
        if (bytes[0] == 'C' && bytes[1] == 'S' && bytes[2] == 'B' &&
            bytes[3] == 0x01u) {
            return CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CSB1;
        }
        if (bytes[0] == 'D' && bytes[1] == 'M' &&
            bytes[2] == 0x00u && bytes[3] == 0x01u) {
            return CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_DM01;
        }
        if (bytes[0] == 'C' && bytes[1] == 'E' &&
            bytes[2] == 'D' && bytes[3] == 'T') {
            return CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CEDT;
        }
    }

    /* Fallback: delegate to the existing byte-precise classifier.
     * The classify() call already rejects too-small / oversized
     * count / overflow / non-table-shaped buffers. */
    {
        CSB_V1_CSBGraphicsIndex tmp;
        int rc = csb_v1_csbgraphics_dat_classify(bytes, size, &tmp);
        if (rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK) {
            return CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS;
        }
    }

    return CSB_V1_CSBGRAPHICS_SHAPE_UNKNOWN;
}

const char *csb_v1_csbgraphics_dat_shape_name(
    CSB_V1_CSBGraphicsShape shape)
{
    switch (shape) {
    case CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS:      return "csbgraphics";
    case CSB_V1_CSBGRAPHICS_SHAPE_DM1_RAW_RDMCSB15: return "dm1_raw_rdmcsb15";
    case CSB_V1_CSBGRAPHICS_SHAPE_CSB_SAVE_CSBGAME: return "csb_save_csbgame";
    case CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CSB1:  return "csbwin_512_csb1";
    case CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_DM01:  return "csbwin_512_dm01";
    case CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CEDT:  return "csbwin_512_cedt";
    case CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL:        return "too_small";
    case CSB_V1_CSBGRAPHICS_SHAPE_UNKNOWN:          return "unknown";
    case CSB_V1_CSBGRAPHICS_SHAPE_COUNT:            return "shape_count";
    default:                                        return "unknown";
    }
}

/* ── Inventory walker ────────────────────────────────────────────── */

int csb_v1_csbgraphics_dat_inventory(
    const CSB_V1_CSBGraphicsIndex *index,
    CSB_V1_CSBGraphicsInventory *out_inventory)
{
    if (!index || !out_inventory) {
        return CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT;
    }
    memset(out_inventory, 0, sizeof(*out_inventory));

    out_inventory->count              = index->count;
    out_inventory->max_compressed     = index->max_compressed;
    out_inventory->max_decompressed   = index->max_decompressed;
    out_inventory->total_compressed   = index->total_compressed;
    out_inventory->total_decompressed = index->total_decompressed;
    out_inventory->payload_offset     = index->payload_offset;
    out_inventory->payload_used       = index->total_compressed;
    out_inventory->payload_avail      = index->payload_bytes_avail;
    if (index->payload_bytes_avail >= index->total_compressed) {
        out_inventory->payload_tail_bytes =
            index->payload_bytes_avail - index->total_compressed;
    } else {
        /* The classifier already rejected this case as OVERFLOW;
         * the walker reports tail=0 to keep the contract sane. */
        out_inventory->payload_tail_bytes = 0u;
    }
    out_inventory->end_aligned =
        (out_inventory->payload_tail_bytes == 0u) ? 1 : 0;

    /* The per-entry sparse / dense / zero-length / identical
     * counters require a second pass over the size tables and
     * therefore need the raw byte buffer. The index-only walker
     * above surfaces the fields the parsed index alone can prove;
     * the richer per-entry inventory lives in
     * csb_v1_csbgraphics_dat_inventory_from_bytes() below. */

    return CSB_V1_CSBGRAPHICS_INVENTORY_OK;
}

int csb_v1_csbgraphics_dat_inventory_from_bytes(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBGraphicsIndex *out_index,
    CSB_V1_CSBGraphicsInventory *out_inventory)
{
    CSB_V1_CSBGraphicsIndex index;
    int rc;

    if (!bytes || !out_index || !out_inventory) {
        return CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT;
    }
    memset(out_index, 0, sizeof(*out_index));
    memset(out_inventory, 0, sizeof(*out_inventory));

    rc = csb_v1_csbgraphics_dat_classify(bytes, size, &index);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK) {
        return CSB_V1_CSBGRAPHICS_INVENTORY_ERR_CLASSIFY;
    }

    /* Walk the size tables once to populate sparse / dense /
     * zero-length / identical counters. The byte order matches
     * the classify() result. */
    {
        size_t comp_off = (index.byte_order ==
                           CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                              ? 4u
                              : 2u;
        size_t deco_off = comp_off + (size_t)index.count * 2u;
        uint32_t i;
        if (deco_off + (size_t)index.count * 2u > size) {
            return CSB_V1_CSBGRAPHICS_INVENTORY_ERR_CLASSIFY;
        }
        for (i = 0u; i < index.count; ++i) {
            uint16_t comp, deco;
            if (index.byte_order ==
                CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER) {
                comp = (uint16_t)((uint16_t)bytes[comp_off + i * 2u] |
                                  ((uint16_t)bytes[comp_off + i * 2u + 1u] << 8));
                deco = (uint16_t)((uint16_t)bytes[deco_off + i * 2u] |
                                  ((uint16_t)bytes[deco_off + i * 2u + 1u] << 8));
            } else {
                comp = (uint16_t)(((uint16_t)bytes[comp_off + i * 2u] << 8) |
                                  (uint16_t)bytes[comp_off + i * 2u + 1u]);
                deco = (uint16_t)(((uint16_t)bytes[deco_off + i * 2u] << 8) |
                                  (uint16_t)bytes[deco_off + i * 2u + 1u]);
            }
            if (comp == 0u && deco == 0u) {
                ++out_inventory->zero_length_count;
            } else if (comp == 0u) {
                ++out_inventory->sparse_count;
            } else {
                ++out_inventory->dense_count;
            }
            if (comp != 0u && comp == deco) {
                ++out_inventory->identical_count;
            }
        }
    }

    *out_index = index;
    return csb_v1_csbgraphics_dat_inventory(&index, out_inventory);
}

const char *csb_v1_csbgraphics_dat_inventory_source_evidence(void)
{
    return
        "CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile\n"
        "CSBWin/Graphics.cpp:1851 OPEN(\"CSBgraphics.dat\",\"rb\")\n"
        "CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count + size tables, 0x8001 LE sentinel)\n"
        "CSBWin/Graphics.cpp:1643 LocateNthGraphic (payload offset rule)\n"
        "CSBWin/SaveGame.cpp:927/1711/2111 (CSBWin 512-byte XOR header magics)\n"
        "CSBWin/Data.h:590 (SaveGameFilename CEDT magic)\n"
        "CSBWin/CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals)\n"
        "CSBWin/data.cpp:1936 Signature (file MD5 split as signature1/signature2)\n"
        "ReDMCSB F0200_DMMISC_ReadCompressedGraphic + GRAPH21.C F0914\n"
        "ReDMCSB CEDTINC8.C:101-118 (DMSAVE / CSBGAME.DAT routing)\n"
        "ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic)\n"
        "greatstone d_items.html \"Graphics.dat file format\"\n"
        "dmweb Data Files page (graphics.dat layout)\n"
        "include/memory_savegame_pc34_compat.h:227 (RDMCSB15 magic)\n"
        "include/csb_v1_csbgraphics_dat_classify.h (count + size tables contract)\n"
        "include/csb_v1_csbwin_save_loader_boundary_pc34_compat.h (save-shape contract)\n"
        "docs/FIRESTAFF_GAP_LIST.md row CSBWin custom resource handling (csbgraphics.dat + dmsave + csbgame)";
}
