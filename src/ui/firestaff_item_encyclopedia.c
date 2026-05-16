
#include "firestaff_item_encyclopedia.h"
#include <stddef.h>

/* Item Encyclopedia — representative items from all DM games.
 * Item properties based on general DM1 game mechanics. */

static const FS_ItemEntry g_items[] = {
    /* Weapons */
    {"Falchion",       FS_ITEM_CAT_WEAPON, "A curved single-edged sword.",           18, 30, 0},
    {"Rapier",         FS_ITEM_CAT_WEAPON, "A thin thrusting blade.",                14, 24, 4},
    {"Mace",           FS_ITEM_CAT_WEAPON, "A heavy blunt weapon.",                  30, 32, 0},
    {"Club",           FS_ITEM_CAT_WEAPON, "A simple wooden club.",                  20, 16, 0},
    {"Staff",          FS_ITEM_CAT_WEAPON, "A wooden staff. Also useful for spells.", 12, 10, 2},
    {"Sword",          FS_ITEM_CAT_WEAPON, "A standard double-edged sword.",         22, 34, 2},
    {"Axe",            FS_ITEM_CAT_WEAPON, "A single-bladed battle axe.",            26, 36, 0},
    {"Dagger",         FS_ITEM_CAT_WEAPON, "A small blade. Can be thrown.",            6, 14, 0},
    {"Arrow",          FS_ITEM_CAT_WEAPON, "Ammunition for bows.",                    1, 10, 0},
    {"Slayer",         FS_ITEM_CAT_WEAPON, "A legendary demon-slaying blade.",       28, 50, 6},
    {"Vorpal Blade",   FS_ITEM_CAT_WEAPON, "Enchanted blade of supreme sharpness.",  20, 48, 4},
    {"Firestaff",      FS_ITEM_CAT_WEAPON, "The legendary Firestaff of power.",      16, 40, 10},

    /* Armor */
    {"Leather Jerkin", FS_ITEM_CAT_ARMOR, "Light leather protection.",                8, 0, 8},
    {"Mail Aketon",    FS_ITEM_CAT_ARMOR, "Chainmail vest.",                         24, 0, 14},
    {"Plate Armor",    FS_ITEM_CAT_ARMOR, "Full plate mail. Heavy but strong.",      40, 0, 22},
    {"Shield",         FS_ITEM_CAT_ARMOR, "A wooden shield.",                        16, 0, 12},
    {"Helmet",         FS_ITEM_CAT_ARMOR, "Head protection.",                        10, 0, 6},
    {"Boots",          FS_ITEM_CAT_ARMOR, "Sturdy leather boots.",                    6, 0, 4},

    /* Potions */
    {"Health Potion",  FS_ITEM_CAT_POTION, "Restores health when consumed.",          2, 0, 0},
    {"Mana Potion",    FS_ITEM_CAT_POTION, "Restores mana when consumed.",            2, 0, 0},
    {"Stamina Potion", FS_ITEM_CAT_POTION, "Restores stamina.",                       2, 0, 0},
    {"Antidote",       FS_ITEM_CAT_POTION, "Cures poison.",                           2, 0, 0},

    /* Scrolls */
    {"Scroll",         FS_ITEM_CAT_SCROLL, "Contains written text or a spell.",       1, 0, 0},

    /* Containers */
    {"Chest",          FS_ITEM_CAT_CONTAINER, "Can hold multiple items.",            10, 0, 0},
    {"Sack",           FS_ITEM_CAT_CONTAINER, "A bag for carrying items.",            2, 0, 0},

    /* Keys */
    {"Gold Key",       FS_ITEM_CAT_KEY, "Opens gold locks.",                          1, 0, 0},
    {"Silver Key",     FS_ITEM_CAT_KEY, "Opens silver locks.",                        1, 0, 0},
    {"Skeleton Key",   FS_ITEM_CAT_KEY, "Opens most common locks.",                   1, 0, 0},

    /* Misc */
    {"Torch",          FS_ITEM_CAT_MISC, "Provides light. Burns out over time.",      6, 8, 0},
    {"Compass",        FS_ITEM_CAT_MISC, "Shows cardinal direction.",                 2, 0, 0},
    {"Rabbit Foot",    FS_ITEM_CAT_MISC, "Increases luck while carried.",             1, 0, 0},
    {"Corn",           FS_ITEM_CAT_MISC, "Food. Restores hunger.",                    3, 0, 0},
    {"Water Flask",    FS_ITEM_CAT_MISC, "Contains water. Restores thirst.",          4, 0, 0},

    {NULL, 0, NULL, 0, 0, 0}
};

static const char *g_cat_names[] = {
    "Weapons", "Armor", "Potions", "Scrolls", "Containers", "Miscellaneous", "Keys"
};

int fs_item_encyclopedia_count(void) {
    int i = 0; while (g_items[i].name) i++; return i;
}

const FS_ItemEntry *fs_item_encyclopedia_get(int index) {
    if (index < 0 || index >= fs_item_encyclopedia_count()) return NULL;
    return &g_items[index];
}

int fs_item_encyclopedia_count_in_category(FS_ItemCategory cat) {
    int i, c = 0;
    for (i = 0; g_items[i].name; i++)
        if (g_items[i].category == cat) c++;
    return c;
}

const char *fs_item_category_name(FS_ItemCategory cat) {
    if (cat < 0 || cat >= FS_ITEM_CAT_COUNT) return "Unknown";
    return g_cat_names[cat];
}

