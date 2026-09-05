
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Simple PO file loader for Firestaff.
 * Reads msgid/msgstr pairs from .po files.
 * Not a full gettext implementation — just enough for UI strings.
 *
 * Multi-domain catalog: each domain (dm1, csb, dm2, startup-menu,
 * firestaff, nexus) loads into its own slot so callers can mix
 * dm1+csb+startup-menu catalogs concurrently.  Domain routing is
 * done at lookup time: fs_po_gettext_in_domain() resolves against
 * the named domain only.
 *
 * Capacity: DM1 ships 548 msgids, startup-menu 67, firestaff 33,
 * nexus 31, csb 1, dm2 1.  FS_PO_MAX_STRINGS is sized to fit the
 * largest catalog with headroom for new extraction passes. */

#define FS_PO_MAX_STRINGS 1024
#define FS_PO_MAX_LEN 512
#define FS_PO_DOMAIN_COUNT 8
#define FS_PO_DOMAIN_NAME_MAX 16

typedef struct {
    char msgid[FS_PO_MAX_LEN];
    char msgstr[FS_PO_MAX_LEN];
} FS_POEntry;

typedef struct {
    FS_POEntry entries[FS_PO_MAX_STRINGS];
    int count;
    int loaded;            /* 1 if this slot has a valid catalog */
    char domain[FS_PO_DOMAIN_NAME_MAX];
    char language[8];
} FS_POCatalog;

static FS_POCatalog g_catalogs[FS_PO_DOMAIN_COUNT];
static int g_active_domain = -1; /* index into g_catalogs, or -1 for "none loaded" */

