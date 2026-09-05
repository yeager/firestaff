# Localization

Human-editable `.po` files and `.pot` templates under `po/` are the only
tracked translation sources. Binary gettext `.mo` catalogs are generated in
`<build-dir>/po/` by the `firestaff_gettext_catalogs` build target; they must
not be committed.

`po/firestaff_studio.pot` is the English source template for Firestaff Studio.
Its translatable entries have empty `msgstr` values and are checked against
`po/studio/en.po` during every CMake build. Use it when creating a new language
catalog, then add `po/studio/<language>.po`.

The native launcher and all five games use the seven domain templates and
catalogs documented in `po/README.md`. Release packaging installs every one of
those runtime `.po` catalogs under `/usr/share/firestaff/po`; runtime lookup
also honors `FIRESTAFF_LOCALE_DIR`. The game-data directory is not the source
of translations.

For source-tree execution of the Python studios, set
`FIRESTAFF_LOCALE_DIR=<build-dir>/po/locale` to use the generated catalogs.
Standalone Studio bundlers generate the same `.mo` tree in their own output
directory before invoking PyInstaller; they never write generated catalogs
under `po/`.

M11 translates player-presented status and inspect strings at the final
presentation boundary using the active game's domain selected from
`M11_GameViewState.sourceKind`. A catalog's percentage alone does not prove
that every runtime literal was extracted. DM1-only M11 literals therefore use
the identity marker `M11_DM1_PRESENTED("...")`; the marker does not perform a
lookup or bypass domain selection. `tools/verify_dm1_m11_presented_catalog.py`
requires every marked literal to exist in `dm1.pot`, have a nonblank Swedish
translation, and retain the explicit CSB/DM2/Nexus/Theron domain mapping.
This prevents shared-file literals owned by another game from being added to
DM1 merely to preserve a misleading 100% figure.

The first two audited groups contain ten marked literals: six pickup/drop/
spell strings and four source-locked CLIKVIEW.C fountain strings. The latter
cover the fountain label, drinking, waterskin/container refill, and empty
flask refill feedback.
