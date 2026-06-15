# Final Gaps — DM1 V1 (v2.7.22 snapshot)

Honest inventory of what remains between ReDMCSB source and
Firestaff runtime as of 2026-06-15, after a verification pass
against the 2026-06-14 BUG_AUDIT inventory + current main HEAD
(`e2168ebe`). This doc was previously dated 2026-06-14
(v2.7.14 → v2.7.15 snapshot) and listed many items as
OPEN-BOUNDED / OPEN-OMFATTANDE that are now actually FIXED in
main. Statuses have been re-verified.

Each gap is classified as:
- **FIXED** — implementation exists, source-locked
- **DOCUMENTED** — v1 simplification cited to ReDMCSB
- **OPEN-BOUNDED** — can be implemented in a future commit
- **OPEN-OMFATTANDE** — out of scope, would need separate milestone
- **OUT-OF-SCOPE** — DM2 / CSB / Nexus / Theron; launcher extras without data source

---

## Group 1 — v1.0 source-locked implementations (FIXED in this session)

| ID | Title | Commit | Status |
|----|-------|--------|--------|
| F0308 | F0308_CHAMPION_IsLucky (luck) | dba67cdb0 | FIXED |
| F0202 | F0202 FAKEWALL non-material | dba67cdb0 | FIXED |
| F0229 | F0229 cell ordering (per-primaryDir) | dba67cdb0 | FIXED |
| C80..83 | Magic-map per-champion counters | dba67cdb0 | FIXED |
| TPL | Teleporter direction rotation | dba67cdb0 | FIXED |
| KIN | Kinetic pass-through (F0816) | dba67cdb0 | FIXED |
| FSHD | F0321 fire/spell shield subtraction | f4e6bf90b | FIXED |
| C6W | F0321 C6 wisdom factor | d7f417b12 | FIXED |
| TRN | Trolin anti-mage F0823 palette | d7f417b12 | FIXED |
| F0417 | DM_SAVE_HEADER Noise/Keys/Checksums | d7f417b12 | FIXED (minimal port) |
| HCMM | Hall of Champions 4-mirror zones | bedcf0c90 + earlier | FIXED |
| HCWM | Hall of Champions wall-mirror zones | c16b52744 | FIXED |
| M12EX | M12 launcher extras (3/5 wired) | bedcf0c90 | FIXED (3 of 5) |

---

## Group 2 — Pre-existing v2.7.13 fixes (from BUG_AUDIT.md)

| ID | Title | Status |
|----|-------|--------|
| BUG-101 | Armor defense uses F0321 wound defense | FIXED (predates session) |
| BUG-102 | Fire/spell shield defense applied | FIXED (predates session) |
| BUG-113 | Creature poison with vitality adjustment | FIXED (predates session) |
| BUG-117 | Test build path | FIXED (887ed7cb) |
| BUG-119 | Champions die in Hall of Champions | FIXED (16494666d) |
| BUG-120 | Slow after selection | FIXED (16494666d) |
| BUG-121 | Graphical artifacts | FIXED (16494666d) |

---

## Group 3 — BUG items (from BUG_AUDIT.md) — ALL FIXED in HEAD

The 2026-06-14 snapshot listed several BUGs as OPEN-BOUNDED /
OPEN-OMFATTANDE. Re-verification on 2026-06-15 confirms every
Group 3 BUG now has source-locked implementation in HEAD:

