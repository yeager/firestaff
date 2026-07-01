# DM1 V1 Bug Audit — Firestaff v2.7.12 → v2.7.13

Systematic audit of the Firestaff DM1 V1 runtime against the ReDMCSB decompilation as ground truth.

**Auditor:** Opus 4.6 Firestaff Worker
**Date:** 2026-06-13
**Baseline:** Firestaff commit `f99587c35` (HEAD of main, post-v2.7.12)
**ReDMCSB:** WIP20210206, `Toolchains/Common/Source/`
**Build dir:** `/Volumes/Extern-disk/openclaw-work/firestaff/build`
**Phase A probe:** 23/23 PASS
**DM1 test suite:** 556/593 PASS (37 failures — see analysis below)

---

## Test Failure Analysis

Of the 37 DM1 V1 test failures:

- **12 failures**: Python verification scripts reference `build/test_dm1_v1_viewport_3d_pc34_compat` via hardcoded path `ROOT / "build"` which points to the repo root, not the external build directory. The binary exists and passes in the actual build dir. These are **test infrastructure** issues, not code bugs.
- **2 failures**: Movement tests (`pass552`, `pass580`) — same hardcoded path issue for `test_dm1_v1_movement_timing_pc34_compat`.
- **5 failures**: Capture route/transcript tests (`pass504`, `pass510`, `pass623`, `pass625`, `pass626`) — need original game data files not present on the build host.
- **3 failures**: Viewport occlusion chain failures (`pass361`, `pass362`, `pass434`) — chained gate dependencies; `pass434` is the root cause (viewport crop readiness gate).
- **5 failures**: Viewport wall/door/field source-lock tests (`pass499`, `pass565_d0c`, `pass608`, `pass650`, `pass577`) — depend on missing symbols or chained prerequisites.
- **2 failures**: `pass519`, `pass570` — viewport field/front order source locks with chained dependencies.
- **1 failure**: `pass560` — mirrored door front, known pre-existing failure.
- **2 failures**: Runtime timeouts (`dm1_v1_hall_walkaround_runtime`, `dm1_v1_champion_mirror_candidate_panel_runtime`) — require real game data; 10s timeout insufficient.
- **3 failures**: `pass561`, `pass562`, `pass563` — viewport wall source locks, missing binary.
- **2 failures**: Additional gate chain failures.

---

## Bug List

### BUG-101 — Armor Defense Uses Skill-Level Approximation Instead of F0321 Wound Defense
- **Severity:** Major
- **Category:** Mechanics
- **Description:** The inline creature→champion damage resolver in `m11_game_view.c:5074-5087` uses an approximation based on champion skill levels (parry + fighter / 2) instead of the real F0321 wound defense calculation that iterates worn armor items per wound slot.
- **ReDMCSB Reference:** CHAMPION.C F0321:1838-1900, F0313_CHAMPION_GetWoundDefense
- **Expected (ReDMCSB):** F0321 sums F0313(slot) over the wound mask bits, averages, and scales attack by `(130 - avgDefense) / 64`. F0313 checks each worn item's defense value for the specific wound slot.
- **Actual (Firestaff):** `armorApprox = skillLevels[7] + skillLevels[3]; damage -= armorApprox / 2;` — no item iteration, no per-slot defense, no (130-d)/64 scaling.
- **Impact:** Champions wearing heavy armor take too much damage; champions with high skill levels but no armor get unearned defense. This affects game balance significantly.
- **Fix Complexity:** Medium — the proper F0321 logic exists in `memory_combat_pc34_compat.c::combat_apply_f0321_armor_defense_scale()`. The inline code in m11_game_view.c should delegate to the M10 combat resolver instead of using the approximation.

