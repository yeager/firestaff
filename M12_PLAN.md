# Firestaff M12 — User-Facing Configuration Plan

**Status:** Plan (implementation follows M11 first playable).
**Created:** 2026-04-20
**Depends on:** M10 (complete — 20-phase deterministic engine core, 500+ invariants), M11 (parallel — SDL3 integration).
**Parallel with:** M11 late phases (M12 Phase A–D can begin as soon as M11 Phase G lands `fs_portable_compat`).

---

## §1  Scope

### What M12 is

M12 transforms Firestaff from a running game with hardcoded defaults (M11's output) into a **product** that users can install, configure, and enjoy without touching a terminal. Every user-visible setting becomes first-class: persisted, editable, validated, and internationalised.

This milestone is about **polish and configurability**, not new game features. M10 provides the engine; M11 wires it to SDL3 for rendering, audio, and input; M12 wraps the whole thing in a professional shell.

### In scope

| Area | Deliverable |
|------|-------------|
| **Startup menu** | Game picker with box-art (DM1 / CSB / DM2), greyed-out unavailable games, first-run data-path popup, settings tabs |
| **Asset validator** | MD5-based file discovery across user-specified directory; per-game asset lists; caching |
| **Bug-fix toggle UI** | Full 201-entry UI (93 bugs + 108 changes); version presets; custom mix; per-game gating; save-profile mismatch warning |
| **Internationalisation** | Runtime language switch (no restart); .po file pipeline; en/sv/fr/de; RTL readiness |
| **Graphics settings** | Graphics level (Original / Upscaled / AI-HD); scaling mode; resolution; window mode; optional filters |
| **Audio settings** | Master / SFX / Music / UI volume; device selection; buffer size |
| **Input settings** | Full key rebinding; mouse sensitivity; gamepad mapping (SDL3); keyboard layout for key-name display |
| **Persistent config** | Cross-platform config file (TOML); schema versioning; atomic writes; defaults layering |
| **Credits / About** | Version, engine credits, original team, Fontanel, licence info |

### Out of scope (deferred)

| Deferral | Target |
|----------|--------|
| Steam Workshop / mod browser | M14+ |
| Online leaderboards | far future |
| Cloud save sync | far future |
| Achievements | far future |
| Replay playback UI | M13 (recording already in M10 Phase 20) |
| Lore Museum / preservation section | M13 |
| DM2 integration | M13 (slot visible in game picker, greyed out) |
| Custom dungeon editor | M14+ |
| Mobile ports (iOS/Android) | V3+ |

### Architecture principles

1. **Build on M11** — M12 consumes M11's SDL + fs + input modules. No duplication.
2. **Menu is just another SDL scene** — renders with M11's render pipeline, receives input via M11's input handler. No separate "menu framework".
3. **Declarative UI** — menu layouts defined in TOML data, not hardcoded C structs. Easier for i18n and theming.
4. **Config-first** — every user-visible option is first-class in the config file. No "magic defaults" buried in C.
5. **Bug-toggles integrate with M10** — each toggle maps to a bit in `bug_flags_compat` bit-mask consumed by M10's tick orchestrator. Changing a toggle recomputes world-hash.
6. **Zero M10 source edits** — M12 is additive only. All 20 M10 phases continue to PASS unmodified.

---

## §2  Module Decomposition

All new M12 modules use the `_m12_` infix or the `_compat` suffix where they extend existing compat-layer conventions. Each module is pure C (C11), no C++, no OS-specific headers beyond what M11 already abstracts.

### §2.1  `config_io_m12.{h,c}` — TOML Config Read/Write

**Purpose:** Parse, serialise, and validate the user's `config.toml` file. Provides typed getters/setters for every config key with compile-time schema enforcement.

**Public API:**
```c
/* Load config from disk. Returns 0 on success, -1 on error (populates errBuf). */
int config_io_m12_load(const char *path, FirestaffConfig_M12 *out, char *errBuf, size_t errBufSize);

/* Save config atomically (write to .tmp, fsync, rename). Returns 0 on success. */
int config_io_m12_save(const char *path, const FirestaffConfig_M12 *cfg);

/* Apply defaults: fills any missing keys from the built-in defaults. */
void config_io_m12_apply_defaults(FirestaffConfig_M12 *cfg);

/* Merge user config over defaults (user wins). */
void config_io_m12_merge(FirestaffConfig_M12 *base, const FirestaffConfig_M12 *overlay);

/* Round-trip test: serialise → parse → compare. Returns 0 if identical. */
int config_io_m12_roundtrip_check(const FirestaffConfig_M12 *cfg);
```

**Dependencies:** `toml.c` (see §3.1 library choice). M11 `fs_portable_compat` for path resolution.

**Size estimate:** ~1200 LOC.

---

### §2.2  `config_schema_m12.{h,c}` — Validation + Defaults

**Purpose:** Define the `FirestaffConfig_M12` struct, compile-time defaults, and a validation pass that rejects invalid values (e.g. volume > 100, unknown language codes).

**Public API:**
```c
/* The config struct. Every field has a known default. */
typedef struct {
    ConfigGeneral_M12    general;      /* language, first_run_done, data_path */
    ConfigGraphics_M12   graphics;     /* level, scaling, resolution, window_mode, filters */
    ConfigAudio_M12      audio;        /* master/sfx/music/ui volumes, device, buffer_size */
    ConfigInput_M12      input;        /* key bindings, mouse_sensitivity, gamepad, kbd_layout */
    ConfigGame_M12       game;         /* selected_game, bug_profile per game */
    uint32_t             schema_version; /* for forward-compat migration */
} FirestaffConfig_M12;

/* Returns 0 if valid, populates issues[] with human-readable descriptions. */
int config_schema_m12_validate(const FirestaffConfig_M12 *cfg, char issues[][256], int *issueCount, int maxIssues);

/* Returns a config struct with all compile-time defaults. */
FirestaffConfig_M12 config_schema_m12_defaults(void);
```

**Dependencies:** None (pure data definitions).

**Size estimate:** ~600 LOC (mostly struct definitions and default initialisation).

---

### §2.3  `asset_validator_compat.{h,c}` — MD5-Based File Discovery

**Purpose:** Scan a user-specified directory (recursively), compute MD5 hashes, and match against the known-good asset database to determine which games are playable.

**Public API:**
```c
/* Result for a single expected asset. */
typedef struct {
    const char *asset_id;        /* e.g. "DUNGEON.DAT" */
    int         found;           /* 1 if a file with matching MD5 was found */
    char        found_path[512]; /* actual filesystem path of the matched file */
    char        found_md5[33];   /* hex MD5 of matched file */
    char        expected_md5[33];
} AssetMatch_Compat;

/* Result for a single game variant. */
typedef struct {
    const char *game_id;         /* e.g. "DM1" */
    const char *variant_id;      /* e.g. "PC_3.4_EN" */
    int         total_assets;
    int         matched_assets;
    int         playable;        /* 1 if all required assets matched */
    AssetMatch_Compat matches[ASSET_MAX_PER_VARIANT]; /* 16 slots */
} GameValidationResult_Compat;

/* Scan directory, populate results[]. Returns number of game variants checked. */
int asset_validator_compat_scan(
    const char *search_dir,
    const AssetDatabase_M12 *db,
    GameValidationResult_Compat results[],
    int max_results
);

/* Cache-aware version: skip files whose mtime+size match cached values. */
int asset_validator_compat_scan_cached(
    const char *search_dir,
    const AssetDatabase_M12 *db,
    AssetValidatorCache_Compat *cache,
    GameValidationResult_Compat results[],
    int max_results
);

/* Persist / load cache (JSON). */
int asset_validator_compat_cache_save(const char *path, const AssetValidatorCache_Compat *cache);
int asset_validator_compat_cache_load(const char *path, AssetValidatorCache_Compat *cache);
```

**Dependencies:** `md5.c` (see §2.14). M11 `fs_portable_compat` for directory walking.

**Size estimate:** ~900 LOC.

---

### §2.4  `asset_database_m12.{h,c}` — Known-Good MD5 Tables

**Purpose:** Load and query the structured JSON database of known-good MD5 hashes per game/variant/file. Ships as `assets.json` alongside the binary; users can also contribute variant hashes.

**Public API:**
```c
/* Load the asset database from a JSON file. */
int asset_database_m12_load(const char *json_path, AssetDatabase_M12 *db);

/* Free database memory. */
void asset_database_m12_free(AssetDatabase_M12 *db);

/* Query: given a game_id and variant_id, return expected asset list. */
const AssetExpected_M12 *asset_database_m12_query(
    const AssetDatabase_M12 *db,
    const char *game_id,
    const char *variant_id,
    int *count
);

/* Query by MD5: given an MD5 hex string, return all matching entries (for discovery). */
int asset_database_m12_lookup_md5(
    const AssetDatabase_M12 *db,
    const char *md5_hex,
    AssetDatabaseHit_M12 hits[],
    int max_hits
);
```

**Dependencies:** `cJSON.c` (see §2.14 — vendored, MIT-licensed, pure C). No M10/M11 dependency.

**Size estimate:** ~500 LOC.

---

### §2.5  `bug_flags_compat.{h,c}` — 256-Bit Mask + Activation Helpers

**Purpose:** Provide a 256-bit flag mask (4 × `uint64_t`) for the 201 bug/change toggles, with helpers to set/clear/test individual flags, apply presets, compute a profile hash, and compare profiles.

**Public API:**
```c
/* 256-bit mask (supports up to 256 flags; 201 used in v1). */
typedef struct {
    uint64_t words[4]; /* words[0] bits 0-63, words[1] bits 64-127, etc. */
} BugFlagMask_Compat;

/* Set/clear/test individual flag by bit-index. */
void     bug_flags_compat_set(BugFlagMask_Compat *mask, int bit_index);
void     bug_flags_compat_clear(BugFlagMask_Compat *mask, int bit_index);
int      bug_flags_compat_test(const BugFlagMask_Compat *mask, int bit_index);

/* Convenience: check if a named bug is active (original buggy behaviour). */
#define BUG_ACTIVE(world, bug_id) \
    bug_flags_compat_test(&(world)->bugFlags, (bug_id))

/* Apply a preset (overwrites the mask). */
void     bug_flags_compat_apply_preset(BugFlagMask_Compat *mask, BugPreset_Compat preset);

/* Compute CRC32 profile hash for save-file/replay embedding. */
uint32_t bug_flags_compat_profile_hash(const BugFlagMask_Compat *mask);

/* Compare two profiles. Returns 0 if identical, bitmask of differing words otherwise. */
int      bug_flags_compat_compare(const BugFlagMask_Compat *a, const BugFlagMask_Compat *b);

/* Serialise/deserialise to/from 32 bytes (LE). */
void     bug_flags_compat_serialise(const BugFlagMask_Compat *mask, uint8_t out[32]);
void     bug_flags_compat_deserialise(const uint8_t in[32], BugFlagMask_Compat *mask);

/* Count of active flags. */
int      bug_flags_compat_popcount(const BugFlagMask_Compat *mask);

/* Presets. */
typedef enum {
    BUG_PRESET_DM_1_0A_EN_PURIST = 0,
    BUG_PRESET_DM_1_0B_EN,
    BUG_PRESET_DM_1_1_EN,
    BUG_PRESET_DM_1_2_EN,
    BUG_PRESET_DM_1_2_GE,
    BUG_PRESET_DM_1_3A_FR,
    BUG_PRESET_DM_1_3B_FR,
    BUG_PRESET_CSB_2_0_EN,
    BUG_PRESET_CSB_2_1_EN,
    BUG_PRESET_PC_3_4_BASELINE,
    BUG_PRESET_MODERN,
    BUG_PRESET_COUNT
} BugPreset_Compat;
```

**Dependencies:** CRC32 from M10 Phase 15. No other dependency.

**Size estimate:** ~500 LOC.

---

### §2.6  `bug_database_m12.{h,c}` — Bug/Change Database Loader

**Purpose:** Load the 201-entry `firestaff-bugs.json` database at startup; provide query APIs for the bug-toggle UI (filter by game, by category, by affected version, by text search).

**Public API:**
```c
/* A single bug/change entry (loaded from JSON). */
typedef struct {
    char     id[32];              /* "BUG0_02" or "CHANGE7_01_FIX" */
    char     type[8];             /* "bug" or "change" */
    char     symptom[512];
    char     cause[1024];
    int      affected_versions[9];
    int      affected_version_count;
    char     affected_files[40][32]; /* up to 40 filenames */
    int      affected_file_count;
    char     resolution[64];
    char     fixed_by_change[32];
    char     category[16];        /* FIX / OPTIMIZATION / LOCALIZATION / IMPROVEMENT / "" */
    int      firestaff_flag_id;   /* bit-index in BugFlagMask_Compat */
    char     applicable_games[3][8]; /* "DM1", "CSB", "DM2" */
    int      applicable_game_count;
} BugDatabaseEntry_M12;

/* The full database. */
typedef struct {
    BugDatabaseEntry_M12 entries[BUG_DATABASE_MAX_ENTRIES]; /* 256 capacity */
    int                  count;                             /* 201 in v1 */
} BugDatabase_M12;

/* Load from JSON file. Returns 0 on success. */
int bug_database_m12_load(const char *json_path, BugDatabase_M12 *db);

/* Query: filter entries visible for a given game. */
int bug_database_m12_filter_by_game(
    const BugDatabase_M12 *db,
    const char *game_id,
    const BugDatabaseEntry_M12 *out[],
    int max_results
);

/* Query: filter by category. */
int bug_database_m12_filter_by_category(
    const BugDatabase_M12 *db,
    const char *category,
    const BugDatabaseEntry_M12 *out[],
    int max_results
);

/* Query: text search across symptom+cause fields. */
int bug_database_m12_search(
    const BugDatabase_M12 *db,
    const char *query,
    const BugDatabaseEntry_M12 *out[],
    int max_results
);
```

**Dependencies:** `cJSON.c`. No M10/M11 dependency.

**Size estimate:** ~700 LOC.

---

### §2.7  `i18n_compat.{h,c}` — String Lookup + Language Switching

**Purpose:** Runtime internationalisation engine. Translates string keys to localised text. Supports runtime language switching without restart. Falls back to English for missing translations.

**Public API:**
```c
/* Initialise i18n subsystem. Loads the default language. */
int i18n_compat_init(const char *locale_dir, const char *default_lang);

/* Shut down and free all loaded catalogues. */
void i18n_compat_shutdown(void);

/* Switch language at runtime. Loads new catalogue if not already loaded.
   Returns 0 on success. UI should re-render after this call. */
int i18n_compat_set_language(const char *lang_code);

/* Get current language code (e.g. "sv", "fr", "de", "en"). */
const char *i18n_compat_get_language(void);

/* Translate a string key. Returns translated string, or English fallback,
   or the key itself if no translation exists anywhere. */
const char *i18n_compat_tr(const char *key);

/* Translate with printf-style formatting (thread-local buffer, max 4 KB). */
const char *i18n_compat_trf(const char *key, ...);

/* Get list of available languages. */
int i18n_compat_available_languages(const char *out[], int max);

/* Statistics: how many keys are untranslated in the current language? */
int i18n_compat_missing_count(void);

/* Iterate all keys (for invariant checking). */
int i18n_compat_key_count(void);
const char *i18n_compat_key_at(int index);
```

**Dependencies:** `i18n_po_loader_m12` for .po parsing. M11 `fs_portable_compat` for locale directory.

**Size estimate:** ~800 LOC.

---

### §2.8  `i18n_po_loader_m12.{h,c}` — .po File Parser

**Purpose:** Parse GNU gettext `.po` files into an in-memory string table (hash map). Handles multi-line `msgstr`, escaped characters, plural forms (for future use), and UTF-8 throughout.

**Public API:**
```c
/* A single .po catalogue (one language, one domain). */
typedef struct I18nCatalogue_M12 I18nCatalogue_M12; /* opaque */

/* Parse a .po file into a catalogue. Returns NULL on error. */
I18nCatalogue_M12 *i18n_po_loader_m12_parse(const char *po_path);

/* Look up a msgid. Returns msgstr or NULL if not found. */
const char *i18n_po_loader_m12_lookup(const I18nCatalogue_M12 *cat, const char *msgid);

/* Number of entries in catalogue. */
int i18n_po_loader_m12_entry_count(const I18nCatalogue_M12 *cat);

/* Iterate all msgids (for completeness checking). */
const char *i18n_po_loader_m12_msgid_at(const I18nCatalogue_M12 *cat, int index);

/* Free catalogue. */
void i18n_po_loader_m12_free(I18nCatalogue_M12 *cat);
```

**Dependencies:** None (pure string parsing). All UTF-8; no iconv dependency.

**Size estimate:** ~600 LOC.

---

### §2.9  `menu_main_m12.{h,c}` — Game Picker / Startup Menu

**Purpose:** The main startup screen. Renders box-art for DM1, CSB, DM2 in a grid. Shows validation status per game (✓/✗). Handles first-run popup. Routes to settings tabs or game launch.

**Public API:**
```c
/* Scene lifecycle — called by M11's scene manager. */
void menu_main_m12_init(MenuMainState_M12 *state, const FirestaffConfig_M12 *cfg,
                         const GameValidationResult_Compat results[], int resultCount);
void menu_main_m12_handle_input(MenuMainState_M12 *state, const InputEvent_M11 *event);
void menu_main_m12_update(MenuMainState_M12 *state, float dt);
void menu_main_m12_render(const MenuMainState_M12 *state, RenderContext_M11 *rc);
void menu_main_m12_shutdown(MenuMainState_M12 *state);

/* Query: which game did the user select? Returns NULL if still browsing. */
const char *menu_main_m12_selected_game(const MenuMainState_M12 *state);

/* Query: did user request settings? */
int menu_main_m12_wants_settings(const MenuMainState_M12 *state);

/* Query: is first-run popup active? */
int menu_main_m12_first_run_active(const MenuMainState_M12 *state);
```

**Dependencies:** M11 render pipeline, M11 input handler. `boxart_cache_m12` for images. `asset_validator_compat` for game status. `i18n_compat` for all strings. `menu_layout_m12` for declarative layout.

**Size estimate:** ~1000 LOC.

---

### §2.10  `menu_settings_m12.{h,c}` — Settings Tabs

**Purpose:** Tabbed settings interface: Game / Graphics / Audio / Input / Language / Bug Toggles / Credits. Each tab reads from and writes to `FirestaffConfig_M12`. Changes are applied immediately (preview) and persisted on "Save".

**Public API:**
```c
typedef enum {
    SETTINGS_TAB_GAME = 0,
    SETTINGS_TAB_GRAPHICS,
    SETTINGS_TAB_AUDIO,
    SETTINGS_TAB_INPUT,
    SETTINGS_TAB_LANGUAGE,
    SETTINGS_TAB_BUGTOGGLES,
    SETTINGS_TAB_CREDITS,
    SETTINGS_TAB_COUNT
} SettingsTab_M12;

/* Scene lifecycle. */
void menu_settings_m12_init(MenuSettingsState_M12 *state, FirestaffConfig_M12 *cfg,
                             const BugDatabase_M12 *bugDb);
void menu_settings_m12_handle_input(MenuSettingsState_M12 *state, const InputEvent_M11 *event);
void menu_settings_m12_update(MenuSettingsState_M12 *state, float dt);
void menu_settings_m12_render(const MenuSettingsState_M12 *state, RenderContext_M11 *rc);
void menu_settings_m12_shutdown(MenuSettingsState_M12 *state);

/* Did user press "Back"? */
int menu_settings_m12_wants_back(const MenuSettingsState_M12 *state);

/* Did config change? (For dirty-flag / auto-save logic.) */
int menu_settings_m12_config_dirty(const MenuSettingsState_M12 *state);
```

**Dependencies:** M11 render + input. `config_schema_m12` for typed access. `i18n_compat`. `menu_layout_m12`. `menu_bugtoggles_m12` (embedded as subtab). `menu_credits_m12` (embedded as subtab).

**Size estimate:** ~1500 LOC (largest UI module — coordinates 7 tabs).

---

### §2.11  `menu_bugtoggles_m12.{h,c}` — Bug-Fix Toggle UI

**Purpose:** Full implementation of the bug-toggle spec (`BUGFIX_TOGGLE_SPEC.md`). Renders the 201-entry expandable tree with tooltips, version-preset dropdown, custom mix, per-game filtering, search, and save-profile mismatch warnings.

**Public API:**
```c
/* Init with current bug flags + database. */
void menu_bugtoggles_m12_init(MenuBugTogglesState_M12 *state,
                               BugFlagMask_Compat *currentFlags,
                               const BugDatabase_M12 *db,
                               const char *current_game_id);

/* Standard scene callbacks. */
void menu_bugtoggles_m12_handle_input(MenuBugTogglesState_M12 *state, const InputEvent_M11 *event);
void menu_bugtoggles_m12_update(MenuBugTogglesState_M12 *state, float dt);
void menu_bugtoggles_m12_render(const MenuBugTogglesState_M12 *state, RenderContext_M11 *rc);

/* Apply preset (overwrites flags, marks dirty). */
void menu_bugtoggles_m12_apply_preset(MenuBugTogglesState_M12 *state, BugPreset_Compat preset);

/* Get current preset name (or "Custom" if user modified). */
const char *menu_bugtoggles_m12_current_preset_name(const MenuBugTogglesState_M12 *state);

/* Check save-file mismatch. */
int menu_bugtoggles_m12_check_save_mismatch(const MenuBugTogglesState_M12 *state,
                                              uint32_t save_profile_hash);
```

**Dependencies:** `bug_flags_compat`, `bug_database_m12`, M11 render + input, `i18n_compat`, `menu_layout_m12`.

**Size estimate:** ~1200 LOC.

---

### §2.12  `menu_credits_m12.{h,c}` — About / Credits

**Purpose:** Scrolling credits screen. Firestaff version, engine credits, original DM/CSB team, Fontanel's ReDMCSB contribution, licence info.

**Public API:**
```c
void menu_credits_m12_init(MenuCreditsState_M12 *state);
void menu_credits_m12_handle_input(MenuCreditsState_M12 *state, const InputEvent_M11 *event);
void menu_credits_m12_update(MenuCreditsState_M12 *state, float dt);
void menu_credits_m12_render(const MenuCreditsState_M12 *state, RenderContext_M11 *rc);
```

**Dependencies:** M11 render, `i18n_compat`.

**Size estimate:** ~300 LOC.

---

### §2.13  `menu_layout_m12.{h,c}` — Declarative Layout Engine

**Purpose:** Lightweight layout engine for menu UI. Reads layout definitions from TOML data (embedded or loaded) describing widgets: labels, buttons, sliders, dropdowns, toggles, grids, tabs, scrollable lists. Handles positioning, spacing, focus navigation (keyboard + gamepad + mouse), and animation states.

This is NOT a general GUI framework. It handles exactly the widget types Firestaff needs, no more. Widgets are structs with render callbacks; layout resolves positions and sizes.

**Public API:**
```c
/* Widget types. */
typedef enum {
    WIDGET_LABEL, WIDGET_BUTTON, WIDGET_SLIDER, WIDGET_DROPDOWN,
    WIDGET_TOGGLE, WIDGET_GRID, WIDGET_TABS, WIDGET_SCROLLLIST,
    WIDGET_SEPARATOR, WIDGET_IMAGE, WIDGET_TEXT_INPUT,
    WIDGET_TYPE_COUNT
} WidgetType_M12;

/* A layout definition (loaded from TOML or built programmatically). */
typedef struct MenuLayout_M12 MenuLayout_M12; /* opaque */

/* Load layout from TOML string. */
MenuLayout_M12 *menu_layout_m12_parse(const char *toml_str, int len);

/* Build layout from a file. */
MenuLayout_M12 *menu_layout_m12_load(const char *path);

/* Free. */
void menu_layout_m12_free(MenuLayout_M12 *layout);

/* Resolve positions given viewport size. Call after load and on resize. */
void menu_layout_m12_resolve(MenuLayout_M12 *layout, int viewport_w, int viewport_h);

/* Focus navigation. */
void menu_layout_m12_focus_next(MenuLayout_M12 *layout);
void menu_layout_m12_focus_prev(MenuLayout_M12 *layout);
void menu_layout_m12_focus_activate(MenuLayout_M12 *layout);

/* Hit test (mouse). Returns widget index or -1. */
int menu_layout_m12_hit_test(const MenuLayout_M12 *layout, int x, int y);

/* Render all widgets. Calls per-widget render callbacks. */
void menu_layout_m12_render(const MenuLayout_M12 *layout, RenderContext_M11 *rc);

/* Get/set widget value by ID (for binding to config). */
int menu_layout_m12_get_int(const MenuLayout_M12 *layout, const char *widget_id);
void menu_layout_m12_set_int(MenuLayout_M12 *layout, const char *widget_id, int value);
const char *menu_layout_m12_get_str(const MenuLayout_M12 *layout, const char *widget_id);
void menu_layout_m12_set_str(MenuLayout_M12 *layout, const char *widget_id, const char *value);
```

**Dependencies:** `toml.c`, M11 render pipeline.

**Size estimate:** ~1400 LOC.

---

### §2.14  `boxart_cache_m12.{h,c}` — Image Loading + Caching

**Purpose:** Load box-art images (PNG/JPG) from the data directory, decode to SDL textures, cache for reuse. Handles missing images (fallback to a generated title card with the game name). Supports greyscale desaturation for unavailable games.

**Public API:**
```c
/* Initialise the cache. */
void boxart_cache_m12_init(BoxartCache_M12 *cache, RenderContext_M11 *rc);

/* Get texture for a game. Loads on first access. Returns fallback if missing. */
SDL_Texture *boxart_cache_m12_get(BoxartCache_M12 *cache, const char *game_id, int greyed_out);

/* Preload all known game images. */
void boxart_cache_m12_preload(BoxartCache_M12 *cache);

/* Free all cached textures. */
void boxart_cache_m12_shutdown(BoxartCache_M12 *cache);
```

**Dependencies:** SDL3 (`SDL_image` or `stb_image.h` — see Risk Register). M11 render context.

**Size estimate:** ~400 LOC.

---

### §2.15  Vendored Libraries (not new modules, but critical dependencies)

| Library | Purpose | Licence | LOC | Why vendored |
|---------|---------|---------|-----|--------------|
| `toml.c` ([cktan/tomlc99](https://github.com/cktan/tomlc99)) | TOML parsing | MIT | ~1600 | Pure C, single-file, well-tested, actively maintained |
| `cJSON.c` ([DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)) | JSON parsing (asset database, bug database) | MIT | ~2700 | Pure C, single-file, ubiquitous |
| `md5.c` (RFC 1321 reference or [Alexander Peslyak](https://openwall.info/wiki/people/solar/software/public-domain-source-code/md5)) | MD5 hashing for asset validation | Public domain | ~300 | Tiny, no dependency, stable forever |
| `stb_image.h` ([nothings/stb](https://github.com/nothings/stb)) | PNG/JPG decoding for box-art | Public domain | ~7500 (but single-header, compiles to ~150 KB) | Avoids SDL_image dependency; pure C; well-proven |

All vendored into `vendor/` directory in the repo. Pinned to specific commit hashes. No dynamic linking.

---

### Module Dependency Graph

```
                    ┌─────────────────────┐
                    │  menu_main_m12      │
                    │  (game picker)      │
                    └─────┬───┬───┬───────┘
                          │   │   │
              ┌───────────┘   │   └───────────────┐
              ▼               ▼                   ▼
    ┌──────────────┐  ┌─────────────┐   ┌──────────────────┐
    │ boxart_cache │  │ asset_valid │   │ menu_settings    │
    │ _m12         │  │ _compat     │   │ _m12             │
    └──────────────┘  └──────┬──────┘   └──┬──┬──┬──┬──┬───┘
                             │             │  │  │  │  │
                             ▼             │  │  │  │  │
                    ┌──────────────┐       │  │  │  │  │
                    │ asset_db     │       │  │  │  │  │
                    │ _m12         │       │  │  │  │  │
                    └──────────────┘       │  │  │  │  │
                                          │  │  │  │  │
    ┌─────────────────────────────────────┘  │  │  │  │
    ▼                                        │  │  │  │
┌──────────────┐  ┌──────────────────────────┘  │  │  │
│ menu_bug     │  │                             │  │  │
│ toggles_m12  │  │  ┌──────────────────────────┘  │  │
└──────┬───────┘  │  │                             │  │
       │          ▼  ▼                             │  │
       │   ┌──────────────┐   ┌────────────────────┘  │
       │   │ config_io    │   │                       │
       │   │ _m12         │   │  ┌────────────────────┘
       │   └──────┬───────┘   │  │
       │          │           ▼  ▼
       │          │    ┌──────────────┐  ┌──────────────┐
       │          │    │ i18n_compat  │  │ menu_credits │
       │          │    └──────┬───────┘  │ _m12         │
       │          │           │          └──────────────┘
       │          │           ▼
       │          │    ┌──────────────┐
       │          │    │ i18n_po      │
       │          │    │ _loader_m12  │
       │          │    └──────────────┘
       │          ▼
       │   ┌──────────────┐
       │   │ config_schema│
       │   │ _m12         │
       │   └──────────────┘
       ▼
┌──────────────┐   ┌──────────────┐
│ bug_flags    │   │ bug_database │
│ _compat      │   │ _m12         │
└──────────────┘   └──────────────┘

Cross-cutting: menu_layout_m12 used by all menu_* modules.
Vendored libs: toml.c, cJSON.c, md5.c, stb_image.h
M11 deps: RenderContext_M11, InputEvent_M11, fs_portable_compat
M10 deps: CRC32 (Phase 15), BugFlagMask consumed by tick orchestrator (Phase 20)
```

---

## §3  Config File Format

### §3.1  Format Choice: TOML

**Decision:** TOML, not YAML.

**Rationale:**

| Criterion | TOML | YAML |
|-----------|------|------|
| Pure C parser availability | `tomlc99` — MIT, ~1600 LOC, single file, actively maintained, C99 | `libyaml` — MIT, but ~8000 LOC, complex API, C library not single-file |
| Spec complexity | Simple, unambiguous, no "gotchas" | Notorious complexity (Norway problem, `on`=`true`, implicit typing) |
| Human readability | Excellent for config files (its design goal) | Good but error-prone indentation |
| Comment support | Yes (`#`) | Yes (`#`) |
| Cross-platform safety | No indentation sensitivity | Indentation-sensitive → editor/platform issues |
| Game industry precedent | Cargo (Rust), Hugo, many game configs | Docker, Kubernetes, CI/CD (not games) |

TOML wins on every axis relevant to a game config file: smaller parser, fewer gotchas, better fit for flat/shallow structured data, and a pure C implementation that vendors trivially.

### §3.2  Config Schema

```toml
# Firestaff configuration file
# Schema version: 1
# Auto-generated defaults — edit freely.

[general]
schema_version = 1
language = "en"                              # en | sv | fr | de
first_run_done = false
data_path = ""                               # path to original game data directory
last_played_game = "DM1"                     # DM1 | CSB | DM2

[graphics]
level = "original"                           # original | upscaled | ai_hd
scaling_mode = "integer"                     # integer | fit_to_aspect | fill_and_stretch
resolution = "auto"                          # "auto" or "WxH" e.g. "1920x1080"
window_mode = "windowed"                     # fullscreen | windowed | windowed_fullscreen | resizable
filter_crt_scanline = false
filter_scaling_softness = 0                  # 0-100
filter_colour_correction = "none"            # none | warm | cool | sepia

[audio]
master_volume = 80                           # 0-100
sfx_volume = 100                             # 0-100
music_volume = 70                            # 0-100
ui_volume = 60                               # 0-100
master_mute = false
sfx_mute = false
music_mute = false
ui_mute = false
device = "auto"                              # "auto" or device name from SDL
buffer_size = 2048                           # samples: 512 | 1024 | 2048 | 4096

[input]
mouse_sensitivity = 50                       # 0-100
gamepad_enabled = true
keyboard_layout = "en_us"                    # en_us | sv_se | fr_fr | de_de

[input.keybindings]
move_north = "W"
move_south = "S"
move_east = "D"
move_west = "A"
turn_left = "Q"
turn_right = "E"
attack_left = "Z"
attack_right = "X"
cast_spell = "C"
use_item = "U"
rest = "R"
open_inventory = "I"
open_map = "M"
quick_save = "F5"
quick_load = "F9"
toggle_fullscreen = "F11"
screenshot = "F12"
pause = "Escape"

[input.gamepad]
move_north = "DPad_Up"
move_south = "DPad_Down"
move_east = "DPad_Right"
move_west = "DPad_Left"
turn_left = "LeftShoulder"
turn_right = "RightShoulder"
attack_left = "X"
attack_right = "B"
cast_spell = "Y"
confirm = "A"
back = "Back"
pause = "Start"

[game.DM1]
bug_profile = "PC_3.4_baseline"             # preset name or "Custom"
# Custom flags are stored only when bug_profile = "Custom".
# Format: array of flag IDs that are ACTIVE (original buggy behaviour).
custom_active_flags = []

[game.CSB]
bug_profile = "CSB_2.1_EN"
custom_active_flags = []

[game.DM2]
bug_profile = "Modern"
custom_active_flags = []

[asset_cache]
# Internal: cached MD5 validation results. Do not edit manually.
last_scan_time = 0
cache_file = "asset_cache.json"
```

### §3.3  Schema Versioning

- `schema_version = 1` in v1.
- On load: if `schema_version` is missing or < current, run migration functions chained from 1 → 2 → ... → current.
- Migration functions are registered in `config_io_m12.c` as an array of `(from_version, to_version, migrate_fn)` triples.
- Unknown keys are preserved (TOML parser discards them, so we re-emit them via a "passthrough" section). **Decision:** actually, unknown keys are dropped on round-trip. This is acceptable for a game config — users shouldn't hand-edit keys we don't understand. Document this in comments at top of config file.

### §3.4  Atomic Writes

Save procedure:
1. Serialise config to memory buffer.
2. Write to `config.toml.tmp` in the same directory.
3. `fsync()` the file descriptor.
4. `rename("config.toml.tmp", "config.toml")` — atomic on POSIX.
5. On Windows: `MoveFileEx` with `MOVEFILE_REPLACE_EXISTING` (atomic on NTFS since Vista).

If the process crashes during step 2, the `.tmp` file is left behind. On next load, if `config.toml` exists it's used; if only `.tmp` exists, it's treated as corrupt (deleted) and defaults are regenerated.

### §3.5  Defaults Layering

```
Built-in defaults (compiled into binary)
        ↓ merge (user wins)
User config.toml (from disk)
        ↓ merge (CLI wins)
Command-line overrides (e.g. --fullscreen, --language=sv)
        = Final runtime config
```

Each layer is a `FirestaffConfig_M12` struct. Merge is field-by-field: if the overlay field is non-default (non-zero / non-empty), it wins.

---

## §4  Asset Database Format

### §4.1  JSON Schema

The asset database ships as `data/assets.json` alongside the binary.

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "Firestaff Asset Database",
  "description": "Known-good MD5 hashes for original Dungeon Master game files",
  "type": "object",
  "properties": {
    "schema_version": { "type": "integer", "const": 1 },
    "generated_at": { "type": "string", "format": "date-time" },
    "games": {
      "type": "object",
      "additionalProperties": {
        "$ref": "#/$defs/game"
      }
    }
  },
  "required": ["schema_version", "games"],
  "$defs": {
    "game": {
      "type": "object",
      "properties": {
        "title": { "type": "string" },
        "variants": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/variant"
          }
        }
      },
      "required": ["title", "variants"]
    },
    "variant": {
      "type": "object",
      "properties": {
        "description": { "type": "string" },
        "platform": { "type": "string" },
        "version": { "type": "string" },
        "language": { "type": "string" },
        "assets": {
          "type": "array",
          "items": { "$ref": "#/$defs/asset" }
        }
      },
      "required": ["description", "platform", "assets"]
    },
    "asset": {
      "type": "object",
      "properties": {
        "id": { "type": "string" },
        "description": { "type": "string" },
        "required": { "type": "boolean" },
        "md5": {
          "oneOf": [
            { "type": "string", "pattern": "^[a-f0-9]{32}$" },
            {
              "type": "array",
              "items": { "type": "string", "pattern": "^[a-f0-9]{32}$" },
              "minItems": 1
            }
          ]
        },
        "size_bytes": { "type": "integer" },
        "notes": { "type": "string" }
      },
      "required": ["id", "required", "md5"]
    }
  }
}
```

### §4.2  Concrete Examples

```json
{
  "schema_version": 1,
  "generated_at": "2026-04-20T10:00:00Z",
  "games": {
    "DM1": {
      "title": "Dungeon Master",
      "variants": {
        "PC_3.4_EN": {
          "description": "Dungeon Master PC version 3.4, English",
          "platform": "PC",
          "version": "3.4",
          "language": "EN",
          "assets": [
            {
              "id": "DUNGEON.DAT",
              "description": "Dungeon data (maps, objects, creatures, events)",
              "required": true,
              "md5": "b076a54e8b3c89tried9d6f3a2e1c8b4",
              "size_bytes": 33286,
              "notes": "Primary game data. Identical across all PC 3.4 language variants."
            },
            {
              "id": "GRAPHICS.DAT",
              "description": "All game graphics (sprites, tiles, UI elements)",
              "required": true,
              "md5": "d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9",
              "size_bytes": 527964,
              "notes": "Contains IMG3-encoded bitmaps. Format documented in Fontanel's ReDMCSB."
            },
            {
              "id": "SOUND.DAT",
              "description": "Sound effects and music samples",
              "required": false,
              "md5": "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6",
              "size_bytes": 89420,
              "notes": "Optional — game plays without sound. PC Sound Blaster format."
            },
            {
              "id": "HINT.DAT",
              "description": "Hint Oracle data (LZW-compressed text)",
              "required": false,
              "md5": "f1e2d3c4b5a6f7e8d9c0b1a2f3e4d5c6",
              "size_bytes": 12288,
              "notes": "Only needed if Hint Oracle feature is used."
            }
          ]
        },
        "ATARI_1.3a_FR": {
          "description": "Dungeon Master Atari ST version 1.3a, French",
          "platform": "Atari_ST",
          "version": "1.3a",
          "language": "FR",
          "assets": [
            {
              "id": "DUNGEON.DAT",
              "description": "Dungeon data (French localisation)",
              "required": true,
              "md5": [
                "aabbccdd11223344aabbccdd11223344",
                "11223344aabbccdd11223344aabbccdd"
              ],
              "size_bytes": 33286,
              "notes": "Two known dumps exist with different headers but identical gameplay data. Both are valid."
            },
            {
              "id": "GRAPHICS.DAT",
              "description": "Graphics data (shared across Atari ST versions)",
              "required": true,
              "md5": "99887766554433221100ffeeddccbbaa",
              "size_bytes": 412800,
              "notes": "Atari ST graphics format. Smaller than PC due to lower resolution palette."
            }
          ]
        }
      }
    },
    "CSB": {
      "title": "Chaos Strikes Back",
      "variants": {
        "ATARI_2.1_EN": {
          "description": "Chaos Strikes Back Atari ST version 2.1, English",
          "platform": "Atari_ST",
          "version": "2.1",
          "language": "EN",
          "assets": [
            {
              "id": "DUNGEON.DAT",
              "description": "CSB dungeon data",
              "required": true,
              "md5": "fedcba9876543210fedcba9876543210",
              "size_bytes": 52480,
              "notes": "Larger than DM1 — more complex dungeon layout."
            },
            {
              "id": "GRAPHICS.DAT",
              "description": "CSB graphics data (extends DM1 sprite set)",
              "required": true,
              "md5": "0123456789abcdef0123456789abcdef",
              "size_bytes": 445440,
              "notes": "Includes new creature sprites and dungeon textures."
            },
            {
              "id": "SOUND.DAT",
              "description": "CSB sound effects",
              "required": false,
              "md5": "abcdef0123456789abcdef0123456789",
              "size_bytes": 95200,
              "notes": "New sound effects for CSB-specific creatures."
            }
          ]
        }
      }
    },
    "DM2": {
      "title": "Dungeon Master II: The Legend of Skullkeep",
      "variants": {}
    }
  }
}
```

### §4.3  Handling Variant Files

**Multiple valid MD5s per file:** The `md5` field accepts either a single hex string or an array. This handles:
- Different ROM dumps of the same game (common with Atari ST floppies)
- Minor header differences between distribution versions
- Re-releases with patched copy protection

**Localised text resources:** Different language versions may share `GRAPHICS.DAT` but have different `DUNGEON.DAT` (embedded text). Each language variant is a separate entry under `variants`.

**Discovery strategy:** The validator hashes every file in the user's directory (recursively, up to configurable depth), then looks up each hash in a reverse-index built from the database. This means:
- File names don't matter (users can rename files freely)
- Directory structure doesn't matter (nested folders are fine)
- Multiple games in the same directory are detected simultaneously
- A single scan discovers all playable variants at once

**Community contributions:** The database has a `generated_at` timestamp. Users can submit new variant hashes via GitHub pull requests to the `data/assets.json` file. The build system validates the JSON schema on CI.

---

## §5  Bug-Toggle ↔ M10 Wiring

### §5.1  Architecture Overview

The bug-toggle system bridges M12's user-facing UI to M10's pure tick orchestrator via a single shared data structure: the `BugFlagMask_Compat` (256-bit mask).

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  M12 UI Layer   │     │  Config Layer   │     │  M10 Engine     │
│                 │     │                 │     │                 │
│ menu_bugtoggles │────▶│ config_io_m12   │────▶│ GameWorld_Compat│
│ _m12            │     │                 │     │  .bugFlags      │
│                 │     │ config.toml     │     │                 │
│ (user toggles   │     │ [game.DM1]      │     │ BUG_ACTIVE()    │
│  individual     │     │ bug_profile=    │     │ macro checks    │
│  flags on/off)  │     │ "Custom"        │     │ per tick        │
│                 │     │ custom_active_  │     │                 │
│                 │     │ flags=[...]     │     │                 │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

### §5.2  Round-Trip: Toggle Change → Game Behaviour

**Step 1: User toggles a flag in the UI**
```
User clicks BUG0_02 toggle in menu_bugtoggles_m12
  → MenuBugTogglesState_M12.flags changes
  → Preset label updates to "Custom" if it was a named preset
  → Dirty flag set
```

**Step 2: Config persists**
```
User clicks "Save" (or auto-save on back)
  → config_io_m12_save() writes config.toml atomically
  → [game.DM1].bug_profile = "Custom"
  → [game.DM1].custom_active_flags = ["BUG0_02", "BUG0_08", ...]
  → Profile hash computed: bug_flags_compat_profile_hash(&flags)
```

**Step 3: Bit-mask rebuild at game launch**
```
On game start:
  → config_io_m12_load() reads config.toml
  → If bug_profile is a named preset:
      bug_flags_compat_apply_preset(&mask, preset_enum)
  → If bug_profile is "Custom":
      Clear mask, then set each flag listed in custom_active_flags
  → mask is injected into GameWorld_Compat.bugFlags
```

**Step 4: M10 tick orchestrator consults flags**
```c
/* In Phase 12 timeline processing (example: BUG0_02 time overflow) */
if (BUG_ACTIVE(world, BUG0_02_FLAG)) {
    /* Original buggy behaviour — 24-bit overflow */
    scheduled_time = (game_time + delay) & 0xFFFFFF;
} else {
    /* Fixed — use full 32-bit time */
    scheduled_time = game_time + delay;
}
```

Each M10 phase that implements a toggleable behaviour wraps the divergence in a `BUG_ACTIVE()` check. The flag ID constants are defined in `bug_flags_compat.h`.

**Step 5: World-hash changes**

The `BugFlagMask_Compat` is serialised into the `GameWorld_Compat` struct (see Phase 20 §2.1). Since the world-hash is CRC32 over the full serialised state, any flag change produces a different world-hash on the very first tick where the flag affects behaviour.

**Step 6: Save-file embeds profile hash**
```c
/* In Phase 15 save routine */
save_header.bugProfileHash = bug_flags_compat_profile_hash(&world->bugFlags);
```

**Step 7: Load-time mismatch warning**
```c
/* On load */
uint32_t saved_hash = loaded_header.bugProfileHash;
uint32_t current_hash = bug_flags_compat_profile_hash(&current_config_flags);
if (saved_hash != current_hash) {
    /* UI popup: "This save was made with a different bug-toggle profile.
       Continue? [Yes] [No] [Snap to save profile]" */
}
```

### §5.3  Replay Determinism

Replay files (M10 Phase 20 `TickStreamRecord_Compat`) embed the profile hash in the stream header. Replay verification requires:

1. Load the replay file header → extract `bugProfileHash`.
2. Before playback: reconstruct the exact `BugFlagMask_Compat` from the profile.
3. Run ticks with the reconstructed flags → world-hash after N ticks must match recorded hash.

**Consequence:** Custom flag profiles must be fully serialisable. A profile hash alone is not sufficient for reconstruction — we also need to store the full mask (32 bytes) in the replay header. The hash is for quick mismatch detection; the mask is for faithful reconstruction.

**Extension of TickStreamHeader (Phase 20 addition for M12):**
```c
/* Added fields in TickStreamHeader_Compat for M12 */
uint8_t  bugFlagMask[32];    /* full 256-bit flag mask */
uint32_t bugProfileHash;      /* CRC32 for quick comparison */
```

### §5.4  Preset → Mask Mapping

Each preset maps to a specific set of active flags:

| Preset | Logic |
|--------|-------|
| DM 1.0a EN (purist) | ALL 89 BUG0_* flags active. NO changes active. |
| DM 1.0b EN | Same as 1.0a but deactivate bugs fixed by CHANGE1_* |
| DM 1.1 EN | Deactivate bugs fixed by CHANGE1_* + CHANGE2_*. Activate BUG2_00. |
| DM 1.2 EN | Further deactivate bugs fixed by CHANGE3_*. |
| DM 1.2 GE | Same as 1.2 EN + CHANGE4_00_LOCALIZATION active. |
| DM 1.3a FR | Further deactivate + CHANGE5_*. Activate BUG5_00. |
| DM 1.3b FR | Further + CHANGE6_*. |
| CSB 2.0 EN | Further + CHANGE7_*. Activate BUG7_00, BUG7_01. |
| CSB 2.1 EN | Further + CHANGE8_*. |
| PC 3.4 baseline | Maps to Fontanel's WIP code state. **Default for new users.** |
| Modern | All FIX changes active. All OPTIMIZATION active. Selected IMPROVEMENT active. All bugs deactivated. |

The preset definitions are data-driven, loaded from `data/presets.toml`:

```toml
[presets.DM_1_0a_EN_purist]
description = "Original 1987 release. All 89 known bugs present."
activate_all_bugs_up_to_version = 0
deactivate_changes = true

[presets.PC_3_4_baseline]
description = "PC version 3.4 — Fontanel's reference implementation."
# Start with all bugs, then apply all fixes through version 8
activate_all_bugs_up_to_version = 8
apply_changes_through_version = 8
# Then overlay PC-specific adjustments
overlay_flags = ["BUG0_02:active"]  # Time overflow NOT fixed in PC 3.4

[presets.Modern]
description = "All fixes applied. Quality-of-life improvements enabled."
deactivate_all_bugs = true
activate_categories = ["FIX", "OPTIMIZATION", "IMPROVEMENT"]
```

---

## §6  i18n Pipeline

### §6.1  String Extraction

All user-facing strings in M12 are wrapped in `TR()` macros:

```c
/* In menu_main_m12.c */
draw_text(rc, x, y, TR("menu.game_picker.title"));        /* "Select Game" */
draw_text(rc, x, y, TR("menu.game_picker.unavailable"));   /* "Game data not found" */
draw_text(rc, x, y, TRF("menu.game_picker.found", game_name)); /* "Found: %s" */
```

`TR()` is a macro that expands to `i18n_compat_tr()`. `TRF()` expands to `i18n_compat_trf()`.

**Extraction tooling:**

A custom Python script (`tools/extract_strings.py`) scans all `_m12_` source files for `TR("...")` and `TRF("...")` calls, extracts the string keys, and generates a `firestaff.pot` (PO Template) file.

```bash
# Generate .pot from source
python3 tools/extract_strings.py src/*_m12_*.c > locale/firestaff.pot

# Initialise new language
msginit -i locale/firestaff.pot -o locale/sv/firestaff.po -l sv

# Update existing language after source changes
msgmerge -U locale/sv/firestaff.po locale/firestaff.pot
```

We use standard gettext `.pot` / `.po` format for maximum compatibility with translation platforms, but our runtime parser is custom (`i18n_po_loader_m12`) — no dependency on libintl/gettext at runtime.

### §6.2  .po File Layout

```
locale/
├── firestaff.pot              # Template (source strings, no translations)
├── en/
│   └── firestaff.po           # English (mostly identity — msgid = msgstr)
├── sv/
│   └── firestaff.po           # Swedish
├── fr/
│   └── firestaff.po           # French
└── de/
    └── firestaff.po           # German
```

Each `.po` file follows standard GNU gettext format:

```po
# Firestaff UI strings — Swedish translation
# Copyright (C) 2026 Firestaff contributors
# SPDX-License-Identifier: MIT
#
msgid ""
msgstr ""
"Project-Id-Version: firestaff 1.0\n"
"Language: sv\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

#: menu_main_m12.c:42
msgid "menu.game_picker.title"
msgstr "Välj spel"

#: menu_main_m12.c:55
msgid "menu.game_picker.unavailable"
msgstr "Speldata hittades inte"

#: menu_main_m12.c:58
#, c-format
msgid "menu.game_picker.found"
msgstr "Hittade: %s"

#: menu_settings_m12.c:120
msgid "settings.tab.graphics"
msgstr "Grafik"

#: menu_settings_m12.c:121
msgid "settings.tab.audio"
msgstr "Ljud"

#: menu_settings_m12.c:122
msgid "settings.tab.input"
msgstr "Kontroller"

#: menu_settings_m12.c:123
msgid "settings.tab.language"
msgstr "Språk"

#: menu_bugtoggles_m12.c:30
msgid "bugtoggles.preset.purist"
msgstr "DM 1.0a EN (purist)"

#: menu_bugtoggles_m12.c:88
msgid "bugtoggles.save_mismatch.title"
msgstr "Profil-varning"

#: menu_bugtoggles_m12.c:89
msgid "bugtoggles.save_mismatch.body"
msgstr "Denna sparfil skapades med en annan buggprofil. Fortsätt ändå?"
```

### §6.3  Runtime Loading Strategy

**Lazy per-language loading** — the right choice for a game config UI:

1. On startup: load `en/firestaff.po` (always — it's the fallback).
2. Load the user's configured language `.po` file.
3. On language switch: if the target language isn't loaded, load it on demand (takes <5ms for ~500 strings).
4. Previously loaded catalogues stay in memory (total cost: ~200 KB for 4 languages × 500 strings).

**Lookup order:**
1. Current language catalogue → if found, return `msgstr`.
2. English catalogue → if found, return `msgstr`.
3. Return the key itself (developer sees untranslated keys immediately).

**Hot-reload for development:** In debug builds, watch `.po` file mtimes and reload on change. Enables translators to see changes without restarting.

### §6.4  Integration with Translation Workflow

Daniel (Danne) coordinates translations professionally via Weblate and Transifex. Firestaff's `.po` files integrate directly:

**Weblate:**
1. Connect Firestaff GitHub repo to a Weblate project.
2. Weblate auto-discovers `locale/*/firestaff.po`.
3. Translators work in Weblate's web UI.
4. Weblate commits translations back via PR.
5. CI validates: run `tools/check_translations.py` to ensure no missing keys.

**Transifex:**
1. Push `firestaff.pot` to Transifex as the source resource.
2. Pull translated `.po` files via `tx pull`.
3. Commit to repo.

**Quality gate** (CI):
```bash
# Check all languages have all keys
python3 tools/check_translations.py locale/firestaff.pot locale/*/firestaff.po
# Exit 1 if any language is missing >0 keys (warning) or >10% keys (error)
```

### §6.5  Font Handling

DM's original font is a bitmap font embedded in `GRAPHICS.DAT`. For the menu UI, we need a vector or high-res bitmap font that supports all target scripts.

**Font strategy:**

| Script | Font | Licence | Coverage |
|--------|------|---------|----------|
| Latin (en/sv/fr/de) | **Noto Sans** subset | OFL 1.1 | Full Latin + diacritics (å, ä, ö, ü, é, è, ê, ç, ß) |
| Cyrillic (future) | Noto Sans Cyrillic | OFL 1.1 | Full |
| CJK (far future) | Noto Sans CJK | OFL 1.1 | Full |

**Font loading:**
- Ship `NotoSans-Regular.ttf` (~300 KB, Latin subset) in `data/fonts/`.
- Use `stb_truetype.h` (public domain, single-header, pure C) for runtime rasterisation.
- Cache rasterised glyphs in an SDL texture atlas.
- Font size: 16px for body, 24px for headers, 32px for title.

**RTL readiness:**
- `i18n_compat` stores text direction per language (`ltr` or `rtl`).
- `menu_layout_m12` flips horizontal alignment when direction is `rtl`.
- No RTL language in v1, but the plumbing is in place.

**Estimated font data size:** ~400 KB for Latin subset. Acceptable.

---

## §7  Implementation Order

M12 is divided into 12 phases (A–L). Each phase has a probe, invariants, and clear entry/exit criteria. Phases are designed to be independently testable — each produces a working state, not a half-baked intermediate.

M12 can begin as soon as M11 Phase G (`fs_portable_compat`) lands. Phases A–D have no rendering dependency and can run in parallel with M11's later phases.

### Phase A: Config Foundation

**Goal:** TOML config read/write with schema validation, atomic saves, and round-trip stability.

**Modules:** `config_io_m12`, `config_schema_m12`

**Work:**
1. Vendor `toml.c` (tomlc99) into `vendor/`.
2. Define `FirestaffConfig_M12` struct with all fields and defaults.
3. Implement `config_io_m12_load()` / `config_io_m12_save()` with atomic write.
4. Implement `config_schema_m12_validate()` — rejects invalid values.
5. Implement `config_io_m12_apply_defaults()` and `config_io_m12_merge()`.
6. Write probe: `firestaff_m12_config_probe.c`.

**Probe:** Creates a config with non-default values → saves → loads → validates → compares. Also tests: missing file (generates defaults), corrupt file (error path), schema migration (future-proofing test with version 0 → 1).

**Invariants (8):**
- INV-A01: Round-trip: save → load → compare = identical for all fields.
- INV-A02: Round-trip: save → load → save → byte-compare files = identical.
- INV-A03: Missing config file → defaults generated → all fields valid.
- INV-A04: Corrupt config file → error returned, no crash, defaults available.
- INV-A05: Schema validation rejects volume > 100.
- INV-A06: Schema validation rejects unknown window_mode string.
- INV-A07: Merge: user value wins over default for every non-empty field.
- INV-A08: Atomic write: `.tmp` file does not persist after successful save.

**Exit criteria:** Config can be written, read, validated, and round-tripped with zero data loss.

**Estimated effort:** 3 days.

---

### Phase B: Asset Database + Validator

**Goal:** MD5-based asset discovery with caching. Given a directory, determine which games are playable.

**Modules:** `asset_database_m12`, `asset_validator_compat`

**Work:**
1. Vendor `cJSON.c` and `md5.c` into `vendor/`.
2. Define `assets.json` schema and populate with known DM1/CSB hashes.
3. Implement `asset_database_m12_load()` and query APIs.
4. Implement `asset_validator_compat_scan()` — recursive directory walk, hash computation, matching.
5. Implement cache layer (JSON file with `{path, size, mtime, md5}` entries).
6. Write probe: `firestaff_m12_asset_validator_probe.c`.

**Probe:** Creates a temp directory with synthetic files matching known MD5s → scans → validates matches. Also tests: wrong hash, missing file, extra files, nested directories, cache hit (second scan skips re-hashing), MD5 array (multiple valid hashes per file).

**Invariants (12):**
- INV-B01: Known DM1 PC 3.4 DUNGEON.DAT MD5 → match found, `playable = 1`.
- INV-B02: Known DM1 PC 3.4 GRAPHICS.DAT MD5 → match found.
- INV-B03: Wrong MD5 → `found = 0`, `playable = 0`.
- INV-B04: Missing required file → `playable = 0`, missing file listed.
- INV-B05: Extra files in directory → ignored, no false positives.
- INV-B06: Nested directory → files found recursively.
- INV-B07: Renamed file → still matched (MD5-based, not filename-based).
- INV-B08: Cache hit: second scan with unchanged files → no re-hashing (verified via mock/counter).
- INV-B09: Cache invalidation: file mtime changes → re-hash triggered.
- INV-B10: Multiple valid MD5s (array) → any match counts as found.
- INV-B11: Database with 0 variants → empty results, no crash.
- INV-B12: Stress test: 100 files (50 valid, 25 wrong-hash, 25 missing expected) → correct categorisation for all.

**Exit criteria:** Validator correctly identifies all game variants from a directory of game files.

**Estimated effort:** 4 days.

---

### Phase C: Bug Flags + Database

**Goal:** 256-bit flag mask with preset support, profile hashing, and the full 201-entry database loaded from JSON.

**Modules:** `bug_flags_compat`, `bug_database_m12`

**Work:**
1. Implement `BugFlagMask_Compat` with set/clear/test/popcount.
2. Implement preset application (data-driven from `firestaff-bugs.json` affected_versions).
3. Implement profile hash (CRC32 over serialised 32 bytes).
4. Implement `bug_database_m12_load()` from existing `firestaff-bugs.json`.
5. Implement query/filter APIs.
6. Write probe: `firestaff_m12_bug_flags_probe.c`.

**Probe:** Loads the 201-entry database. Applies each preset. Verifies flag counts. Tests set/clear/test. Tests profile hash stability. Tests serialise/deserialise round-trip. Tests game-filtering.

**Invariants (14):**
- INV-C01: Empty mask → popcount = 0.
- INV-C02: Set bit 0 → test bit 0 = 1, popcount = 1.
- INV-C03: Set bit 255 → test bit 255 = 1 (boundary).
- INV-C04: Set then clear bit 100 → test = 0.
- INV-C05: Serialise → deserialise → compare = identical.
- INV-C06: Profile hash of same mask = same value (deterministic).
- INV-C07: Profile hash of different masks = different values.
- INV-C08: Preset "DM 1.0a EN purist" → 89 bugs active, 0 changes active.
- INV-C09: Preset "Modern" → 0 bugs active, all FIX+OPT+IMP changes active.
- INV-C10: Preset "PC 3.4 baseline" → profile hash matches expected constant.
- INV-C11: Database load → 201 entries.
- INV-C12: Filter by game "DM1" → excludes CSB-only entries (BUG7_*).
- INV-C13: Filter by category "FIX" → returns only FIX-type changes.
- INV-C14: Search "overflow" → returns BUG0_02 (symptom contains "hours").

**Exit criteria:** Flag mask is bit-correct, all presets produce expected flag counts, database is fully loaded and queryable.

**Estimated effort:** 3 days.

---

### Phase D: i18n Engine

**Goal:** .po file parsing, runtime string lookup, language switching, completeness validation.

**Modules:** `i18n_compat`, `i18n_po_loader_m12`

**Work:**
1. Implement `.po` parser (handle `msgid`/`msgstr`, multi-line, escapes, `#` comments, plural forms).
2. Implement string-table hash map (FNV-1a hash, open addressing, ~2048 buckets).
3. Implement `i18n_compat_init()`, `set_language()`, `tr()`, `trf()`.
4. Create `locale/en/firestaff.po` with ~100 initial strings covering all M12 UI.
5. Create `locale/sv/firestaff.po` (Swedish — Danne's domain).
6. Create stub `locale/fr/firestaff.po` and `locale/de/firestaff.po`.
7. Write extraction script `tools/extract_strings.py`.
8. Write completeness checker `tools/check_translations.py`.
9. Write probe: `firestaff_m12_i18n_probe.c`.

**Probe:** Loads all 4 languages. Tests key lookup. Tests fallback (missing key → English). Tests language switch. Tests trf() formatting. Tests completeness (enumerate all keys in all languages).

**Invariants (8):**
- INV-D01: English catalogue has ≥100 entries (all M12 strings).
- INV-D02: `tr("menu.game_picker.title")` in English → "Select Game".
- INV-D03: `tr("menu.game_picker.title")` in Swedish → "Välj spel".
- INV-D04: `tr("nonexistent.key")` → returns the key itself.
- INV-D05: Language switch en → sv → en → same results as initial en.
- INV-D06: `trf("menu.game_picker.found", "DM1")` → "Found: DM1" (en).
- INV-D07: Every key in `en/firestaff.po` exists in `sv/firestaff.po` (completeness).
- INV-D08: Parsing a malformed .po file → returns error, no crash.

**Exit criteria:** Full i18n pipeline works end-to-end. All 4 languages load. No missing keys in Swedish (primary non-English language).

**Estimated effort:** 4 days.

---

### Phase E: Declarative Layout Engine

**Goal:** TOML-driven menu layout engine with widget types, focus navigation, and hit testing. No rendering yet — structure only.

**Modules:** `menu_layout_m12`

**Work:**
1. Define widget types: label, button, slider, dropdown, toggle, grid, tabs, scrolllist, separator, image, text_input.
2. Implement TOML layout parser.
3. Implement position/size resolver (flexbox-inspired: direction, gap, padding, alignment).
4. Implement focus navigation (next/prev/activate).
5. Implement hit testing.
6. Write probe: `firestaff_m12_layout_probe.c`.

**Probe:** Loads a test layout TOML. Resolves at 1920x1080. Verifies widget positions. Tests focus order. Tests hit testing. Tests resize behaviour.

**Invariants (7):**
- INV-E01: Parse a layout with 3 buttons → 3 widgets resolved.
- INV-E02: Vertical layout: widgets don't overlap.
- INV-E03: Focus navigation cycles through all focusable widgets.
- INV-E04: Hit test at button centre → returns button index.
- INV-E05: Hit test outside all widgets → returns -1.
- INV-E06: Resize from 1920x1080 to 1280x720 → widgets reposition without overlap.
- INV-E07: Round-trip: layout to TOML to layout → widget positions identical.

**Exit criteria:** Layout engine handles all M12 widget types with correct positioning and navigation.

**Estimated effort:** 5 days.

---

### Phase F: Box-Art + Image Pipeline

**Goal:** Load box-art images, cache as SDL textures, generate fallback title cards, support greyscale desaturation.

**Modules:** `boxart_cache_m12`

**Work:**
1. Vendor `stb_image.h` into `vendor/`.
2. Implement image loading → SDL texture creation.
3. Implement greyscale shader (or CPU-side desaturation before texture upload).
4. Implement fallback: generate a simple title card with game name when box-art is missing.
5. Implement texture cache (hash map: game_id → texture, with greyed variant).
6. Write probe: `firestaff_m12_boxart_probe.c`.

**Probe:** Loads test PNG images. Verifies texture dimensions. Tests missing image fallback. Tests greyscale variant. Tests cache hit.

**Note:** This phase requires M11's SDL3 renderer to be available. If M11 is delayed, this phase can use a mock renderer that captures calls without actual SDL.

**Invariants (5):**
- INV-F01: Load valid PNG → texture created with correct dimensions.
- INV-F02: Load missing file → fallback texture created (not NULL).
- INV-F03: Greyed-out request → texture is desaturated (pixel check: R=G=B for sampled pixel).
- INV-F04: Second request for same game_id → same texture pointer (cache hit).
- INV-F05: Shutdown → all textures freed (no leaks in valgrind/ASAN).

**Estimated effort:** 3 days.

---

### Phase G: Startup Menu (Game Picker)

**Goal:** Main startup screen with game picker grid, validation status indicators, and first-run popup.

**Modules:** `menu_main_m12`

**Work:**
1. Design game picker layout in TOML (3-column grid of box-art).
2. Implement rendering: box-art with ✓/✗ overlays, hover/selected states.
3. Implement first-run popup (directory picker — text input + file browser button).
4. Implement "Where do I get the original files?" info panel.
5. Implement navigation: select game → launch or grey-out feedback.
6. Wire to asset_validator_compat for real-time validation status.
7. Write probe: `firestaff_m12_menu_main_probe.c`.

**Probe:** Initialises the menu with mock validation results (DM1 playable, CSB not, DM2 not). Simulates keyboard navigation. Verifies correct game is selected. Tests first-run popup appears exactly once. Tests greyed-out game cannot be selected.

**Invariants (6):**
- INV-G01: DM1 playable + CSB not → DM1 shows ✓, CSB shows ✗.
- INV-G02: Selecting greyed-out game → no launch, feedback shown.
- INV-G03: First run (no config) → popup appears.
- INV-G04: Second run (config exists with data_path) → popup does NOT appear.
- INV-G05: Keyboard navigation cycles through available games only (skips greyed).
- INV-G06: Selecting DM1 → `menu_main_m12_selected_game()` returns "DM1".

**Estimated effort:** 5 days.

---

### Phase H: Settings Tabs — Graphics + Audio + Input

**Goal:** Three settings tabs with live preview. Graphics level, resolution, window mode, filters. Audio volumes with mutes. Input rebinding with keyboard layout display.

**Modules:** `menu_settings_m12` (partial — 3 of 7 tabs)

**Work:**
1. Design settings layout in TOML (tab container with sub-layouts per tab).
2. Implement Graphics tab: level dropdown, scaling mode dropdown, resolution dropdown (auto-populated from SDL), window mode dropdown, filter toggles/sliders.
3. Implement Audio tab: 4 volume sliders with mute toggles, device dropdown, buffer size dropdown.
4. Implement Input tab: key rebinding grid (action → key display), mouse sensitivity slider, gamepad toggle, keyboard layout dropdown.
5. Live preview: changing graphics settings → M11 applies immediately (resolution change, fullscreen toggle).
6. Write probe: `firestaff_m12_settings_probe.c`.

**Invariants (6):**
- INV-H01: Change graphics level to "upscaled" → config reflects "upscaled".
- INV-H02: Change master volume to 50 → config.audio.master_volume = 50.
- INV-H03: Rebind move_north to "Up" → config.input.keybindings.move_north = "Up".
- INV-H04: Tab navigation visits all 3 tabs.
- INV-H05: "Back" without changes → config_dirty = false.
- INV-H06: "Back" after changes → config_dirty = true, save triggered.

**Estimated effort:** 5 days.

---

### Phase I: Settings Tabs — Language + Bug Toggles + Credits

**Goal:** Remaining settings tabs. Language picker with live switch. Full bug-toggle UI. Credits scroll.

**Modules:** `menu_settings_m12` (complete), `menu_bugtoggles_m12`, `menu_credits_m12`

**Work:**
1. Implement Language tab: list of available languages, current selection highlighted, live switch on selection.
2. Integrate `menu_bugtoggles_m12` as Bug Toggles tab:
   - Preset dropdown at top.
   - Expandable tree below (bugs / changes, grouped by category).
   - Per-entry: toggle switch, tooltip expand (symptom, cause, affected versions).
   - Search bar.
   - Per-game filtering (hides irrelevant entries based on selected game).
3. Implement Credits tab: scrolling text with Firestaff version, team credits, Fontanel acknowledgement, licence.
4. Save-profile mismatch: wire warning popup into game-launch flow.
5. Write probe: `firestaff_m12_bugtoggles_probe.c`.

**Invariants (8):**
- INV-I01: Language switch to "sv" → all visible strings change to Swedish.
- INV-I02: Language switch to "sv" then "en" → all strings back to English.
- INV-I03: Bug toggle UI shows 201 entries total (unfiltered).
- INV-I04: Filter by "DM1" → hides BUG7_* and CHANGE7_* entries.
- INV-I05: Apply preset "DM 1.0a EN purist" → 89 bugs active.
- INV-I06: Toggle one bug after preset → label changes to "Custom".
- INV-I07: Save mismatch: mock save with different profile hash → warning shown.
- INV-I08: Credits screen contains "Christophe Fontanel" and "FTL Games".

**Estimated effort:** 6 days.

---

### Phase J: Config Persistence End-to-End

**Goal:** Full cycle: user changes settings → config saves → restart → settings restored exactly.

**Modules:** Integration of all prior modules.

**Work:**
1. Wire config save to settings "Back" / "Save" flow.
2. Wire config load to startup (before menu init).
3. Test: change every configurable value → save → quit → reload → verify all values match.
4. Test: CLI overrides (`--fullscreen`, `--language=sv`) take precedence.
5. Test: corrupt config → graceful fallback to defaults.
6. Write probe: `firestaff_m12_persistence_probe.c`.

**Invariants (5):**
- INV-J01: Save → load → all fields identical (full config round-trip with all fields non-default).
- INV-J02: Save → load → save → byte-identical files.
- INV-J03: Delete config → next launch shows first-run popup + defaults.
- INV-J04: CLI `--language=sv` overrides config file `language = "en"`.
- INV-J05: Corrupt config (random bytes) → defaults loaded, no crash.

**Estimated effort:** 3 days.

---

### Phase K: Integration Testing + M10/M11 Regression

**Goal:** Verify M12 doesn't break anything. All 20 M10 phases pass. M11 tests pass. Bug flags wire correctly through the tick orchestrator.

**Modules:** No new modules. Integration harness.

**Work:**
1. Run `run_firestaff_m10_verify.sh` → all 20 phases PASS.
2. Run M11 test suite → all tests PASS.
3. Test bug-flag wiring: load a config with BUG0_02 active → run tick orchestrator for 100 ticks → verify 24-bit time behaviour. Load config with BUG0_02 inactive → run → verify 32-bit time behaviour. World-hashes differ.
4. Test replay: record 100 ticks with preset A → replay with preset A → hashes match. Replay with preset B → hashes differ.
5. Asset validator against real DUNGEON.DAT (if available in test environment).
6. Write probe: `firestaff_m12_integration_probe.c`.

**Invariants (7):**
- INV-K01: All 20 M10 phases PASS with no M10 source modifications.
- INV-K02: Bug flag BUG0_02 active → tick 16777200 exhibits 24-bit wrap.
- INV-K03: Bug flag BUG0_02 inactive → tick 16777200 does NOT wrap.
- INV-K04: World-hash after 100 ticks with preset A ≠ world-hash with preset B (if presets differ on any flag that fires in 100 ticks).
- INV-K05: Replay: record 100 ticks → replay → all world-hashes match.
- INV-K06: Replay with wrong flag profile → mismatch detected.
- INV-K07: M11 test suite passes (if available; skip-with-note if M11 not yet complete).

**Estimated effort:** 4 days.

---

### Phase L: Polish + Platform Testing

**Goal:** Final polish. Test on macOS, Linux, Windows. Fix platform-specific issues. Documentation.

**Modules:** No new modules.

**Work:**
1. Test on macOS (Apple Silicon): build, run, verify all menus render correctly.
2. Test on Linux (Ubuntu 22.04, x86_64): build, run, verify.
3. Test on Windows 10 (x86_64): build, run, verify.
4. Fix any platform-specific issues (font rendering, file paths, atomic write semantics).
5. Write user documentation: `docs/configuration.md` — explains every config option.
6. Write contributor documentation: `docs/translations.md` — how to add a new language.
7. Update `README.md` with M12 features.
8. Update `MILESTONES.md` with M12 completion.
9. Final verify: `run_firestaff_m12_verify.sh` runs all M12 probes + M10 regression.

**Invariants (4):**
- INV-L01: Build succeeds on macOS (arm64).
- INV-L02: Build succeeds on Linux (x86_64).
- INV-L03: Build succeeds on Windows (x86_64, MSVC or MinGW).
- INV-L04: `run_firestaff_m12_verify.sh` exits 0 with all probes PASS.

**Estimated effort:** 4 days.

---

### Phase Summary Table

| Phase | Name | Modules | Invariants | Dependencies | Est. Days |
|-------|------|---------|------------|--------------|-----------|
| A | Config Foundation | config_io, config_schema | 8 | M11 Phase G (fs_portable) | 3 |
| B | Asset Database + Validator | asset_database, asset_validator | 12 | Phase A | 4 |
| C | Bug Flags + Database | bug_flags, bug_database | 14 | Phase A (for config integration) | 3 |
| D | i18n Engine | i18n_compat, i18n_po_loader | 8 | None (standalone) | 4 |
| E | Declarative Layout Engine | menu_layout | 7 | toml.c (vendored in A) | 5 |
| F | Box-Art + Image Pipeline | boxart_cache | 5 | M11 SDL3 renderer | 3 |
| G | Startup Menu | menu_main | 6 | Phases B, D, E, F | 5 |
| H | Settings: Gfx/Audio/Input | menu_settings (partial) | 6 | Phases D, E | 5 |
| I | Settings: Lang/Bugs/Credits | menu_bugtoggles, menu_credits | 8 | Phases C, D, E, H | 6 |
| J | Config Persistence E2E | (integration) | 5 | Phases A–I | 3 |
| K | Integration + Regression | (integration) | 7 | Phases A–J, M10, M11 | 4 |
| L | Polish + Platform Testing | (docs + fixes) | 4 | Phase K | 4 |
| **Total** | | **14 modules** | **95** | | **49 days** |

### Parallelism Opportunities

```
M11 Phase G lands
    │
    ├── Phase A (config)  ─── 3d ───┐
    │                               │
    ├── Phase D (i18n)    ─── 4d ───┤
    │                               │
    │   Phase B (assets)  ─── 4d ───┤  (after A)
    │                               │
    │   Phase C (bugflags)─── 3d ───┤  (after A)
    │                               │
    │   Phase E (layout)  ─── 5d ───┤  (after A)
    │                               │
    │                               │  M11 SDL3 renderer available
    │                               │       │
    │   Phase F (boxart)  ─── 3d ───┤  ─────┘
    │                               │
    │   Phase G (menu)    ─── 5d ───┤  (after B, D, E, F)
    │                               │
    │   Phase H (settings)─── 5d ───┤  (after D, E)
    │                               │
    │   Phase I (bugs UI) ─── 6d ───┤  (after C, D, E, H)
    │                               │
    │   Phase J (persist) ─── 3d ───┤  (after A-I)
    │                               │
    │   Phase K (integr.) ─── 4d ───┤  (after J)
    │                               │
    │   Phase L (polish)  ─── 4d ───┘  (after K)

Critical path: A(3) → E(5) → G(5) → I(6) → J(3) → K(4) → L(4) = 30 days
With parallelism: ~30-35 working days (6-7 weeks)
Without parallelism: 49 working days (~10 weeks)
```

---

## §8  Invariant Philosophy for M12

### §8.1  Approach

M10's invariant philosophy (500+ invariants across 20 phases) applied to a deterministic engine where every output is predictable. M12 presents a different challenge: UI rendering produces pixels that depend on fonts, SDL driver, display resolution, and platform.

**Solution:** Test what's testable deterministically; mock what isn't.

| Layer | Testing approach |
|-------|-----------------|
| Config read/write | Pure data → fully deterministic invariants |
| Asset validation | Synthetic files with known MD5s → fully deterministic |
| Bug flags | Pure bit manipulation → fully deterministic |
| i18n | String table lookups → fully deterministic |
| Layout engine | Widget positions at fixed viewport → deterministic |
| Menu logic | State machines with mock input → deterministic |
| Rendering | Mock renderer that captures draw calls → structure tests |

We do NOT test pixel output in invariants. We test **widget structure, state transitions, and data integrity**.

### §8.2  Invariant Categories and Counts

| Category | Description | Count |
|----------|-------------|-------|
| Config round-trip | TOML write → read → compare | 8 |
| Asset validation | MD5 matching, caching, stress | 12 |
| Bug flag bit-mask | Set/clear/test, presets, hashing | 14 |
| i18n string-table | Lookup, fallback, completeness | 8 |
| Layout engine | Positioning, navigation, hit-test | 7 |
| Box-art cache | Loading, fallback, greyscale | 5 |
| Game picker menu | Selection, gating, first-run | 6 |
| Settings tabs | Value binding, tab navigation | 6 |
| Bug toggle UI | Filtering, presets, mismatch | 8 |
| Config persistence | End-to-end save/load cycle | 5 |
| Integration + regression | M10 compat, replay, flag wiring | 7 |
| Platform + polish | Build success, verify gate | 4 |
| **Grand total** | | **95** |

**Target exceeded:** 95 invariants vs. the 60 minimum target. The asset validator stress test (INV-B12) alone exercises 100 file combinations.

### §8.3  Invariant Naming Convention

`INV-{Phase}{Number}`: e.g. `INV-A01`, `INV-B12`, `INV-K07`.

All invariants are codified in probe source files (`firestaff_m12_*_probe.c`) and checked by a single verify gate script: `run_firestaff_m12_verify.sh`.

The verify gate prints:
```
# Phase A: config_io_m12 ................ PASS (8/8 invariants)
# Phase B: asset_validator_compat ....... PASS (12/12 invariants)
...
# Phase L: platform_polish .............. PASS (4/4 invariants)
# M12 TOTAL: 95/95 invariants PASS
```

---

## §9  Risk Register

### R1: TOML Library Choice Affects Cross-Platform (MEDIUM)

**Risk:** `tomlc99` is pure C99 and well-tested, but it's maintained by a single developer. If it's abandoned, we carry a vendored fork.

**Mitigation:**
- Vendor at a specific commit hash, not `HEAD`.
- tomlc99 is ~1600 LOC — small enough to maintain a fork if needed.
- Our config format uses only basic TOML features (strings, ints, bools, tables, arrays of strings). No inline tables, no datetimes, no complex nesting. This limits our exposure to parser edge cases.
- Fallback plan: if tomlc99 proves problematic, swap to `inih` (INI format) with a schema migration. The config file is simple enough that INI would work with `[section]` headers.

**Likelihood:** Low. **Impact:** Medium. **Priority:** 3.

---

### R2: Config File Corruption Recovery (HIGH)

**Risk:** A crash or power loss during config write leaves a corrupt or missing file. Users lose all their settings.

**Mitigation:**
- Atomic writes via write-tmp-fsync-rename pattern (see §3.4).
- On load: if `config.toml` is valid, use it. If corrupt, try `config.toml.bak` (created on each successful save before overwriting). If both corrupt, regenerate defaults.
- Backup rotation: keep one `.bak` file (previous good save).
- Never delete the user's config — only overwrite atomically.
- Log corruption events to `firestaff.log` so users can report.

**Likelihood:** Low (with atomic writes). **Impact:** High (lost settings = bad UX). **Priority:** 1.

---

### R3: MD5 Database Maintenance (MEDIUM)

**Risk:** The MD5 database gets outdated as the community discovers new legitimate variant dumps. Users with valid game files can't play because their MD5 doesn't match.

**Mitigation:**
- Accept MD5 arrays per file (multiple valid hashes).
- Publish the database as `data/assets.json` in the GitHub repo — easy for community PRs.
- Include a "Report unknown file" button in the first-run popup that shows the MD5 of unrecognised files so users can report them.
- Version the database independently of the binary (auto-update check on startup, download new `assets.json` from GitHub Releases if available).
- Escape hatch: config option `data_path_override = true` that skips MD5 validation entirely and trusts the user's files. Displayed as "Advanced: skip validation" in the UI with a warning.

**Likelihood:** Medium (DM community is niche but active). **Impact:** Medium (users blocked from playing). **Priority:** 2.

---

### R4: Bug-Toggle Granularity (MEDIUM)

**Risk:** Some Fontanel-documented bugs span multiple functions across multiple phases. A single bit-flag may not be sufficient if the bug's original behaviour requires changes in 3+ code sites. Worse, some bugs interact: fixing BUG0_08 (thing-pool exhaustion) may change the conditions under which BUG0_09 (discard-thing sensor trigger) manifests.

**Mitigation:**
- One flag per Fontanel-documented bug/change ID. If a single bug requires changes in 5 code sites, all 5 check the same flag.
- Document inter-bug dependencies in the database: `"interacts_with": ["BUG0_09"]`.
- UI tooltip for dependent bugs: "Note: this bug interacts with BUG0_09. Toggling one without the other may produce unexpected behaviour."
- Testing: probe scripts test each preset as a coherent unit (all flags in the preset are set/cleared together). Individual flag tests verify single-site behaviour only.
- Escape hatch: if a flag combination is known to be unstable, mark it in the database as `"unsupported_combinations": [{"BUG0_08": true, "BUG0_09": false}]` and warn in the UI.

**Likelihood:** Medium. **Impact:** Medium (subtle gameplay differences, hard to debug). **Priority:** 4.

---

### R5: Box-Art Licensing (LOW-MEDIUM)

**Risk:** Shipping copyrighted box-art images (1987/1989 FTL Games artwork) may trigger DMCA complaints, even in a preservation context.

**Mitigation:**
- Do NOT ship box-art in the binary distribution.
- Ship a `data/boxart/` directory with a `README.md` explaining how to add your own images.
- On first run, if box-art is missing, display generated fallback title cards (game name + simple dungeon motif in Firestaff's visual style).
- The game picker works perfectly with fallback images — box-art is cosmetic enhancement only.
- Community can distribute box-art packs separately (not our legal responsibility).
- If a rights-holder complains about any community distribution, we're not involved — the binary ships clean.
- In the `assets.json`, include `boxart_url` fields pointing to MobyGames/dmweb.free.fr galleries where users can download their own copies.

**Likelihood:** Low (FTL Games is defunct; rights are murky). **Impact:** Medium (legal hassle). **Priority:** 5.

---

### Risks Considered but Not in Top 5

| Risk | Why deprioritised |
|------|-------------------|
| R2 (i18n font sizes) | Noto Sans Latin subset is ~300 KB. Acceptable. Only a risk if we add CJK (not in v1). |
| R7 (Gamepad edge cases) | SDL3 handles gamepad abstraction well. Edge cases are minor and non-blocking — game is primarily KB+mouse. |

---

## §10  Acceptance Criteria

M12 is complete when ALL of the following are true:

### Functional Criteria

| # | Criterion | Verification |
|---|-----------|--------------|
| AC-01 | Startup menu opens and displays correctly on macOS, Linux, and Windows | Manual test on all 3 platforms |
| AC-02 | Game picker correctly greys out unavailable games based on asset validation | INV-G01, INV-G02 |
| AC-03 | First-run popup appears exactly once (on first launch with no config file) | INV-G03, INV-G04 |
| AC-04 | Bug-toggle UI shows all 201 entries with correct per-game filtering | INV-I03, INV-I04 |
| AC-05 | Version presets produce correct flag counts | INV-C08, INV-C09, INV-I05 |
| AC-06 | Settings persist across sessions (quit → relaunch → all values restored) | INV-J01, INV-J02 |
| AC-07 | Runtime language switch changes ALL visible strings immediately | INV-I01, INV-I02 |
| AC-08 | Asset validator correctly identifies valid DUNGEON.DAT for DM1 PC 3.4 EN | INV-B01 |
| AC-09 | Asset validator rejects bit-rotted or wrong-hash files | INV-B03 |
| AC-10 | All 20 M10 phases still PASS (zero regressions) | INV-K01 |
| AC-11 | All M11 tests still PASS (zero regressions) | INV-K07 |
| AC-12 | Bug flags wire correctly through tick orchestrator (BUG0_02 test) | INV-K02, INV-K03 |
| AC-13 | Replay determinism maintained with flag profiles | INV-K05, INV-K06 |
| AC-14 | Save-file profile mismatch produces user warning | INV-I07 |

### Quality Criteria

| # | Criterion |
|---|-----------|
| QC-01 | ≥95 invariants PASS in `run_firestaff_m12_verify.sh` |
| QC-02 | No compiler warnings on any platform (GCC -Wall -Wextra -Werror, Clang, MSVC /W4) |
| QC-03 | No memory leaks detected by ASAN/Valgrind on probe runs |
| QC-04 | Config file atomic write verified (kill -9 during save → no corruption) |
| QC-05 | All .po files have 100% key coverage for Swedish, ≥90% for French/German |

### Documentation Criteria

| # | Criterion |
|---|-----------|
| DC-01 | `docs/configuration.md` documents every config option |
| DC-02 | `docs/translations.md` documents how to add a new language |
| DC-03 | `MILESTONES.md` updated with M12 status |
| DC-04 | `README.md` updated with M12 features |

---

## §11  Estimated Effort

### Per-Phase Breakdown

| Phase | Name | Estimated Days | Cumulative |
|-------|------|---------------|------------|
| A | Config Foundation | 3 | 3 |
| B | Asset Database + Validator | 4 | 7 |
| C | Bug Flags + Database | 3 | 10 |
| D | i18n Engine | 4 | 14 |
| E | Declarative Layout Engine | 5 | 19 |
| F | Box-Art + Image Pipeline | 3 | 22 |
| G | Startup Menu (Game Picker) | 5 | 27 |
| H | Settings: Gfx/Audio/Input | 5 | 32 |
| I | Settings: Lang/Bugs/Credits | 6 | 38 |
| J | Config Persistence E2E | 3 | 41 |
| K | Integration + Regression | 4 | 45 |
| L | Polish + Platform Testing | 4 | 49 |

### Total Milestone Estimate

| Scenario | Duration | Notes |
|----------|----------|-------|
| **Sequential (single developer)** | **49 working days (~10 weeks)** | All phases one after another |
| **With parallelism** | **30-35 working days (~6-7 weeks)** | Phases A+D parallel; B+C parallel after A; E after A; F after SDL3; G+H after deps |
| **Critical path** | **30 working days (~6 weeks)** | A → E → G → I → J → K → L |

### Assumptions

- Developer is familiar with the codebase (has worked on M10/M11).
- M11 Phase G (`fs_portable_compat`) is available before M12 Phase A starts.
- M11 SDL3 renderer is available before M12 Phase F starts.
- Translation effort (Swedish .po file) is done by Danne concurrently with development.
- French and German translations are stubs in v1 (community-contributed post-release).

### Comparison with M10

M10 (20 phases, 500+ invariants, pure engine core) took significantly more effort due to Fontanel reverse-engineering research, disassembly analysis, and determinism requirements. M12 is architecturally simpler (no reverse engineering, no determinism constraints on the UI layer) but broader in surface area (14 modules, 3 platforms, 4 languages).

---

## Appendix A: File Listing

New files created by M12 (all under `src/m12/` or project root as appropriate):

```
src/m12/
├── config_io_m12.h
├── config_io_m12.c
├── config_schema_m12.h
├── config_schema_m12.c
├── asset_validator_compat.h
├── asset_validator_compat.c
├── asset_database_m12.h
├── asset_database_m12.c
├── bug_flags_compat.h
├── bug_flags_compat.c
├── bug_database_m12.h
├── bug_database_m12.c
├── i18n_compat.h
├── i18n_compat.c
├── i18n_po_loader_m12.h
├── i18n_po_loader_m12.c
├── menu_main_m12.h
├── menu_main_m12.c
├── menu_settings_m12.h
├── menu_settings_m12.c
├── menu_bugtoggles_m12.h
├── menu_bugtoggles_m12.c
├── menu_credits_m12.h
├── menu_credits_m12.c
├── menu_layout_m12.h
├── menu_layout_m12.c
├── boxart_cache_m12.h
├── boxart_cache_m12.c

probes/
├── firestaff_m12_config_probe.c
├── firestaff_m12_asset_validator_probe.c
├── firestaff_m12_bug_flags_probe.c
├── firestaff_m12_i18n_probe.c
├── firestaff_m12_layout_probe.c
├── firestaff_m12_boxart_probe.c
├── firestaff_m12_menu_main_probe.c
├── firestaff_m12_settings_probe.c
├── firestaff_m12_bugtoggles_probe.c
├── firestaff_m12_persistence_probe.c
├── firestaff_m12_integration_probe.c

vendor/
├── toml.c
├── toml.h
├── cJSON.c
├── cJSON.h
├── md5.c
├── md5.h
├── stb_image.h
├── stb_truetype.h

data/
├── assets.json
├── presets.toml
├── fonts/
│   └── NotoSans-Regular.ttf

locale/
├── firestaff.pot
├── en/
│   └── firestaff.po
├── sv/
│   └── firestaff.po
├── fr/
│   └── firestaff.po
└── de/
    └── firestaff.po

layouts/
├── game_picker.toml
├── settings_graphics.toml
├── settings_audio.toml
├── settings_input.toml
├── settings_language.toml
├── settings_bugtoggles.toml
├── settings_credits.toml
├── first_run_popup.toml

tools/
├── extract_strings.py
├── check_translations.py

docs/
├── configuration.md
├── translations.md

scripts/
└── run_firestaff_m12_verify.sh
```

**Estimated total new code:** ~10,500 LOC (modules) + ~3,000 LOC (probes) + ~12,000 LOC (vendored) = ~25,500 LOC.

---

## Appendix B: Config Migration Example

When `schema_version` changes in a future release:

```c
/* Migration registry in config_io_m12.c */
static const ConfigMigration_M12 migrations[] = {
    { 1, 2, migrate_v1_to_v2 },  /* e.g. rename "scaling_mode" → "scale_mode" */
    { 2, 3, migrate_v2_to_v3 },  /* e.g. add new [accessibility] section */
};

static int migrate_v1_to_v2(FirestaffConfig_M12 *cfg) {
    /* Rename field, set new defaults for added fields */
    cfg->schema_version = 2;
    return 0;
}
```

Migrations run sequentially: 1→2→3→...→current. Each migration is a pure function on the config struct.

---

## Appendix C: Bug-Toggle Bit Index Assignment

The `firestaff_flag_id` field in `firestaff-bugs.json` maps directly to bit indices in `BugFlagMask_Compat`:

| Range | Contents |
|-------|----------|
| 0–88 | BUG0_00 through BUG0_88 (89 original DM 1.0a bugs) |
| 89 | BUG2_00 (introduced in DM 1.1 EN) |
| 90 | BUG5_00 (introduced in DM 1.3a FR) |
| 91–92 | BUG7_00, BUG7_01 (introduced in CSB 2.0 EN) |
| 93–95 | Reserved for future bug discoveries |
| 96–203 | CHANGE1_* through CHANGE8_* (108 changes, in version order) |
| 204–255 | Reserved for PC 3.4-specific and community-discovered entries |

This assignment is stable across releases. New entries are appended to the reserved ranges. Existing bit indices never change (would break save-file profile hashes).

---

*End of M12 Plan.*
