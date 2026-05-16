#include "dm1_v2_journal_pc34.h"
#include <string.h>
#include <stdio.h>

static M11_V2_Journal g_v2_journal;

void v2_journal_init(void) {
    memset(&g_v2_journal, 0, sizeof(M11_V2_Journal));
    g_v2_journal.count = 0;
    g_v2_journal.page = 0;
    g_v2_journal.entries_per_page = 8;
}

void v2_journal_add(M11_V2_JournalCategory category, const char* text, int level, uint32_t tick) {
    if (g_v2_journal.count >= 128) return;
    if (!text) return;
    
    M11_V2_JournalEntry* entry = &g_v2_journal.entries[g_v2_journal.count];
    entry->tick = tick;
    entry->category = category;
    entry->level = level;
    strncpy(entry->text, text, 255);
    entry->text[255] = '\0';
    g_v2_journal.count++;
}

M11_V2_JournalEntry* v2_journal_get_page(int page) {
    int max_pages = (g_v2_journal.count + g_v2_journal.entries_per_page - 1) / g_v2_journal.entries_per_page;
    if (max_pages == 0) max_pages = 1;
    if (page < 0) page = 0;
    if (page >= max_pages) page = max_pages - 1;
    
    int start_idx = page * g_v2_journal.entries_per_page;
    if (start_idx >= g_v2_journal.count) {
        return NULL;
    }
    return &g_v2_journal.entries[start_idx];
}

int v2_journal_get_page_count(int page) {
    int max_pages = (g_v2_journal.count + g_v2_journal.entries_per_page - 1) / g_v2_journal.entries_per_page;
    if (max_pages == 0) max_pages = 1;
    if (page < 0) page = 0;
    if (page >= max_pages) page = max_pages - 1;
    
    int start_idx = page * g_v2_journal.entries_per_page;
    int remaining = g_v2_journal.count - start_idx;
    return (remaining < g_v2_journal.entries_per_page) ? remaining : g_v2_journal.entries_per_page;
}

void v2_journal_next_page(void) {
    int max_pages = (g_v2_journal.count + g_v2_journal.entries_per_page - 1) / g_v2_journal.entries_per_page;
    if (max_pages == 0) max_pages = 1;
    if (g_v2_journal.page < max_pages - 1) {
        g_v2_journal.page++;
    }
}

void v2_journal_prev_page(void) {
    if (g_v2_journal.page > 0) {
        g_v2_journal.page--;
    }
}

void v2_journal_clear(void) {
    memset(&g_v2_journal, 0, sizeof(M11_V2_Journal));
    g_v2_journal.entries_per_page = 8;
}

bool v2_journal_save(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    
    fwrite(&g_v2_journal.count, sizeof(int), 1, f);
    fwrite(&g_v2_journal.page, sizeof(int), 1, f);
    fwrite(&g_v2_journal.entries_per_page, sizeof(int), 1, f);
    fwrite(g_v2_journal.entries, sizeof(M11_V2_JournalEntry), g_v2_journal.count, f);
    
    fclose(f);
    return true;
}

bool v2_journal_load(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    
    memset(&g_v2_journal, 0, sizeof(M11_V2_Journal));
    fread(&g_v2_journal.count, sizeof(int), 1, f);
    fread(&g_v2_journal.page, sizeof(int), 1, f);
    fread(&g_v2_journal.entries_per_page, sizeof(int), 1, f);
    
    if (g_v2_journal.count > 128) g_v2_journal.count = 128;
    if (g_v2_journal.count > 0) {
        fread(g_v2_journal.entries, sizeof(M11_V2_JournalEntry), g_v2_journal.count, f);
    }
    
    fclose(f);
    return true;
}

/* V2 Journal — exploration and event log */

#define V2_JOURNAL_MAX_ENTRIES 256
#define V2_JOURNAL_TEXT_MAX 128

typedef struct {
    uint32_t game_tick;
    int level;
    int map_x, map_y;
    char text[V2_JOURNAL_TEXT_MAX];
    int entry_type; /* 0=explore, 1=combat, 2=puzzle, 3=item, 4=death */
} V2_JournalEntry;

static V2_JournalEntry g_journal[V2_JOURNAL_MAX_ENTRIES];
static int g_journal_count = 0;
static int g_journal_read_index = 0;

void v2_journal_add(uint32_t tick, int level, int x, int y,
    int type, const char *text)
{
    V2_JournalEntry *e;
    if (g_journal_count >= V2_JOURNAL_MAX_ENTRIES) {
        /* Shift entries down (circular would be better but this is simple) */
        memmove(&g_journal[0], &g_journal[1],
            (V2_JOURNAL_MAX_ENTRIES - 1) * sizeof(V2_JournalEntry));
        g_journal_count = V2_JOURNAL_MAX_ENTRIES - 1;
    }
    e = &g_journal[g_journal_count++];
    e->game_tick = tick;
    e->level = level;
    e->map_x = x; e->map_y = y;
    e->entry_type = type;
    if (text) {
        strncpy(e->text, text, V2_JOURNAL_TEXT_MAX - 1);
        e->text[V2_JOURNAL_TEXT_MAX - 1] = '\0';
    } else {
        e->text[0] = '\0';
    }
}

int v2_journal_count(void) { return g_journal_count; }

const V2_JournalEntry *v2_journal_get(int index) {
    if (index < 0 || index >= g_journal_count) return NULL;
    return &g_journal[index];
}

void v2_journal_mark_read(int index) {
    if (index >= g_journal_read_index) g_journal_read_index = index + 1;
}

int v2_journal_unread_count(void) {
    return g_journal_count - g_journal_read_index;
}

void v2_journal_clear(void) { g_journal_count = 0; g_journal_read_index = 0; }

