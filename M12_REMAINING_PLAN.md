# M12 — Remaining Work to Practical Completion

**Updated:** 2026-04-21
**Baseline:** 20/20 startup menu invariants passing (launcher, settings, game options, config persistence, MD5 validation)
**Status:** ~30% complete — launcher shell is solid, remaining work is bug-toggle UI, i18n, declarative layout, and integration polish

---

## §1  Current Verified Baseline

### What is already done (verified by probes)

| Area | Status | Probe evidence |
|------|--------|---------------|
| **Startup menu (game picker)** | ✅ Done | INV_M12_01 (three games + settings exposed) |
| **4-zone card layout** | ✅ Done | INV_M12_12 (structured non-empty menu pixels) |
| **Asset validation (MD5)** | ✅ Done | INV_M12_02–03 (MD5-based scan, checksum coverage) |
| **Card art (built-in + external)** | ✅ Done | INV_M12_02B (external slot promotion) |
| **Game options panel** | ✅ Done | INV_M12_06–06F (cheats, speed, hotkeys, launch) |
| **Keyboard navigation** | ✅ Done | INV_M12_04–05 (up/down row navigation) |
| **Settings screen** | ✅ Done | INV_M12_09–10 (opens, cycles persisted values) |
| **Config persistence** | ✅ Done | INV_M12_11 (settings, palette, data dir survive reload) |
| **Language + graphics selection** | ✅ Done | INV_M12_12B (changes rendered output) |
| **Message view** | ✅ Done | INV_M12_07–08 (escape returns, scaffold-only explains) |
| **Exit flow** | ✅ Done | INV_M12_13 (escape on top-level requests exit) |
| **Branding logo** | ✅ Done | `branding_logo_m12.{h,c}` compiled and rendered |

### Existing M12 source files

| File | LOC | Purpose |
|------|-----|---------|
| `menu_startup_m12.c` | ~2900 | Main launcher logic, views, navigation |
| `menu_startup_m12.h` | ~180 | Public API + state structs |
| `config_m12.c` | ~780 | TOML-like config read/write |
| `config_m12.h` | ~70 | Config struct |
| `asset_status_m12.c` | ~1560 | MD5-based asset discovery |
| `asset_status_m12.h` | ~95 | Asset status structs |
| `card_art_m12.c` | ~1470 | Card art generation + loading |
| `card_art_m12.h` | ~150 | Card art API |
| `branding_logo_m12.c` | ~15200 | Embedded logo pixel data |
| `branding_logo_m12.h` | ~47 | Logo accessor |
| `asset_validator_checksums_m12.json` | ~1570 | Known-good MD5 database |

### What is NOT done

| Area | Status | Notes |
|------|--------|-------|
| **Bug-fix toggle UI** | ❌ Not started | 201-entry database, version presets, custom mix — the big feature |
| **i18n engine** | ❌ Not started | .po parsing, runtime string lookup, language switch |
| **Translation files** | ❌ Not started | en/sv/fr/de .po files |
| **Declarative layout engine** | ❌ Not started | TOML-driven widget positioning, the original plan's Phase E |
| **Full key rebinding UI** | ❌ Not started | Settings screen has placeholders |
| **Gamepad mapping** | ❌ Not started | |
| **Credits/About screen** | ❌ Not started | |
| **Box-art from PNG files** | 🟡 Partial | Built-in card art works; external PNG loading not yet implemented |
| **Audio settings** | 🟡 Partial | Volume stored in config; no device selection or buffer size |
| **Graphics settings detail** | 🟡 Partial | Level selection works; resolution/filter options not wired |
| **Mouse input in menu** | 🟡 Partial | Keyboard navigation works; mouse hit testing incomplete |
| **Schema versioning** | ❌ Not started | Config has no migration path |

---

## §2  Remaining Scope

### What constitutes M12 "practically complete"

