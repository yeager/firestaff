# Final Gaps — v2.7.14 → v2.7.15

Honest inventory of what remains between ReDMCSB source and
Firestaff runtime as of 2026-06-14, after this session's
documentation + implementation pass (commits a49337973, dba67cdb0,
f4e6bf90b, c16b52744, d7f417b12).

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
| BUG-117 | Test build path | PARTIALLY FIXED |
| BUG-119 | Champions die in Hall of Champions | FIXED (16494666d) |
| BUG-120 | Slow after selection | FIXED (16494666d) |
| BUG-121 | Graphical artifacts | FIXED (16494666d) |

---

## Group 3 — BUG items remaining (from BUG_AUDIT.md)

| ID | Title | Severity | Status | ReDMCSB Citation |
|----|-------|----------|--------|------------------|
| BUG-103 | Luck system in combat | Major | FIXED (F0308) | CHAMPION.C:1123-1155 |
| BUG-104 | Creature STUB profiles | Major | FIXED (all 27 FULL per d69549628) | GROUP.C F0207 |
| BUG-105 | Creature attack ordering | Minor | FIXED (F0229) | PROJEXPL.C:1284-1305 |
| BUG-106 | Creature flee behavior | Minor | **OPEN-BOUNDED** | GROUP.C:2147 F0201 negated |
| BUG-107 | Thieves eye duration | Minor | OPEN-OMFATTANDE | PANEL.C F0356-0361 |
| BUG-108 | Light amount table | Minor | **OPEN-BOUNDED** | DATA.C:225 G0039 16-entry table |
| BUG-109 | Champion stat gain cycle | Minor | **OPEN-BOUNDED** | CHAMPION.C:1700-1820 |
| BUG-110 | Magic map per-champion | Minor | FIXED (C80..C83) | CHAMDRAW.C:1069 |
| BUG-111 | Projectile sub-cell hit mask | Minor | **OPEN-BOUNDED** | DEFS.H M550 (quarter cells) |
| BUG-112 | Savegame field mask semantics | Minor | OPEN-OMFATTANDE | SAVEHEAD.C:44 F0417 full port |
| BUG-114 | Psychic spell damage | Minor | FIXED (C6 wisdom) | CHAMPION.C:1908-1932 |
| BUG-115 | F0306 stamina compiler order | Minor | DOCUMENTED | CHAMPION.C:1078-1103 |
| BUG-116 | Runtime dynamics table | Minor | **OPEN-BOUNDED** | GROUP.C:512-520 |
| BUG-118 | Viewport occlusion gate chain | Minor | OPEN-OMFATTANDE | DUNVIEW.C:8318-8542 F0128 |

---

## Group 4 — Pre-existing test failures (2 tests, 21 sub-failures)

| Test | Status | Failures |
|------|--------|----------|
| dm1_v1_projectile_explosion_render_source_lock | FAILING | 1: poison cloud attack>>5 expected 3 got 4 |
| m11_inventory_full_panel_runtime_source_lock | FAILING | 20: world hash helper + mixed-type pickup + C544 |

These have been failing since before this session. Predate
commits a49337973 and earlier. The pre-existing test failure
analysis is in `parity-evidence/`.

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

- **CSB** — partial runtime coverage, not parity
- **DM2** — Boot/profile/utility/V2 presentation slices; not parity
- **Nexus** — Saturn DMDF/DGN data + render slices; not parity
- **Theron** — V1 parser + render + mechanics + progression; not parity

These are NOT considered gaps for DM1 V1 parity. They are
separate milestones.

---

## Group 7 — M12 visual capture text rendering (BUG)

The `firestaff_m12_extras_views_visual_capture` probe writes
3 PPM files but the rendered framebuffer contains only background
colors (bandColor + frame) — NO white pixels from m12_draw_text.

The 4 distinct non-zero bytes in the smoke probe output are
just the background palette indices. m12_draw_text is being
called but produces 0 visible pixels.

**Status:** OPEN-BOUNDED — investigate m12_draw_text + brand
logo overlap (logoX=58, logoY=20, size 448x224 covers text
at margin+14, margin+18).

---

## Group 8 — Functional divergence report findings

`docs/dm1-v1-functional-divergence-report.md` lists 68 findings
across 13 modules. Severity-classified:
- Major: ~5 (most are FIXED or open design decisions)
- Minor: ~50 (mostly "two parallel implementations" or
  "F0377/F0378 not called from new compat path")
- Cosmetic: ~13

These are documented in the report itself and tracked against
the F0377/F0378/F0380 amalgam path which still passes all
PC 3.4-emulation tests.

---

## Summary

**NOT 100% parity.** The remaining work is:

1. **Group 3 BUG items (6 OPEN-BOUNDED)** — implementable in
   a focused session: BUG-106 flee direction, BUG-108 light
   table, BUG-111 sub-cell hit mask, BUG-116 runtime dynamics
   adjacency, BUG-109 stat gain cycle. All have bounded
   patches.

2. **Group 4 pre-existing tests (2 tests, 21 sub-failures)**
   — real failures in inventory world hash and poison cloud
   attack scaling. Need investigation.

3. **Group 7 M12 text rendering bug** — visual capture probe
   shows text not being drawn. Real bug.

4. **Group 8 functional divergence findings (~68)** — most are
   "Minor" design clarifications, not blocking parity.

5. **DM2 / CSB / Nexus / Theron** — separate milestones, not
   considered gaps for DM1 V1 parity.
