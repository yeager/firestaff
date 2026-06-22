#ifndef FIRESTAFF_MAPFILE_H
#define FIRESTAFF_MAPFILE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_MAPFILE_MAX_PROPERTY_KEY 32
#define FIRESTAFF_MAPFILE_MAX_PROPERTY_VALUE 128
#define FIRESTAFF_MAPFILE_MAX_PROPERTIES 16
#define FIRESTAFF_MAPFILE_MAX_ITEM_NUMBER 32
#define FIRESTAFF_MAPFILE_MAX_ITEM_TYPE 32
#define FIRESTAFF_MAPFILE_MAX_ITEM_ATTRIBUTES 256
#define FIRESTAFF_MAPFILE_MAX_ITEM_DESCRIPTION 160
#define FIRESTAFF_MAPFILE_MAX_ITEM_LONG_DESCRIPTION 256
#define FIRESTAFF_MAPFILE_MAX_ITEM_COMMENT 256

typedef enum Firestaff_MapfileResult {
    FIRESTAFF_MAPFILE_OK = 0,
    FIRESTAFF_MAPFILE_ERROR_INVALID_ARGUMENT,
    FIRESTAFF_MAPFILE_ERROR_LINE_TOO_LONG,
    FIRESTAFF_MAPFILE_ERROR_TOO_MANY_PROPERTIES,
    FIRESTAFF_MAPFILE_ERROR_TOO_MANY_ITEMS,
    FIRESTAFF_MAPFILE_ERROR_FIELD_TOO_LONG,
    FIRESTAFF_MAPFILE_ERROR_BAD_HEADER,
    FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_ROW,
    FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_NUMBER,
    FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_TYPE,
    FIRESTAFF_MAPFILE_ERROR_BAD_ATTRIBUTE
} Firestaff_MapfileResult;

typedef struct Firestaff_MapfileProperty {
    char key[FIRESTAFF_MAPFILE_MAX_PROPERTY_KEY];
    char value[FIRESTAFF_MAPFILE_MAX_PROPERTY_VALUE];
    unsigned int line;
} Firestaff_MapfileProperty;

typedef struct Firestaff_MapfileItem {
    char number[FIRESTAFF_MAPFILE_MAX_ITEM_NUMBER];
    char type[FIRESTAFF_MAPFILE_MAX_ITEM_TYPE];
    char attributes[FIRESTAFF_MAPFILE_MAX_ITEM_ATTRIBUTES];
    char description[FIRESTAFF_MAPFILE_MAX_ITEM_DESCRIPTION];
    char long_description[FIRESTAFF_MAPFILE_MAX_ITEM_LONG_DESCRIPTION];
    char comment[FIRESTAFF_MAPFILE_MAX_ITEM_COMMENT];
    unsigned int line;
    int has_size;
    uint32_t size;
} Firestaff_MapfileItem;

typedef struct Firestaff_MapfileDocument {
    Firestaff_MapfileProperty properties[FIRESTAFF_MAPFILE_MAX_PROPERTIES];
    size_t property_count;
    size_t item_count;
    unsigned int error_line;
} Firestaff_MapfileDocument;

Firestaff_MapfileResult Firestaff_Mapfile_ParseText(
    const char *text,
    Firestaff_MapfileDocument *document,
    Firestaff_MapfileItem *items,
    size_t item_capacity);

Firestaff_MapfileResult Firestaff_Mapfile_FindAttribute(
    const Firestaff_MapfileItem *item,
    const char *key,
    char *value,
    size_t value_capacity);

const char *Firestaff_Mapfile_ResultString(Firestaff_MapfileResult result);

#ifdef __cplusplus
}
#endif

#endif
