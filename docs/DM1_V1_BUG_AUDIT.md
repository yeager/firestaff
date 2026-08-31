# DM1 V1 Bug Audit — Firestaff v2.7.12 → v2.7.13

Systematic audit of the Firestaff DM1 V1 runtime against the ReDMCSB decompilation as ground truth.

**Auditor:** Opus 4.6 Firestaff Worker
**Date:** 2026-06-13
**Baseline:** Firestaff commit `f99587c35` (HEAD of main, post-v2.7.12)
**ReDMCSB:** WIP20210206, `Toolchains/Common/Source/`
**Build dir:** `/Volumes/Extern-disk/openclaw-work/firestaff/build`
**Phase A probe:** 23/23 PASS
**DM1 test suite:** 556/593 PASS (37 failures — see analysis below)

> Historical baseline: the counts and host paths above describe the 2026-06-13
> audit and must not be read as the current result. On 2026-08-31, the current
> out-of-tree `build-dm1-csb-native` completed all 84 tests carrying the `dm1`
> label against the supplied real-data root without a failure record. That is
> regression evidence, not a claim of full-campaign or pixel parity. The
> current source-review status and remaining capture requirements live in
> `docs/parity/REDMCSB_DM1_CSB_AUDIT.md`.

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

### BUG-101 FIXED — Armor Defense Uses F0321 Wound Defense
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **Description:** The former skill-level approximation has been replaced by the shared F0321/F0313 compatibility route.
- **ReDMCSB Reference:** CHAMPION.C F0321:1838-1900, F0313_CHAMPION_GetWoundDefense
- **Expected (ReDMCSB):** F0321 sums F0313(slot) over the wound mask bits, averages, and scales attack by `(130 - avgDefense) / 64`. F0313 checks each worn item's defense value for the specific wound slot.
- **Actual (Firestaff):** M10 and M11 build source-backed per-wound snapshots, use F0313 defence, then apply F0321's `(130 - avgDefense) / 64` scale.
- **Evidence:** `dm1_v1_orch_pending_damage_flow` and combat integration coverage.

### BUG-102 FIXED — Fire/Spell Shield Defense Applied
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **Description:** The former gap in F0321's fire and magic branches is closed.
- **ReDMCSB Reference:** CHAMPION.C F0321:1842-1857 `P0663_i_Attack -= G0407_s_Party.FireShieldDefense` (C1_FIRE case) and `P0663_i_Attack -= G0407_s_Party.SpellShieldDefense` (C5_MAGIC case)
- **Expected (ReDMCSB):** After F0307 statistic adjustment, subtract party shield defense from attack value.
- **Actual (Firestaff):** `combat_apply_defender_statistic_adjustment` performs F0307 followed by the corresponding source party-shield subtraction; magic then takes the source no-armour-scale exit.

### BUG-103 FIXED — Luck System Is Runtime-Bound in Combat
- **Severity:** Major
- **Category:** Mechanics
- **Description:** F0308's two-stage random path and bounded Luck writeback are implemented in the combat resolver.
- **ReDMCSB Reference:** CHAMPION.C F0308:1120-1145
- **Expected (ReDMCSB):** Champions with high Luck stat have a chance to avoid hits. Cursed items with negative Luck exploit (BUG0_38 in original). Luck decrements by 2 on lucky roll, increments by 2 on unlucky roll.
- **Actual (Firestaff):** The live lifecycle Luck field is copied to the combat snapshot and written back after resolution. The PC/I34E non-positive-Luck branch is modelled; the unsafe negative-Luck exploit is intentionally not claimed.

### BUG-104 FIXED — Creature AI Profiles Are Full-Tier
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **Description:** The historical all-STUB state no longer reflects the runtime.
- **ReDMCSB Reference:** GROUP.C F0028/F0029/F0194/F0196/F0199/F0200/F0201/F0202/F0203, DEFS.H G0243_as_Graphic559_CreatureInfo
- **Expected (ReDMCSB):** Each creature type has specific movement patterns, attack behaviors, and special abilities (levitation, non-material, side attack, back-row preference, etc.).
- **Actual (Firestaff):** All 27 `G0243` profiles are `CREATURE_IMPL_TIER_FULL`; the F0804 per-type dispatch owns poison, theft, ranged, levitation, non-material and archenemy branches. The STUB fallback remains defensive only and has no registered profile.
- **Evidence:** `dm1_v1_creature_ai_behavior_source_lock`.

