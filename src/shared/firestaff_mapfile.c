#include "firestaff_mapfile.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define FIRESTAFF_MAPFILE_MAX_LINE 1024
#define FIRESTAFF_MAPFILE_MAX_FIELDS 6

static char *mapfile_ltrim(char *s)
{
    while (*s != '\0' && isspace((unsigned char)*s)) {
        ++s;
    }
    return s;
}

static void mapfile_rtrim(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        --n;
    }
}

static void mapfile_trim_in_place(char **s)
{
    *s = mapfile_ltrim(*s);
    mapfile_rtrim(*s);
}

static size_t mapfile_bounded_len(const char *s, size_t cap)
{
    size_t n = 0;
    while (n < cap && s[n] != '\0') {
        ++n;
    }
    return n;
}

static Firestaff_MapfileResult mapfile_copy(char *dst, size_t dst_cap, const char *src)
{
    size_t n;
    if (dst == NULL || dst_cap == 0 || src == NULL) {
        return FIRESTAFF_MAPFILE_ERROR_INVALID_ARGUMENT;
    }
    n = mapfile_bounded_len(src, dst_cap);
    if (n >= dst_cap) {
        dst[0] = '\0';
        return FIRESTAFF_MAPFILE_ERROR_FIELD_TOO_LONG;
    }
    memcpy(dst, src, n + 1);
    return FIRESTAFF_MAPFILE_OK;
}

static void mapfile_unquote(char **s)
{
    size_t n;
    char quote;
    mapfile_trim_in_place(s);
    n = strlen(*s);
    if (n < 2) {
        return;
    }
    quote = (*s)[0];
    if ((quote == '"' || quote == '\'') && (*s)[n - 1] == quote) {
        (*s)[n - 1] = '\0';
        ++(*s);
        mapfile_trim_in_place(s);
    }
}

static int mapfile_is_property_key(const char *s)
{
    size_t i;
    if (s == NULL || s[0] == '\0') {
        return 0;
    }
    for (i = 0; s[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isupper(c) || isdigit(c) || c == '_')) {
            return 0;
        }
    }
    return 1;
}

static int mapfile_is_item_number(const char *s)
{
    size_t i;
    if (s == NULL || s[0] == '\0') {
        return 0;
    }
    for (i = 0; s[i] != '\0'; ++i) {
        if (!(isalnum((unsigned char)s[i]) || s[i] == '_')) {
            return 0;
        }
    }
    return 1;
}

static int mapfile_is_item_type(const char *s)
{
    size_t i;
    if (s == NULL || s[0] == '\0') {
        return 0;
    }
    for (i = 0; s[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isupper(c) || isdigit(c) || c == '_')) {
            return 0;
        }
    }
    return 1;
}

static size_t mapfile_split_csv(char *line, char *fields[FIRESTAFF_MAPFILE_MAX_FIELDS])
{
    size_t count = 0;
    char *p = line;

    fields[count++] = p;
    while (*p != '\0') {
        if (*p == ',') {
            if (count >= FIRESTAFF_MAPFILE_MAX_FIELDS) {
                return FIRESTAFF_MAPFILE_MAX_FIELDS + 1;
            }
            *p = '\0';
            fields[count++] = p + 1;
        }
        ++p;
    }
    return count;
}

static Firestaff_MapfileResult mapfile_parse_properties(
    char *line,
    Firestaff_MapfileDocument *document,
    unsigned int line_number)
{
    char *fields[FIRESTAFF_MAPFILE_MAX_FIELDS];
    size_t count;
    size_t i;

    count = mapfile_split_csv(line, fields);
    if (count > FIRESTAFF_MAPFILE_MAX_FIELDS) {
        return FIRESTAFF_MAPFILE_ERROR_BAD_HEADER;
    }

    for (i = 0; i < count; ++i) {
        char *entry = fields[i];
        char *eq;
        char *key;
        char *value;
        Firestaff_MapfileResult r;

        mapfile_trim_in_place(&entry);
        if (entry[0] == '\0') {
            continue;
        }
        eq = strchr(entry, '=');
        if (eq == NULL) {
            return FIRESTAFF_MAPFILE_ERROR_BAD_HEADER;
        }
        *eq = '\0';
        key = entry;
        value = eq + 1;
        mapfile_trim_in_place(&key);
        mapfile_unquote(&value);
        if (!mapfile_is_property_key(key)) {
            return FIRESTAFF_MAPFILE_ERROR_BAD_HEADER;
        }
        if (document->property_count >= FIRESTAFF_MAPFILE_MAX_PROPERTIES) {
            return FIRESTAFF_MAPFILE_ERROR_TOO_MANY_PROPERTIES;
        }
        r = mapfile_copy(
            document->properties[document->property_count].key,
            sizeof(document->properties[document->property_count].key),
            key);
        if (r != FIRESTAFF_MAPFILE_OK) {
            return r;
        }
        r = mapfile_copy(
            document->properties[document->property_count].value,
            sizeof(document->properties[document->property_count].value),
            value);
        if (r != FIRESTAFF_MAPFILE_OK) {
            return r;
        }
        document->properties[document->property_count].line = line_number;
        ++document->property_count;
    }

    return FIRESTAFF_MAPFILE_OK;
}

