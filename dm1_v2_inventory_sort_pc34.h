#ifndef FIRESTAFF_DM1_V2_INVENTORY_SORT_PC34_H
#define FIRESTAFF_DM1_V2_INVENTORY_SORT_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    SORT_BY_NAME,
    SORT_BY_TYPE,
    SORT_BY_WEIGHT,
    SORT_BY_VALUE
} M11_V2_SortMode;

typedef enum {
    ICAT_WEAPON,
    ICAT_ARMOR,
    ICAT_POTION,
    ICAT_SCROLL,
    ICAT_KEY,
    ICAT_FOOD,
    ICAT_MISC
} M11_V2_ItemCategory;

typedef struct {
    char name[32];
    int weight;
    int value;
    M11_V2_ItemCategory cat;
    int slot_idx;
} M11_V2_InventoryItem;

typedef struct {
    M11_V2_InventoryItem items[64];
    int count;
    M11_V2_SortMode mode;
    M11_V2_ItemCategory filter;
    bool filter_active;
} M11_V2_InventoryView;

void v2_inv_init(void);
bool v2_inv_add_item(const char* name, int weight, int value, M11_V2_ItemCategory cat);
void v2_inv_sort(M11_V2_SortMode mode);
void v2_inv_filter(M11_V2_ItemCategory cat);
void v2_inv_clear_filter(void);
bool v2_inv_remove(int idx);
const M11_V2_InventoryItem* v2_inv_get(int idx);
int v2_inv_count(void);

#endif
