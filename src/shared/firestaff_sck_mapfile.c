#include "firestaff_sck_mapfile.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_err(char* dst, size_t cap, const char* msg) {
    size_t n;
    if (!dst || cap == 0) {
        return;
    }
    n = strlen(msg);
    if (n >= cap) {
        n = cap - 1;
    }
    memcpy(dst, msg, n);
    dst[n] = '\0';
}

static const char* skip_space_no_newline(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\r') {
        ++p;
    }
    return p;
}

static const char* skip_to_next_line(const char* p) {
    while (*p && *p != '\n') {
        ++p;
    }
    return (*p == '\n') ? (p + 1) : p;
}

static int token_char(int c) {
    return c != '\0' && c != '\n' && c != '\r' && !isspace((unsigned char)c) && c != '#';
}

static int read_token(const char** cursor, char* dst, size_t dstBytes) {
    const char* p = skip_space_no_newline(*cursor);
    size_t n = 0;
    if (!token_char((unsigned char)*p)) {
        return 0;
    }
    while (token_char((unsigned char)*p)) {
        if (n + 1u >= dstBytes) {
            return -1;
        }
        dst[n++] = *p++;
    }
    dst[n] = '\0';
    *cursor = p;
    return 1;
}

static int read_u32_token(const char** cursor, uint32_t* outValue) {
    const char* p = skip_space_no_newline(*cursor);
    char* end = NULL;
    unsigned long value;
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }
    errno = 0;
    value = strtoul(p, &end, 0);
    if (errno != 0 || end == p || value > 0xffffffffUL) {
        return -1;
    }
    if (token_char((unsigned char)*end)) {
        return -1;
    }
    *outValue = (uint32_t)value;
    *cursor = end;
    return 1;
}

static int copy_field(char* dst, size_t dstBytes, const char* start, size_t len) {
    while (len > 0u && (*start == ' ' || *start == '\t' || *start == '\r')) {
        ++start;
        --len;
    }
    while (len > 0u &&
           (start[len - 1u] == ' ' ||
            start[len - 1u] == '\t' ||
            start[len - 1u] == '\r')) {
        --len;
    }
    if (len + 1u > dstBytes) {
        return 0;
    }
    if (len != 0u) {
        memcpy(dst, start, len);
    }
    dst[len] = '\0';
    return 1;
}

static const char* line_end(const char* p) {
    while (*p && *p != '\n') {
        ++p;
    }
    return p;
}

static int split_csv6(const char* start,
                      const char* end,
                      char fields[6][FIRESTAFF_SCK_MAPFILE_ATTR_BYTES],
                      char* errMsg,
                      size_t errMsgBytes) {
    unsigned int field = 0u;
    const char* part = start;
    const char* p = start;
    memset(fields, 0, 6u * FIRESTAFF_SCK_MAPFILE_ATTR_BYTES);
    while (p <= end && field < 6u) {
        if (p == end || *p == ',') {
            size_t cap = FIRESTAFF_SCK_MAPFILE_DESC_BYTES;
            if (field == 2u) {
                cap = FIRESTAFF_SCK_MAPFILE_ATTR_BYTES;
            }
            if (!copy_field(fields[field], cap, part, (size_t)(p - part))) {
                set_err(errMsg, errMsgBytes, "SCK mapfile field too long");
                return 0;
            }
            ++field;
            part = p + 1;
        }
        ++p;
    }
    if (field < 4u) {
        set_err(errMsg, errMsgBytes, "SCK mapfile item has too few fields");
        return 0;
    }
    return 1;
}

static int parse_u32_string(const char* text, uint32_t* outValue) {
    char* end = NULL;
    unsigned long value;
    if (!text || !*text || !isdigit((unsigned char)*text)) {
        return 0;
    }
    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value > 0xffffffffUL) {
        return 0;
    }
    *outValue = (uint32_t)value;
    return 1;
}

static int parse_attr_size(const char* attrs, uint32_t* outSize) {
    const char* p = attrs;
    while (p && *p) {
        const char* next = strchr(p, '&');
        size_t len = next ? (size_t)(next - p) : strlen(p);
        if (len > 5u && strncmp(p, "SIZE=", 5u) == 0) {
            char value[32];
            if (len - 5u >= sizeof(value)) {
                return 0;
            }
            memcpy(value, p + 5, len - 5u);
            value[len - 5u] = '\0';
            return parse_u32_string(value, outSize);
        }
        p = next ? next + 1 : NULL;
    }
    return 0;
}