Firestaff_MapfileResult Firestaff_Mapfile_FindAttribute(
    const Firestaff_MapfileItem *item,
    const char *key,
    char *value,
    size_t value_capacity)
{
    char attrs[FIRESTAFF_MAPFILE_MAX_ITEM_ATTRIBUTES];
    char *segment;
    char *next;
    Firestaff_MapfileResult r;

    if (item == NULL || key == NULL || key[0] == '\0' || value == NULL || value_capacity == 0) {
        return FIRESTAFF_MAPFILE_ERROR_INVALID_ARGUMENT;
    }
    value[0] = '\0';
    r = mapfile_copy(attrs, sizeof(attrs), item->attributes);
    if (r != FIRESTAFF_MAPFILE_OK) {
        return r;
    }

    segment = attrs;
    while (segment != NULL) {
        char *eq;
        char *name;
        char *attr_value;

        next = strchr(segment, '&');
        if (next != NULL) {
            *next = '\0';
            ++next;
        }
        mapfile_trim_in_place(&segment);
        eq = strchr(segment, '=');
        if (eq != NULL) {
            *eq = '\0';
            name = segment;
            attr_value = eq + 1;
            mapfile_trim_in_place(&name);
            mapfile_unquote(&attr_value);
            if (strcmp(name, key) == 0) {
                return mapfile_copy(value, value_capacity, attr_value);
            }
        } else if (strcmp(segment, key) == 0) {
            return mapfile_copy(value, value_capacity, segment);
        }
        segment = next;
    }

    return FIRESTAFF_MAPFILE_ERROR_BAD_ATTRIBUTE;
}

static void mapfile_parse_size_attribute(Firestaff_MapfileItem *item)
{
    char value[32];
    char *end = NULL;
    unsigned long parsed;

    item->has_size = 0;
    item->size = 0;
    if (Firestaff_Mapfile_FindAttribute(item, "SIZE", value, sizeof(value)) != FIRESTAFF_MAPFILE_OK) {
        return;
    }

    errno = 0;
    parsed = strtoul(value, &end, 0);
    if (errno == 0 && end != value && end != NULL && *end == '\0' && parsed <= UINT32_MAX) {
        item->has_size = 1;
        item->size = (uint32_t)parsed;
    }
}

static Firestaff_MapfileResult mapfile_parse_item(
    char *line,
    Firestaff_MapfileDocument *document,
    Firestaff_MapfileItem *items,
    size_t item_capacity,
    unsigned int line_number)
{
    char *fields[FIRESTAFF_MAPFILE_MAX_FIELDS];
    size_t count;
    size_t i;
    Firestaff_MapfileItem *item;
    Firestaff_MapfileResult r;

    count = mapfile_split_csv(line, fields);
    if (count > FIRESTAFF_MAPFILE_MAX_FIELDS || count < 4) {
        return FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_ROW;
    }
    for (i = 0; i < count; ++i) {
        mapfile_trim_in_place(&fields[i]);
    }

    if (!mapfile_is_item_number(fields[0])) {
        return FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_NUMBER;
    }
    if (!mapfile_is_item_type(fields[1])) {
        return FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_TYPE;
    }
    if (document->item_count >= item_capacity) {
        return FIRESTAFF_MAPFILE_ERROR_TOO_MANY_ITEMS;
    }

    item = &items[document->item_count];
    memset(item, 0, sizeof(*item));
    item->line = line_number;

    r = mapfile_copy(item->number, sizeof(item->number), fields[0]);
    if (r != FIRESTAFF_MAPFILE_OK) {
        return r;
    }
    r = mapfile_copy(item->type, sizeof(item->type), fields[1]);
    if (r != FIRESTAFF_MAPFILE_OK) {
        return r;
    }
    r = mapfile_copy(item->attributes, sizeof(item->attributes), fields[2]);
    if (r != FIRESTAFF_MAPFILE_OK) {
        return r;
    }
    r = mapfile_copy(item->description, sizeof(item->description), fields[3]);
    if (r != FIRESTAFF_MAPFILE_OK) {
        return r;
    }
    if (count > 4) {
        r = mapfile_copy(item->long_description, sizeof(item->long_description), fields[4]);
        if (r != FIRESTAFF_MAPFILE_OK) {
            return r;
        }
    }
    if (count > 5) {
        r = mapfile_copy(item->comment, sizeof(item->comment), fields[5]);
        if (r != FIRESTAFF_MAPFILE_OK) {
            return r;
        }
    }

    mapfile_parse_size_attribute(item);
    ++document->item_count;
    return FIRESTAFF_MAPFILE_OK;
}