### BUG-105 FIXED — Creature Attack Ordering Is Source-Locked
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **Description:** The old F0229 review marker was superseded by the source-owned ordered-cell table and dispatch bridge.
- **ReDMCSB Reference:** GROUP.C F0229:685-710
- **Expected (ReDMCSB):** Creatures attack party members in a specific cell-based priority order determined by the creature's position and direction.
- **Actual (Firestaff):** `F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat` resolves the original G0023 row and M10/M11 consume it for target selection.
- **Evidence:** `dm1_v1_ordered_cells_to_attack_pc34_compat` (222 assertions) and `pass803_dm1_v1_ordered_cells_to_attack`.

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

### BUG-107 FIXED — Thieves Eye Duration Is Source-Locked
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **Description:** The earlier approximation was replaced by the F0412 source expression.
- **ReDMCSB Reference:** MENU.C:1960-1963 (T0412032 tail)
- **Expected (ReDMCSB):** `T0412032` shifts the source spell power once and squares it before scheduling C73.
- **Actual (Firestaff):** `F0757_MAGIC_ProduceOtherEffect_Compat` uses `spellPower >>= 1; durationTicks = spellPower * spellPower;` and schedules the C73 timeout.
- **Evidence:** `test_dm1_v1_mnu03_f0757_spell_duration_source_lock_pc34_compat` plus the M10 status-timeline regression.

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

- **Severity:** Resolved runtime
- **Category:** Mechanics
- **ReDMCSB Reference:** `CHAMPION.C:F0331` 2362-2415 and 2487-2497
- **Actual (Firestaff):** `dm1_v1_champion_needs_pc34_compat.c` follows the
  source stamina-cycle loop, food/water thresholds, 64/16 outer cadence,
  rest multiplier, and the `current -= current / maximum` over-maximum
  statistic recovery rule. The former review-marker text was stale.
- **Evidence:** the champion-needs and clock-tick source-locked regressions;
  this remains distinct from broad campaign-capture parity.

### BUG-110 NOT APPLICABLE TO DM1 — CSB Magic-Map State Is Isolated
- **Category:** Scope correction
- **ReDMCSB Reference:** `DEFS.H:695-703`, `MENU.C:1873-1915`, and
  `TIMELINE.C:2002-2012` explicitly mark C80..C83 magic-map fields/events as
  Chaos Strikes Back-only.
- **Actual (Firestaff):** DM1's 25-entry spell table admits no magic-map rows;
  CSB owns the per-champion counters and C80..C83 expiry handling in the
  lifecycle path. This is not a missing DM1 mechanic.
- **Remaining work:** CSB needs real-media magic-map capture coverage before
  this can earn broad CSB UI/runtime parity credit.

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


### BUG-112 NOT AN ORIGINAL-SAVE FORMAT GAP — Field Masks Are Internal
- **Category:** Scope correction
- **ReDMCSB Reference:** `LOADSAVE.C:F0433` writes source game/champion data
  as opaque byte blocks; `F0435` restores those blocks. It has no
  per-field mutation-mask format.
- **Actual (Firestaff):** `DungeonMutation.fieldMask` is documented as a
  bounded internal replay abstraction. The original PC34 F0433/F0435 envelope
  is independently authenticated and round-tripped by the original-save
  handoff path.
- **Remaining work:** external original saves are still needed to prove broad
  interop across real campaigns; the supplied game archives do not contain a
  real save corpus.

### BUG-113 FIXED — Poison Applies Vitality Adjustment
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **Description:** Creature and projectile poison route through F0307 Vitality before mutation.
- **ReDMCSB Reference:** CHAMPION.C F0307 vs Vitality statistic
- **Expected (ReDMCSB):** Poison attack value is adjusted by defender's Vitality via F0307_CHAMPION_GetStatisticAdjustedAttack before applying.
- **Actual (Firestaff):** The source-scaled poison value is used before the poison event is scheduled.

### BUG-114 FIXED — Psychic Defense Uses Source F0030 Truncation
- **Severity:** Resolved runtime
- **Category:** Mechanics
- **ReDMCSB Reference:** `CHAMPION.C:F0321` C6 attack branch and
  `BASE.C:F0030`.
- **Expected:** psychic attack uses `((attack * (115 - Wisdom)) >> 6)` with
  the original integer truncation.
- **Actual (Firestaff):** `F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat`
  now uses the same truncating scaled product. The regression boundary
  `5 * 100 >> 6 == 7` rejects the former half-up result of 8.
- **Scope:** DM1 does not expose a player psychic spell, but Ghost/Rive and
  Screamer creature attacks consume this F0321 C6 defense route.