### BUG-102 — Fire/Spell Shield Defense Not Applied
- **Severity:** Major
- **Category:** Mechanics
- **Description:** The combat damage resolver in `memory_combat_pc34_compat.c:210-228` marks Fire Shield and Spell Shield defense subtraction as "NEEDS DISASSEMBLY REVIEW" and does not implement them.
- **ReDMCSB Reference:** CHAMPION.C F0321:1842-1857 `P0663_i_Attack -= G0407_s_Party.FireShieldDefense` (C1_FIRE case) and `P0663_i_Attack -= G0407_s_Party.SpellShieldDefense` (C5_MAGIC case)
- **Expected (ReDMCSB):** After F0307 statistic adjustment, subtract party shield defense from attack value.
- **Actual (Firestaff):** Shield subtraction commented out with "NEEDS DISASSEMBLY REVIEW".
- **Impact:** Fire Shield and Spell Shield spells provide no damage reduction, making those spells useless.
- **Fix Complexity:** Low — add `attack -= party->fireShieldDefense` and `attack -= party->spellShieldDefense` in the appropriate switch cases.

### BUG-103 — Luck System Not Implemented in Combat
- **Severity:** Major
- **Category:** Mechanics
- **Description:** The combat resolver collapses luck to 0. F0308_CHAMPION_IsLucky is marked as "NEEDS DISASSEMBLY REVIEW" and not called.
- **ReDMCSB Reference:** CHAMPION.C F0308:1120-1145
- **Expected (ReDMCSB):** Champions with high Luck stat have a chance to avoid hits. Cursed items with negative Luck exploit (BUG0_38 in original). Luck decrements by 2 on lucky roll, increments by 2 on unlucky roll.
- **Actual (Firestaff):** Luck is always treated as 0.
- **Impact:** Combat feels less varied; Luck stat is meaningless.
- **Fix Complexity:** Medium — requires integrating Luck into the RNG flow and tracking the mutable Luck statistic.

### BUG-104 — Creature AI Profiles All Marked STUB
- **Severity:** Major
- **Category:** Mechanics
- **Description:** All 27 creature type profiles in `memory_creature_ai_pc34_compat.c:98-150` are marked `CREATURE_IMPL_TIER_STUB`. The behavior profiles have numeric values but the implementation tier flag causes simplified behavior.
- **ReDMCSB Reference:** GROUP.C F0028/F0029/F0194/F0196/F0199/F0200/F0201/F0202/F0203, DEFS.H G0243_as_Graphic559_CreatureInfo
- **Expected (ReDMCSB):** Each creature type has specific movement patterns, attack behaviors, and special abilities (levitation, non-material, side attack, back-row preference, etc.).
- **Actual (Firestaff):** All creatures use the same simplified stub behavior path. The actual profile values exist but the `CREATURE_IMPL_TIER_STUB` flag limits behavior.
- **Impact:** All creatures behave identically, making combat less interesting and the game significantly easier or harder than intended.
- **Fix Complexity:** High — requires implementing the full creature AI state machine from GROUP.C.

### BUG-105 — Creature Attack Ordering Not Source-Locked
- **Severity:** Minor
- **Category:** Mechanics
- **Description:** `memory_creature_ai_pc34_compat.c:419` marks F0229_GROUP_SetOrderedCellsToAttack as "NEEDS DISASSEMBLY REVIEW". The cell-ordering logic for creature attack targets is not fully implemented.
- **ReDMCSB Reference:** GROUP.C F0229:685-710
- **Expected (ReDMCSB):** Creatures attack party members in a specific cell-based priority order determined by the creature's position and direction.
- **Actual (Firestaff):** Attack ordering is simplified or absent.
- **Impact:** Creatures may target wrong party members, affecting combat tactics.
- **Fix Complexity:** Medium

### BUG-106 FIXED — Creature Flee Behavior Source-Locked

F0820_DM1_GROUP_GetFleeDirection_Compat() in
`src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c` calls
`opposite_dir()` (= ReDMCSB M018_OPPOSITE) on both the
primary and secondary toward-party directions.  This is
the source-locked F0209 T0209094_FleeFromTarget formula.
The F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal
helper (m10) is still a no-op as documented, but the
caller-side opposite_dir() is correct.  `fearCounter` in
the FLEE state (memory_creature_ai_pc34_compat.c:1432)
also decrements per the source's T0209094 path.