Firestaff_MapfileResult Firestaff_Mapfile_ParseText(
    const char *text,
    Firestaff_MapfileDocument *document,
    Firestaff_MapfileItem *items,
    size_t item_capacity)
{
    const char *p;
    unsigned int line_number = 1;
    int saw_item = 0;

    if (text == NULL || document == NULL || (items == NULL && item_capacity > 0)) {
        return FIRESTAFF_MAPFILE_ERROR_INVALID_ARGUMENT;
    }

    memset(document, 0, sizeof(*document));
    p = text;

    while (*p != '\0') {
        char line[FIRESTAFF_MAPFILE_MAX_LINE];
        char *trimmed;
        const char *start = p;
        size_t len;
        Firestaff_MapfileResult r;
        char *first_comma;
        char *first_equals;

        while (*p != '\0' && *p != '\n') {
            ++p;
        }
        len = (size_t)(p - start);
        if (len > 0 && start[len - 1] == '\r') {
            --len;
        }
        if (len >= sizeof(line)) {
            document->error_line = line_number;
            return FIRESTAFF_MAPFILE_ERROR_LINE_TOO_LONG;
        }
        memcpy(line, start, len);
        line[len] = '\0';
        if (*p == '\n') {
            ++p;
        }

        trimmed = line;
        mapfile_trim_in_place(&trimmed);
        if (trimmed[0] == '\0' || trimmed[0] == '#') {
            ++line_number;
            continue;
        }

        first_comma = strchr(trimmed, ',');
        first_equals = strchr(trimmed, '=');
        if (first_equals != NULL && (first_comma == NULL || first_equals < first_comma)) {
            if (saw_item) {
                document->error_line = line_number;
                return FIRESTAFF_MAPFILE_ERROR_BAD_HEADER;
            }
            r = mapfile_parse_properties(trimmed, document, line_number);
        } else {
            saw_item = 1;
            r = mapfile_parse_item(trimmed, document, items, item_capacity, line_number);
        }

        if (r != FIRESTAFF_MAPFILE_OK) {
            document->error_line = line_number;
            return r;
        }
        ++line_number;
    }

    return FIRESTAFF_MAPFILE_OK;
}

const char *Firestaff_Mapfile_ResultString(Firestaff_MapfileResult result)
{
    switch (result) {
        case FIRESTAFF_MAPFILE_OK:
            return "ok";
        case FIRESTAFF_MAPFILE_ERROR_INVALID_ARGUMENT:
            return "invalid argument";
        case FIRESTAFF_MAPFILE_ERROR_LINE_TOO_LONG:
            return "line too long";
        case FIRESTAFF_MAPFILE_ERROR_TOO_MANY_PROPERTIES:
            return "too many properties";
        case FIRESTAFF_MAPFILE_ERROR_TOO_MANY_ITEMS:
            return "too many items";
        case FIRESTAFF_MAPFILE_ERROR_FIELD_TOO_LONG:
            return "field too long";
        case FIRESTAFF_MAPFILE_ERROR_BAD_HEADER:
            return "bad header";
        case FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_ROW:
            return "bad item row";
        case FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_NUMBER:
            return "bad item number";
        case FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_TYPE:
            return "bad item type";
        case FIRESTAFF_MAPFILE_ERROR_BAD_ATTRIBUTE:
            return "bad attribute";
        default:
            return "unknown mapfile result";
    }
}
