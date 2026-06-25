/*
 * firestaff_sck_asset_bridge.c — bounded mapfile-to-asset-loader bridge.
 *
 * Wires Greatstone/SCK `_mapping.xml` + per-file `.map` metadata
 * into a single selection call that the Firestaff asset-loader
 * (or any runtime probe) can consume.  See the header for the
 * scope/non-scope contract.
 *
 * The XML parser is intentionally tiny: the bundled `_mapping.xml`
 * has a fixed shape (top-level `<map>` rows, optional `<game id="..."/>`
 * children, comments, no nested `<map>` elements), so we scan for
 * `<map ...>` and `<map .../>` tokens directly rather than building a
 * generic XML walker.  See `consume_next_map_block` for the loop body.
 */

#include "firestaff_sck_asset_bridge.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void copy_bounded(char* dst, size_t dstBytes, const char* src) {
    size_t n;
    if (!dst || dstBytes == 0u) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    n = strlen(src);
    if (n >= dstBytes) {
        n = dstBytes - 1u;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void set_err(char* dst, size_t cap, const char* msg) {
    if (!dst || cap == 0u) {
        return;
    }
    copy_bounded(dst, cap, msg);
}

static int ascii_ieq(int a, int b) {
    return tolower((unsigned char)a) == tolower((unsigned char)b);
}

static int str_ieq(const char* a, const char* b) {
    if (!a || !b) {
        return 0;
    }
    while (*a && *b) {
        if (!ascii_ieq((unsigned char)*a, (unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static int str_starts_with_i(const char* s, const char* prefix) {
    if (!s || !prefix) {
        return 0;
    }
    while (*prefix) {
        if (!*s) {
            return 0;
        }
        if (!ascii_ieq((unsigned char)*s, (unsigned char)*prefix)) {
            return 0;
        }
        ++s;
        ++prefix;
    }
    return 1;
}

/* Copy up to `cap-1` bytes from `src` (start at `start`, max `maxLen`
 * bytes) into `dst`, trimming leading/trailing ASCII whitespace. */
static void copy_trim(char* dst, size_t cap, const char* start, size_t maxLen) {
    size_t n = 0u;
    const char* end = start + maxLen;
    const char* s = start;
    if (cap == 0u) {
        return;
    }
    while (s < end && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')) {
        ++s;
    }
    while (s < end && *s != '"' && *s != '\'') {
        if (n + 1u >= cap) {
            break;
        }
        dst[n++] = *s++;
    }
    if (n > 0u) {
        while (n > 0u && (dst[n - 1u] == ' ' || dst[n - 1u] == '\t' ||
                           dst[n - 1u] == '\r' || dst[n - 1u] == '\n')) {
            --n;
        }
    }
    dst[n] = '\0';
}

/* Find the next occurrence of `needle` in `haystack` starting at `from`.
 * Like strstr but accepts a position argument. */
static const char* find_at(const char* haystack, const char* needle, const char* from) {
    if (!haystack || !needle || !from) {
        return NULL;
    }
    if (from < haystack) {
        from = haystack;
    }
    return strstr(from, needle);
}

/* Locate an attribute named `name` inside an XML element opening tag
 * that runs from `tagStart` (position right after '<') to `tagEnd`
 * (position of the matching '>' or '/').  Writes the (whitespace-
 * trimmed, attribute-decoded) value into `out` (capacity `outCap`).
 * Returns 1 on success, 0 otherwise. */
static int find_attr(const char* tagStart,
                     const char* tagEnd,
                     const char* name,
                     char* out,
                     size_t outCap) {
    size_t nameLen = strlen(name);
    const char* p = tagStart;

    /* Skip the element-name token that always leads an opening tag
     * (e.g. "map" in `<map md5=...>`).  The token is whatever sits
     * before the first whitespace or '/' character. */
    while (p < tagEnd && *p != ' ' && *p != '\t' && *p != '\n' &&
           *p != '\r' && *p != '/' && *p != '>') {
        ++p;
    }
    while (p < tagEnd) {
        const char* nameStart;
        const char* nameEnd;
        /* Skip whitespace between attributes. */
        while (p < tagEnd && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
            ++p;
        }
        if (p >= tagEnd) {
            return 0;
        }
        if (*p == '/' || *p == '>') {
            return 0;
        }
        nameStart = p;
        while (p < tagEnd && *p != '=' && *p != ' ' && *p != '\t' &&
               *p != '\n' && *p != '\r' && *p != '/' && *p != '>') {
            ++p;
        }
        nameEnd = p;
        if (p >= tagEnd || *p != '=') {
            return 0;
        }
        if ((size_t)(nameEnd - nameStart) == nameLen &&
            strncmp(nameStart, name, nameLen) == 0) {
            const char* v = p + 1;
            while (v < tagEnd && (*v == ' ' || *v == '\t')) {
                ++v;
            }
            if (v >= tagEnd || (*v != '"' && *v != '\'')) {
                return 0;
            }
            ++v;
            copy_trim(out, outCap, v, (size_t)(tagEnd - v));
            return 1;
        }
        /* Skip past this attribute's value (including its closing quote).
         * We are currently at the '='; the value is wrapped in a quoted
         * region, so consume the opening quote, the value body, and the
         * closing quote before resuming the search for the next name. */
        ++p;
        if (p < tagEnd && (*p == '"' || *p == '\'')) {
            char quote = *p;
            ++p;
            while (p < tagEnd && *p != quote) {
                ++p;
            }
            if (p < tagEnd) {
                ++p; /* consume closing quote */
            }
        }
    }
    return 0;
}

/* Skip an XML comment `<!-- ... -->` that begins at `p`.  Returns the
 * new cursor just past `-->`, or NULL if the comment is unterminated. */
static const char* skip_xml_comment(const char* p) {
    if (!p) {
        return NULL;
    }
    if (p[0] != '<' || p[1] != '!' || p[2] != '-' || p[3] != '-') {
        return p;
    }
    {
        const char* end = strstr(p, "-->");
        return end ? end + 3 : NULL;
    }
}

/* Skip an XML processing instruction `<? ... ?>` that begins at `p`. */
static const char* skip_xml_pi(const char* p) {
    if (!p || p[0] != '<' || p[1] != '?') {
        return p;
    }
    {
        const char* end = strstr(p, "?>");
        return end ? end + 2 : NULL;
    }
}

/* Outcome of consume_next_map_block. */
typedef struct MapBlock {
    const char* tagStart;       /* right after '<' */
    const char* tagEnd;         /* position of '>' or '/' of the open tag */
    const char* bodyStart;      /* right after '>' of open tag, or NULL */
    const char* bodyEnd;        /* position of '<' of </map>, or NULL */
    const char* docEnd;         /* position just after the element */
    int selfClosing;
} MapBlock;

/* Try to consume the next <map ...>...</map> or <map .../>` element.
 * Skips comments, processing instructions, and non-<map> tags along the
 * way.  Always advances *cursor to the new position, even on failure,
 * so the outer loop can never spin on the same input.  Returns 1 when
 * a <map> element was located, 0 when the document ended. */
static int consume_next_map_block(const char** cursor, MapBlock* out) {
    const char* p = *cursor;
    while (*p) {
        const char* lt;
        const char* tagStart;
        const char* tagEnd;
        const char* endPos;

        /* Skip comments and PIs. */
        if (*p == '<') {
            if (p[1] == '!' && p[2] == '-' && p[3] == '-') {
                const char* after = skip_xml_comment(p);
                if (!after) {
                    *cursor = p;
                    return 0;
                }
                p = after;
                continue;
            }
            if (p[1] == '?') {
                const char* after = skip_xml_pi(p);
                if (!after) {
                    *cursor = p;
                    return 0;
                }
                p = after;
                continue;
            }
        }

        lt = strchr(p, '<');
        if (!lt) {
            /* No more tags in the document; advance to end-of-string. */
            *cursor = p + strlen(p);
            return 0;
        }

        /* Must start with `<map` followed by space, tab, '/', or '>'. */
        if (strncmp(lt, "<map", 4) != 0 ||
            (lt[4] != ' ' && lt[4] != '\t' && lt[4] != '\n' &&
             lt[4] != '\r' && lt[4] != '/' && lt[4] != '>')) {
            /* Not a <map> element: skip to the next '>' and continue. */
            const char* gt = strchr(lt, '>');
            if (!gt) {
                *cursor = lt;
                return 0;
            }
            p = gt + 1;
            continue;
        }

        tagStart = lt + 1;
        tagEnd = tagStart;
        while (tagEnd < lt + 4096 && *tagEnd && *tagEnd != '>') {
            ++tagEnd;
        }
        if (tagEnd >= lt + 4096 || !*tagEnd) {
            *cursor = lt;
            return 0;
        }

        if (tagEnd > lt + 4 && tagEnd[-1] == '/') {
            out->tagStart = tagStart;
            out->tagEnd = tagEnd - 1;
            out->bodyStart = NULL;
            out->bodyEnd = NULL;
            out->docEnd = tagEnd + 1;
            out->selfClosing = 1;
            *cursor = out->docEnd;
            return 1;
        }

        /* <map ...>...</map>: locate </map> in the remainder. */
        endPos = find_at(lt, "</map>", tagEnd + 1);
        if (!endPos) {
            *cursor = lt;
            return 0;
        }
        out->tagStart = tagStart;
        out->tagEnd = tagEnd;
        out->bodyStart = tagEnd + 1;
        out->bodyEnd = endPos;
        out->docEnd = endPos + strlen("</map>");
        out->selfClosing = 0;
        *cursor = out->docEnd;
        return 1;
    }
    *cursor = p;
    return 0;
}

/* Read every <game id="..."/> child of a <map>...</map> body.
 * Returns 1 on success, 0 if too many games. */
static int read_games(const MapBlock* blk, FirestaffSckBridgeMappingRow* row) {
    const char* p = blk->bodyStart;
    const char* end = blk->bodyEnd;
    if (!p || !end) {
        return 1;
    }
    while (p < end) {
        const char* lt;
        if (*p == '<') {
            if (p[1] == '!' && p[2] == '-' && p[3] == '-') {
                const char* after = skip_xml_comment(p);
                if (!after || after > end) {
                    return 1;
                }
                p = after;
                continue;
            }
        }
        lt = strchr(p, '<');
        if (!lt || lt >= end) {
            return 1;
        }
        if (strncmp(lt, "<game", 5) == 0 &&
            (lt[5] == ' ' || lt[5] == '\t' || lt[5] == '/' || lt[5] == '>')) {
            const char* tagStart = lt + 1;
            const char* tagEnd = tagStart;
            char id[FIRESTAFF_SCK_BRIDGE_GAME_BYTES];
            const char* gt;
            memset(id, 0, sizeof(id));
            while (tagEnd < end && *tagEnd && *tagEnd != '>') {
                ++tagEnd;
            }
            if (find_attr(tagStart, tagEnd, "id", id, sizeof(id)) && id[0] != '\0') {
                if (row->gameCount >= FIRESTAFF_SCK_BRIDGE_MAX_GAMES) {
                    return 0;
                }
                copy_bounded(row->games[row->gameCount].id, sizeof(row->games[0].id), id);
                ++row->gameCount;
            }
            gt = strchr(lt, '>');
            p = gt ? gt + 1 : lt + 1;
            continue;
        }
        ++p;
    }
    return 1;
}

FirestaffSckBridgeResult FirestaffSckBridge_ParseMappingXml(
    const char* xmlText,
    FirestaffSckBridgeMapping* outMapping) {
    const char* cursor;
    if (!xmlText || !outMapping) {
        return FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG;
    }
    memset(outMapping, 0, sizeof(*outMapping));
    cursor = xmlText;
    while (*cursor) {
        MapBlock blk;
        FirestaffSckBridgeMappingRow* row;
        const char* prevCursor = cursor;
        if (!consume_next_map_block(&cursor, &blk)) {
            /* consume_next_map_block always advances the cursor on
             * non-match (past the foreign element it tried), so a
             * non-progress means we ran out of input.  Bail. */
            if (cursor == prevCursor) {
                break;
            }
            continue;
        }
        if (outMapping->rowCount >= FIRESTAFF_SCK_BRIDGE_MAX_ROWS) {
            return FIRESTAFF_SCK_BRIDGE_ERR_TOO_MANY_ROWS;
        }
        row = &outMapping->rows[outMapping->rowCount++];
        memset(row, 0, sizeof(*row));
        if (!find_attr(blk.tagStart, blk.tagEnd, "md5", row->md5, sizeof(row->md5)) ||
            !find_attr(blk.tagStart, blk.tagEnd, "path", row->path, sizeof(row->path)) ||
            !find_attr(blk.tagStart, blk.tagEnd, "file", row->file, sizeof(row->file))) {
            /* Missing a required attribute: drop the row. */
            --outMapping->rowCount;
            continue;
        }
        if (!blk.selfClosing && !read_games(&blk, row)) {
            return FIRESTAFF_SCK_BRIDGE_ERR_TOO_MANY_GAMES;
        }
    }
    return FIRESTAFF_SCK_BRIDGE_OK;
}

FirestaffSckBridgeResult FirestaffSckBridge_Lookup(
    const FirestaffSckBridgeMapping* mapping,
    const char* md5Hex,
    const char* file,
    const FirestaffSckBridgeMappingRow** outRow) {
    unsigned int i;
    int hasMd5 = md5Hex && md5Hex[0] != '\0';
    int hasFile = file && file[0] != '\0';
    if (!mapping || !outRow) {
        return FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG;
    }
    if (!hasMd5 && !hasFile) {
        return FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND;
    }
    for (i = 0u; i < mapping->rowCount; ++i) {
        const FirestaffSckBridgeMappingRow* row = &mapping->rows[i];
        int md5Match = hasMd5 ? (str_ieq(row->md5, md5Hex) != 0) : 1;
        int fileMatch = hasFile ? (str_ieq(row->file, file) != 0) : 1;
        if (md5Match && fileMatch) {
            *outRow = row;
            return FIRESTAFF_SCK_BRIDGE_OK;
        }
    }
    return FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND;
}

/* Internal: select a sized slice for the SCK item identified by
 * `itemNumber` (or `descriptionSubstr` when number is NULL). */
static FirestaffSckBridgeResult select_slice_internal(
    const char* mapfileText,
    const char* itemNumber,
    const char* descriptionSubstr,
    const char* acceptTypePrefix,
    uint32_t targetFileBytes,
    FirestaffSckBridgeSelection* outSelection,
    char* errMsg,
    size_t errMsgBytes) {
    FirestaffSckMapfileV2 map;
    unsigned int i;
    int foundIndex = -1;

    if (!mapfileText || !outSelection) {
        set_err(errMsg, errMsgBytes, "null selection argument");
        return FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG;
    }
    memset(&map, 0, sizeof(map));
    memset(outSelection, 0, sizeof(*outSelection));

    if (FirestaffSckMapfile_ParseSck2Text(mapfileText, &map, errMsg, errMsgBytes) != 1) {
        return FIRESTAFF_SCK_BRIDGE_ERR_MAPFILE_PARSE;
    }

    for (i = 0u; i < map.itemCount; ++i) {
        const FirestaffSckMapfileV2Item* item = &map.items[i];
        int matches = 0;
        if (!item->hasNumericNumber || !item->hasSizeBytes) {
            continue;
        }
        if (itemNumber != NULL && itemNumber[0] != '\0') {
            if (strcmp(item->number, itemNumber) == 0) {
                matches = 1;
            }
        } else if (descriptionSubstr != NULL && descriptionSubstr[0] != '\0') {
            if (strstr(item->description, descriptionSubstr) != NULL ||
                strstr(item->longDescription, descriptionSubstr) != NULL) {
                matches = 1;
            }
        } else {
            continue;
        }
        if (!matches) {
            continue;
        }
        if (acceptTypePrefix != NULL && acceptTypePrefix[0] != '\0' &&
            !str_starts_with_i(item->type, acceptTypePrefix)) {
            continue;
        }
        foundIndex = (int)i;
        break;
    }
    if (foundIndex < 0) {
        /* Distinguish "not found" from "not sized". */
        unsigned int j;
        for (j = 0u; j < map.itemCount; ++j) {
            const FirestaffSckMapfileV2Item* item = &map.items[j];
            int matches = 0;
            if (itemNumber != NULL && itemNumber[0] != '\0') {
                if (strcmp(item->number, itemNumber) == 0) {
                    matches = 1;
                }
            } else if (descriptionSubstr != NULL && descriptionSubstr[0] != '\0') {
                if (strstr(item->description, descriptionSubstr) != NULL ||
                    strstr(item->longDescription, descriptionSubstr) != NULL) {
                    matches = 1;
                }
            }
            if (matches) {
                set_err(errMsg, errMsgBytes, "matched SCK item has no SIZE attribute");
                return FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED;
            }
        }
        if (itemNumber && itemNumber[0]) {
            set_err(errMsg, errMsgBytes, "no sized SCK item matched the requested number");
        } else {
            set_err(errMsg, errMsgBytes, "no sized SCK item matched the description");
        }
        return FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND;
    }

    {
        const FirestaffSckMapfileV2Item* item = &map.items[foundIndex];
        if (item->numericNumber > targetFileBytes ||
            item->sizeBytes > targetFileBytes - item->numericNumber) {
            set_err(errMsg, errMsgBytes, "SCK item slice exceeds target file size");
            return FIRESTAFF_SCK_BRIDGE_ERR_SLICE_OUT_OF_BOUNDS;
        }
        copy_bounded(outSelection->itemNumber, sizeof(outSelection->itemNumber), item->number);
        copy_bounded(outSelection->itemDescription,
                      sizeof(outSelection->itemDescription),
                      item->description);
        copy_bounded(outSelection->itemType, sizeof(outSelection->itemType), item->type);
        outSelection->itemLine = item->lineNumber;
        outSelection->hasNumericNumber = item->hasNumericNumber;
        outSelection->hasSizeBytes = item->hasSizeBytes;
        outSelection->slice.offset = item->numericNumber;
        outSelection->slice.size = item->sizeBytes;
        copy_bounded(outSelection->slice.number,
                      sizeof(outSelection->slice.number),
                      item->number);
        copy_bounded(outSelection->slice.type,
                      sizeof(outSelection->slice.type),
                      item->type);
        copy_bounded(outSelection->slice.description,
                      sizeof(outSelection->slice.description),
                      item->description);
    }
    return FIRESTAFF_SCK_BRIDGE_OK;
}

FirestaffSckBridgeResult FirestaffSckBridge_SelectSlice(
    const char* mapfileText,
    const char* itemNumber,
    const char* acceptTypePrefix,
    uint32_t targetFileBytes,
    FirestaffSckBridgeSelection* outSelection,
    char* errMsg,
    size_t errMsgBytes) {
    return select_slice_internal(mapfileText,
                                 itemNumber,
                                 NULL,
                                 acceptTypePrefix,
                                 targetFileBytes,
                                 outSelection,
                                 errMsg,
                                 errMsgBytes);
}

FirestaffSckBridgeResult FirestaffSckBridge_SelectSliceByDescription(
    const char* mapfileText,
    const char* descriptionSubstr,
    const char* acceptTypePrefix,
    uint32_t targetFileBytes,
    FirestaffSckBridgeSelection* outSelection,
    char* errMsg,
    size_t errMsgBytes) {
    return select_slice_internal(mapfileText,
                                 NULL,
                                 descriptionSubstr,
                                 acceptTypePrefix,
                                 targetFileBytes,
                                 outSelection,
                                 errMsg,
                                 errMsgBytes);
}

const char* FirestaffSckBridge_ResultString(FirestaffSckBridgeResult result) {
    switch (result) {
    case FIRESTAFF_SCK_BRIDGE_OK:
        return "ok";
    case FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG:
        return "null argument";
    case FIRESTAFF_SCK_BRIDGE_ERR_BAD_XML:
        return "malformed _mapping.xml";
    case FIRESTAFF_SCK_BRIDGE_ERR_TOO_MANY_ROWS:
        return "too many mapping rows";
    case FIRESTAFF_SCK_BRIDGE_ERR_TOO_MANY_GAMES:
        return "too many game ids per row";
    case FIRESTAFF_SCK_BRIDGE_ERR_FIELD_TOO_LONG:
        return "attribute field too long";
    case FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND:
        return "row/item not found";
    case FIRESTAFF_SCK_BRIDGE_ERR_MAPFILE_PARSE:
        return "SCK mapfile parse failed";
    case FIRESTAFF_SCK_BRIDGE_ERR_SLICE_OUT_OF_BOUNDS:
        return "slice exceeds target file size";
    case FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED:
        return "item has no SIZE attribute";
    }
    return "unknown";
}
