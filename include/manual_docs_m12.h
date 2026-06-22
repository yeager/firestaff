#ifndef FIRESTAFF_MANUAL_DOCS_M12_H
#define FIRESTAFF_MANUAL_DOCS_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *id;
    const char *title;
    const char *summary;
    const char *repoPath;
    const char *url;
} M12_ManualDocsEntry;

int M12_ManualDocs_EntryCount(void);
const M12_ManualDocsEntry *M12_ManualDocs_GetEntry(int index);
const M12_ManualDocsEntry *M12_ManualDocs_FindById(const char *id);
const char *M12_ManualDocs_DefaultUrl(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MANUAL_DOCS_M12_H */
