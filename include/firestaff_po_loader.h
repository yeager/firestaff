#ifndef FIRESTAFF_PO_LOADER_H
#define FIRESTAFF_PO_LOADER_H

int fs_po_load(const char *path);
int fs_po_load_for_language(const char *po_dir, const char *lang);
const char *fs_po_gettext(const char *msgid);
int fs_po_is_loaded(void);

/* Convenience macro — use like gettext _() */
#define _(s) fs_po_gettext(s)

/* i18n stage 1: allow static initialization with gettext-style macro.
 * When NLS is not available, fall back to plain string literals. */
#ifndef FS_I18N_STATIC_GETTEXT
#define FS_I18N_STATIC_GETTEXT 1
#endif

#if FS_I18N_STATIC_GETTEXT && !defined(I18N_USE_GETTEXT)
#undef _
#define _(s) (s)
#endif

#endif /* FIRESTAFF_PO_LOADER_H */
