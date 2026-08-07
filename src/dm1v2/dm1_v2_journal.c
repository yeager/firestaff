#include "dm1_v2_journal.h"

/* The older V2 API is also compatibility-only.  It must not turn source
 * messages into a host-owned event history or paint a new RGBA panel. */

void dm1_v2_journal_init(DM1_V2_Journal *journal) {
    (void)journal;
}

void dm1_v2_journal_add(DM1_V2_Journal *journal, int tick, int level,
                        int type, const char *text) {
    (void)journal;
    (void)tick;
    (void)level;
    (void)type;
    (void)text;
}

void dm1_v2_journal_render(const DM1_V2_Journal *journal, uint32_t *rgba,
                           int width, int height) {
    (void)journal;
    (void)rgba;
    (void)width;
    (void)height;
}
