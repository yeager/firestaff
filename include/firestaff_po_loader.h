#ifndef FIRESTAFF_PO_LOADER_H
#define FIRESTAFF_PO_LOADER_H

/*
 * Firestaff PO loader — multi-domain i18n.
 *
 * Domains: dm1, csb, dm2, startup-menu, firestaff, nexus. Each
 * domain can be loaded into its own catalog slot via fs_po_load().
 * Lookup is by domain: fs_po_gettext_in_domain("dm1", "FOOD")
 * resolves the Swedish translation of "FOOD" if the dm1.sv.po
 * catalog is loaded.
 *
 * fs_po_gettext() looks up against the active domain (set via
 * fs_po_set_active_domain()).
 */

/* Load a .po file. Derives the domain from the path basename
 * (e.g. "po/dm1.sv.po" -> domain "dm1", language "sv"). */
int fs_po_load(const char *path);

/* Legacy single-domain loader for the firestaff.* domain. */
int fs_po_load_for_language(const char *po_dir, const char *lang);

/* Look up msgid in a specific domain. Returns msgid itself if no
 * translation is loaded, the slot is missing, or the entry is empty. */
const char *fs_po_gettext_in_domain(const char *domain, const char *msgid);

/* Look up msgid in the active domain (or pass-through if no
 * catalog is loaded). */
const char *fs_po_gettext(const char *msgid);

/* Switch the active domain for fs_po_gettext() lookups. */
int fs_po_set_active_domain(const char *domain);

/* Diagnostic helpers. */
int fs_po_get_loaded_count_in_domain(const char *domain);
int fs_po_get_loaded_count(void);
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
