/*
 * csb_v1_csbgraphics_dat_inventory_pc34_compat.h
 *
 * Bounded on-disk CSBWin "CSBgraphics.dat" custom-graphics
 * shape classifier + inventory walker.
 *
 * Closes a narrow slice of docs/FIRESTAFF_GAP_LIST.md row
 * "CSBWin custom resource handling (csbgraphics.dat + dmsave +
 * csbgame)": the existing modules
 *
 *   - csb_v1_csbgraphics_dat_classify  (count + size tables
 *     contract, CSBWin/Graphics.cpp:1918 ReadGraphicsIndex)
 *   - csb_v1_csbgraphics_dat_entry_span (one entry's compressed
 *     payload span via CSBWin/Graphics.cpp:1643 LocateNthGraphic)
 *   - csb_v1_csbgraphics_dat_real_scan  (hash-based discovery
 *     + cache handoff)
 *
 * already prove the file-shape contract for a CSBWin-produced
 * CSBgraphics.dat. This module adds the two missing pieces
 * CSBWin's own tooling expects from its custom-resource surface:
 *
 *   1. A bounded on-disk *shape* classifier — a one-shot verdict
 *      that distinguishes "this looks like a CSBgraphics.dat
 *      header" from "this is one of the other CSBWin / DM1 / CSB
 *      raw asset shapes a launcher or M11 might receive". No
 *      LZW decode, no payload override, no launcher / M11 wiring.
 *      This is the same contract the sibling save-side module
 *      `csb_v1_csbwin_save_classify_pc34_compat` provides for
 *      dmsave / csbgame (gapbug_20260626 lane), expressed here
 *      for csbgraphics.dat.
 *
 *   2. A bounded inventory walker — given a parsed
 *      `CSB_V1_CSBGraphicsIndex`, walks every entry and produces
 *      a deterministic inventory report (count, sparse / dense /
 *      zero-length counts, total payload used, payload tail
 *      bytes, end-aligned invariant vs the file size). The
 *      inventory lets the future M11 override hook ask "is this
 *      override slot inside the payload region and within the
 *      documented CSBWin decompressed-budget?" without re-walking
 *      the parallel size tables itself.
 *
 * Source references:
 *   - CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile
 *   - CSBWin/Graphics.cpp:1851 OPEN("CSBgraphics.dat","rb")
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count +
 *     parallel compressed/decompressed size tables, optional
 *     0x8001 little-endian sentinel)
 *   - CSBWin/Graphics.cpp:1643 LocateNthGraphic (offset =
 *     2 [+2 if LE marker] + NumGraphic*4 + sum(comp[0..n-1]))
 *   - CSBWin/data.cpp:1936 Signature (file MD5 split into
 *     signature1/signature2 uint32 words; this module does NOT
 *     re-derive the signature — the real-scan module owns it)
 *   - ReDMCSB F0200_DMMISC_ReadCompressedGraphic + GRAPH21.C F0914
 *   - greatstone d_items.html "Graphics.dat file format"
 *   - dmweb Data Files page (graphics.dat layout)
 *
 * Non-claims:
 *   - No LZW / RLE decompression.
 *   - No payload decode.
 *   - No M11 viewport override hook.
 *   - No CSBWin custom-dungeon support claim — that remains
 *     tracked under docs/FIRESTAFF_GAP_LIST.md row C3 / A3
 *     "CSBWin custom resource handling" (OPEN-LARGE).
 */

#ifndef FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_INVENTORY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_INVENTORY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_dat_classify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── On-disk shape classification ────────────────────────────────────
 *
 * CSBWin / DM1 / CSBLauncher surface produces several distinct
 * raw-asset byte shapes. A launcher that wants to drop a file
 * into ~/.firestaff/data/csbwin-custom/ needs a one-shot verdict
 * that distinguishes CSBgraphics.dat from the other shapes
 * without invoking any decoder — exactly the contract the
 * sibling save-side csb_v1_csbwin_save_classify_pc34_compat
 * already provides for csbgame.dat / csbgame.bak / dmsave.dat /
 * dmsave.bak.
 *
 * The list is intentionally narrow: it covers the shapes the
 * CSBWin / DM1 / CSB V1 docs and tools we already cite actually
 * produce today. Future shapes (CSBWin 512-byte XOR-pad header,
 * etc.) extend it.
 */
typedef enum {
    /* File header matches the CSBgraphics.dat contract: 2-byte
     * big-endian count or 4-byte 0x8001+count little-endian
     * marker, then two parallel uint16 size tables. This is the
     * shape csb_v1_csbgraphics_dat_classify() recognises. */
    CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS = 0,
    /* File starts with "RDMCSB15" — a DM1 PC 3.4 raw save, not
     * a CSBgraphics.dat. */
    CSB_V1_CSBGRAPHICS_SHAPE_DM1_RAW_RDMCSB15 = 1,
    /* File starts with "CSBGAME\0" — a CSB v2.0 / v2.1 save,
     * not a CSBgraphics.dat. */
    CSB_V1_CSBGRAPHICS_SHAPE_CSB_SAVE_CSBGAME = 2,
    /* File starts with the 4-byte CSBWin 512-byte XOR header
     * ("CSB\1", "DM\0\1", "CEDT"). The 512-byte obfuscated
     * payload is not decoded here — the shape detector only
     * recognises the magic. */
    CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CSB1 = 3,
    CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_DM01 = 4,
    CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CEDT = 5,
    /* File < 4 bytes — too small to classify. */
    CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL = 6,
    /* File >= 4 bytes but no recognised magic / contract header. */
    CSB_V1_CSBGRAPHICS_SHAPE_UNKNOWN = 7,

    CSB_V1_CSBGRAPHICS_SHAPE_COUNT
} CSB_V1_CSBGraphicsShape;