M12 is done when:
1. The launcher lets users pick DM1 (and greyed CSB/DM2), configure settings, and launch
2. Bug-fix toggles are exposed with version presets (purist/modern/custom)
3. At least English + Swedish UI strings use the i18n pipeline
4. Settings persist across restarts
5. The credits screen shows version + attribution

### What can be deferred

- Full declarative TOML layout engine → simplify to hardcoded C layouts (the existing approach works fine)
- .po file extraction tooling → manual string management for v1
- French/German translations → stubs only, fill later
- Gamepad mapping → defer entirely
- Box-art PNG loading → built-in card art is sufficient
- RTL readiness → far future
- Schema versioning migration → add when schema actually changes

---

## §3  Execution Roadmap (Ordered Slices)

### Slice 1: Bug-Fix Toggle Infrastructure (Priority: HIGH)

**Goal:** 256-bit flag mask with preset support, wired to M10's `bug_flags_compat` bitmask.

**Files involved:**
- `bug_flags_compat.h` (new)
- `bug_flags_compat.c` (new)
- `bug_database_m12.h` (new)
- `bug_database_m12.c` (new)
- `firestaff-bugs.json` (new — 201-entry database)

**Implementation notes:**
- Each toggle maps to a bit index 0–255 in a `uint8_t mask[32]` array.
- Operations: set, clear, test, popcount, serialize (32 bytes), deserialize.
- Profile hash: CRC32 over the 32 serialized bytes → deterministic fingerprint.
- Presets:
  - "DM 1.0a EN purist" → enable all 93 original bugs (reproduce authentic behavior)
  - "PC 3.4 baseline" → enable bug-fixes from the official patches
  - "Modern" → all fixes + optional improvements active
  - "Custom" → user picks individual toggles
- The database JSON has 201 entries, each with: id, title, description, category (BUG/FIX/OPT/IMP), affected_versions, severity.
- M10's tick orchestrator already has a `bug_flags` field in `GameWorld_Compat`. Changing a flag recomputes the world hash.

**Invariants to add (12):**
- `INV_BF_01`: Empty mask → popcount = 0
- `INV_BF_02`: Set bit 0 → test bit 0 = 1
- `INV_BF_03`: Set bit 255 → test bit 255 = 1
- `INV_BF_04`: Set then clear → test = 0
- `INV_BF_05`: Serialize → deserialize round-trip identical
- `INV_BF_06`: Profile hash is deterministic (same mask = same hash)
- `INV_BF_07`: Different masks produce different hashes
- `INV_BF_08`: Purist preset → 93 bugs active
- `INV_BF_09`: Modern preset → all FIX+OPT+IMP active, 0 bugs active
- `INV_BF_10`: PC 3.4 baseline → expected flag count
- `INV_BF_11`: Database loads 201 entries from JSON
- `INV_BF_12`: Filter by game "DM1" excludes CSB-only entries

**Verification:**
```bash
cc -std=c99 -Wall -O2 -I. -o firestaff_m12_bug_flags_probe_bin \
    firestaff_m12_bug_flags_probe.c bug_flags_compat.c bug_database_m12.c
./firestaff_m12_bug_flags_probe_bin
```

**Effort:** 3 days

---

### Slice 2: Bug-Fix Toggle UI (Priority: HIGH)

**Goal:** Add a bug-toggle screen to the launcher accessible from settings or game options.

**Files involved:**
- `menu_startup_m12.c` (extend with bug-toggle view)
- `menu_startup_m12.h` (add M12_MENU_VIEW_BUG_TOGGLES)
- `config_m12.c` (persist selected preset + custom flags)

