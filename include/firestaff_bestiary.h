
#ifndef FIRESTAFF_BESTIARY_H
#define FIRESTAFF_BESTIARY_H
#include <stdint.h>

#define FS_BESTIARY_MAX 64

typedef struct {
    const char *name;
    const char *description;
    int game;           /* 0=DM1, 1=CSB, 2=DM2 */
    int health;
    int attack;
    int defense;
    int armor;
    int speed;
    int poison;
    int see_invisible;
    int flies;
    int non_material;
    int graphic_index;  /* GRAPHICS.DAT sprite index */
} FS_BestiaryEntry;

int fs_bestiary_count(void);
const FS_BestiaryEntry *fs_bestiary_get(int index);
const FS_BestiaryEntry *fs_bestiary_find(const char *name);

#endif

