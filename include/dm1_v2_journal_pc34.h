#ifndef FIRESTAFF_DM1_V2_JOURNAL_PC34_H
#define FIRESTAFF_DM1_V2_JOURNAL_PC34_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_V2_JC_DISCOVERY,
    M11_V2_JC_COMBAT,
    M11_V2_JC_PUZZLE,
    M11_V2_JC_LORE
} M11_V2_JournalCategory;

typedef struct {
    uint32_t tick;
    M11_V2_JournalCategory category;
    char text[256];
    int level;
} M11_V2_JournalEntry;

typedef struct {
    M11_V2_JournalEntry entries[128];
    int count;
    int page;
    int entries_per_page;
} M11_V2_Journal;

void v2_journal_init(void);
void v2_journal_add(M11_V2_JournalCategory category, const char* text, int level, uint32_t tick);
M11_V2_JournalEntry* v2_journal_get_page(int page);
int v2_journal_get_page_count(int page);
void v2_journal_next_page(void);
void v2_journal_prev_page(void);
void v2_journal_clear(void);
bool v2_journal_save(const char* path);
bool v2_journal_load(const char* path);

#ifdef __cplusplus
}
#endif

#endif
