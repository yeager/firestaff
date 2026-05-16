
#include "dm1_v2_journal.h"
#include <string.h>
#include <stdio.h>

void dm1_v2_journal_init(DM1_V2_Journal *j) {
    if (j) { memset(j, 0, sizeof(*j)); j->visible = 0; }
}

void dm1_v2_journal_add(DM1_V2_Journal *j, int tick, int level, int type, const char *text) {
    DM1_V2_JournalEntry *e;
    if (!j || !text) return;
    if (j->count >= JOURNAL_MAX_ENTRIES) {
        /* Shift oldest out */
        memmove(&j->entries[0], &j->entries[1],
            (JOURNAL_MAX_ENTRIES - 1) * sizeof(DM1_V2_JournalEntry));
        j->count = JOURNAL_MAX_ENTRIES - 1;
    }
    e = &j->entries[j->count++];
    e->tick = tick;
    e->level = level;
    e->event_type = type;
    strncpy(e->text, text, JOURNAL_MAX_TEXT - 1);
}

void dm1_v2_journal_render(const DM1_V2_Journal *j, uint32_t *rgba, int w, int h) {
    int i, y_start, visible_count;
    if (!j || !rgba || !j->visible || j->count == 0) return;

    /* Semi-transparent overlay on right side */
    {
        int panel_w = w / 3;
        int panel_x = w - panel_w;
        int y2, x2;
        for (y2 = 0; y2 < h; y2++)
            for (x2 = panel_x; x2 < w; x2++) {
                uint32_t c = rgba[y2*w+x2];
                int r = ((c>>16)&0xFF) / 3;
                int g = ((c>>8)&0xFF) / 3;
                int b = (c&0xFF) / 3;
                rgba[y2*w+x2] = 0xFF000000|(r<<16)|(g<<8)|b;
            }
    }

    /* Render entries (newest at bottom, scrollable) */
    visible_count = h / 12;
    y_start = j->count - visible_count - j->scroll_offset;
    if (y_start < 0) y_start = 0;

    for (i = y_start; i < j->count && (i - y_start) < visible_count; i++) {
        /* Entry indicator pixel (color by type) */
        int ey = 4 + (i - y_start) * 12;
        int ex = w - w/3 + 4;
        uint32_t type_color;
        switch (j->entries[i].event_type) {
            case 1: type_color = 0xFFFF4444; break; /* combat=red */
            case 2: type_color = 0xFFFFD700; break; /* item=gold */
            case 3: type_color = 0xFF808080; break; /* death=gray */
            case 4: type_color = 0xFF4488FF; break; /* spell=blue */
            case 5: type_color = 0xFF8B4513; break; /* door=brown */
            default: type_color = 0xFFCCCCCC; break; /* move=white */
        }
        if (ex >= 0 && ex < w && ey >= 0 && ey < h) {
            int dx2, dy2;
            for (dy2 = 0; dy2 < 3; dy2++)
                for (dx2 = 0; dx2 < 3; dx2++)
                    if (ex+dx2 < w && ey+dy2 < h)
                        rgba[(ey+dy2)*w+ex+dx2] = type_color;
        }
    }
}