/* Result codes (mirror the classify module's contract style). */
typedef enum {
    CSB_V1_CSBGRAPHICS_INVENTORY_OK = 0,
    CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT = -1,
    CSB_V1_CSBGRAPHICS_INVENTORY_ERR_CLASSIFY = -2
} CSB_V1_CSBGraphicsInventoryResult;

/* One-shot shape verdict for an in-memory byte buffer. Returns
 * the matching CSB_V1_CSBGraphicsShape enum value. The detector
 * is read-only — it never invokes a payload decoder, never
 * touches the file system, and never binds to M11 / M12.
 *
 * Bounds:
 *   - size < 4 ⇒ CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL
 *   - first 8 bytes == "RDMCSB15" ⇒ DM1 raw save shape
 *   - first 8 bytes == "CSBGAME\0" ⇒ CSB v2.0 / v2.1 save
 *     shape (the detector does NOT distinguish v2.0 from v2.1
 *     here; both carry the same magic and the version word lives
 *     past the magic)
 *   - first 4 bytes == "CSB\1" / "DM\0\1" / "CEDT" ⇒ CSBWin
 *     512-byte XOR-pad header shape
 *   - first 2 bytes == 0x8001 little-endian marker OR first 2
 *     bytes == a plausible uint16 count followed by a valid
 *     CSBgraphics.dat size-table layout ⇒ CSBgraphics.dat shape
 *     (delegates to csb_v1_csbgraphics_dat_classify() for the
 *     final accept/reject verdict)
 *   - everything else ⇒ UNKNOWN
 */
CSB_V1_CSBGraphicsShape csb_v1_csbgraphics_dat_shape_classify(
    const uint8_t *bytes, size_t size);

/* Human-readable label for a CSB_V1_CSBGraphicsShape value. */
const char *csb_v1_csbgraphics_dat_shape_name(
    CSB_V1_CSBGraphicsShape shape);

/* ── Inventory walker ─────────────────────────────────────────────── */

/* Deterministic inventory report for one parsed CSBgraphics.dat
 * index. The walker is read-only and only consults the parallel
 * size tables; it does NOT decompress any payload. It is the
 * shape a future M11 override hook can consume to decide which
 * override slots are real overrides vs zero-length placeholder
 * slots, how much payload is consumed, and whether the file ends
 * cleanly inside the payload region. */
typedef struct {
    uint32_t count;                   /* total entries               */
    uint32_t sparse_count;            /* comp == 0  (deco may be 0)   */
    uint32_t dense_count;             /* comp >  0                   */
    uint32_t zero_length_count;       /* comp == 0 AND deco == 0      */
    uint32_t identical_count;         /* comp == deco (no inflation)  */
    uint32_t max_compressed;          /* largest single compressed    */
    uint32_t max_decompressed;        /* largest single decompressed  */
    uint64_t total_compressed;        /* sum of comp entries          */
    uint64_t total_decompressed;      /* sum of deco entries          */
    uint64_t payload_offset;          /* where the payload starts     */
    uint64_t payload_used;            /* sum(comp)                    */
    uint64_t payload_avail;           /* file_size - payload_offset   */
    uint64_t payload_tail_bytes;      /* payload_avail - payload_used */
    int      end_aligned;             /* payload_used == payload_avail */
} CSB_V1_CSBGraphicsInventory;

/* Walk the parsed `index` and produce a deterministic inventory
 * report in `out_inventory`. Returns CSB_V1_CSBGRAPHICS_INVENTORY_OK
 * on success, or a negative CSB_V1_CSBGraphicsInventoryResult.
 *
 * The walker uses only the parsed parallel size tables — no
 * second pass over the file. end_aligned is 1 when every byte
 * past `payload_offset` is consumed by the concatenated payloads
 * (the strict CSBWin contract); 0 when there is padding between
 * the last payload byte and the file end (an "extended-payload"
 * file that reserves trailing bytes for the per-file signature
 * word documented at CSBWin/data.cpp:1936 — the walker reports
 * the gap honestly without trying to interpret it).
 */
int csb_v1_csbgraphics_dat_inventory(
    const CSB_V1_CSBGraphicsIndex *index,
    CSB_V1_CSBGraphicsInventory *out_inventory);

/* Convenience: classify + inventory in one call. Returns
 * CSB_V1_CSBGRAPHICS_INVENTORY_OK when the buffer passes the
 * csbgraphics.dat classify contract; otherwise returns the
 * classify error code and leaves out_inventory zeroed.
 *
 * The caller still owns the byte buffer; this helper does not
 * allocate. */
int csb_v1_csbgraphics_dat_inventory_from_bytes(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBGraphicsIndex *out_index,
    CSB_V1_CSBGraphicsInventory *out_inventory);

/* Source-evidence citation chain (tests + docs quote this). */
const char *csb_v1_csbgraphics_dat_inventory_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_INVENTORY_PC34_COMPAT_H */