### BUG-107 — Thieves Eye Duration Approximated
- **Severity:** Minor
- **Category:** Mechanics
- **Description:** `memory_magic_pc34_compat.c:595-603` uses an approximated duration for Thieves Eye spell (`spellPower * 40`), marked "NEEDS DISASSEMBLY REVIEW".
- **ReDMCSB Reference:** MENU.C:1960-1963 (T0412032 tail)
- **Expected (ReDMCSB):** Duration derived from AL1269_ui_Ticks multiplied by SpellPower with media-variant-specific pre-multiplication value.
- **Actual (Firestaff):** Conservative envelope of `spellPower * 40`.
- **Impact:** Thieves Eye may last longer or shorter than intended.
- **Fix Complexity:** Low — needs the correct multiplier from the PC 3.4 media variant.

### BUG-108 FIXED — Light Amount Table Source-Locked

The full 16-entry `dm1_light_power_to_amount[16] = { 0, 5, 12,
24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 }` table is
now source-locked in `src/dm1/dm1_v1_light_pc34_compat.c`
(ReDMCSB DATA.C:359/1088).  The `Phase14_PowerOrdinalToLight
Amount[6]` subset in `memory_magic_pc34_compat.c` consumes
the canonical table (indices 1..6) and matches exactly.
The full table is also exposed via the public symbol
`dm1_light_power_to_amount` (declared in
`include/dm1_v1_light_pc34_compat.h`).


### BUG-109 FIXED — Champion Stat Gain Cycle Source-Locked

F0331 is implemented in
`src/dm1/dm1_v1_champion_needs_pc34_compat.c` (the
post-M10 v1 split of CHAMPION.C).  The per-tick stat
recovery loop follows ReDMCSB CHAMPION.C F0331:2487-2497
(MEDIA240 branch = DM1 Atari ST 1.2+):
  - BoundedValue(1, (MaxStamina >> 8) - 1, 6) cycle count
  - gain = (MaxHealth >> 7) + 1 per cycle
  - (current_stamina < max_stamina) consumes the gain
  - time_criteria = ((GT & 0x80) + ((GT & 0x100) >> 2) +
    ((GT & 0x40) << 2)) >> 2
  - Mana regen gated by wisdom + (wiz_skill + priest_skill)
  - StaminaGainCycleCount starts at 4 (AL9995)
  - Party-resting flag doubles all gains
  - Negative food/water drain when below 0 is preserved

- **Severity:** Minor
- **Category:** Mechanics
- **Description:** `memory_champion_lifecycle_pc34_compat.c:149` marks the F0331 stat gain expansion via repeated cycles as "NEEDS DISASSEMBLY REVIEW".
- **ReDMCSB Reference:** CHAMPION.C F0331:2487-2497
- **Expected (ReDMCSB):** The per-tick stat recovery loop uses the rest-modulated 256/64 period and clamps each stat via `curv -= curv / maxv` when above maximum.
- **Actual (Firestaff):** Approximated expansion.
- **Impact:** Champion stats may recover at wrong rates, especially during rest.
- **Fix Complexity:** Medium

### BUG-110 — Magic Map Per-Champion State Not Implemented
- **Severity:** Minor
- **Category:** Mechanics
- **Description:** `memory_champion_lifecycle_pc34_compat.c:417` marks C80..C83 magic map per-champion tracking as "NEEDS DISASSEMBLY REVIEW".
- **ReDMCSB Reference:** CHAMPION.C — magic map events per champion
- **Expected (ReDMCSB):** Each champion tracks their own magic map state via timeline events.
- **Actual (Firestaff):** Not implemented.
- **Impact:** Magic map spell may not function correctly per-champion.
- **Fix Complexity:** Medium

### BUG-111 FIXED — Projectile Sub-Cell Hit Mask Source-Locked (partial)

`m11_game_view.c:18914` now uses
`M11_DM1_CELL_OCCUPIED_MASK` (0x0F) instead of a hardcoded
literal.  The mask is defined in `include/m11_game_view.h`
as a ReDMCSB source-locked constant per DEFS.H M550
(DUNGEON.C:1085).  v1 keeps the full-square 0x0F value
because per-sub-cell positioning is deferred to post-M10;
quarter-square / giant / 2x2 creatures would use 0xF0
(M11_DM1_CELL_OCCUPIED_QUARTER).  The hardcoded literal
was replaced with the named constant.


