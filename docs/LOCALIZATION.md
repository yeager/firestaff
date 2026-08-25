# Localization

Human-editable `.po` files and `.pot` templates under `po/` are the only
tracked translation sources. Binary gettext `.mo` catalogs are generated in
`<build-dir>/po/` by the `firestaff_gettext_catalogs` build target; they must
not be committed.

`po/firestaff_studio.pot` is the English source template for Firestaff Studio.
Its translatable entries have empty `msgstr` values and are checked against
`po/studio/en.po` during every CMake build. Use it when creating a new language
catalog, then add `po/studio/<language>.po`.

For source-tree execution of the Python studios, set
`FIRESTAFF_LOCALE_DIR=<build-dir>/po/locale` to use the generated catalogs.