static int find_domain_slot(const char* domain) {
    int i;
    if (!domain) return -1;
    for (i = 0; i < FS_PO_DOMAIN_COUNT; ++i) {
        if (g_catalogs[i].loaded &&
            strcmp(g_catalogs[i].domain, domain) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void) {
    int i;
    for (i = 0; i < FS_PO_DOMAIN_COUNT; ++i) {
        if (!g_catalogs[i].loaded) return i;
    }
    return -1;
}

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

static void append_po_quoted(char *dst, size_t dst_size, const char *quoted)
{
    char part[FS_PO_MAX_LEN];
    size_t used;
    if (!dst || dst_size == 0u || !quoted) return;
    strncpy(part, quoted, sizeof(part) - 1u);
    part[sizeof(part) - 1u] = '\0';
    strip_quotes(part);
    used = strlen(dst);
    if (used + strlen(part) < dst_size) {
        strncat(dst, part, dst_size - 1u - used);
    }
}

static void commit_po_entry(FS_POCatalog *cat, const char *msgid,
                            const char *msgstr, int fuzzy)
{
    if (!cat || !msgid || !msgstr || fuzzy || !msgid[0] || !msgstr[0] ||
        cat->count >= FS_PO_MAX_STRINGS) return;
    strncpy(cat->entries[cat->count].msgid, msgid, FS_PO_MAX_LEN - 1);
    cat->entries[cat->count].msgid[FS_PO_MAX_LEN - 1] = '\0';
    strncpy(cat->entries[cat->count].msgstr, msgstr, FS_PO_MAX_LEN - 1);
    cat->entries[cat->count].msgstr[FS_PO_MAX_LEN - 1] = '\0';
    cat->count++;
}

/* Derive domain from path: e.g. "po/dm1.sv.po" -> "dm1". */
static void extract_domain_from_path(const char* path, char* out, size_t outSize) {
    const char* base;
    const char* dot;
    size_t len;
    if (!path || outSize == 0) {
        if (outSize > 0) out[0] = '\0';
        return;
    }
    /* Find last path separator */
    base = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    /* Find first dot after base */
    dot = strchr(base, '.');
    if (!dot) {
        /* No dot — use whole basename */
        dot = base + strlen(base);
    }
    len = (size_t)(dot - base);
    if (len >= outSize) len = outSize - 1;
    memcpy(out, base, len);
    out[len] = '\0';
}

/* Derive language from path: e.g. "po/dm1.sv.po" -> "sv". */
static void extract_lang_from_path(const char* path, char* out, size_t outSize) {
    const char* base;
    const char* dot1;
    const char* dot2;
    size_t len;
    if (!path || outSize == 0) {
        if (outSize > 0) out[0] = '\0';
        return;
    }
    base = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    dot1 = strchr(base, '.');
    if (!dot1) {
        if (outSize > 0) out[0] = '\0';
        return;
    }
    dot2 = strchr(dot1 + 1, '.');
    if (!dot2) {
        if (outSize > 0) out[0] = '\0';
        return;
    }
    len = (size_t)(dot2 - (dot1 + 1));
    if (len >= outSize) len = outSize - 1;
    if (len > 7) len = 7; /* language[8] is 8 bytes */
    memcpy(out, dot1 + 1, len);
    out[len] = '\0';
}

int fs_po_load(const char *path) {
    FILE* f;
    char line[1024];
    char current_msgid[FS_PO_MAX_LEN] = {0};
    char current_msgstr[FS_PO_MAX_LEN] = {0};
    int field = 0; /* 1 = msgid, 2 = msgstr */
    int fuzzy = 0;
    int slot;
    char domain[FS_PO_DOMAIN_NAME_MAX];
    char lang[8];
    FS_POCatalog* cat;

    if (!path) return -1;

    extract_domain_from_path(path, domain, sizeof(domain));
    extract_lang_from_path(path, lang, sizeof(lang));
    if (domain[0] == '\0') return -1;

    /* Reuse existing slot for this domain, else take a free one. */
    slot = find_domain_slot(domain);
    if (slot < 0) {
        slot = find_free_slot();
    }
    if (slot < 0) return -1; /* no room */

    cat = &g_catalogs[slot];
    memset(cat, 0, sizeof(*cat));
    strncpy(cat->domain, domain, sizeof(cat->domain) - 1);
    strncpy(cat->language, lang, sizeof(cat->language) - 1);

    f = fopen(path, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        /* Strip newline */
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = 0;

        if (strncmp(line, "#,", 2) == 0) {
            if (strstr(line, "fuzzy") != NULL) fuzzy = 1;
        } else if (strncmp(line, "msgid ", 6) == 0) {
            commit_po_entry(cat, current_msgid, current_msgstr, fuzzy);
            current_msgid[0] = '\0';
            current_msgstr[0] = '\0';
            strncpy(current_msgid, line + 6, FS_PO_MAX_LEN - 1);
            current_msgid[FS_PO_MAX_LEN - 1] = '\0';
            strip_quotes(current_msgid);
            field = 1;
        } else if (strncmp(line, "msgstr ", 7) == 0) {
            strncpy(current_msgstr, line + 7, FS_PO_MAX_LEN - 1);
            current_msgstr[FS_PO_MAX_LEN - 1] = '\0';
            strip_quotes(current_msgstr);
            field = 2;
        } else if (line[0] == '"') {
            if (field == 1)
                append_po_quoted(current_msgid, sizeof(current_msgid), line);
            else if (field == 2)
                append_po_quoted(current_msgstr, sizeof(current_msgstr), line);
        } else if (line[0] == '\0') {
            commit_po_entry(cat, current_msgid, current_msgstr, fuzzy);
            current_msgid[0] = '\0';
            current_msgstr[0] = '\0';
            field = 0;
            fuzzy = 0;
        }
    }

    commit_po_entry(cat, current_msgid, current_msgstr, fuzzy);

    fclose(f);
    cat->loaded = 1;
    if (g_active_domain < 0) g_active_domain = slot;
    return cat->count;
}

const char* fs_po_gettext_in_domain(const char* domain, const char* msgid) {
    int slot;
    int i;
    if (!msgid) return msgid;
    if (!domain) return msgid;
    slot = find_domain_slot(domain);
    if (slot < 0) return msgid;
    for (i = 0; i < g_catalogs[slot].count; i++) {
        if (strcmp(g_catalogs[slot].entries[i].msgid, msgid) == 0) {
            if (g_catalogs[slot].entries[i].msgstr[0])
                return g_catalogs[slot].entries[i].msgstr;
            return msgid; /* empty msgstr = use original */
        }
    }
    return msgid; /* not found = use original */
}

const char* fs_po_gettext(const char* msgid) {
    if (g_active_domain < 0) return msgid;
    return fs_po_gettext_in_domain(g_catalogs[g_active_domain].domain, msgid);
}

int fs_po_set_active_domain(const char* domain) {
    int slot;
    if (!domain) return -1;
    slot = find_domain_slot(domain);
    if (slot < 0) return -1;
    g_active_domain = slot;
    return 0;
}

int fs_po_get_loaded_count_in_domain(const char* domain) {
    int slot = find_domain_slot(domain);
    return (slot < 0) ? 0 : g_catalogs[slot].count;
}

int fs_po_get_loaded_count(void) {
    if (g_active_domain < 0) return 0;
    return g_catalogs[g_active_domain].count;
}

int fs_po_is_loaded(void) {
    return fs_po_get_loaded_count() > 0;
}

/* Convenience: load PO for current language (legacy single-domain API).
 * Uses active domain if set, otherwise looks up by basename. */
int fs_po_load_for_language(const char* po_dir, const char* lang) {
    char path[512];
    snprintf(path, sizeof(path), "%s/firestaff.%s.po", po_dir, lang);
    return fs_po_load(path);
}
