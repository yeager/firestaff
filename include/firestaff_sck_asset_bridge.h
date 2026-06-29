#ifndef FIRESTAFF_SCK_ASSET_BRIDGE_H
#define FIRESTAFF_SCK_ASSET_BRIDGE_H

/*
 * firestaff_sck_asset_bridge.h
 *
 * Bounded bridge between Greatstone/SCK mapfile metadata
 * (`_mapping.xml` index + per-file `.map` slices) and the
 * Firestaff asset-loader/runtime selection path.
 *
 * Scope:
 *   - Parse the `_mapping.xml` index produced by sck into a
 *     bounded list of (md5, path, file) rows.
 *   - Look up an index row by MD5 signature + filename match.
 *   - Load a referenced `.map` and validate bounds against a
 *     real target file (caller-owned blob).
 *   - Select a single concrete asset slice by SCK item number
 *     (or by description substring) and hand the offset/size
 *     to the runtime loader.
 *
 * Non-SCOPE (kept out by design to bound this commit):
 *   - Item formats without `SIZE=` attributes are recognized
 *     but never selected as runtime slices.
 *   - Item types that lack a Firestaff-side decoder (anything
 *     other than the IMG family's existing backend and the RAW
 *     identity handoff below) are filtered out by selector helpers.
 *   - No real network/Greatstone download happens inside this
 *     module; callers pass already-fetched text in.
 *
 * Refs: greatstone `d_mapfile.html` (mapfile 2.x spec),
 * `_mapping.xml` index shape bundled with `sck.jar@1.5.1`.
 */

#include <stddef.h>
#include <stdint.h>

#include "firestaff_sck_mapfile.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_SCK_BRIDGE_MAX_ROWS 256u
#define FIRESTAFF_SCK_BRIDGE_MAX_GAMES 32u
#define FIRESTAFF_SCK_BRIDGE_MD5_BYTES 33u
#define FIRESTAFF_SCK_BRIDGE_PATH_BYTES 128u
#define FIRESTAFF_SCK_BRIDGE_FILE_BYTES 64u
#define FIRESTAFF_SCK_BRIDGE_GAME_BYTES 64u

typedef struct FirestaffSckBridgeMappingGame {
    char id[FIRESTAFF_SCK_BRIDGE_GAME_BYTES];
} FirestaffSckBridgeMappingGame;

typedef struct FirestaffSckBridgeMappingRow {
    char md5[FIRESTAFF_SCK_BRIDGE_MD5_BYTES];
    char path[FIRESTAFF_SCK_BRIDGE_PATH_BYTES];
    char file[FIRESTAFF_SCK_BRIDGE_FILE_BYTES];
    FirestaffSckBridgeMappingGame games[FIRESTAFF_SCK_BRIDGE_MAX_GAMES];
    unsigned int gameCount;
} FirestaffSckBridgeMappingRow;

typedef struct FirestaffSckBridgeMapping {
    unsigned int rowCount;
    FirestaffSckBridgeMappingRow rows[FIRESTAFF_SCK_BRIDGE_MAX_ROWS];
} FirestaffSckBridgeMapping;

/* Result codes for the bridge API.  Non-zero values indicate a
 * non-recoverable caller-visible failure; 0 means OK. */
typedef enum FirestaffSckBridgeResult {
    FIRESTAFF_SCK_BRIDGE_OK = 0,
    FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG,
    FIRESTAFF_SCK_BRIDGE_ERR_BAD_XML,
    FIRESTAFF_SCK_BRIDGE_ERR_TOO_MANY_ROWS,
    FIRESTAFF_SCK_BRIDGE_ERR_TOO_MANY_GAMES,
    FIRESTAFF_SCK_BRIDGE_ERR_FIELD_TOO_LONG,
    FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND,
    FIRESTAFF_SCK_BRIDGE_ERR_MAPFILE_PARSE,
    FIRESTAFF_SCK_BRIDGE_ERR_SLICE_OUT_OF_BOUNDS,
    FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED,
    FIRESTAFF_SCK_BRIDGE_ERR_UNSUPPORTED_DECODER
} FirestaffSckBridgeResult;

/* Parse a Greatstone/SCK `_mapping.xml` text blob into rows.
 * The parser is intentionally bounded: it only reads `<map>` top-level
 * elements and a flat set of `<game id="..."/>` children.  Unknown
 * elements are ignored.  Comments (`<!-- ... -->`) are skipped.
 * Returns OK or one of the FIRESTAFF_SCK_BRIDGE_ERR_* codes. */
FirestaffSckBridgeResult FirestaffSckBridge_ParseMappingXml(
    const char* xmlText,
    FirestaffSckBridgeMapping* outMapping);

