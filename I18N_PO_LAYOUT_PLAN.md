# I18N / L10N Plan: `po/` Layout, Per-Game Catalogs, and `.pot` Templates

## Goal

Move Firestaff toward a translation layout that is simple, explicit, and product-aligned:

- one catalog per game or front-end surface
- all catalogs stored under `po/`
- filenames follow **`<domain>.<lang>.po`**
- English source templates live as **one `.pot` per domain**
- the startup menu is treated as its own domain, not mixed into a game catalog

This matches the product split better than a single monolithic `firestaff.po` file.

---

## Required File Layout

```text
po/
  startup-menu.pot
  startup-menu.en.po
  startup-menu.sv.po

  dm1.pot
  dm1.en.po
  dm1.sv.po

  csb.pot
  csb.en.po
  csb.sv.po

  dm2.pot
  dm2.en.po
  dm2.sv.po
```

Optional later:

```text
  shared-ui.pot
  shared-ui.en.po
  shared-ui.sv.po
```

But **do not start with `shared-ui` unless it is genuinely needed**. Prefer a cleaner first pass with the four explicit domains above.

---

## Naming Rules

### Domain names

Use these stable domain names:

- `startup-menu`
- `dm1`
- `csb`
- `dm2`

These names should be used consistently in:
- filenames
- extraction scripts
- runtime catalog loading
- probes/tests
- documentation

### Translation files

Pattern:

```text
<domain>.<lang>.po
```

Examples:

- `startup-menu.sv.po`
- `dm1.en.po`
- `csb.sv.po`
- `dm2.en.po`

### Template files

Pattern:

```text
<domain>.pot
```

Examples:

- `startup-menu.pot`
- `dm1.pot`
- `csb.pot`
- `dm2.pot`

---

## Domain Ownership

### 1. `startup-menu`

Contains strings for:
- top-level launcher
- settings visible before entering a game
- mode selection (V1 / V2 / V3)
- game cards and labels
- credits/about shown from the launcher
- launcher-only errors/status text

**Important:** startup-menu strings must not be mixed into `dm1.pot`.

### 2. `dm1`

Contains strings for:
- DM1 in-game HUD
- DM1 runtime messages
- DM1-specific prompts, labels, and menus
- any DM1-specific story/ending text that Firestaff surfaces

### 3. `csb`

Contains strings for:
- CSB-specific runtime flow
- CSB-specific launcher/runtime text
- champion selection / utility / new-adventure / resume semantics once implemented
- Hint Oracle text later, if it becomes part of Firestaff V1 support

### 4. `dm2`

Contains strings for:
- DM2-specific runtime/UI content once DM2 is implemented
- keep the domain present early so the structure is stable, even if content is initially minimal

---

## English Strategy

English should exist in **two forms**:

1. `.pot` templates
   - canonical source-string templates per domain
2. `.en.po` catalogs
   - explicit English catalog per domain
   - useful for runtime symmetry, testing, tooling, and future editorial rewrites without recompiling string literals

### Rule

The runtime should load `.en.po` as the actual English catalog.
`.pot` is the template source for translators and merge tools.

---

## Runtime Loading Model

Introduce a runtime model based on **domain + language**.

Example API direction:

```c
int i18n_compat_load_domain(const char *domain, const char *lang_code);
const char *i18n_compat_tr(const char *domain, const char *key);
```

Or, if the active game domain is implicit in the current game state:

```c
const char *i18n_compat_tr(const char *key);
```

with internal state like:
- active language = `sv`
- active domain = `startup-menu` or `dm1` or `csb` or `dm2`

### Recommended behavior

- launcher loads `startup-menu.<lang>.po`
- entering DM1 loads `dm1.<lang>.po`
- entering CSB loads `csb.<lang>.po`
- entering DM2 loads `dm2.<lang>.po`

### Fallback chain

For any lookup:
1. active domain in active language
2. same domain in English
3. key itself

Do **not** silently fall back across unrelated domains.
For example, `csb` should not quietly read from `dm1` unless explicitly designed later.

---

## Extraction Plan

Create one extraction target per domain.

### Output targets

- `po/startup-menu.pot`
- `po/dm1.pot`
- `po/csb.pot`
- `po/dm2.pot`

### First-pass extraction ownership

- `startup-menu.pot`
  - `menu_startup_m12.c`
  - launcher-related helpers
- `dm1.pot`
  - `m11_game_view.c`
  - DM1 runtime/HUD text paths
- `csb.pot`
  - initially mostly stub/manual until real CSB runtime exists