static int extract_property_value(const char* props,
                                  const char* key,
                                  char* dst,
                                  size_t dstBytes) {
    size_t keyLen = strlen(key);
    const char* p = props;
    while (p && *p) {
        const char* next = strchr(p, ',');
        size_t len = next ? (size_t)(next - p) : strlen(p);
        if (len > keyLen && strncmp(p, key, keyLen) == 0 && p[keyLen] == '=') {
            return copy_field(dst, dstBytes, p + keyLen + 1u, len - keyLen - 1u);
        }
        p = next ? next + 1 : NULL;
    }
    if (dstBytes != 0u) {
        dst[0] = '\0';
    }
    return 1;
}

int FirestaffSckMapfile_ParseText(const char* text,
                                  FirestaffSckMapfile* outMap,
                                  char* errMsg,
                                  size_t errMsgBytes) {
    const char* p;
    unsigned int line = 1u;

    if (!text || !outMap) {
        set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outMap, 0, sizeof(*outMap));
    p = text;

    while (*p) {
        const char* lineStart = p;
        FirestaffSckMapfileItem item;
        int rc;
        memset(&item, 0, sizeof(item));

        p = skip_space_no_newline(p);
        if (*p == '\n') {
            ++p;
            ++line;
            continue;
        }
        if (*p == '#' || *p == '\0') {
            p = skip_to_next_line(p);
            if (*p || p > lineStart) {
                ++line;
            }
            continue;
        }

        rc = read_token(&p, item.type, sizeof(item.type));
        if (rc <= 0) {
            set_err(errMsg, errMsgBytes, rc < 0 ? "mapfile type token too long" : "missing mapfile type");
            return 0;
        }
        rc = read_token(&p, item.name, sizeof(item.name));
        if (rc <= 0) {
            set_err(errMsg, errMsgBytes, rc < 0 ? "mapfile name token too long" : "missing mapfile name");
            return 0;
        }
        rc = read_u32_token(&p, &item.offset);
        if (rc <= 0) {
            set_err(errMsg, errMsgBytes, "invalid mapfile offset");
            return 0;
        }
        rc = read_u32_token(&p, &item.size);
        if (rc <= 0) {
            set_err(errMsg, errMsgBytes, "invalid mapfile size");
            return 0;
        }
        p = skip_space_no_newline(p);
        if (*p != '\0' && *p != '\n' && *p != '#') {
            set_err(errMsg, errMsgBytes, "unexpected mapfile trailing token");
            return 0;
        }
        if (outMap->itemCount >= FIRESTAFF_SCK_MAPFILE_MAX_ITEMS) {
            set_err(errMsg, errMsgBytes, "too many mapfile items");
            return 0;
        }
        item.lineNumber = line;
        outMap->items[outMap->itemCount++] = item;
        p = skip_to_next_line(p);
        ++line;
    }

    if (outMap->itemCount == 0u) {
        set_err(errMsg, errMsgBytes, "empty mapfile");
        return 0;
    }
    return 1;
}

int FirestaffSckMapfile_ValidateBounds(const FirestaffSckMapfile* map,
                                       uint32_t fileBytes,
                                       char* errMsg,
                                       size_t errMsgBytes) {
    unsigned int i;
    if (!map) {
        set_err(errMsg, errMsgBytes, "null mapfile");
        return 0;
    }
    for (i = 0; i < map->itemCount; ++i) {
        const FirestaffSckMapfileItem* item = &map->items[i];
        uint32_t end;
        if (item->offset > fileBytes) {
            set_err(errMsg, errMsgBytes, "mapfile offset exceeds file size");
            return 0;
        }
        if (item->size > fileBytes - item->offset) {
            set_err(errMsg, errMsgBytes, "mapfile item exceeds file size");
            return 0;
        }
        end = item->offset + item->size;
        if (end < item->offset) {
            set_err(errMsg, errMsgBytes, "mapfile item overflows");
            return 0;
        }
    }
    return 1;
}

const FirestaffSckMapfileItem* FirestaffSckMapfile_FindByName(
    const FirestaffSckMapfile* map,
    const char* name) {
    unsigned int i;
    if (!map || !name) {
        return NULL;
    }
    for (i = 0; i < map->itemCount; ++i) {
        if (strcmp(map->items[i].name, name) == 0) {
            return &map->items[i];
        }
    }
    return NULL;
}

