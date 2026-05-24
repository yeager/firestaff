# TODO.md GAP Audit — DM1 V1 Sections
Generated: 2026-05-24

## GAP Items Found in DM1 V1 Sections

---

### GAP 1: C25 Lord Order + C26 Grey Lord creature behavior

**Location:** TODO.md → DM1 V1 → Creature System  
**Files:** `dm1_v1_creature_ai_behavior_pc34_compat.c`  
**Header:** `dm1_v1_creature_ai_behavior_pc34_compat.h` (no DM1_CREATURE_TYPE_LORD_ORDER/GREY_LORD constants)

**Evidence:**
- TODO.md: default FIREBALL handles them — acceptable for normal play
- BUG0_13 analysis in commit bb6ed91d confirms unreachable in original dungeons
- Source route is by-accident not by-design
- No champion stat source-lock needed (not in Hall of Champions)

**Severity:** Low — FIREBALL fallback correct, not reachable in canonical dungeons

---

### GAP 2: C01-C24 Hall of Champions stats (stub values)

**Location:** TODO.md → DM1 V1 → Champion System  
**Evidence:** manifest at `data/firestaff-hall-champions/manifest.json` (commit 00244f3f)  
ReDMCSB G0243 is source reference; corrections not yet committed.

- 5 flag bugs: C12 Black Flame, C20 Materializer, C14 Couatl, C15 Vexirk, C21 Water Elemental, C24 Lord Chaos
- TIER_FULL champions C10/C11/C13 need stat corrections

**Severity:** Medium — affects panel display for all 24 champions; all playable with stub stats

---

### GAP 3: portrait sensorData ordinal indexing (m11_game_view.c)

**Location:** `src/m11/m11_game_view.c` line ~8995  
- Stores raw ordinal+1 without -1 correction for champion portrait sheet indexing
- Blocked on missing external artifacts (pass449/pass450 framebuffer evidence)
- Commit 62411518 clarified comment but fix not applied

**Severity:** Low — latent, no test failure reported

---

## Summary

| GAP | Area | Evidence |
|-----|------|----------|
| C25/C26 Lords | Creature | bb6ed91d BUG0_13 analysis, FIREBALL fallback |
| C01-C24 stats | Champion | 00244f3f audit done, corrections pending |
| portrait sensorData | Champion | 62411518 comment added, fix pending |
| V2.0 | Roadmap | Phase 1 not started, not a code gap |

**No urgent code gaps.** C01-C24 stat corrections are most impactful remaining work.
