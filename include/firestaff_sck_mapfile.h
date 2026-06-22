#ifndef FIRESTAFF_SCK_MAPFILE_H
#define FIRESTAFF_SCK_MAPFILE_H

#include <stddef.h>
#include <stdint.h>

#define FIRESTAFF_SCK_MAPFILE_MAX_ITEMS 1024u
#define FIRESTAFF_SCK_MAPFILE_TYPE_BYTES 16u
#define FIRESTAFF_SCK_MAPFILE_NAME_BYTES 64u
#define FIRESTAFF_SCK_MAPFILE_ATTR_BYTES 160u
#define FIRESTAFF_SCK_MAPFILE_DESC_BYTES 128u
#define FIRESTAFF_SCK_MAPFILE_PROP_BYTES 256u

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

typedef struct FirestaffSckMapfileV2Item {
    char number[16];
    char type[FIRESTAFF_SCK_MAPFILE_TYPE_BYTES];
    char attributes[FIRESTAFF_SCK_MAPFILE_ATTR_BYTES];
    char description[FIRESTAFF_SCK_MAPFILE_DESC_BYTES];
    char longDescription[FIRESTAFF_SCK_MAPFILE_DESC_BYTES];
    char comment[FIRESTAFF_SCK_MAPFILE_DESC_BYTES];
    uint32_t numericNumber;
    uint32_t sizeBytes;
    unsigned int lineNumber;
    int hasNumericNumber;
    int hasSizeBytes;
} FirestaffSckMapfileV2Item;

typedef struct FirestaffSckMapfileV2 {
    char headerProperties[FIRESTAFF_SCK_MAPFILE_PROP_BYTES];
    char format[32];
    char endian[16];
    unsigned int itemCount;
    FirestaffSckMapfileV2Item items[FIRESTAFF_SCK_MAPFILE_MAX_ITEMS];
} FirestaffSckMapfileV2;

typedef struct FirestaffSckAssetSlice {
    char number[16];
    char type[FIRESTAFF_SCK_MAPFILE_TYPE_BYTES];
    char description[FIRESTAFF_SCK_MAPFILE_DESC_BYTES];
    uint32_t offset;
    uint32_t size;
} FirestaffSckAssetSlice;

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

int FirestaffSckMapfile_ParseSck2Text(const char* text,
                                      FirestaffSckMapfileV2* outMap,
                                      char* errMsg,
                                      size_t errMsgBytes);

const FirestaffSckMapfileV2Item* FirestaffSckMapfileV2_FindByNumber(
    const FirestaffSckMapfileV2* map,
    const char* number);

int FirestaffSckMapfileV2_BuildSizedSlices(const FirestaffSckMapfileV2* map,
                                           uint32_t fileBytes,
                                           FirestaffSckAssetSlice* outSlices,
                                           unsigned int maxSlices,
                                           unsigned int* outSliceCount,
                                           char* errMsg,
                                           size_t errMsgBytes);

#endif