int FirestaffSckMapfile_ParseSck2Text(const char* text,
                                      FirestaffSckMapfileV2* outMap,
                                      char* errMsg,
                                      size_t errMsgBytes) {
    const char* p;
    unsigned int line = 1u;
    int sawHeader = 0;

    if (!text || !outMap) {
        set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outMap, 0, sizeof(*outMap));
    p = text;

    while (*p) {
        const char* start;
        const char* end;
        char fields[6][FIRESTAFF_SCK_MAPFILE_ATTR_BYTES];
        FirestaffSckMapfileV2Item item;

        start = skip_space_no_newline(p);
        end = line_end(start);
        if (start == end || *start == '#') {
            p = (*end == '\n') ? end + 1 : end;
            ++line;
            continue;
        }

        if (!sawHeader) {
            if (!copy_field(outMap->headerProperties,
                            sizeof(outMap->headerProperties),
                            start,
                            (size_t)(end - start))) {
                set_err(errMsg, errMsgBytes, "SCK mapfile header too long");
                return 0;
            }
            (void)extract_property_value(outMap->headerProperties,
                                         "FORMAT",
                                         outMap->format,
                                         sizeof(outMap->format));
            (void)extract_property_value(outMap->headerProperties,
                                         "ENDIAN",
                                         outMap->endian,
                                         sizeof(outMap->endian));
            sawHeader = 1;
            p = (*end == '\n') ? end + 1 : end;
            ++line;
            continue;
        }

        memset(&item, 0, sizeof(item));
        if (!split_csv6(start, end, fields, errMsg, errMsgBytes)) {
            return 0;
        }
        if (!copy_field(item.number, sizeof(item.number), fields[0], strlen(fields[0])) ||
            !copy_field(item.type, sizeof(item.type), fields[1], strlen(fields[1])) ||
            !copy_field(item.attributes, sizeof(item.attributes), fields[2], strlen(fields[2])) ||
            !copy_field(item.description, sizeof(item.description), fields[3], strlen(fields[3])) ||
            !copy_field(item.longDescription, sizeof(item.longDescription), fields[4], strlen(fields[4])) ||
            !copy_field(item.comment, sizeof(item.comment), fields[5], strlen(fields[5]))) {
            set_err(errMsg, errMsgBytes, "SCK mapfile item field too long");
            return 0;
        }
        item.lineNumber = line;
        item.hasNumericNumber = parse_u32_string(item.number, &item.numericNumber);
        item.hasSizeBytes = parse_attr_size(item.attributes, &item.sizeBytes);
        if (outMap->itemCount >= FIRESTAFF_SCK_MAPFILE_MAX_ITEMS) {
            set_err(errMsg, errMsgBytes, "too many SCK mapfile items");
            return 0;
        }
        outMap->items[outMap->itemCount++] = item;
        p = (*end == '\n') ? end + 1 : end;
        ++line;
    }

    if (!sawHeader) {
        set_err(errMsg, errMsgBytes, "missing SCK mapfile header");
        return 0;
    }
    if (outMap->itemCount == 0u) {
        set_err(errMsg, errMsgBytes, "empty SCK mapfile");
        return 0;
    }
    return 1;
}

const FirestaffSckMapfileV2Item* FirestaffSckMapfileV2_FindByNumber(
    const FirestaffSckMapfileV2* map,
    const char* number) {
    unsigned int i;
    if (!map || !number) {
        return NULL;
    }
    for (i = 0; i < map->itemCount; ++i) {
        if (strcmp(map->items[i].number, number) == 0) {
            return &map->items[i];
        }
    }
    return NULL;
}

int FirestaffSckMapfileV2_BuildSizedSlices(const FirestaffSckMapfileV2* map,
                                           uint32_t fileBytes,
                                           FirestaffSckAssetSlice* outSlices,
                                           unsigned int maxSlices,
                                           unsigned int* outSliceCount,
                                           char* errMsg,
                                           size_t errMsgBytes) {
    unsigned int i;
    unsigned int count = 0u;

    if (outSliceCount) {
        *outSliceCount = 0u;
    }
    if (!map || !outSlices || !outSliceCount) {
        set_err(errMsg, errMsgBytes, "null slice argument");
        return 0;
    }
    for (i = 0; i < map->itemCount; ++i) {
        const FirestaffSckMapfileV2Item* item = &map->items[i];
        FirestaffSckAssetSlice* slice;
        if (!item->hasNumericNumber || !item->hasSizeBytes) {
            continue;
        }
        if (item->numericNumber > fileBytes ||
            item->sizeBytes > fileBytes - item->numericNumber) {
            set_err(errMsg, errMsgBytes, "SCK mapfile slice exceeds file size");
            return 0;
        }
        if (count >= maxSlices) {
            set_err(errMsg, errMsgBytes, "too many SCK mapfile slices");
            return 0;
        }
        slice = &outSlices[count++];
        memset(slice, 0, sizeof(*slice));
        snprintf(slice->number, sizeof(slice->number), "%s", item->number);
        snprintf(slice->type, sizeof(slice->type), "%s", item->type);
        snprintf(slice->description, sizeof(slice->description), "%s", item->description);
        slice->offset = item->numericNumber;
        slice->size = item->sizeBytes;
    }
    *outSliceCount = count;
    return 1;
}
