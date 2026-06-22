#ifndef FIRESTAFF_SCK_MAPFILE_H
#define FIRESTAFF_SCK_MAPFILE_H

#include <stddef.h>
#include <stdint.h>

#define FIRESTAFF_SCK_MAPFILE_MAX_ITEMS 1024u
#define FIRESTAFF_SCK_MAPFILE_TYPE_BYTES 16u
#define FIRESTAFF_SCK_MAPFILE_NAME_BYTES 64u

typedef struct FirestaffSckMapfileItem {
    char type[FIRESTAFF_SCK_MAPFILE_TYPE_BYTES];
    char name[FIRESTAFF_SCK_MAPFILE_NAME_BYTES];
    uint32_t offset;
    uint32_t size;
    unsigned int lineNumber;
} FirestaffSckMapfileItem;

typedef struct FirestaffSckMapfile {
    unsigned int itemCount;
    FirestaffSckMapfileItem items[FIRESTAFF_SCK_MAPFILE_MAX_ITEMS];
} FirestaffSckMapfile;

int FirestaffSckMapfile_ParseText(const char* text,
                                  FirestaffSckMapfile* outMap,
                                  char* errMsg,
                                  size_t errMsgBytes);

int FirestaffSckMapfile_ValidateBounds(const FirestaffSckMapfile* map,
                                       uint32_t fileBytes,
                                       char* errMsg,
                                       size_t errMsgBytes);

const FirestaffSckMapfileItem* FirestaffSckMapfile_FindByName(
    const FirestaffSckMapfile* map,
    const char* name);

#endif
