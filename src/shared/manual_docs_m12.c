#include "manual_docs_m12.h"

#include <string.h>

static const M12_ManualDocsEntry g_manualDocsEntries[] = {
    {
        "readme",
        "Firestaff Manual",
        "Project overview, supported games, data setup, build steps, and legal note.",
        "README.md",
        "https://github.com/yeager/firestaff#readme"
    },
    {
        "data-setup",
        "Game Data Setup",
        "How to provide user-owned game files and organize the Firestaff data root.",
        "docs/DATA_SETUP.md",
        "https://github.com/yeager/firestaff/blob/main/docs/DATA_SETUP.md"
    },
    {
        "platform-matrix",
        "Platform Matrix",
        "Current per-game and per-version support matrix.",
        "docs/PLATFORM_MATRIX.md",
        "https://github.com/yeager/firestaff/blob/main/docs/PLATFORM_MATRIX.md"
    },
    {
        "dmweb-reference",
        "Reference Sources",
        "Reviewed dmweb and Greatstone reference pages for source-fidelity work.",
        "docs/DMWEB_REFERENCE.md",
        "https://github.com/yeager/firestaff/blob/main/docs/DMWEB_REFERENCE.md"
    },
    {
        "release-notes",
        "Release Notes",
        "User-facing release notes for current and recent Firestaff builds.",
        "RELEASE_NOTES.md",
        "https://github.com/yeager/firestaff/blob/main/RELEASE_NOTES.md"
    }
};

int M12_ManualDocs_EntryCount(void) {
    return (int)(sizeof(g_manualDocsEntries) / sizeof(g_manualDocsEntries[0]));
}

const M12_ManualDocsEntry *M12_ManualDocs_GetEntry(int index) {
    if (index < 0 || index >= M12_ManualDocs_EntryCount()) {
        return NULL;
    }
    return &g_manualDocsEntries[index];
}

const M12_ManualDocsEntry *M12_ManualDocs_FindById(const char *id) {
    int i;
    if (!id || !id[0]) {
        return NULL;
    }
    for (i = 0; i < M12_ManualDocs_EntryCount(); ++i) {
        if (strcmp(g_manualDocsEntries[i].id, id) == 0) {
            return &g_manualDocsEntries[i];
        }
    }
    return NULL;
}

const char *M12_ManualDocs_DefaultUrl(void) {
    const M12_ManualDocsEntry *entry = M12_ManualDocs_FindById("readme");
    return entry ? entry->url : "https://github.com/yeager/firestaff#readme";
}