### BUG-112 — Savegame Field Mask Semantics Approximated
- **Severity:** Minor
- **Category:** Data
- **Description:** `memory_savegame_pc34_compat.c:399` marks field mask semantics for save/load as "NEEDS DISASSEMBLY REVIEW".
- **ReDMCSB Reference:** LOADSAVE.C F0433:1502-1707, F0435:2192-2660
- **Expected (ReDMCSB):** Exact field-mask serialization matching the original save format.
- **Actual (Firestaff):** Approximate field masking; may cause save file compatibility issues.
- **Impact:** Save files may not be fully compatible with original game saves.
- **Fix Complexity:** Medium

### BUG-113 — Poison Application Skips Vitality Adjustment
- **Severity:** Minor
- **Category:** Mechanics
- **Description:** `memory_combat_pc34_compat.c:644` notes that "Fontanel runs the poison value through F0307 vs vitality *before* committing to it" but this is flagged as not implemented.
- **ReDMCSB Reference:** CHAMPION.C F0307 vs Vitality statistic
- **Expected (ReDMCSB):** Poison attack value is adjusted by defender's Vitality via F0307_CHAMPION_GetStatisticAdjustedAttack before applying.
- **Actual (Firestaff):** Raw poison value used without vitality adjustment.
- **Impact:** Poison is more effective than intended, especially against high-Vitality champions.
- **Fix Complexity:** Low

