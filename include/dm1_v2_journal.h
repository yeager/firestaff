
#ifndef DM1_V2_JOURNAL_H
#define DM1_V2_JOURNAL_H

/* Auto-journal: records game events for player reference.
 * Events: level changes, champion deaths/resurrections,
 * boss encounters, key items found, doors opened. */

#define JOURNAL_MAX_ENTRIES 256
#define JOURNAL_MAX_TEXT 128

typedef struct {
    int tick;               /* game tick when event occurred */
    int level;
    int event_type;         /* 0=move, 1=combat, 2=item, 3=death, 4=spell, 5=door */
    char text[JOURNAL_MAX_TEXT];
} DM1_V2_JournalEntry;

typedef struct {
    DM1_V2_JournalEntry entries[JOURNAL_MAX_ENTRIES];
    int count;
    int visible;            /* overlay visible */
    int scroll_offset;
} DM1_V2_Journal;

void dm1_v2_journal_init(DM1_V2_Journal *j);
void dm1_v2_journal_add(DM1_V2_Journal *j, int tick, int level, int type, const char *text);
void dm1_v2_journal_render(const DM1_V2_Journal *j, uint32_t *rgba, int w, int h);

#endif

