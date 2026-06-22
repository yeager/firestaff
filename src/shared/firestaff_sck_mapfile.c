#include "firestaff_sck_mapfile.h"

#include <ctype.h>
#include <errno.h>
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
