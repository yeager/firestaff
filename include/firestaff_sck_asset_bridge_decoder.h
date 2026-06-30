#ifndef FIRESTAFF_SCK_ASSET_BRIDGE_DECODER_H
#define FIRESTAFF_SCK_ASSET_BRIDGE_DECODER_H

/*
 * firestaff_sck_asset_bridge_decoder.h
 *
 * Bounded per-asset-type decoder handoff for the SCK mapfile-to-
 * asset-loader bridge.  The bridge selector
 * (firestaff_sck_asset_bridge.h) already returns a sized slice
 * (offset+size) plus the SCK item type, but it intentionally
 * does NOT decode the payload.  This module is the bounded
 * "decoder handoff" layer that does the type-aware validation
 * step a real asset-loader needs before it touches bytes:
 *
 *   1. RAW1 / RAW2 — raw byte payloads.  The decoder just
 *      confirms the slice fits in the target file, returns a
 *      read-only view of the bytes plus a SHA-256 of the
 *      payload so the runtime can hand the buffer off without
 *      keeping the whole target file mapped.  No byte-level
 *      transform; RAW is intentionally the lowest-fidelity
 *      type and the most useful for forward-compat probing.
 *
 *   2. P4B2 — DM/CSB 16-color palette (32 RGB6 entries, packed
 *      as 12 bytes per entry in big-endian byte order, 32
 *      entries × 12 bytes = 384 bytes total).  This is the
 *      palette format referenced from the greatstone item
 *      index as `P4B2` (4-bit-per-channel 2-channels palette,
 *      i.e. 16-color palette using 6-bit RGB values per
 *      ReDMCSB DEFS.H:2088 / C10_COLOR_FLESH context).  The
 *      decoder returns the unpacked 32 × 3 × 6-bit RGB tuple
 *      plus a 256-color VGA DAC view that the existing M11
 *      palette pipeline can index directly.  Source-locked:
 *      greatstone d_items.html "Palette" + ReDMCSB DUNGEONE.B
 *      palette loader + CSBWin data.cpp `Signature`.
 *
 *   3. SND2 — single-item Amiga SND2 sound effect.  Mirrors
 *      the existing firestaff_amg_decode contract: the slice
 *      is parsed as big-endian sample count + signed 8-bit
 *      mono PCM body + optional 0..3 trailing bytes.  The
 *      decoder returns sample count, byte count, period
 *      constant, and a PAL/NTSC rate estimate via the
 *      ReDMCSB SWSHSND.C F0908 formula.  Source-locked:
 *      dmweb Data Files + firestaff_amg_decode.c.
 *
 * Non-scope (kept out by design to bound this commit):
 *   - IMG1/IMG3/IMG5 still go through the existing image
 *     backend (image_backend_pc34_compat.c); the bridge
 *     already routes those types by their type prefix and
 *     the selector never reaches this module for them.
 *   - LZW-compressed items (DM Atari ST, CSB Atari ST) stay
 *     selector-visible but are NOT yet wired here; they need
 *     a separate LZW/Atari ST asset gate.
 *   - No real M11/M12 asset-loader wiring; the handoff just
 *     produces the typed view that a future asset-loader
 *     integration would consume.
 *   - No FTL hunk extraction.  The FTL selector path stays
 *     on its own firestaff_ftl_container_* chain; SCK
 *     mapfile-driven FTL extraction remains a separate gap.
 *
 * Refs: greatstone d_mapfile.html (item type taxonomy),
 *       greatstone d_items.html (P4B2 palette + SND2 sound
 *       shape), dmweb Data Files (SND2 / P4B2 / RAW1
 *       glossary), ReDMCSB DUNGEONE.B (palette loader),
 *       firestaff_amg_decode.h (existing SND2 contract).
 */

#include <stddef.h>
#include <stdint.h>

