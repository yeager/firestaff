#include "dm1_v2_journal_pc34.h"

#include <stddef.h>

/* PC34 has no persistent journal or journal page.  Game messages belong to
 * the source-owned message area, so this compatibility surface retains no
 * text, paging state or save file. */

void v2_journal_init(void) {
}

void v2_journal_add(M11_V2_JournalCategory category, const char* text,
                    int level, uint32_t tick) {
    (void)category;
    (void)text;
    (void)level;
    (void)tick;
}

M11_V2_JournalEntry* v2_journal_get_page(int page) {
    (void)page;
    return NULL;
}

int v2_journal_get_page_count(int page) {
    (void)page;
    return 0;
}

void v2_journal_next_page(void) {
}

void v2_journal_prev_page(void) {
}

void v2_journal_clear(void) {
}

bool v2_journal_save(const char* path) {
    (void)path;
    return false;
}

bool v2_journal_load(const char* path) {
    (void)path;
    return false;
}