/* Look up the first mapping row whose MD5 matches `md5Hex` AND whose
 * `file` attribute matches `file` (case-insensitive, exact match).
 * Pass an empty or NULL `md5Hex` to match by filename only; pass an
 * empty or NULL `file` to match by MD5 only.
 * Returns OK with `*outRow` set on success, or NOT_FOUND. */
FirestaffSckBridgeResult FirestaffSckBridge_Lookup(
    const FirestaffSckBridgeMapping* mapping,
    const char* md5Hex,
    const char* file,
    const FirestaffSckBridgeMappingRow** outRow);

/* Outcome of FirestaffSckBridge_SelectSlice. */
typedef struct FirestaffSckBridgeSelection {
    FirestaffSckAssetSlice slice;
    char mapfile[FIRESTAFF_SCK_BRIDGE_PATH_BYTES];
    char itemNumber[16];
    char itemDescription[FIRESTAFF_SCK_MAPFILE_DESC_BYTES];
    char itemType[FIRESTAFF_SCK_MAPFILE_TYPE_BYTES];
    unsigned int itemLine;
    int hasNumericNumber;
    int hasSizeBytes;
} FirestaffSckBridgeSelection;

/* RAW asset handoff result.  RAW rows in the Greatstone/SCK corpus are
 * intentionally an identity decoder: the bridge validates that the selected
 * RAW slice is bounded inside the caller-owned asset bytes and returns a
 * borrowed view into that buffer.  This lets selector-visible RAW items enter
 * a Firestaff-side decoder surface without persisting or transforming
 * copyrighted corpus bytes. */
typedef struct FirestaffSckBridgeRawHandoff {
    const uint8_t* bytes;
    uint32_t byteCount;
    uint32_t offset;
    uint32_t checksum32;
    char itemNumber[16];
    char itemType[FIRESTAFF_SCK_MAPFILE_TYPE_BYTES];
    char itemDescription[FIRESTAFF_SCK_MAPFILE_DESC_BYTES];
} FirestaffSckBridgeRawHandoff;

/* Select the SCK item identified by `itemNumber` from `mapfileText`
 * and validate the resulting slice against a target file of
 * `targetFileBytes` bytes.
 *
 * `acceptTypePrefix` is a case-insensitive type prefix filter: when
 * non-empty, only items whose `type` starts with the prefix are
 * considered (e.g. "IMG" matches IMG1/IMG3/IMG5/IMG14).  When empty,
 * any SCK item type is acceptable.
 *
 * Returns OK with `outSelection` populated when an item with both a
 * numeric item number AND a SIZE= attribute exists and the slice fits
 * inside the target file.  Returns NOT_FOUND when no matching item
 * exists, NOT_SIZED when no item carried SIZE=, or
 * SLICE_OUT_OF_BOUNDS when the slice overflows the target file. */
FirestaffSckBridgeResult FirestaffSckBridge_SelectSlice(
    const char* mapfileText,
    const char* itemNumber,
    const char* acceptTypePrefix,
    uint32_t targetFileBytes,
    FirestaffSckBridgeSelection* outSelection,
    char* errMsg,
    size_t errMsgBytes);

/* Convenience selector: scan the mapfile for the first item whose
 * `description` substring matches `descriptionSubstr` (case-sensitive)
 * AND that has SIZE= AND whose type passes the prefix filter.  This
 * is the path Firestaff callers use when they know the asset by its
 * human-readable label (e.g. "Dungeon Graphics,Ceiling"). */
FirestaffSckBridgeResult FirestaffSckBridge_SelectSliceByDescription(
    const char* mapfileText,
    const char* descriptionSubstr,
    const char* acceptTypePrefix,
    uint32_t targetFileBytes,
    FirestaffSckBridgeSelection* outSelection,
    char* errMsg,
    size_t errMsgBytes);

/* Decode a selected RAW slice from `assetBytes`.
 *
 * The selected item must have a type prefix of RAW (RAW1, RAW2, ...).
 * Returns OK with `outRaw` as a borrowed view into `assetBytes` when the
 * selected offset/size fits inside `assetByteCount`.  Unsupported types
 * return UNSUPPORTED_DECODER, so PAL/SND/FTL rows stay visible to selectors
 * but cannot accidentally masquerade as decoded bytes. */
FirestaffSckBridgeResult FirestaffSckBridge_DecodeRawSelection(
    const uint8_t* assetBytes,
    uint32_t assetByteCount,
    const FirestaffSckBridgeSelection* selection,
    FirestaffSckBridgeRawHandoff* outRaw,
    char* errMsg,
    size_t errMsgBytes);

const char* FirestaffSckBridge_ResultString(FirestaffSckBridgeResult result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCK_ASSET_BRIDGE_H */
