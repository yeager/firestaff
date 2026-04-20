# Firestaff, Bug/Change Toggle Spec

**Status:** Spec (post-M10 implementation)
**Created:** 2026-04-20
**Canonical source:** http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm
**Primary database:** Fontanel's `BUGX_YY` and `CHANGEX_YY_CATEGORY` tags in the ReDMCSB codebase

---

## 1. Overview

Firestaff lets the player choose which historical version of Dungeon Master / Chaos Strikes Back they want to play, all the way down to individual bugs and changes. Fontanel's documentation already gives us 93 bugs and 108 changes spread across 9 game versions. Firestaff should respect that taxonomy and let players mix and match deliberately.

---

## 2. Data source and structure

### 2.1 Fontanel taxonomy

**Version codes (0-8):**

| Code | Version |
|------|---------|
| 0 | DM Atari ST 1.0a EN |
| 1 | DM Atari ST 1.0b EN |
| 2 | DM Atari ST 1.1 EN |
| 3 | DM Atari ST 1.2 EN |
| 4 | DM Atari ST 1.2 GE |
| 5 | DM Atari ST 1.3a FR |
| 6 | DM Atari ST 1.3b FR |
| 7 | CSB Atari ST 2.0 EN |
| 8 | CSB Atari ST 2.1 EN |

**Note:** PC 3.4, our baseline, is not numbered in the Atari timeline. We have to determine ourselves which Atari-era fixes are effectively present in PC 3.4 and which are not.

**Bug ID:** `BUGX_YY`, where X is the version that introduced the bug and YY is a running number.

**Change ID:** `CHANGEX_YY_CATEGORY`, where X is the version, YY is a running number, and CATEGORY ∈ {FIX, OPTIMIZATION, LOCALIZATION, IMPROVEMENT}.

### 2.2 Totals (per Fontanel)

- **Bugs: 93**
  - Introduced in 1.0a EN: 89
  - Introduced in 1.1 EN: 1
  - Introduced in 1.3a FR: 1
  - Introduced in CSB 2.0 EN: 2
- **Changes: 108**
  - 1.0b EN: 3
  - 1.1 EN: 21
  - 1.2 EN: 21
  - 1.2 GE: 1
  - 1.3a FR: 7
  - 1.3b FR: 1
  - CSB 2.0 EN: 40
  - CSB 2.1 EN: 14

Total: **201 individual toggles**.

### 2.3 Data model (JSON / YAML)

```json
{
  "id": "BUG0_02",
  "type": "bug",
  "symptom": "After 850-1000 hours the game hangs",
  "cause": "24-bit event-time overflow in timeline",
  "affected_versions": [0, 1, 2, 3, 4, 5, 6, 7, 8],
  "affected_files": ["GAMELOOP.C"],
  "resolution": "none",
  "fixed_by_change": null,
  "category": null,
  "default_state": "original",
  "requires_fontanel_review": false,
  "our_implementation_status": "implemented | deferred | na",
  "firestaff_flag_id": 2,
  "applicable_games": ["DM1", "CSB"]
}
```

The full database is roughly 201 entries × 11 fields. Initial import comes from dmweb.free.fr, then we curate and version it inside the project.

---

## 3. UI choices

### 3.1 Version presets (one-click)

Quick presets for common play styles:

| Preset | Description | Beginner default? |
|--------|-------------|-------------------|
| **DM 1.0a EN (purist)** | Original 1987 build. All 89 launch-era bugs enabled. No later changes. | ❌ |
| **DM 1.0b EN** | Original plus the first 3 fixes. | ❌ |
| **DM 1.1 EN** | 1988 build, 21 changes plus BUG2_00 introduced. | ❌ |
| **DM 1.2 EN (latest EN)** | Final English Atari DM. | ❌ |
| **DM 1.3b FR (latest DM)** | All Atari DM fixes through 1.3b, French-localised. | ❌ |
| **CSB 2.1 EN (latest CSB)** | All Atari CSB fixes through 2.1. | ❌ |
| **PC 3.4 baseline** | What Fontanel's WIP code actually implements. **Recommended default.** | ✅ |
| **Modern** | All FIX changes plus a curated set of IMPROVEMENT changes. | ❌ |
| **Custom** | User-defined mix. | — |

Implementation-wise, each preset should just be a JSON file containing `{id, state}` pairs. Pick preset, populate toggles, then let the player save a modified combination as Custom.

### 3.2 Custom mix (advanced)

**Full control:** individual toggles for every bug and change, roughly 201 total.

**UI grouping:**
- Bugs (93)
  - grouped by introducing version
- Changes (108)
  - grouped by category:
    - FIX
    - OPTIMIZATION
    - LOCALIZATION
    - IMPROVEMENT
  - optionally also by version

**Per-entry UI:**
- ID + one-line symptom
- Full cause / explanation
- Affected versions
- Resolution
- Affected files (for archaeology-minded users)
- Impact category: Silent / Visual / Gameplay / Breaking
- Recommended state: original or fixed

**Search filters:**
- show only bugs affecting the selected version
- show only changes not already included in the selected version
- free-text symptom search
- file-name search

