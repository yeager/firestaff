
#include "firestaff_dungeon_text.h"
#include <string.h>
#include <stdio.h>

/* Dungeon text extractor.
 * DM1 DUNGEON.DAT text things contain ASCII text for scrolls
 * and wall inscriptions. These are the language-dependent parts. */

int fs_dungeon_extract_texts(FS_DungeonTextTable *out,
    const uint8_t *data, int size)
{
    int i, count = 0;
    if (!out || !data || size < 10) return -1;
    memset(out, 0, sizeof(*out));

    /* Scan for text-like sequences in dungeon data.
     * Real implementation would parse thing list per ReDMCSB DUNGEON.C.
     * This simplified version scans for printable ASCII runs. */
    for (i = 0; i < size - 4 && count < FS_DUNGEON_TEXT_MAX; i++) {
        /* Look for sequences of 4+ printable chars */
        int run = 0;
        while (i + run < size && data[i + run] >= 32 && data[i + run] < 127)
            run++;
        if (run >= 8 && run < FS_DUNGEON_TEXT_LEN) {
            memcpy(out->texts[count].text, data + i, run);
            out->texts[count].text[run] = 0;
            out->texts[count].thing_index = i;
            count++;
            i += run;
        }
    }

    out->count = count;
    return count;
}

const char *fs_dungeon_get_text(const FS_DungeonTextTable *t, int index) {
    if (!t || index < 0 || index >= t->count) return NULL;
    return t->texts[index].text;
}

