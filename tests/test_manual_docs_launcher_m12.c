#include "manual_docs_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int g_failures = 0;

static void check(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int file_exists(const char *path) {
    struct stat st;
    return path && stat(path, &st) == 0;
}

static int starts_with(const char *text, const char *prefix) {
    size_t n;
    if (!text || !prefix) {
        return 0;
    }
    n = strlen(prefix);
    return strncmp(text, prefix, n) == 0;
}

static void check_entry_shape(void) {
    int i;
    int count = M12_ManualDocs_EntryCount();
    check(count == 5, "manual/docs catalog should expose the five launcher docs");
    for (i = 0; i < count; ++i) {
        int j;
        const M12_ManualDocsEntry *entry = M12_ManualDocs_GetEntry(i);
        check(entry != NULL, "entry should be addressable by index");
        if (!entry) {
            continue;
        }
        check(entry->id && entry->id[0], "entry id should be non-empty");
        check(entry->title && entry->title[0], "entry title should be non-empty");
        check(entry->summary && entry->summary[0], "entry summary should be non-empty");
        check(entry->repoPath && entry->repoPath[0], "entry repo path should be non-empty");
        check(entry->url && entry->url[0], "entry URL should be non-empty");
        check(entry->repoPath[0] != '/', "entry repo path should be repository-relative");
        check(strstr(entry->repoPath, "..") == NULL, "entry repo path should not escape the repository");
        check(starts_with(entry->url, "https://github.com/yeager/firestaff"),
              "entry URL should stay on the Firestaff GitHub project");
        check(file_exists(entry->repoPath), "entry repo path should exist from the source root");
        for (j = i + 1; j < count; ++j) {
            const M12_ManualDocsEntry *other = M12_ManualDocs_GetEntry(j);
            if (other) {
                check(strcmp(entry->id, other->id) != 0, "entry ids should be unique");
            }
        }
    }
}

static void check_lookup_contract(void) {
    const M12_ManualDocsEntry *readme = M12_ManualDocs_FindById("readme");
    check(readme != NULL, "readme should be findable by id");
    check(M12_ManualDocs_FindById("data-setup") != NULL, "data setup should be findable by id");
    check(M12_ManualDocs_FindById("platform-matrix") != NULL, "platform matrix should be findable by id");
    check(M12_ManualDocs_FindById("dmweb-reference") != NULL, "reference docs should be findable by id");
    check(M12_ManualDocs_FindById("release-notes") != NULL, "release notes should be findable by id");
    check(M12_ManualDocs_FindById(NULL) == NULL, "NULL id should not match");
    check(M12_ManualDocs_FindById("") == NULL, "empty id should not match");
    check(M12_ManualDocs_FindById("gap-list") == NULL, "internal gap list should not be exposed as user manual docs");
    check(M12_ManualDocs_GetEntry(-1) == NULL, "negative index should be rejected");
    check(M12_ManualDocs_GetEntry(M12_ManualDocs_EntryCount()) == NULL, "end index should be rejected");
    if (readme) {
        check(strcmp(M12_ManualDocs_DefaultUrl(), readme->url) == 0,
              "default launcher URL should point to the README manual entry");
    }
}

int main(void) {
    check_entry_shape();
    check_lookup_contract();
    if (g_failures != 0) {
        fprintf(stderr, "manual docs launcher: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("manual docs launcher: PASS\n");
    return 0;
}
