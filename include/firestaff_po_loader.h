
#ifndef FIRESTAFF_PO_LOADER_H
#define FIRESTAFF_PO_LOADER_H

int fs_po_load(const char *path);
int fs_po_load_for_language(const char *po_dir, const char *lang);
const char *fs_po_gettext(const char *msgid);
int fs_po_is_loaded(void);

/* Convenience macro — use like gettext _() */
#define _(s) fs_po_gettext(s)

#endif