### 3.3 Changes must be toggleable too, not just bugs

This matters because Fontanel's "changes" are not all bug fixes.

Examples:
- `CHANGE3_14_IMPROVEMENT`, maybe a gameplay tweak a purist does not want
- `CHANGE4_00_LOCALIZATION`, maybe irrelevant for an English player
- `CHANGE7_01_FIX`, fixes BUG0_03 and should default ON
- `CHANGE7_14_FIX`, no visible gameplay effect but still changes the binary

**Default UI behaviour:**
- FIX changes: ON by default, but still reversible
- IMPROVEMENT changes: ON only when the chosen preset includes them
- LOCALIZATION changes: ON only when the selected language matches
- OPTIMIZATION changes: ON by default unless they change semantics

### 3.4 Per-game gating

DM1-only bugs should not clutter the UI when the player selects CSB or DM2, and vice versa.

Each entry therefore carries `applicable_games`, for example `["DM1", "CSB"]`.

**Rough mapping:**
- `BUG0_*` affecting only versions 0-6 → DM1 only
- `BUG0_*` affecting 7-8 too → DM1 + CSB
- `BUG7_*` and `CHANGE7_*` → CSB only
- DM2 gets its own list later

**UX rule:** switching the selected game in the startup menu filters the toggle list automatically. Custom selections should be stored per game.

---

## 4. Community value

### 4.1 Publish the database

The bug/change database (`firestaff-bugs.json` or YAML) should ideally be publishable as:
- a separate GitHub repo or exportable subproject
- preservation-friendly licensed data
- schema-versioned so other tools can depend on it

### 4.2 Reuse by other projects

Potential downstream consumers:
- ScummVM
- Dungeon Master Java / dmjava
- Return to Chaos style projects
- custom dungeon editors
- preservation / analysis tools

### 4.3 Credit

Every exported data file should say clearly:

```json
{
  "data_source": "Based on ReDMCSB documentation by Christophe Fontanel",
  "data_source_url": "http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm",
  "scraped_at": "2026-04-20",
  "contributors": []
}
```

Fontanel gets the credit. Firestaff's value-add is the structured data, UI mapping, and implementation hooks.

### 4.4 Future expansion

- video or GIF demos per bug
- replay files that reproduce specific bugs
- community-submitted bugs Fontanel never catalogued
- PC 3.4-specific bug entries absent from the Atari timeline

---

## 5. Technical implementation (post-M10)

### 5.1 Firestaff side

**Module:** `bug_flags_compat`
- bit-mask with at least 256 bits
- per-game masks (DM1 / CSB / DM2)
- serialised into config and saves
- every behaviour site can branch on the active flag

```c
if (BUG_ACTIVE(game, BUG0_02)) {
    /* Original buggy behaviour, 24-bit overflow */
    scheduled_time = (game_time + delay) & 0xFFFFFF;
} else {
    /* Fixed behaviour, full 32-bit time */
    scheduled_time = game_time + delay;
}
```

### 5.2 Save-file integration

Save files must record the profile used to create them:
- extend the save blob with `ProfileHash_Compat`
- on load, compare current profile vs saved profile
- if mismatched, show a warning popup
- optional convenience path: snap runtime toggles to the save's profile automatically

### 5.3 Replay integration

Replay determinism requires the exact same flag set as recording:
- `TickStreamRecord_Compat` includes profile hash
- replay verifier rejects mismatched profiles
- same flags must lead to the same world hash after N ticks

### 5.4 Startup-menu flow

1. Player opens startup menu
2. Selects game (DM1 / CSB / DM2)
3. Chooses a version preset
4. Default for new players is `PC 3.4 baseline`
5. Advanced button opens the full custom-mix dialog
6. Player may save their custom preset
7. When starting a new game, the chosen profile is frozen into the save

---

## 6. Priority

### Must-have (V2)
- [ ] scrape dmweb.free.fr into a structured database of ~201 entries
- [ ] `bug_flags_compat` module
- [ ] 5+ version presets
- [ ] per-game gating
- [ ] save-file profile hash
- [ ] preset dropdown in the UI

### Should-have (V2.1)
- [ ] custom-mix UI
- [ ] search filters
- [ ] full tooltips / expanded explanations
- [ ] save-profile mismatch warning

### Nice-to-have (V3+)
- [ ] publish as a community repo
- [ ] video demos per bug
- [ ] replay files demonstrating bugs
- [ ] contribution workflow

---

## 7. Open questions

1. **What should be the default for a new player?** Recommendation: `PC 3.4 baseline`, because it is the most complete and testable starting point. Purists can still choose 1.0a EN.
2. **Should DM2 be included now or later?** Later. DM2 is outside the ReDMCSB bug/change canon and needs separate research.
3. **What licence should the bug database use?** Daniel decides.
4. **Should changes be toggleable in the default UI or only under Advanced?** Recommendation: show bugs by default, hide change-toggles behind Advanced.
5. **How should PC 3.4-specific bugs and changes be named if they are not in the Atari docs?** `BUGPC_YY` is the cleanest option unless Fontanel suggests a better continuation scheme.