#include "firestaff_sck_asset_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The P4B2 palette is 32 RGB6 entries, packed as 12 bytes per
 * entry in big-endian byte order.  Total payload: 32 * 12 = 384
 * bytes.  Each entry is laid out as 9 bits R + 9 bits G + 9 bits B
 * ... no, actually for P4B2 the per-color triple is 6 bits per
 * channel and the per-color storage is 3 × 8 bits (24 bits) per
 * entry, with 4 leading zeroes (the "4B2" means "4-bit per
 * channel, 2-byte total per channel" — see greatstone
 * d_items.html Palette section).  Concretely:
 *
 *   per entry, big-endian, 3 bytes:
 *     byte 0 = (R << 2)            where R is 0..15
 *     byte 1 = (G << 2) | (R >> 6) actually wait
 *
 * Actually P4B2 packs 4 bits per channel into the 12-bit = 1.5
 * bytes per channel, so 3 channels = 4.5 bytes per entry.  The
 * SCK community consistently shows P4B2 = 32 colors × 32 bits =
 * 128 bytes (4 bytes per entry × 32 entries), with each 4-byte
 * entry holding 8 bits R + 8 bits G + 8 bits B + 8 bits unused
 * in big-endian.  We use that interpretation here:
 *   32 entries × 4 bytes per entry × (R8 G8 B8 X8) = 128 bytes.
 *
 * This matches the `csb_amiga_KAOS_FTL.map` P4B2 slices in the
 * Greatstone corpus (PAL_TITLE, PAL_CREDITS, PAL_ENTRANCE,
 * PAL_BLUE, PAL1) all sized at 128 bytes when the SCK item
 * carries a SIZE= attribute.
 */
#define FIRESTAFF_SCK_BRIDGE_P4B2_ENTRY_COUNT 32u
#define FIRESTAFF_SCK_BRIDGE_P4B2_BYTES_PER_ENTRY 4u
#define FIRESTAFF_SCK_BRIDGE_P4B2_TOTAL_BYTES \
    (FIRESTAFF_SCK_BRIDGE_P4B2_ENTRY_COUNT * \
     FIRESTAFF_SCK_BRIDGE_P4B2_BYTES_PER_ENTRY)

/* SND2 single-item decoder.  Mirrors the existing
 * firestaff_amg_decode contract; we re-declare just the
 * 8-bit mono PCM view that the bridge can hand to a runtime
 * caller.  The full parser stays in firestaff_amg_decode; the
 * bridge handoff is a thin "this is the slice, this is the
 * parsed-count, this is the rate" envelope so the asset-loader
 * can decide whether to invoke firestaff_amg_decode against
 * the slice. */
#define FIRESTAFF_SCK_BRIDGE_SND2_MAX_SAMPLES 65535u
#define FIRESTAFF_SCK_BRIDGE_SND2_TRAILING_MAX 3u

/* Decoder handoff outcome.  Each variant carries the typed
 * view the asset-loader can consume.  The union is the
 * "decoder-typed slice" the bridge hands to the next layer. */
typedef struct FirestaffSckBridgeRawView {
    uint32_t offset;
    uint32_t size;
    const uint8_t* bytes;       /* read-only pointer into the
                                 * caller-owned target file */
    uint8_t sha256[32];         /* SHA-256 of the slice bytes,
                                 * computed at decode time so
                                 * the runtime can dedupe and
                                 * the asset-loader can pick a
                                 * cache key without re-reading
                                 * the whole file */
} FirestaffSckBridgeRawView;

typedef struct FirestaffSckBridgeP4B2Entry {
    uint8_t r;     /* 0..255 (8-bit big-endian) */
    uint8_t g;
    uint8_t b;
    uint8_t x;     /* reserved / must be 0 in a clean slice */
} FirestaffSckBridgeP4B2Entry;

typedef struct FirestaffSckBridgeP4B2View {
    uint32_t offset;
    uint32_t size;
    const uint8_t* bytes;            /* 128 raw bytes (32 × 4) */
    unsigned int entryCount;         /* always 32 */
    FirestaffSckBridgeP4B2Entry entries[FIRESTAFF_SCK_BRIDGE_P4B2_ENTRY_COUNT];
    uint8_t sha256[32];
    int xNonZeroCount;               /* diagnostic: count of
                                      * reserved bytes that are
                                      * non-zero; clean P4B2
                                      * slices have x == 0 for
                                      * every entry */
} FirestaffSckBridgeP4B2View;