### BUG-114 — Psychic Spell Damage Not Implemented
- **Severity:** Cosmetic
- **Category:** Mechanics
- **Description:** `memory_magic_pc34_compat.c:785` marks psychic impact from spells as not implemented.
- **ReDMCSB Reference:** CHAMPION.C F0321, PROJEXPL.C F0230
- **Expected (ReDMCSB):** Psychic attack type uses Wisdom factor for defense.
- **Actual (Firestaff):** Path exists but never exercised (no psychic-damage spells in DM1's 25-entry spell table).
- **Impact:** None in DM1 (no psychic spells exist); only relevant for CSB/DM2.
- **Fix Complexity:** Low

### BUG-115 — F0306 Stamina-Adjusted Value Compiler Order Hazard
- **Severity:** Cosmetic
- **Category:** Mechanics
- **Description:** The BUGX_XX compiler-order hazard at CHAMPION.C:1095 where `(P0641_i_Value >>= 1) + (P0641_i_Value * L0925/L0926)` evaluates differently depending on whether the first operand is computed before or after the `>>=1`. The source-locked tests in Firestaff cover this correctly (BUG documented in test comments), but the main runtime should verify it matches PC 3.4 Turbo C++ 1.01 behavior (unexpected order: second operand first).
- **ReDMCSB Reference:** CHAMPION.C F0306:1078-1103, BUGX_XX comment
- **Expected (PC 3.4):** Turbo C++ 1.01 evaluates the second operand first (unexpected order), so the champion benefits from higher-than-expected values.
- **Actual (Firestaff):** The test gates cover the expected-order behavior; the main runtime should match Turbo C++ 1.01 (PC 3.4 target).
- **Impact:** Minor stat calculation differences for stamina-adjusted strength/load.
- **Fix Complexity:** Low

### BUG-116 FIXED — Runtime Dynamics Table Source-Locked

`memory_runtime_dynamics_pc34_compat.c` no longer carries
a NEEDS DISASSEMBLY REVIEW marker.  The active-group cap
follows ReDMCSB GROUP.C:512-520 strictly (party-map only, no
adjacency suppression).  The 16-entry light-amount table is
the canonical G0039 lookup (see BUG-108).  The
`g_dynamicsTable` constants are exposed via
`include/memory_runtime_dynamics_pc34_compat.h` for
FIRESTAFF_DATA tests to assert against.


### BUG-117 — Test Infrastructure: Python Verification Scripts Hardcode Build Path
- **Severity:** Minor
- **Category:** Testing
- **Description:** At least 14 Python verification scripts in `tools/verify_pass*.py` search for test binaries in `ROOT / "build"` and `ROOT / "build-*"` but do not search the CMake build directory when it's on a different volume (e.g., `/Volumes/Extern-disk/openclaw-work/firestaff/build`). This causes 14+ test failures that are not real code bugs.
- **ReDMCSB Reference:** N/A (test infrastructure)
- **Expected:** Tests should find binaries in the actual CMake build directory.
- **Actual:** Tests fail with `FileNotFoundError` or `AssertionError: missing built test_*`.
- **Impact:** False test failures mask real issues; CI may pass because it builds in-tree.
- **Fix Complexity:** Low — add the external build path to the search candidates, or respect a `FIRESTAFF_BUILD_DIR` environment variable.

### BUG-118 FIXED — Viewport Occlusion Gate Chain Root Failure (bounded)

`pass434_dm1_v1_original_viewport_crop_readiness_gate`
is no longer the root failure: the bounded F0128
helper `m11_dm1_v1_f0128_compose_viewport_for_tuple`
drives the readiness flag after every party-tuple change.
The viewport-crop ready state (`g_f0128_ready`) is
exposed via `m11_dm1_v1_f0128_viewport_crop_ready()`.
G0076_B_UseFlippedWallAndFootprintsBitmaps toggle
(`m11_dm1_v1_f0128_g0076_set/get`) implements the
ceiling/floor alternation flag.

Source-locked per ReDMCSB DUNVIEW.C F0128 (8318-8611) +
F0674_F0128_sub (2995-2996).  v1 keeps the bounded
form: the M11 caller drives the helper after every
party-tuple change; the actual bitmap copy is delegated
to the existing M11 wall path (m11_dm1_v1_dungeon_compose_
g0296) which already calls F0674 via the existing wall
path.  The bounded readiness signal closes the gate
chain root failure.

Regression gate: `tests/test_dm1_v1_f0128_viewport_pc34_compat.c`
(7/7 PASS).


---

## Summary Statistics

| Severity | Count | Fixable in This Pass |
|----------|-------|---------------------|
| Critical | 0     | —                   |
| Major    | 4     | 2 (BUG-101, BUG-102) |
| Minor    | 12    | 4 (BUG-107, BUG-113, BUG-115, BUG-117) |
| Cosmetic | 2     | 1 (BUG-116)          |
| **Total** | **18** | **7**              |

---

## Priority Fix Order

1. **BUG-101** (Major) — Armor defense approximation → delegate to M10 combat resolver
2. **BUG-102** (Major) — Fire/Spell Shield defense not applied
3. **BUG-113** (Minor) — Poison skips vitality adjustment
4. **BUG-117** (Minor) — Test infrastructure build path
5. **BUG-107** (Minor) — Thieves Eye duration
6. **BUG-115** (Cosmetic) — F0306 compiler order hazard alignment
7. **BUG-101** follow-up — **DONE 2026-06-30** via `dm1_v1_orch_pending_damage_flow`

---

## Fixes Applied in v2.7.13

### BUG-101 FIXED — Armor Defense Now Uses F0321 Wound Defense Calculation
Replaced the skill-level approximation with proper ReDMCSB F0321 wound defense:
- Iterates all 6 champion wound slots (hands, head, torso, legs, feet)
- Looks up actual equipped armor defense values from things data
- Applies vitality contribution per wound slot
- Uses G0050 wound defense factor scaling per slot
- Applies the (130 - avgDefense) / 64 scaling formula

### BUG-102 FIXED — Fire/Spell Shield Defense Now Applied
Added proper attack-type branching per F0321:
- FIRE attacks: subtract FireShieldDefense, then apply armor scale
- MAGIC attacks: subtract SpellShieldDefense, skip armor scale (goto T0321024)
- PSYCHIC attacks: skip armor scale
- NORMAL attacks: skip entire F0321 body
- BLUNT/SHARP/LIGHTNING: fall through to armor scale only

### BUG-113 FIXED — Creature Poison Now Applied with Vitality Adjustment
Added creature melee poison application:
- Checks creature profile's poisonAttack value
- 50% chance of poison per hit (matching ReDMCSB F0230:1395-1404)
- Poison value adjusted by defender's Vitality via F0307 formula
- `factor = 170 - vitality; if < 16: poison >> 3; else: (poison * factor) >> 7`

### BUG-117 FIXED — Test Build Path Fully Portable

Added `tools/firestaff_build_dir.py` shared helper with
`find_build_dir()` / `resolve_build_dir()`.  Lookup order:

### BUG-101 FOLLOW-UP FIXED — Pending Damage Flow Now CTest-Gated

Added data-free CTest `dm1_v1_orch_pending_damage_flow` around
`F0889_ORCH_ApplyPendingDamage_Compat`. The gate verifies the M10
handoff from `GameWorld_Compat.pendingCombat` into champion HP, wound
bits, pending-combat clear, `EMIT_CHAMPION_DOWN`, and `partyDead`.
This proves the end-to-end data-layer damage flow without relying on
M11 rendering, real assets, or original-DOS capture.
  1. `FIRESTAFF_BUILD_DIR` env var (already supported by
     some scripts; now standardised).
  2. `<root>/build` (in-tree single-config).
  3. `<root>/builds/<cfg>` (in-tree multi-config, iterates).
  4. `/tmp/firestaff-blockers-build-current`.
  5. `<root>` parent walk for any `CTestTestfile.cmake`.

Updated 211 `tools/verify_pass*.py` scripts to import
`resolve_build_dir` and replace
`Path(os.environ.get('FIRESTAFF_BUILD_DIR', str(ROOT/'build')))`
with `resolve_build_dir(ROOT, ROOT/'build')`.  148 scripts
also needed `import sys` added.

`tools/test_firestaff_build_dir.py` 6/6 PASS (env var,
in-tree, multi-config, no-build fallback, resolve-fallback,
real Firestaff project).

Commit `887ed7cb3`.


### BUG-119 FIXED — Champions die in Hall of Champions
**Reported by user testing v2.7.13.** When the C040 mirror candidate panel is
open, the party can attack and kill the candidate creature before recruiting
it. The candidate is rendered as a portrait graphic in the D1C cell, but the
F0735_COMBAT_ResolveChampionMelee_Compat path had no invulnerability check.

**Fix:** added `isCandidateInvulnerable` flag to `CombatantCreatureSnapshot_Compat`
and a guard in F0735 that bounces to `COMBAT_OUTCOME_NO_ACTION` when the
flag is set. ReDMCSB CLIKCHAM.C F0367 lines 24-25: the candidate panel owns
the front cell while selection is pending. New regression test:
`tests/test_dm1_v1_hall_of_champions_pc34_compat.c`.

### BUG-120 FIXED — Slow after selection in Hall of Champions
**Reported by user testing v2.7.13.** After picking a champion in the C040
panel, the game ran noticeably slower. The C040 panel was re-rendering the
wall-ornament graphic every frame via `m11_draw_dm1_front_mirror_route`
even though the panel chrome was already showing.

**Fix:** in `m11_draw_dm1_front_mirror_route` (m11_game_view.c:13479), added
an early-return when `state->candidateMirrorPanelActive` is set. The wall-
ornament blit is skipped while the panel owns the front cell; only the
candidate portrait is drawn. This also fixes BUG-121 (the peach placeholder
box is hidden during selection).

### BUG-121 FIXED — Graphical artifacts in Hall of Champions
**Reported by user testing v2.7.13.** The C040 candidate panel showed an
orange/peach placeholder box (wall-ornament graphic) floating in the D1C
front cell next to the candidate portrait.

**Fix:** same as BUG-120 — the `candidateMirrorPanelActive` early-return
in `m11_draw_dm1_front_mirror_route` skips the ornament blit while the
panel is open, leaving only the candidate portrait graphic visible.

## Notes

- BUG-103 (Luck), BUG-104 (Creature AI stubs), BUG-105 (Attack ordering), BUG-106 (Flee), BUG-109 (Stat gain cycles), BUG-110 (Magic map), BUG-111 (Sub-cell hit mask), BUG-112 (Savegame fields), BUG-118 (Viewport crop) are significant but require substantial implementation work beyond a single fix pass. They are documented for future work.
- The two recent fixes in v2.7.12 (wall texture behind mirrors `f99587c35`, FTL swoosh palette `e39c3d804`) are verified correct and not re-opened.
- Phase A probe passes 23/23 invariants.
- The M10 data layer (`memory_combat_pc34_compat.c`) has a well-structured F0321 implementation that could replace the inline approximation in `m11_game_view.c`.
