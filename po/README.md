# po/ — Firestaff Translation Catalogs

Per-domain i18n layout for Firestaff. See `I18N_PO_LAYOUT_PLAN.md` at repo root for the full specification.

## Domains

| Domain         | Scope                                          | Status       |
|----------------|-------------------------------------------------|-------------|
| `startup-menu` | All launcher, scan, cards, settings and errors  | Active |
| `dm1`          | Dungeon Master 1 player-presented text           | Active |
| `csb`          | Chaos Strikes Back player-presented text         | Active |
| `dm2`          | Dungeon Master 2 player-presented text           | Active |
| `nexus`        | Dungeon Master Nexus player-presented text       | Active |
| `theron`       | Theron's Quest player-presented text             | Active |

`firestaff` is the shared in-game shell/settings domain. Firestaff Studio's
three Python applications use the separate `firestaff_studio` gettext domain,
whose template is `firestaff_studio.pot` and whose catalogs live in `studio/`.

## Complete file inventory

Every tracked file in this directory has one of these explicit roles:

- `startup-menu.pot` and `firestaff_studio.pot`: regenerated from explicitly
  marked C/Python presentation calls by `update.sh`.
- `firestaff.pot`, `dm1.pot`, `csb.pot`, `nexus.pot`, and `theron.pot`:
  canonical templates that also contain authenticated, data-decoded game text
  unavailable to `xgettext`. CI canonicalizes and validates these templates,
  then merges every corresponding PO catalog.
- `dm2_source_strings.c` and `dm2.pot`: key-annotated extraction markers and
  their generated template. The markers inventory player-visible strings
  decoded from the hash-admitted PC-DOS English `GRAPHICS.DAT`; command,
  animation and debug metadata are excluded. This marker file is not compiled
  and never replaces the selected platform's original GDAT fallback text.
- `<domain>.<lang>.po`: native runtime source catalog, loaded directly by the
  C application and shipped by DEB, RPM and Steam Deck/AppImage packaging.
- `firestaff_studio.pot` and `studio/<lang>.po`: Python Artpack Studio,
  Dungeon Studio, and Savegame Editor template and source catalogs. CMake and
  each standalone bundler compile these to `.mo` below their build/output
  directory. Recompilation also removes obsolete `firestaff_studio.mo` files
  for languages whose source catalog no longer exists, without touching other
  gettext domains in the same locale tree.
- `dm1_es.po`: authentic item-indexed Spanish `GRAPHICS.DAT` extraction
  evidence. Its contextual IDs are not runtime IDs yet; it is retained as a
  developer source for mapping real media text into `dm1.es.po`.
- `dm1_translations_seed.py` and `dm1_translations_complete.py`: the DM1
  catalog maintenance generator and its imported translation table.
- `translations_other.py`: maintenance generator for the other domains.
- `update.sh`: reproducible source extraction for source-backed templates,
  canonicalization of every template, all-catalog merge, statistics, and CI
  drift checker.
- `generate_completion_table.py`: generates the language/domain matrix below.
- `normalize_pot_headers.py`: applies complete deterministic gettext metadata
  to every source-generated and canonical template.
- `validate_po_layout.sh`: structural, template and coverage CI gate.
- `README.md`: this contract.

Editor backup files, obsolete flat `<lang>.po` catalogs, and superseded DM1
translation fragments are intentionally not kept here. They had no runtime,
packaging, validation, or active maintenance consumer.

## File Naming

- **Templates:** `<domain>.pot` — canonical source-string templates
- **Catalogs:** `<domain>.<lang>.po` — per-language translations

## Rules

- English catalogs (`.en.po`) are loaded at runtime, not just used as source.
- `.pot` files are the template source for translators and merge tools.
- Every non-header `msgstr` in a `.pot` must be empty. English source text is
  carried by `msgid`; `.en.po` is the runtime English catalog.
- Fallback chain: active domain in active language → same domain in English → key itself.
- No cross-domain fallback (e.g., `csb` never silently reads from `dm1`).
- Localization applies in Original and Modern. Original game bytes are decoded
  first and retained verbatim as fallback; translation occurs only while
  presenting them.
- Every player-visible string belongs to a domain. Debug-only telemetry may
  remain untranslated only when it cannot reach a player-facing surface.

## Translation completion

Each cell is `non-fuzzy translated entries / current template entries`.
English is the source language and therefore uses the template itself.
A dash means that the language has no catalog for that domain.

<!-- completion-table:begin -->
| Language | Launcher | Shared UI | DM1 | CSB | DM2 | Nexus | Theron |
|---|---:|---:|---:|---:|---:|---:|---:|
| `cs` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `da` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `de` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `en` | source (443/443) | source (32/32) | source (604/604) | source (221/221) | source (309/309) | source (30/30) | source (38/38) |
| `es` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `fi` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `fr` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `hu` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `id` | 52/443 (11%) | — | — | — | — | — | — |
| `it` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `ja` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `ko` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `nl` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `no` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `pl` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `pt` | 59/443 (13%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `ru` | 59/443 (13%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `sv` | 443/443 (100%) | 32/32 (100%) | 604/604 (100%) | 221/221 (100%) | 309/309 (100%) | 30/30 (100%) | 38/38 (100%) |
| `tr` | 59/443 (13%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
| `zh` | 52/443 (11%) | 29/32 (90%) | 544/604 (90%) | 119/221 (53%) | — | 30/30 (100%) | 38/38 (100%) |
<!-- completion-table:end -->

¹ `N/A` means that the domain currently has no extracted source entries. It is a coverage gap, not 100% completion.

## Validation

Run `po/validate_po_layout.sh` to verify structural consistency, valid gettext
syntax, and empty template translations. The
validator reports both `nonblank` coverage and `native` coverage:
`msgstr == msgid` counts as fallback/scaffold coverage, not as native
translation. Fallback-only catalogs are marked `FALL` but do not fail the
structural CI gate.

Run `python3 po/generate_completion_table.py --check` to check only this
README's generated completion matrix. Run `bash po/update.sh --check` for the
complete source-template, canonical-template, catalog, and matrix drift gate.

DM1's `581/581` Swedish catalog figure includes ten explicitly admitted live
M11 status/readout literals. It is guarded by
`dm1_m11_presented_catalog_source_lock`; unmarked literals in the shared M11
translation unit are not silently counted as DM1 because they may belong to
CSB, DM2, Nexus, or Theron. Completion is catalog coverage, not whole-game
localization or behavioral parity.