### BUG-115 FIXED — F0306 Stamina-Adjusted Value Compiler Order Hazard
- **Severity:** Resolved runtime / source-package parity
- **Category:** Mechanics
- **Description:** The BUGX_XX compiler-order hazard at CHAMPION.C:1095 makes `(P0641_i_Value >>= 1) + (P0641_i_Value * L0925/L0926)` package-dependent: it depends on the original compiler, not on dungeon bytes or a user option.
- **ReDMCSB Reference:** CHAMPION.C F0306:1078-1103, BUGX_XX comment
- **Expected:** Megamax Atari ST and High C FM Towns evaluate the first operand first; Turbo C++ 1.01 DM1 PC 3.4 and Aztec-built DM1 Amiga 2.x evaluate the second operand first.
- **Actual (Firestaff):** `GameWorld_Compat.pc34F0306FirstOperandFirst` is bound at native DM1 handoff from verified Atari ST or FM Towns media. The M10 combat route and M11 shield-defense route call the source-selected helper directly. PC 3.4 and Amiga 2.x retain second-operand semantics.
- **Evidence:** `dm1_v1_f0306_stamina_pc34_compat` pins both results without the process-global test toggle; real Atari archive boot remains covered by `dm1_v1_atari_st_outer_archive_real_data`.
- **Impact:** Corrects low-stamina strength, load and shield-defense values on affected original packages.

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

### BUG-119 FIXED — Same-Depth Side Content Survives Center-Block Replay

- **Severity:** Major visual parity
- **Category:** Viewport / F0128 ordering
- **ReDMCSB Reference:** `DUNVIEW.C:F0128` dispatches each depth in the
  order DnL, DnR, then DnC (D3 at 8491-8502, D2 at 8514-8525, D1 at
  8527-8542).
- **Former Firestaff behavior:** the deferred M11 side-content pass rejected
  side cells at the same depth as the nearest closed center wall or door.
  That contradicted the source ordering and could make a side floor ornament
  disappear when approaching its depth.
- **Actual:** only side content *farther* than the nearest center blocker is
  rejected. Same-depth side cells remain eligible; the subsequent
  source-material structural replay provides the real overlap/occlusion.
- **Evidence:** `test_dm1_v1_viewport_3d_pc34_compat` source-lock assertion.


---

## Summary Statistics

| Severity | Count | Fixable in This Pass |
|----------|-------|---------------------|
| Critical | 0     | —                   |
| Major    | 0     | 0 |
| Minor    | 7     | 1 (BUG-112) |
| Cosmetic | 2     | 1 (BUG-116)          |
| **Total** | **10** | **1**              |

---

## Priority Fix Order

1. **BUG-112** (Minor) — preserve unproven save-field masks as a fail-closed limitation
2. **BUG-110** (Minor) — verify per-champion magic-map state against a real save/trace

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

### BUG-122 FIXED — Floor-object moves could lose exclusive ownership

- **ReDMCSB reference:** `DUNGEON.C:F0163` (line 1769) links a THING at the
  end of one list, while `F0164` (line 1840) removes its current list owner.
  `MOVESENS.C:804→870` demonstrates unlink-before-link in a source movement
  path.
- **Former Firestaff behaviour:** the compact floor-cell representation could
  append an object to a destination without removing its old cell entry. A
  full destination could also update coordinates without granting a new list
  owner. That could manifest as duplicated, disappearing, or apparently
  thrown items during inventory/floor interaction.
- **Fix:** `DM1_V1_Object_DropPc34Compat` now performs
  `unlink → capacity check → link`, restoring the old list member exactly if
  the destination is full. Pickup rejects a coordinate whose cell does not
  own the object, rather than manufacturing a carried object.
- **Evidence:** `dm1_v1_object_interaction_source_lock`; the actual PC34 ZIP
  handoff companion is `m11_dm1_real_alcove_item_runtime_pc34`.

## Notes

- BUG-110 (Magic Map) and BUG-112 (Savegame fields) remain explicit validation limits. BUG-103, BUG-104, BUG-105, BUG-106, BUG-107, BUG-109 and BUG-111 are covered by current source-locked runtime routes; they are not backlog items.
- The two recent fixes in v2.7.12 (wall texture behind mirrors `f99587c35`, FTL swoosh palette `e39c3d804`) are verified correct and not re-opened.
- Phase A probe passes 23/23 invariants.
- The M10 data layer (`memory_combat_pc34_compat.c`) has a well-structured F0321 implementation that could replace the inline approximation in `m11_game_view.c`.