**Implementation notes:**
- New view: `M12_MENU_VIEW_BUG_TOGGLES`
- Top section: preset selector (dropdown/cycle: Purist / PC 3.4 / Modern / Custom)
- Main section: scrollable list of 201 entries, each showing title + active/inactive toggle
- Selecting a preset sets all flags. Changing any individual toggle switches to "Custom".
- Color coding: BUG entries in red, FIX in green, OPT in blue, IMP in yellow.
- Per-game gating: entries not applicable to current game are greyed out.
- Profile hash shown at bottom: "Profile: 0xABCD1234"
- Save-profile mismatch warning: if loading a save with different bug_flags, show warning.

**Invariants to add (8):**
- `INV_BT_01`: Bug toggle view opens from settings
- `INV_BT_02`: Preset "Purist" activates expected flag count
- `INV_BT_03`: Preset "Modern" activates expected flag count
- `INV_BT_04`: Individual toggle changes preset to "Custom"
- `INV_BT_05`: Scrolling through 201 entries doesn't crash
- `INV_BT_06`: Profile hash updates when flags change
- `INV_BT_07`: Bug flags persist in config across restart
- `INV_BT_08`: Inapplicable entries for current game are non-interactive

**Effort:** 4 days

---

### Slice 3: i18n Engine (Priority: MEDIUM)

**Goal:** Runtime string lookup with language switching, .po file parsing.

**Files involved:**
- `i18n_compat.h` (new)
- `i18n_compat.c` (new)
- `i18n_po_loader_m12.h` (new)
- `i18n_po_loader_m12.c` (new)
- `locale/en/firestaff.po` (new)
- `locale/sv/firestaff.po` (new)

**Implementation notes:**
- .po parser: handle `msgid`/`msgstr`, multi-line (continuation with `"`), `#` comments, escape sequences.
- String table: FNV-1a hash map with ~512 buckets. Keys are msgid strings.
- API: `i18n_init()`, `i18n_set_language(lang_code)`, `tr(key)` → returns translated string.
- Fallback chain: current language → English → key itself.
- `i18n_set_language` switches without restart (hot-reload for menu).
- Start with ~80 strings covering all M12 UI elements.
- Swedish translations by Daniel.

**Invariants to add (8):**
- `INV_I18N_01`: English catalog has ≥80 entries
- `INV_I18N_02`: `tr("menu.title")` in English returns "Select Game" (or similar)
- `INV_I18N_03`: `tr("menu.title")` in Swedish returns Swedish string
- `INV_I18N_04`: Unknown key returns the key itself
- `INV_I18N_05`: Language switch en → sv → en produces same results
- `INV_I18N_06`: Malformed .po file returns error without crash
- `INV_I18N_07`: Empty .po file loads without crash
- `INV_I18N_08`: All English keys exist in Swedish catalog

**Effort:** 4 days

---

### Slice 4: Wire i18n into Launcher (Priority: MEDIUM)

**Goal:** Replace all hardcoded English strings in the launcher with `tr()` calls.

**Files involved:**
- `menu_startup_m12.c` (replace string literals with tr() calls)
- `m11_game_view.c` (replace HUD strings with tr() calls)
- `config_m12.c` (store selected language)

**Implementation notes:**
- Go through all string literals in the menu rendering functions.
- Replace with `tr("menu.settings.title")` etc.
- Language selector in settings already cycles language — wire it to `i18n_set_language()`.
- Game view strings (HUD labels, message log templates) also get tr() calls.

**Invariants to add (4):**
- `INV_I18W_01`: Menu renders without hardcoded English when Swedish is active
- `INV_I18W_02`: Settings language cycle triggers i18n language switch
- `INV_I18W_03`: Game view HUD labels change language
- `INV_I18W_04`: Config stores language selection for next startup

**Effort:** 2 days

---

### Slice 5: Credits/About Screen (Priority: LOW)

**Goal:** Show version, engine credits, original DM team, Fontanel, license info.

**Files involved:**
- `menu_startup_m12.c` (add M12_MENU_VIEW_CREDITS)