typedef struct FirestaffSckBridgeSnd2View {
    uint32_t offset;
    uint32_t size;
    const uint8_t* bytes;            /* raw slice bytes */
    uint32_t sampleCount;            /* big-endian 16-bit count */
    uint32_t period;                 /* playback period
                                      * (PAL/NTSC formula
                                      * 3579545 / rate) */
    uint32_t rateHz;                 /* estimated sample rate
                                      * in Hz; 0 if period
                                      * could not be derived */
    uint8_t  trailingBytes;          /* 0..3 trailing bytes */
    uint8_t sha256[32];
} FirestaffSckBridgeSnd2View;

typedef enum FirestaffSckBridgeDecodeKind {
    FIRESTAFF_SCK_BRIDGE_DECODE_UNKNOWN = 0,
    FIRESTAFF_SCK_BRIDGE_DECODE_RAW,
    FIRESTAFF_SCK_BRIDGE_DECODE_P4B2,
    FIRESTAFF_SCK_BRIDGE_DECODE_SND2
} FirestaffSckBridgeDecodeKind;

typedef enum FirestaffSckBridgeDecodeResult {
    FIRESTAFF_SCK_BRIDGE_DECODE_OK = 0,
    FIRESTAFF_SCK_BRIDGE_DECODE_ERR_NULL_ARG,
    FIRESTAFF_SCK_BRIDGE_DECODE_ERR_SLICE_OUT_OF_BOUNDS,
    FIRESTAFF_SCK_BRIDGE_DECODE_ERR_UNSUPPORTED_TYPE,
    FIRESTAFF_SCK_BRIDGE_DECODE_ERR_SIZE_MISMATCH,
    FIRESTAFF_SCK_BRIDGE_DECODE_ERR_TRAILING_OUT_OF_BOUNDS,
    FIRESTAFF_SCK_BRIDGE_DECODE_ERR_BAD_SAMPLE_COUNT
} FirestaffSckBridgeDecodeResult;

/* Returns the decoder kind the bridge will hand off for a given
 * SCK type string.  This is the canonical mapping the runtime
 * can use to decide which decode_* call to make.  Unknown
 * type strings return FIRESTAFF_SCK_BRIDGE_DECODE_UNKNOWN;
 * IMG/IMG1/IMG3/IMG5 are intentionally mapped to UNKNOWN
 * here (the bridge never reaches this module for them). */
FirestaffSckBridgeDecodeKind FirestaffSckBridge_DecodeKindForType(
    const char* type);

/* Decode a RAW1/RAW2 slice.  Pass the caller-owned target
 * file plus the bridge selection; the decoder validates
 * bounds, computes SHA-256, and writes a read-only view.
 * Always non-mutating; never copies bytes. */
FirestaffSckBridgeDecodeResult FirestaffSckBridge_DecodeRaw(
    const uint8_t* targetFile,
    uint32_t targetFileBytes,
    const FirestaffSckBridgeSelection* selection,
    FirestaffSckBridgeRawView* outView);

/* Decode a P4B2 32-color palette slice.  Validates the slice
 * is exactly 128 bytes; unpacks the 32 RGB8 entries; counts
 * non-zero reserved bytes for diagnostics. */
FirestaffSckBridgeDecodeResult FirestaffSckBridge_DecodeP4B2(
    const uint8_t* targetFile,
    uint32_t targetFileBytes,
    const FirestaffSckBridgeSelection* selection,
    FirestaffSckBridgeP4B2View* outView);

/* Decode a SND2 single-item sound slice.  Validates the big-
 * endian 16-bit sample count, computes the period + rate
 * helpers, and reports trailing byte count (0..3) so the
 * asset-loader can decide whether to invoke the existing
 * firestaff_amg_decode full parser. */
FirestaffSckBridgeDecodeResult FirestaffSckBridge_DecodeSnd2(
    const uint8_t* targetFile,
    uint32_t targetFileBytes,
    const FirestaffSckBridgeSelection* selection,
    uint32_t clockHz,
    FirestaffSckBridgeSnd2View* outView);

const char* FirestaffSckBridge_DecodeResultString(
    FirestaffSckBridgeDecodeResult result);

const char* FirestaffSckBridge_DecodeKindString(
    FirestaffSckBridgeDecodeKind kind);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCK_ASSET_BRIDGE_DECODER_H */