- `dm2.pot`
  - initially stub/manual until DM2 runtime exists

### Tooling direction

Add a script such as:

```text
tools/extract_po_domains.py
```

Responsibilities:
- scan relevant source files
- emit one `.pot` per domain
- optionally bootstrap/update matching `.en.po`
- fail if duplicate keys with conflicting source text appear inside the same domain

---

## String Key Conventions

Use stable semantic keys, not raw English as keys.

Examples:

### startup-menu
- `startup.title`
- `startup.game.dm1`
- `startup.game.csb`
- `startup.game.dm2`
- `startup.mode.v1`
- `startup.mode.v2`
- `startup.mode.v3`
- `startup.settings.title`

### dm1
- `dm1.hud.food`
- `dm1.hud.water`
- `dm1.log.party_attack`
- `dm1.log.torch_dims`
- `dm1.log.torch_burns_out`

### csb
- `csb.menu.utility`
- `csb.menu.make_new_adventure`
- `csb.menu.resume`
- `csb.hint.title`

### dm2
- keep future-facing keys scoped under `dm2.*`

---

## Incremental Implementation Order

### Slice 1: Filesystem and naming contract ✅ DONE

Deliverables:
- create `po/` ✅
- add seed files with valid PO headers and representative keys: ✅
  - `startup-menu.pot` / `startup-menu.en.po`
  - `dm1.pot` / `dm1.en.po`
  - `csb.pot` / `csb.en.po`
  - `dm2.pot` / `dm2.en.po`
- document naming and domain rules ✅ (`po/README.md`)
- structural validation script ✅ (`po/validate_po_layout.sh`)
- M12_REMAINING_PLAN.md updated to reference new layout ✅

### Slice 2: Startup menu domain

Deliverables:
- runtime can load `startup-menu.<lang>.po`
- launcher strings move out of code literals into domain keys
- startup menu probe verifies English load + non-English load

### Slice 3: DM1 domain

Deliverables:
- DM1 HUD/log/runtime strings move to `dm1.<lang>.po`
- DM1 game view uses domain lookups
- M11 probes remain green

### Slice 4: Swedish catalogs

Deliverables:
- `startup-menu.sv.po`
- `dm1.sv.po`
- later `csb.sv.po`, `dm2.sv.po`

### Slice 5: CSB groundwork

Deliverables:
- once CSB runtime flow exists, populate `csb.pot` and `csb.en.po`
- do not fake CSB coverage before runtime exists

### Slice 6: DM2 groundwork

Deliverables:
- same principle for `dm2`

---

## Probe / Verification Plan

### New probes to add

1. **I18N loader probe**
   - loads `startup-menu.en.po`
   - loads `startup-menu.sv.po`
   - verifies fallback to English
   - verifies fallback to key

2. **Domain isolation probe**
   - a key missing from `csb` must not silently resolve from `dm1`

3. **Template/catalog consistency check**
   - every `*.en.po` key must exist in matching `*.pot`
   - every required Swedish file should match the English key set

### Existing probes that must stay green

- `./run_firestaff_m12_startup_menu_probe.sh`
- `./run_firestaff_m12_settings_smoke.sh`
- if DM1 runtime strings are touched:
  - `./run_firestaff_m11_phase_a_probe.sh`
  - `./run_firestaff_m11_game_view_probe.sh`
  - `./run_firestaff_m11_launcher_smoke.sh`
  - `./run_firestaff_m10_verify.sh "$HOME/.firestaff/data/GRAPHICS.DAT"`

---

## Migration Plan from Current M12 i18n Idea

Current planning text assumes a monolithic structure like `locale/<lang>/firestaff.po`.

### Replace that with

- `po/startup-menu.pot`
- `po/startup-menu.<lang>.po`
- `po/dm1.pot`
- `po/dm1.<lang>.po`
- `po/csb.pot`
- `po/csb.<lang>.po`
- `po/dm2.pot`
- `po/dm2.<lang>.po`

### Why this is better

- aligns with product structure
- avoids mixing launcher strings with in-game strings
- avoids mixing DM1/CSB/DM2 semantics
- makes ownership clearer for future extraction and translation review
- keeps CSB and DM2 honest, even when sparsely populated at first

---

## Recommendation

Adopt this as the new i18n/l10n contract:

- `po/` at repo root
- one `.pot` per domain
- one `.po` per domain per language
- startup menu treated as its own domain
- English present in `.en.po`, not only as source literals
- runtime fallback only within the same domain, then English, then key

This is the cleanest path for Firestaff's new V1 / V2 / V3 product structure.