**Implementation notes:**
- Accessible from settings or a dedicated menu entry.
- Shows:
  - Firestaff version (from CMake VERSION)
  - "Based on Dungeon Master by FTL Games (1987)"
  - "Engine reconstruction based on Christophe Fontanel's ReDMCSB"
  - "Deterministic engine core: M10 (20 phases, 500+ invariants)"
  - License: MIT (or whatever is chosen)
  - Contributors list (from CONTRIBUTORS file if it exists)
- Scrollable text view with Escape to return.

**Invariants to add (3):**
- `INV_CR_01`: Credits view opens from settings
- `INV_CR_02`: Credits contain version string
- `INV_CR_03`: Escape returns to settings

**Effort:** 1 day

---

### Slice 6: Mouse Input in Menu (Priority: LOW)

**Goal:** Full mouse interaction in the launcher (click cards, click settings, click toggles).

**Files involved:**
- `menu_startup_m12.c` (add hit-test functions)
- `menu_startup_m12.h` (add hit-test API)

**Implementation notes:**
- Current state: keyboard-only navigation works. Mouse events are received via SDL but not mapped to menu actions.
- Add hit-test regions for each card, each settings control, each toggle.
- Mouse click → determine which element → same action as keyboard Enter on that element.
- Hover state: highlight focused element on mouse move.
- Uses `M11_Render_MapWindowToFramebuffer()` from render_sdl_m11 for coordinate mapping.

**Invariants to add (4):**
- `INV_MO_01`: Click on DM1 card selects it
- `INV_MO_02`: Click on Settings card opens settings
- `INV_MO_03`: Click on volume slider changes volume
- `INV_MO_04`: Mouse hover highlights focused element

**Effort:** 2 days

---

### Slice 7: Integration Polish (Priority: MEDIUM)

**Goal:** End-to-end launcher → game → launcher flow, screenshot verification, README.

**Files involved:**
- Various (bug fixes)
- `README.md` (update M12 section)
- Verification screenshots

**Implementation notes:**
- Full flow test: launch → select DM1 → configure bug toggles → launch game → play → save → return to menu → change language → re-enter game → load → verify.
- Capture verification screenshots for all major screens.
- Update README with M12 features.
- Verify all existing probes still pass.

**Invariants to add (4):**
- `INV_IP_01`: Full launch → play → return → relaunch flow completes without crash
- `INV_IP_02`: All M11 probes still pass
- `INV_IP_03`: All M12 probes still pass
- `INV_IP_04`: Verification screenshots captured for all views

**Effort:** 2 days

---

## §4  Recommended Execution Order

```
Slice 1 (Bug flags infra)    ← foundation for toggle UI
  ↓
Slice 2 (Bug toggle UI)      ← the major M12 feature
  ↓
Slice 3 (i18n engine)        ← foundation for translations
  ↓
Slice 4 (Wire i18n)          ← makes translations visible
  ↓
Slice 5 (Credits)            ← quick win
  ↓
Slice 6 (Mouse input)        ← polish
  ↓
Slice 7 (Integration)        ← final validation
```

**Critical path:** Slices 1→2→3→4→7 = ~15 days
**Full path including optional:** ~18 days

---

## §5  Per-Slice Codex Implementation Briefs

### Codex Prompt: Slice 1 (Bug Flags Infrastructure)

