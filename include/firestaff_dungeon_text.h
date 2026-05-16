
#ifndef FIRESTAFF_DUNGEON_TEXT_H
#define FIRESTAFF_DUNGEON_TEXT_H
#include <stdint.h>

/* Dungeon text extraction — read inscriptions/scrolls from DUNGEON.DAT.
 * DM1 dungeon text is embedded in thing data (scroll objects + wall text).
 * Source: ReDMCSB DUNGEON.C F0168 text thing handling. */

#define FS_DUNGEON_TEXT_MAX 256
#define FS_DUNGEON_TEXT_LEN 128

typedef struct {
    int thing_index;
    int level;
    int x, y;
    char text[FS_DUNGEON_TEXT_LEN];
} FS_DungeonText;

typedef struct {
    FS_DungeonText texts[FS_DUNGEON_TEXT_MAX];
    int count;
} FS_DungeonTextTable;

int fs_dungeon_extract_texts(FS_DungeonTextTable *out,
    const uint8_t *dungeon_dat, int size);
const char *fs_dungeon_get_text(const FS_DungeonTextTable *t, int index);

#endif

