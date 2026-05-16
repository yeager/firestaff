
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Simple PO file loader for Firestaff.
 * Reads msgid/msgstr pairs from .po files.
 * Not a full gettext implementation — just enough for UI strings. */

#define FS_PO_MAX_STRINGS 128
#define FS_PO_MAX_LEN 256

typedef struct {
    char msgid[FS_PO_MAX_LEN];
    char msgstr[FS_PO_MAX_LEN];
} FS_POEntry;

typedef struct {
    FS_POEntry entries[FS_PO_MAX_STRINGS];
    int count;
    char language[8];
} FS_POCatalog;

static FS_POCatalog g_catalog;

static void strip_quotes(char *s) {
    int len = (int)strlen(s);
    if (len >= 2 && s[0] == '"' && s[len-1] == '"') {
        memmove(s, s + 1, len - 2);
        s[len - 2] = 0;
    }
    /* Unescape \n */
    char *p;
    while ((p = strstr(s, "\\n")) != NULL) {
        *p = '\n'; memmove(p + 1, p + 2, strlen(p + 2) + 1);
    }
}

int fs_po_load(const char *path) {
    FILE *f;
    char line[512];
    char current_msgid[FS_PO_MAX_LEN] = {0};
    int in_msgstr = 0;

    if (!path) return -1;
    f = fopen(path, "r");
    if (!f) return -1;

    g_catalog.count = 0;

    while (fgets(line, sizeof(line), f)) {
        /* Strip newline */
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = 0;

        if (strncmp(line, "msgid ", 6) == 0) {
            strncpy(current_msgid, line + 6, FS_PO_MAX_LEN - 1);
            strip_quotes(current_msgid);
            in_msgstr = 0;
        } else if (strncmp(line, "msgstr ", 7) == 0) {
            char msgstr[FS_PO_MAX_LEN];
            strncpy(msgstr, line + 7, FS_PO_MAX_LEN - 1);
            strip_quotes(msgstr);

            if (current_msgid[0] && msgstr[0] && g_catalog.count < FS_PO_MAX_STRINGS) {
                strncpy(g_catalog.entries[g_catalog.count].msgid, current_msgid, FS_PO_MAX_LEN - 1);
                strncpy(g_catalog.entries[g_catalog.count].msgstr, msgstr, FS_PO_MAX_LEN - 1);
                g_catalog.count++;
            }
            in_msgstr = 1;
        }
    }

    fclose(f);
    return g_catalog.count;
}

const char *fs_po_gettext(const char *msgid) {
    int i;
    if (!msgid) return msgid;
    for (i = 0; i < g_catalog.count; i++) {
        if (strcmp(g_catalog.entries[i].msgid, msgid) == 0) {
            if (g_catalog.entries[i].msgstr[0])
                return g_catalog.entries[i].msgstr;
            return msgid; /* empty msgstr = use original */
        }
    }
    return msgid; /* not found = use original */
}

int fs_po_is_loaded(void) {
    return g_catalog.count > 0;
}

/* Convenience: load PO for current language */
int fs_po_load_for_language(const char *po_dir, const char *lang) {
    char path[512];
    snprintf(path, sizeof(path), "%s/firestaff.%s.po", po_dir, lang);
    return fs_po_load(path);
}