```
In the firestaff project (tmp/firestaff/), create bug_flags_compat.h and bug_flags_compat.c:

1. Define BugFlagMask_Compat as a struct with uint8_t bytes[32] (256 bits)
2. Implement: BugFlags_Init (zero), BugFlags_Set(mask, bit), BugFlags_Clear(mask, bit),
   BugFlags_Test(mask, bit) → bool, BugFlags_Popcount(mask) → int
3. Implement: BugFlags_Serialize(mask, buf32), BugFlags_Deserialize(buf32, mask)
4. Implement: BugFlags_ProfileHash(mask) → uint32_t CRC32 over the 32 bytes
5. Bit index range: 0-255. Out-of-range is no-op.

Create bug_database_m12.h and bug_database_m12.c:
1. Define BugEntry_M12 with: int id, char title[128], char desc[256],
   enum category (BUG/FIX/OPT/IMP), char affected_versions[64], int bit_index
2. Load from firestaff-bugs.json (use a minimal embedded JSON parser or sscanf-based line parser)
3. Provide: BugDB_Load(path, entries[], maxEntries) → count
4. Provide: BugDB_ApplyPreset(entries[], count, presetName, mask) → sets flags per preset rules
5. Provide: BugDB_Filter(entries[], count, gameId, results[], maxResults) → filtered count

Create firestaff-bugs.json with at least 20 representative entries covering:
- 5 BUG entries (original DM1 bugs with authentic IDs)
- 5 FIX entries (patch fixes)
- 5 OPT entries (optional behavior changes)
- 5 IMP entries (quality-of-life improvements)
The full 201-entry database can be populated incrementally.

Create firestaff_m12_bug_flags_probe.c with 12 invariants as specified in the plan.
```

### Codex Prompt: Slice 3 (i18n Engine)

```
In the firestaff project (tmp/firestaff/), create i18n_compat.h and i18n_compat.c:

1. Hash map with FNV-1a hashing, 512 buckets, open addressing
2. Each entry: msgid (key) → msgstr (value)
3. API:
   - int i18n_init(void) — initializes empty catalog
   - int i18n_load_po(const char* path) — parses .po file into current catalog
   - int i18n_set_language(const char* lang_code) — loads locale/{lang}/firestaff.po
   - const char* tr(const char* key) — lookup with fallback chain
   - void i18n_shutdown(void)
4. Fallback: current language → English → key itself
5. .po parser handles: msgid "string", msgstr "string", multi-line continuation,
   # comments, escaped quotes (\"), empty msgstr (skip entry)

Create locale/en/firestaff.po with ~80 strings for all M12 UI:
  menu.title, menu.settings, menu.credits, settings.language, settings.volume,
  settings.graphics, settings.controls, game.options.cheats, game.options.speed, etc.

Create locale/sv/firestaff.po with Swedish translations of all keys.

Create firestaff_m12_i18n_probe.c with 8 invariants as specified in the plan.
```

---

## §6  Risks and Deferrals

| Risk | Severity | Mitigation |
|------|----------|------------|
| Bug database 201 entries is labor-intensive | Medium | Start with 20 representative entries; expand incrementally |
| .po parsing edge cases | Low | Test with malformed files; keep parser simple |
| Swedish translations need Daniel's review | Low | Ship draft, iterate |
| Mouse hit testing complex for nested menus | Low | Implement for top-level first |

**Deferred from original M12 plan:**
- Declarative TOML layout engine → not needed; hardcoded C layouts work fine and are simpler for Codex
- Full box-art PNG pipeline (stb_image) → built-in card art suffices
- Gamepad mapping → far future
- RTL support → far future
- Schema versioning → add when needed
- Replay playback UI → M13
- DM2 integration → M13

---

## §7  Verification Strategy

Same pattern as M11:
1. Shell script compiles probe + dependencies
2. Runs binary, captures log
3. Checks `# summary: N/N invariants passed`
4. Exit 1 on any failure

All new probes follow naming: `firestaff_m12_*_probe.c`
Build scripts: `run_firestaff_m12_*_probe.sh`
Output: `verification-m12/*/`

---

## §8  Dependency on M11

M12 slices are independent of M11 remaining slices (audio, CMake, CI). The only dependency is:
- M12 Slice 4 (Wire i18n into game view) depends on `m11_game_view.c` being stable → it already is.
- M12 uses M11's `render_sdl_m11` for framebuffer rendering → already stable.
- M12 probes compile against M11 sources → already verified.

M12 slices can proceed in parallel with M11 remaining work.

---

*End of M12 Remaining Plan. Version 2.0, 2026-04-21.*
