#ifndef FIRESTAFF_DM2_V1_DM1_ITEM_CONVERSION_H
#define FIRESTAFF_DM2_V1_DM1_ITEM_CONVERSION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_DM1_ITEM_CONV_TABLE_SIZE 199

#define DM2_V1_DB_CATEGORY_WEAPON    0
#define DM2_V1_DB_CATEGORY_CLOTHING  1
#define DM2_V1_DB_CATEGORY_SCROLL    2
#define DM2_V1_DB_CATEGORY_POTION    3
#define DM2_V1_DB_CATEGORY_CONTAINER 4
#define DM2_V1_DB_CATEGORY_MISC_ITEM 5

typedef struct {
    uint8_t item_db;
    uint8_t item_id;
} DM2_V1_DM1ItemConvEntry;

typedef struct {
    int valid;
    uint8_t item_db;
    uint8_t item_id;
} DM2_V1_DM1ItemConvReceipt;

extern const DM2_V1_DM1ItemConvEntry dm2_v1_dm1_item_conv_table[DM2_V1_DM1_ITEM_CONV_TABLE_SIZE];

int dm2_v1_dm1_item_conv_lookup(int dm1_index, DM2_V1_DM1ItemConvReceipt *out);

const char *dm2_v1_dm1_item_conversion_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