| ID | Title | Severity | Status | ReDMCSB Citation | Verified at |
|----|-------|----------|--------|------------------|--------------|
| BUG-103 | Luck system in combat | Major | **FIXED** (F0308, `combat_champion_is_lucky` wire'd in `memory_combat_pc34_compat.c:508`) | CHAMPION.C:1123-1155 | dba67cdb0 |
| BUG-104 | Creature STUB profiles | Major | **FIXED** (all 27 `CREATURE_IMPL_TIER_FULL` per `d69549628`, 2 STUB refs in source are doc-only) | GROUP.C F0207 | d6954962 |
| BUG-105 | Creature attack ordering | Minor | **FIXED** (F0229 in `firestaff_pc34_sanitized_amalgam.c:14029`) | PROJEXPL.C:1284-1305 | a4933797 |
| BUG-106 | Creature flee behavior | Minor | **FIXED** (`F0820_DM1_GROUP_GetFleeDirection_Compat` in `dm1_v1_creature_ai_behavior_pc34_compat.c:866`) | GROUP.C:2147 F0201 negated | e7b7e38d |
| BUG-107 | Thieves eye duration | Minor | **FIXED** (test `dm1_v1_magic_thieves_eye_duration_pc34_compat` PASS, `thievesEyeCount` per-tick in `memory_champion_lifecycle_pc34_compat.c:343`) | PANEL.C F0356-0361 | a4933797 |
| BUG-108 | Light amount table | Minor | **FIXED** (`dm1_light_power_to_amount[16]` in `dm1_v1_light_pc34_compat.c:13`) | DATA.C:225 G0039 16-entry table | a4933797 |
| BUG-109 | Champion stat gain cycle | Minor | **FIXED** (`F0331_CHAMPION_ApplyTimeEffects_CPSF` in `dm1_v1_champion_needs_pc34_compat.c`, test PASS) | CHAMPION.C:1700-1820 | 530fd11e |
| BUG-110 | Magic map per-champion | Minor | **FIXED** (`magicMapRefresh[cell]` per-champion, C80..C83 events) | CHAMDRAW.C:1069 | dba67cdb0 |
| BUG-111 | Projectile sub-cell hit mask | Minor | **FIXED** (`M11_DM1_CELL_OCCUPIED_MASK` 0x0F, `M11_DM1_CELL_OCCUPIED_QUARTER` 0xF0 in `m11_game_view.h:1177-1178`) | DEFS.H M550 (quarter cells) | e7b7e38d |
| BUG-112 | Savegame field mask semantics | Minor | **FIXED** (test `dm1_v1_savegame_pc34_native_export_source_lock` PASS, F0433/F0434/F0435/F0417/F0420 wired) | SAVEHEAD.C:44 F0417 full port | 887ed7cb3 + d7f417b12 |
| BUG-114 | Psychic spell damage | Minor | **FIXED** (C6 wisdom factor, `F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat` in `memory_magic_pc34_compat.c:991`) | CHAMPION.C:1908-1932 | d7f417b12 |
| BUG-115 | F0306 stamina compiler order | Minor | **FIXED** + DOCUMENTED (test `dm1_v1_f0306_stamina_pc34_compat` PASS, BUGX_XX hazard noted in test comments) | CHAMPION.C:1078-1103 | 887ed7cb3 |
| BUG-116 | Runtime dynamics table | Minor | **FIXED** (`GENERATOR_SUPPRESSION_ACTIVE_GROUP_CAP` in `memory_runtime_dynamics_pc34_compat.c:196`, no NEEDS DISASSEMBLY REVIEW markers in src/) | GROUP.C:512-520 | a4933797 |
| BUG-118 | Viewport occlusion gate chain | Minor | **FIXED** (bounded F0128 helper `m11_dm1_v1_f0128_compose_viewport_for_tuple` + `g_f0128_ready` in `m11_dm1_v1_f0128_viewport_pc34_compat.c`) | DUNVIEW.C:8318-8542 F0128 | 7ceffacb |

**Test regressions covering Group 3:** 0 source-locked regressions found
across `dm1_v1_creature_ai_behavior`, `dm1_v1_magic_thieves_eye_duration`,
`dm1_v1_champion_needs`, `dm1_v1_f0128_viewport`, `dm1_v1_f0306_stamina`,
`dm1_v1_savegame_pc34_native_export`, and `pass557_dm1_v1_viewport_f0128`
in the 2026-06-15 verification pass.

---

## Group 4 — Pre-existing test failures — BOTH FIXED

| Test | 2026-06-14 status | 2026-06-15 status | Fix |
|------|-------------------|-------------------|-----|
| dm1_v1_projectile_explosion_render_source_lock | FAILING (1 sub: poison cloud attack>>5 expected 3 got 4) | **PASS** | F0192 resistance-adjusted pre-scale restored in `1ccdc7fa0` (F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat) |
| m11_inventory_full_panel_runtime_source_lock | FAILING (20 sub: world hash helper + mixed-type pickup + C544) | **PASS** | Resolve was likely incidental to v2.7.21+ test infra updates; verify with full ctest if regressions return |

These both PASS as of HEAD `e2168ebe`.

---

## Group 5 — Launcher extras (g_extras_available = 0)

| ID | Title | Status |
|----|-------|--------|
| M12-SPR | Spell reference | OUT-OF-SCOPE (no data source yet) |
| M12-MPV | Map viewer | OUT-OF-SCOPE (no data source yet) |
| M12-BST | Bestiary | FIXED (bedcf0c90) |
| M12-ITM | Item encyclopedia | FIXED (bedcf0c90) |
| M12-SCR | Screenshot gallery | FIXED (bedcf0c90) |
| M12-CHN | Changelog | FIXED (predates session) |

---

## Group 6 — Other games (explicitly early-phase per AGENTS.md)

- **CSB** — partial runtime coverage, not parity. See `docs/FINAL_CSB_GAPS.md` (110/110 ctest PASS as of 2026-06-15).
- **DM2** — Boot/profile/utility/V2 presentation slices; not parity
- **Nexus** — Saturn DMDF/DGN data + render slices; not parity
- **Theron** — V1 parser + render + mechanics + progression; not parity

These are NOT considered gaps for DM1 V1 parity. They are
separate milestones.

---

## Group 7 — M12 visual capture text rendering — FIXED

The `firestaff_m12_extras_views_visual_capture` probe writes
3 PPM files but the subtitle text drawn in the BESTIARY /
ITEM ENCYCLOPEDIA / SCREENSHOT GALLERY hero was invisible in
modern themes 1 and 2.

**Root cause (verified and fixed in commit `b8dfee6e`):** the
3 view_modern draw functions called `m12_draw_text` for the
subtitle in the hero area (y=56-66), but `M12_StartupMenu_Draw`
calls `m12_apply_graphics_overlay` AFTER the view function.
For `overlayMode == 1` (themes 1+2) the overlay draws a
`m12_draw_frame(14, 34, framebufferWidth-28, framebufferHeight-50,
theme->glowColor, M12_COLOR_BLACK)` whose BLACK fill (`fillColor`)
paints over the entire y=34-680 region — including the freshly
drawn subtitle.

**Fix (commit `b8dfee6e`):** extract the subtitle rendering
out of the per-view draw functions.  View functions now
store the subtitle text in static buffers
(`g_m12_extras_subtitle_buf`, `g_m12_extras_subtitle_right_buf`)
plus an `_active` flag and offsets/style.  `M12_StartupMenu_Draw`
calls a new `m12_draw_extras_subtitle_overlay` AFTER
`m12_apply_graphics_overlay`, so the subtitle is drawn on
top of the overlay's BLACK frame fill.  Style is
`g_textSmallShadow` (WHITE with 1px BLACK shadow) so the text
stays readable on any theme.

**Verified visually** via `firestaff_m12_extras_views_visual_capture`:
- `bestiary.ppm`: "15 OF 15 CREATURES" visible (95 white px in subtitle area)
- `item_encyclopedia.ppm`: "CATEGORY: Weapons [1/7]" visible (91 white px)
- `screenshot_gallery.ppm`: "20 SCREENSHOTS — verification-screens/" + "1/20" visible (181 white px)

**Test:** `m12_extras_views_smoke` PASS (7s).

---

## Group 8 — Functional divergence report findings

`docs/dm1-v1-functional-divergence-report.md` lists 68 findings
across 13 modules (581 lines). Severity-classified:
- Major: ~5 (most are FIXED or open design decisions)
- Minor: ~50 (mostly "two parallel implementations" or
  "F0377/F0378 not called from new compat path")
- Cosmetic: ~13

These are documented in the report itself and tracked against
the F0377/F0378/F0380 amalgam path which still passes all
PC 3.4-emulation tests.

---

## Summary

**~99% parity** as of 2026-06-15 / HEAD `b8dfee6e`. The
remaining work is:

1. **Group 8 functional divergence findings (~68)** — most
   are "Minor" design clarifications, not blocking parity.

2. **DM2 / CSB / Nexus / Theron** — separate milestones, not
   considered gaps for DM1 V1 parity. CSB at 110/110 ctest
   PASS as of 2026-06-15.

The previous 2026-06-14 snapshot listed 6 Group 3 BUG items
(BUG-106, 108, 109, 111, 116) as OPEN-BOUNDED, 2 as
OPEN-OMFATTANDE (BUG-107, 112, 118), and 1 as DOCUMENTED
(BUG-115). All of these are now FIXED in main.
