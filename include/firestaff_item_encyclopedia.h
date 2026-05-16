
#ifndef FIRESTAFF_ITEM_ENCYCLOPEDIA_H
#define FIRESTAFF_ITEM_ENCYCLOPEDIA_H

typedef enum {
    FS_ITEM_CAT_WEAPON = 0,
    FS_ITEM_CAT_ARMOR,
    FS_ITEM_CAT_POTION,
    FS_ITEM_CAT_SCROLL,
    FS_ITEM_CAT_CONTAINER,
    FS_ITEM_CAT_MISC,
    FS_ITEM_CAT_KEY,
    FS_ITEM_CAT_COUNT
} FS_ItemCategory;

typedef struct {
    const char *name;
    FS_ItemCategory category;
    const char *description;
    int weight;
    int attack;     /* 0 for non-weapons */
    int defense;    /* 0 for non-armor */
} FS_ItemEntry;

int fs_item_encyclopedia_count(void);
const FS_ItemEntry *fs_item_encyclopedia_get(int index);
int fs_item_encyclopedia_count_in_category(FS_ItemCategory cat);
const char *fs_item_category_name(FS_ItemCategory cat);

#endif

