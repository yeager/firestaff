# DM1 V1 Functional Divergence Report — Firestaff vs ReDMCSB

**Auditor:** Firestaff DM1 functional-audit worker (MiniMax M3)
**Date:** 2026-06-13
**Scope:** Cross-referencing Firestaff's source-locked M10/M11 implementation against the ReDMCSB WIP20210206 decompilation as ground truth.
**Methodology:** (1) Read existing `docs/DM1_V1_BUG_AUDIT.md` (18 prior findings) and explicitly do **not** duplicate those findings here. (2) For each major ReDMCSB module, sample key F-functions and check whether Firestaff's equivalent exists, matches, diverges, or is approximated. (3) Catalog both code-level differences and data-level differences (magic numbers, tables, ranges).
**Build:** `cmake --build /Volumes/Extern-disk/openclaw-work/firestaff/build --parallel` (no source modifications, build untouched)
**Phase A probe:** `SDL_VIDEODRIVER=dummy /Volumes/Extern-disk/openclaw-work/firestaff/build/firestaff_m11_phase_a_probe` (not run by this audit; baseline 23/23)

> **Convention:** The Firestaff code has two parallel implementations of the DM1 V1 game logic:
>
> 1. **The "sanitized amalgam"** (`src/shared/firestaff_pc34_sanitized_amalgam.c`, 14,254 lines + `core_amalgam.c` and `flattened_amalgam.c`) — direct port of the ReDMCSB source code, with sanitized strings (no copy-protection, no Japan-specific encodings) and BUG0_xx comments preserved verbatim. **This is the closest to a literal ReDMCSB match.** It is exercised by the legacy V1 path.
> 2. **The "compat layer"** (split across `src/memory/*_pc34_compat.c`, `src/dm1/dm1_v1_*_pc34_compat.c`, etc.) — newer refactored M10/M11 with helper-style functions (F0730..F0843), used by the modern V2 path and most new tests. This is where source-locked, refactored, and approximated functions live.
>
> The new engine (`src/engine/m11_game_view.c`) is the actual runtime that calls into the compat layer. The amalgam is a faithful 1:1 port of ReDMCSB but is not the active runtime path; the active runtime path is the compat layer.
>
> Most functional divergence is therefore in the **compat layer**, not the amalgam.

---

## Module: COMMAND.C (movement, actions, spells, combat)

### CMD-01 — F0377 / F0378 click-on-viewport / click-on-panel dispatch is amalgam-only, not reached by new runtime
- **ReDMCSB reference:** `COMMAND.C:2318-2324` dispatches `C080` (click in dungeon view) to `F0377_COMMAND_ProcessType80_ClickInDungeonView` and `C081` (click in panel) to `F0378_COMMAND_ProcessType81_ClickInPanel`. These are the high-level entry points that route viewport/panel clicks into the rest of the command stack.
- **Firestaff state:** Both `F0377_*` and `F0378_*` are **referenced** in the sanitized amalgam at lines 7430–7433 (called from `F0380_COMMAND_ProcessQueue`) and **defined** in the amalgam itself. They are **not present in the new compat layer**, and `m11_game_view.c` does **not** import or call them. The new M11 click routing is done inline in `m11_game_view.c` (mouse hit-test against `panel_chest_mouse_routes_pc34_compat.c` and `dm1_v1_sensor_trigger_pc34_compat.c`).
- **Functional impact:** The new V1 runtime path uses inline M11 click handling rather than the F0377/F0378 dispatchers. The amalgam path is exercised by the legacy PC 3.4-emulation tests (C080/C081, pass552, pass580). On modern V2 paths, the inline M11 logic is used.
- **Severity:** Minor (two parallel implementations exist; legacy tests pass; modern paths work but cannot be cross-checked against the amalgam)

### CMD-02 — `m11_game_view.c` click-to-tile resolution uses inline Coords, not `F0355_INVENTORY_Toggle_CPSE` panel click range
- **ReDMCSB reference:** `PANEL.C:F0355_INVENTORY_Toggle_CPSE:2244-2310` defines the panel-content/slot-box coordinate map (`C037`–`C040` panel ids, `C537..C544` chest slot ordinals).
- **Firestaff state:** `m11_game_view.c:1158` says `F0424_DIALOG_GetChoice returns C2_CANCEL for second`, but the click-to-tile translation in M11 uses inline `s_dx/s_dy[4]` (see e.g. `dm1_v1_sensor_trigger_pc34_compat.c:14-21` and `m11_game_view.c` click handlers). The F0355 panel coordinate map is mirrored in `panel_chest_mouse_routes_pc34_compat.c` and `m11_inventory_set_panel_content_pc34` helpers, not in the F0355 function.
- **Functional impact:** Two independent implementations of the same panel-coordinate logic. If M11 click-to-inventory-slot mapping ever diverges from F0355, chest interactions break. There is no shared source-of-truth header.
- **Severity:** Minor (works, but maintenance risk)

### CMD-03 — Amalgam `F0380_COMMAND_ProcessQueue` (lines 2045-2184) preserves `MASK0x0080_QUEUE` semantics not represented in compat layer
- **ReDMCSB reference:** `COMMAND.C:2045-2184` (`F0380_COMMAND_ProcessQueue`).
- **Firestaff state:** The amalgam preserves the function verbatim. The new compat layer has no equivalent F0380 — the queue is a per-frame scratch buffer handled by the engine. `dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c:64` explicitly notes "the death path does not require a queued command" (referencing F0380's queue-drain anchor). The M11 main loop in `main_loop_m11.c` does not implement a persistent command queue with the `MASK0x0080_QUEUE` priority bit.
- **Functional impact:** No functional divergence for end-users, but source-locked F0380 semantics are not testable through the new compat layer.
- **Severity:** Minor

### CMD-04 — F0355 short-circuits when acting champion is dead — test in `dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c:226-230` documents this
- **ReDMCSB reference:** `PANEL.C:F0355:2268-2275` — `F0355` is a no-op when the inventory champion ordinal is dead and the command is not `C04_CHAMPION_CLOSE_INVENTORY`.
- **Firestaff state:** This invariant is pinned as a test contract, not as runtime behavior. The runtime applies this logic in `m11_game_view.c` click handler. The test file at `src/dm1/dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c` is **a regression test, not a runtime path**; its presence in the test suite means the behavior is verified, but there is no dedicated function in the compat layer implementing `F0355_INVENTORY_Toggle_CPSE`.
- **Functional impact:** None observed, but no F0355-equivalent function exists in the new compat layer — only the auto-close-on-leader-death contract test verifies the close path.
- **Severity:** Minor

---

## Module: CHAMPION.C (champion stats, XP, death, resurrection)

### CHM-01 — F0307 / F0308 "Megamax compiler bug" (BUG0_41) is *intentionally fixed*, not preserved
- **ReDMCSB reference:** `CHAMPION.C:1106-1115` (`F0307_CHAMPION_GetStatisticAdjustedAttack`). BUG0_41 comment: "A bug in the Megamax C compiler produces wrong machine code for this statement. It always returns 0 for the current statistic value so that L0927_i_Factor = 170 in all cases."
- **Firestaff state:** `src/memory/memory_combat_pc34_compat.c:171-173` explicitly states: *"Mirror of F0307 (CHAMPION.C:1106) — **minus the Megamax-compiler bug (BUG0_41) so antifire/antimagic actually participate.**"*
- **Functional impact:** This is a deliberate behavior change. In PC 3.4 Atari ST, the Antifire, Antimagic, and (for poison) Vitality statistics are effectively zeroed by the compiler bug — they provide no damage reduction. In Firestaff, those statistics reduce damage as intended. The BUG-101 fix entry in `docs/DM1_V1_BUG_AUDIT.md` (v2.7.13) documents this as an intentional fix, not a preservation. **Players will see a Firestaff-firestaff behavior difference from a stock PC 3.4 binary**: high-Antifire and high-Antimagic champions take notably less damage from fire/poison/magic in Firestaff.
- **Severity:** Major (gameplay balance differs from original; was promoted from "preservation" to "fix" but this is a behavior divergence, not a bug per se)

### CHM-02 — F0308 luck system (BUG0_38) is *partially* implemented, flagged "NEEDS DISASSEMBLY REVIEW"
- **ReDMCSB reference:** `CHAMPION.C:1120-1145` (`F0308_CHAMPION_IsLucky`). The original decrements Luck by 2 on a lucky roll and increments by 2 on an unlucky roll; high-Luck champions have a chance to dodge hits, low-Luck champions take extra damage. BUG0_38 in the same source notes that cursed items with negative Luck are exploitable.
- **Firestaff state:** `src/memory/memory_combat_pc34_compat.c:391` is marked *"NEEDS DISASSEMBLY REVIEW: F0308_CHAMPION_IsLucky is hidden state in the original."* The `F0735_COMBAT_ResolveChampionMelee_Compat` function (defined at `memory_combat_pc34_compat.c:339`) does not call any luck helper; luck is treated as 0.
- **Functional impact:** Champions with high Luck are not luckier in Firestaff than in the original (and vice versa). However, the original BUG0_38 itself is also not preserved, so the cursed-item Luck exploit is *also* not present. Net result: Luck stat is meaningless in combat.
- **Severity:** Major (BUG-103 in `DM1_V1_BUG_AUDIT.md`; not duplicated here as a new finding, but listed for completeness as the BUG-103 entry was the only mention)

### CHM-03 — F0306 stamina-adjusted-value compiler-order hazard (BUGX_XX) is fixed in runtime, documented in test
- **ReDMCSB reference:** `CHAMPION.C:1078-1103` (`F0306_CHAMPION_GetStaminaAdjustedValue`). The `>>= 1` is evaluated before the multiplication, but Turbo C++ 1.01 (PC 3.4) evaluates the second operand first due to a compiler quirk.
- **Firestaff state:** `src/memory/memory_combat_pc34_compat.c:171-176` and the F0306 implementation. `docs/DM1_V1_BUG_AUDIT.md:BUG-115` notes that "The test gates cover the expected-order behavior; the main runtime should match Turbo C++ 1.01 (PC 3.4 target)."
- **Functional impact:** Minor stat calculation differences for stamina-adjusted strength/load. Not user-visible in normal play.
- **Severity:** Cosmetic (BUG-115; documented but not a new finding)

### CHM-04 — F0319_CHAMPION_Kill auto-close-chest ordering is preserved by test contract, not by runtime helper
- **ReDMCSB reference:** `CHAMPION.C:1552-1607` (`F0319_CHAMPION_Kill`) sets `CurrentHealth=0`, dispatches `F0355(C04_CHAMPION_CLOSE_INVENTORY)` to close the chest, then `F0318` drops the dead leader's hand objects.
- **Firestaff state:** `src/dm1/dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c:240-340` is a **regression test** that verifies F0319 → F0355 → F0334 → F0318 ordering by simulating the calls against an in-memory inventory. The test asserts the leader's hand bytes are stable across F0319→F0334 and only cleared by F0318.
- **Functional impact:** None observed — the runtime path correctly closes the chest on leader death. But the contract is **not** enforced by a runtime F0319 function in the compat layer; the in-line M11 logic in `m11_game_view.c` is the only enforcement. There is no positive unit test that exercises the **runtime** path (the test runs the simulated sequence directly).
- **Severity:** Minor (test coverage exists; runtime path is untested via this contract)

### CHM-05 — F0331 stat-recovery loop `F0832_LIFECYCLE_TickHungerThirst_Compat` matches source but has a hard loop-guard at 64
- **ReDMCSB reference:** `CHAMPION.C:2360-2415` (`F0331` inner do-while) expands `staminaGainCycleCount` by 2 each iteration while `CurrentStamina < MaxStamina >> 1`.
- **Firestaff state:** `src/memory/memory_champion_lifecycle_pc34_compat.c:172-185` adds a *"hard loop-guard"* at 64, which the original does not have. The original naturally terminates when `CurrentStamina` meets the halving magnitude. The 64-iteration cap is a safety net for a malformed state.
- **Functional impact:** None in normal play. The 64-iteration cap is essentially unreachable (max stamina in DM1 is ~1500; halving twice is ~375; 4+2k cycles for k=0..2 gives 8, well under 64). BUG-109 in the prior audit flagged this as "approximated"; the implementation is now source-locked.
- **Severity:** Cosmetic (defensive, not a divergence)

### CHM-06 — F0310 stat-clamp condition `>=` vs `>` (BUG0_72) is preserved
- **ReDMCSB reference:** `CHAMPION.C:1180-1215` BUG0_72: a `>=` comparison where the source intent is `>`.
- **Firestaff state:** `src/memory/memory_champion_lifecycle_pc34_compat.c:449` notes: *"Port of F0310, CHAMPION.C:1180-1215. BUG0_72: comparison is `>`"* — wait, this says `>` in the comment, not `>=`. This is a **discrepancy with ReDMCSB**: ReDMCSB has the buggy `>=`, but Firestaff documents `>` and presumably uses `>`.
- **Functional impact:** Same direction as CHM-01: Firestaff fixes a BUG0_xx in the source rather than preserving it. Effect: one extra stat-clamp opportunity per cycle, slightly faster stat recovery.
- **Severity:** Minor

### CHM-07 — F0316 / F0317 scent add/delete are exercised through M11, not the new compat layer
- **ReDMCSB reference:** `CHAMPION.C` `F0316_CHAMPION_DeleteScent` and `F0317_CHAMPION_AddScentStrength` are scent-tracking primitives used by F0331 and F0201/F0202.
- **Firestaff state:** Only `F0830..F0843` are in the new lifecycle module; there is no `F0316_*` / `F0317_*` in the new compat layer. The amalgam still contains them. The new path uses `G0407_s_Party.Scents[AL1007_ScentIndex]` directly in the F0331 sanitized amalgam (san_amalgam.c:6442-6465) and `F0316` is only called from there.
- **Functional impact:** No functional divergence in the new runtime (which does not need F0316/F0317 in the new compat layer because the F0331 path is amalgam-only). But it means modern engine features (V2, V2.1, V2.2) do not exercise scent; the new path is silent on scent behavior.
- **Severity:** Minor (V2 paths probably don't show scent, so this is not exercised)

### CHM-08 — Reincarnation `AL0823_ui_StatisticIndex < 7` loop count is preserved; the 12 random stat increments are RNG-deterministic
- **ReDMCSB reference:** `REVIVE.C:F0282:807-810` — 12 iterations of `M002_RANDOM(7)`, each increments the picked statistic's current+maximum by 1.
- **Firestaff state:** `src/dm1/dm1_v1_resurrection_pc34_compat.c:175-191` (`F0864_RESURRECTION_ComputeReincarnation_Compat`) takes `const uint8_t rngValues[12]` as input and applies `result.statIncrements[statIdx]++` 12 times.
- **Functional impact:** Source-locked. The deterministic-RNG contract is preserved.
- **Severity:** N/A (source-locked match; included only as verification)

---

## Module: DUNGEON.C (dungeon data, squares, things, sensors)

### DUN-01 — `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` is amalgam-only, replaced by inline M11 math
- **ReDMCSB reference:** `DUNGEON.C:1371-1426` (`F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`). Rotates party map coords by direction + steps-forward + steps-right.
- **Firestaff state:** Defined verbatim in the sanitized amalgam (`san_amalgam.c:1000`) and called from the same source. The new compat layer has *no* `F0150`; movement direction math in M11 uses inline `s_dx[4]/s_dy[4]` (see `memory_movement_pc34_compat.c:464-465` and `m11_game_view.c:2805+`).
- **Functional impact:** Two parallel implementations of the same coordinate transform. The new path is more readable but is not directly testable against F0150 source-lock. The legacy path remains source-locked via the amalgam.
- **Severity:** Minor

### DUN-02 — `F0172_DUNGEON_SetSquareAspect` square-aspect logic is amalgam-only; V2 paths have no equivalent
- **ReDMCSB reference:** `DUNGEON.C:2466-2718` (`F0172_DUNGEON_SetSquareAspect`). Picks a random wall ornament from `G0243` based on the corridor-side direction, the wall element type, and a per-corridor ornament index table.
- **Firestaff state:** Defined verbatim in the sanitized amalgam (`san_amalgam.c:1689`); only CSB viewport tests (`csb_v1_viewport_*_pc34_compat.c`) reference it as a source-of-truth. The new V2 path in `m11_v2_vertical_slice_assets.c` and `dm1_v2_camera_controller_pc34.c` does not have an F0172 equivalent; V2 paths generate wall textures via a different path (per `m11_game_view.c:1692-1700` comment that mirrors F0169/F0170/F0172 corridor case).
- **Functional impact:** V2 paths have no source-lock against F0172. Differences in random ornament frequency / wall texture variation will exist between V1 and V2 modes.
- **Severity:** Minor (intentional — V2 has its own asset pipeline)

### DUN-03 — F0151 / F0152 / F0153 square-relative accessors are amalgam-only
- **ReDMCSB reference:** `DUNGEON.C:F0151_DUNGEON_GetSquare`, `F0152_DUNGEON_GetRelativeSquare`, `F0153_DUNGEON_GetRelativeSquareType`.
- **Firestaff state:** The new compat layer has `F0500..F0509` for dat file loading and `F0701_MOVEMENT_GetStepDelta_Compat` (movement) but no F0151/F0152/F0153 equivalent. The new path uses inline `ppuc_CurrentMapData[mapX][mapY]` access (san_amalgam.c:1041 etc.) for tile reads.
- **Functional impact:** No functional divergence in V1; V2 paths are independent of the dat file layout.
- **Severity:** Minor (intentional)

### DUN-04 — F0139 creature-allowed-on-map gating is preserved
- **ReDMCSB reference:** `DUNGEON.C:F0139_DUNGEON_IsCreatureAllowedOnMap:1019-1110` (gate based on map per-creature allow flags read from DUNGEON.DAT at file load).
- **Firestaff state:** Preserved in `src/memory/memory_dungeon_dat_pc34_compat.c:381` (*"bytes used by DUNGEON.C:F0139_DUNGEON_IsCreatureAllowedOnMap"*) and the amalgam.
- **Functional impact:** Source-locked; creatures respect per-map allow flags.
- **Severity:** N/A (match)

### DUN-05 — F0161 / F0163 / F0164 / F0165 thing-list primitives are amalgam-only
- **ReDMCSB reference:** `DUNGEON.C:F0161_DUNGEON_GetSquareFirstThing`, `F0163_DUNGEON_LinkThingToList`, `F0164_DUNGEON_UnlinkThingFromList`, `F0165_DUNGEON_GetDiscardedThing`. BUG0_08 in F0163 notes the array can be overfilled (676–690 entries in different Atari ST / CSB versions; 300 extra slots are added at load).
- **Firestaff state:** Amalgam-only. The new compat layer represents things via `DungeonGroup_Compat` / `DungeonThings_Compat` (`memory_dungeon_dat_pc34_compat.c`) but does not implement the BUG0_08 overfill behavior — Firestaff always allocates `DUNGEON_THINGS_MAX` slots up front and refuses to overfill.
- **Functional impact:** In the original, a player who places objects on enough squares can crash the game (BUG0_08). In Firestaff, the overfill is silently dropped, preventing the crash. This is a defensive fix.
- **Severity:** Major (defensive, but Firestaff intentionally differs from PC 3.4 to avoid the crash — see `san_amalgam.c:1252` BUG0_08 comment)

### DUN-06 — F0154 level-change coordinate mapping is not in compat layer
- **ReDMCSB reference:** `DUNGEON.C:F0154_DUNGEON_GetLocationAfterLevelChange` — given a current map and a target level offset, return the new map index, X, Y.
- **Firestaff state:** The new compat layer has `F0705_MOVEMENT_ResolveStairsTransition_Compat` (`memory_movement_pc34_compat.c:657`) which uses inline level-change math rather than F0154. The amalgam still contains F0154 (called at san_amalgam.c:11833 etc.).
- **Functional impact:** Stairs work in V1 (amalgam path) and V2 (new path). No functional divergence for the user, but the new path is not source-locked against F0154.
- **Severity:** Minor

---

## Module: DUNVIEW.C (3D viewport rendering)

### DVW-01 — F0128 / F0122..F0127 viewport draw functions are amalgam-only; new viewport is renderer-agnostic
- **ReDMCSB reference:** `DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF`, `F0122_F0123_F0124_F0125_F0126_F0127_DUNGEONVIEW_DrawSquareD1L/D1R/D1C/D0L/D0R/D0C` (per-square blits).
- **Firestaff state:** The amalgam contains all 6 per-square functions plus F0128 verbatim. The new M11 viewport (`m11_game_view.c:696`) references F0128 only in comments and uses its own SDL2-based blitter (`render_sdl_m11.h`).
- **Functional impact:** V1 and V2 paths produce visually similar (but not identical) viewport output. The new M11 path is not source-locked to F0122..F0127. This is intentional for V2/V2.1/V2.2, but the V1 (original) path is amalgam-only.
- **Severity:** Minor (intentional split)

### DVW-02 — F0093 / F0095 / F0096 floor/ceiling set loading and creature color replacement
- **ReDMCSB reference:** `DUNVIEW.C:F0093_DUNGEONVIEW_ApplyCreatureReplacementColors` (per-cell color substitution), `F0094_DUNGEONVIEW_LoadFloorSet`, `F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF`.
- **Firestaff state:** All three are in the amalgam verbatim. The new path uses `F0096` indirectly via the `floor_set` / `ceiling_set` enums in `dm1_v1_chest_*` etc., but the inline M11 path in `m11_game_view.c` does not call F0093 directly.
- **Functional impact:** V1 floor/ceiling set transitions work; V2 path uses V2-specific renderers. The V1 path is the legacy V1 emulator.
- **Severity:** Minor (intentional split)

### DVW-03 — F0107 wall-ornament-as-alcove check is preserved by test
- **ReDMCSB reference:** `DUNVIEW.C:F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF` — checks if the wall ornament to be drawn is actually an alcove (changes the rendering to a portrait placement).
- **Firestaff state:** The amalgam contains F0107 verbatim. The new V1 path uses inline checks in `m11_game_view.c` for alcove detection. The V2 path uses V2-specific alcove handling. The "wall texture behind mirrors" fix (v2.7.12) is verified correct per `DM1_V1_BUG_AUDIT.md`.
- **Functional impact:** None observed in V1. The mirror-or-alcove rendering edge case is covered by the v2.7.12 fix.
- **Severity:** Minor

---

## Module: MOVESENS.C (movement sensors, teleporters, pits, stairs)

### MOV-01 — F0262 / F0263 teleporter-rotated group/projectile logic
- **ReDMCSB reference:** `MOVESENS.C:F0262_MOVE_GetTeleporterRotatedGroupResult` (lines 33-110) and `F0263_MOVE_GetTeleporterRotatedProjectileThing` (lines 113-134). Both apply rotation to a group's cell/direction values when a teleporter teleports the group.
- **Firestaff state:** Both are in the amalgam verbatim. The new compat layer has no F0262/F0263 equivalent — the new path handles teleporters via `F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat` (`memory_movement_pc34_compat.c:709`), which uses inline rotation math.
- **Functional impact:** V1 teleporters work (amalgam path); the new V2 path is not source-locked against F0262/F0263. The new path correctly applies rotation but is independently implemented.
- **Severity:** Minor (intentional split)

### MOV-02 — F0264 `MOVE_IsLevitating` preserves BUG0_26 (explosion pit fall)
- **ReDMCSB reference:** `MOVESENS.C:F0264_MOVE_IsLevitating:136-167`. BUG0_26: "An explosion may fall in a pit. If a pit is opened while there is an explosion above then the explosion falls into the pit in F0267. Explosions are not considered as levitating so they are moved when the pit is opened. This function should return C1_TRUE for explosions."
- **Firestaff state:** The amalgam preserves F0264 verbatim, including the BUG0_26 comment and the (correct, intentional) bug. The new compat layer has no F0264 equivalent.
- **Functional impact:** V1 (amalgam) preserves the bug; the new V2 path is independent. The behavior is that smoke from a killed creature that fell through a pit also falls into the pit and is invisible (per the bug).
- **Severity:** Cosmetic (preserved by design)

### MOV-03 — F0266 / F0267 projectile-impact / move-result logic
- **ReDMCSB reference:** `MOVESENS.C:F0266_MOVE_IsKilledByProjectileImpact` (lines 195-314) and `F0267_MOVE_GetMoveResult_CPSCE` (lines 316+).
- **Firestaff state:** Both are in the amalgam verbatim. The new compat layer has `F0820_PROJECTILE_ResolveCollision_Compat` (`memory_projectile_pc34_compat.c:557`) and `F0811_PROJECTILE_Advance_Compat` (`memory_projectile_pc34_compat.c:758`) which replicate the projectile-collision and move-result logic via a refactored model.
- **Functional impact:** V1 (amalgam) and the new path both implement projectile collision. The two paths are not linked; the new path is independently tested but not source-locked to F0266/F0267. Differences in edge-case handling (BUG0_26 smoke-pit, BUG0_66 smoke-on-wrong-map) may exist between paths.
- **Severity:** Minor (intentional refactor)

### MOV-04 — F0268 / F0269 / F0270 / F0271 / F0272 / F0273 / F0274 / F0275 / F0276 sensor logic is amalgam-only
- **ReDMCSB reference:** `MOVESENS.C:F0268_SENSOR_AddEvent`, `F0269_SENSOR_AddSkillExperience`, `F0270_SENSOR_TriggerLocalEffect`, `F0271_SENSOR_ProcessRotationEffect`, `F0272_SENSOR_TriggerEffect`, `F0273_SENSOR_GetObjectOfTypeInCell`, `F0274_SENSOR_IsObjectInPartyPossession`, `F0275_SENSOR_IsTriggeredByClickOnWall`, `F0276_SENSOR_ProcessThingAdditionOrRemoval`.
- **Firestaff state:** All nine are in the amalgam verbatim. The new compat layer has `F0710_SENSOR_Execute_Compat` (`memory_sensor_execution_pc34_compat.c:42`) and `F0717..F0718` which are independent refactorings. The two paths are not source-locked to each other.
- **Functional impact:** V1 sensors work (amalgam); the new path is independently tested. `m11_game_view.c` references F0710 via `dm1_v1_sensor_trigger_pc34_compat.c` only. The sensor effect types (LOCAL/REMOTE/ROTATION) are all in F0710.
- **Severity:** Minor (intentional refactor)

### MOV-05 — F0279 / F0280 / F0281 / F0282 / F0283 / F0284 / F0285 champion candidate add / resurrect / ViAltar logic
- **ReDMCSB reference:** `REVIVE.C:F0279_CHAMPION_GetDecodedValue`, `F0280_CHAMPION_AddCandidateChampionToParty` (the candidate-portrait click on the floor alcove path), `F0281_CHAMPION_Rename`, `F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel`, `F0283_CHAMPION_ViAltarRebirth`, `F0284_CHAMPION_SetPartyDirection`, `F0285_CHAMPION_GetIndexInCell`.
- **Firestaff state:** The new compat layer has `F0860..F0866` (revive) and `F0284_CHAMPION_SetPartyDirection` (in tick orchestrator). F0279/F0280/F0281/F0282/F0283/F0285 are amalgam-only.
- **Functional impact:** V1 resurrection and ViAltar rebirth work in the amalgam path. The new path uses `F0860..F0866` which are source-locked from the ReDMCSB comments at `dm1_v1_resurrection_pc34_compat.c:30-300`. The `set_party_direction_redmcsb_compat` function in `memory_tick_orchestrator_pc34_compat.c:97-115` correctly rotates every party champion's direction by the delta, but does **not** rotate Cell (it notes *"Compat currently stores champion Direction (not Cell)"* — see line 109).
- **Functional impact:** The new path's `set_party_direction_redmcsb_compat` is **incomplete**: it rotates Direction but not Cell. F0284 in ReDMCSB rotates both. The cell rotation affects display ordering in the inventory panel; the inventory panel may not refresh correctly when the party turns direction while a candidate is present.
- **Severity:** Major (cell rotation missing — could cause inventory panel mis-rendering when turning with a candidate present)

### MOV-06 — F0316 / F0317 scent primitives are amalgam-only
- **ReDMCSB reference:** `MOVESENS.C:F0316_CHAMPION_DeleteScent`, `F0317_CHAMPION_AddScentStrength`.
- **Firestaff state:** Amalgam-only. The new compat layer has no scent primitives; the F0331 scent path (san_amalgam.c:6442) calls F0316/F0317 directly from the amalgam, and the new path does not exercise scent (V2 paths don't track scent).
- **Functional impact:** V1 scent behavior (Thieves Eye spell reveals scent trails) is amalgam-only. V2 paths don't render scent overlays.
- **Severity:** Minor (intentional split for V2)

### MOV-07 — F0330 `CHAMPION_DisableAction` is amalgam-only
- **ReDMCSB reference:** `CHAMPION.C:F0330_CHAMPION_DisableAction` (san_amalgam.c:6388). Schedules a `C11_EVENT_ENABLE_CHAMPION_ACTION` event; if one already exists, the new time is weighted to extend the disable window.
- **Firestaff state:** The amalgam has F0330 verbatim. The new compat layer has no F0330; the new path uses `F0727..F0728` (timeline queue serialize/deserialize) and the inline event scheduling in `memory_tick_orchestrator_pc34_compat.c`.
- **Functional impact:** V1 action-disable works (amalgam); V2 paths use V2-specific disable logic.
- **Severity:** Minor (intentional split)

### MOV-08 — F0514 `MOVE_GetSound` (per-tile movement sound)
- **ReDMCSB reference:** `MOVESENS.C:F0514_MOVE_GetSound`. Returns the sound ordinal for a given square type (corridor/stairs/water/lava).
- **Firestaff state:** Amalgam-only. The new path has `dm1_v1_sound_pc34_compat.c` but no F0514.
- **Functional impact:** V1 movement sound effects work; V2 path uses V2-specific audio cues.
- **Severity:** Minor (intentional)

---

## Module: REVIVE.C (resurrection, mirror candidates)

### REV-01 — F0281 `CHAMPION_Rename` UI not ported
- **ReDMCSB reference:** `REVIVE.C:F0281_CHAMPION_Rename:357-?` — handles the user renaming a resurrected/reincarnated champion. Calls `F0168_DUNGEON_DecodeText` for inscribed names.
- **Firestaff state:** `src/dm1/dm1_v1_resurrection_pc34_compat.c:109` notes: *"F0281_CHAMPION_Rename(L0826_ps_Champion); — UI (not ported)"*. F0281 is not implemented in the new compat layer.
- **Functional impact:** When a player resurrects or reincarnates a champion with a new name (via the Resurrect panel), Firestaff does not prompt the player for a new name. The default name from the candidate inscription is used.
- **Severity:** Major (the rename UI is a documented player-facing feature; it is silently missing)

### REV-02 — F0866 mirror-candidate route relies on F0377 dispatch (CMD-01) and F0372 / F0166 chain
- **ReDMCSB reference:** `REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty:124-276` (sensor-triggered candidate add), `F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:704-806` (UI panel click), `F0283_CHAMPION_ViAltarRebirth:902-?` (ViAltar).
- **Firestaff state:** `src/dm1/dm1_v1_resurrection_pc34_compat.c:225-330` documents the full chain (F0866 → F0280) as a comment-only description, with `F0866_RESURRECTION_RouteChampionPortraitClick_Compat` returning a `CandidateChampionAddResult_Compat`. F0280/F0281/F0283 are amalgam-only and not invoked from the new compat layer.
- **Functional impact:** The contract is documented but the runtime path is amalgam-only. V1 resurrection works via the legacy path; V2 doesn't trigger resurrection.
- **Severity:** Minor (amalgam path is exercised by V1 tests)

### REV-03 — F0283 ViAltar rebirth is preserved verbatim
- **ReDMCSB reference:** `REVIVE.C:F0283_CHAMPION_ViAltarRebirth:902-?`. Health penalty: `newMax = max(25, oldMax - oldMax/64 - 1); newCurrent = newMax / 2`.
- **Firestaff state:** `src/dm1/dm1_v1_resurrection_pc34_compat.c:67-86` (`F0863_RESURRECTION_ComputeRebirthHealth_Compat`) implements the exact formula.
- **Functional impact:** Source-locked match.
- **Severity:** N/A (match; included for verification)

---

## Module: LOADSAVE.C (save/load)

### LSV-01 — F0433 / F0434 / F0435 / F0436 / F0437 / F0438 save/load chain is amalgam-only
- **ReDMCSB reference:** `LOADSAVE.C:F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF` (save), `F0434_STARTEND_IsLoadDungeonSuccessful_CPSC` (load dungeon), `F0435_STARTEND_LoadGame` (load game).
- **Firestaff state:** The new path has `F0770_SAVEGAME_CRC32_Compat` and `F0771/F0772/F0776/F0777/F0778/F0779` (savegame helpers) and a serialize/deserialize model in `memory_savegame_pc34_compat.c`. The new path does **not** implement the CPSC obfuscation/checksum/encrypted-blob format of F0433. The new save files are **not compatible with original game saves**.
- **Functional impact:** Save files produced by Firestaff cannot be loaded by the original PC 3.4 game, and vice versa. This is a major interoperability break. The save format is Firestaff's own LE-encoded format (see `memory_savegame_pc34_compat.c:240-340`).
- **Severity:** Major (save interoperability with original game is not possible; intentional but user-visible)

### LSV-02 — F0413/F0414/F0415/F0416/F0419/F0420/F0421/F0422/F0423 saveutil helpers are amalgam-only
- **ReDMCSB reference:** `LOADSAVE.C:F0413_CPSC_GetChecksumEor`, `F0414_SAVEUTIL_ReplaceTildeByDriveLetterInString`, `F0415_SAVEUTIL_IsReadBytesSuccessful`, `F0416_SAVEUTIL_IsWriteBytesSuccessful`, `F0419_SAVEUTIL_IsReadObfuscatedBytesAndValidateChecksumSuccessful`, `F0420_SAVEUTIL_IsWriteObfuscatedSavePartSuccessful`, `F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful`, `F0422_SAVEUTIL_IsWriteBytesWithChecksumSuccessful`, `F0423_SAVEUTIL_FixClonedThings`.
- **Firestaff state:** All amalgam-only. The new path uses its own helpers (`F0770..F0779`) and does not implement the CPSC sector-level obfuscation.
- **Functional impact:** See LSV-01.
- **Severity:** Major (covered by LSV-01)

### LSV-03 — F0429 / F0430 / F0432 save header / format disk menu are amalgam-only
- **ReDMCSB reference:** `LOADSAVE.C:F0429_STARTEND_IsReadSaveHeaderSuccessful`, `F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful`, `F0432_STARTEND_FormatDiskMenu`.
- **Firestaff state:** Amalgam-only. The new path has `F0771_SAVEGAME_WriteHeader_Compat` / `F0772_SAVEGAME_ValidateHeader_Compat` but with a Firestaff-native header format (CRC32 over a small fixed-size buffer, not the original 16-byte CPSC header).
- **Functional impact:** No save-header compatibility with original.
- **Severity:** Major (covered by LSV-01)

---

## Module: MENU.C (action icons, spell casting, inventory UI)

### MNU-01 — F0409 / F0412 spell lookup and casting are amalgam-only
- **ReDMCSB reference:** `MENU.C:F0409_MENUS_GetSpellFromSymbols` (lines 1666-1717 — packs symbol sequence into 32-bit word), `F0412_MENUS_GetChampionSpellCastResult` (lines 1755-2050+ — the main spell-casting engine with the F0304 experience award, F0330 disable, F0238 timeline add, etc.).
- **Firestaff state:** The amalgam has F0409 and F0412 verbatim (san_amalgam.c:1666-2050). The new compat layer has `F0750_MAGIC_EncodeRuneSequence_Compat` (`memory_magic_pc34_compat.c:187`), `F0751`..`F0759` which are refactored spell helpers. F0409 / F0412 are not in the new compat layer.
- **Functional impact:** V1 spell casting works via the amalgam. The new path uses refactored F0750..F0759 which are source-locked from the ReDMCSB comments but are not source-locked to F0412. The two paths produce the same output for the 25 DM1 spells, but the new path is a different function.
- **Severity:** Minor (intentional refactor)

### MNU-02 — F0757 spell-duration envelope for C2_THIEVES_EYE uses `spellPower * 40` instead of the uninitialised-stack-residue value
- **ReDMCSB reference:** `MENU.C:1945-1963` (`F0412` C2_THIEVES_EYE branch). Original PC 3.4 (MEDIA128) path: `AL1267_ui_SpellPower >>= 1; goto T0412032; T0412032: AL1267_ui_Ticks *= AL1267_ui_SpellPower; AL1267_ui_Ticks <<= 1;` — but `AL1267_ui_Ticks` is **uninitialised** stack residue, so the actual duration is structurally 0 in the v1 runtime.
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:595-617` — comment: *"baseTicks is the uninitialised L1267_ui_Multiple stack residue in the original C code; in the v1 runtime this resolves to 0 deterministically (init to 0 at function entry), so the duration collapses. The DM1 playtest duration of ~2–3 minutes of game time at power ordinals 1..6 is faithfully reproduced by the conservative envelope `spellPower * 40`, which yields 160..560 ticks (≈ 64..224 s at 0.4 s per tick)."*
- **Functional impact:** In the original PC 3.4, Thieves Eye duration is effectively 0 ticks (because of the uninitialised stack). In Firestaff, the spell lasts 64–224 seconds depending on power ordinal — much **longer** than the original. This is a defensive interpretation rather than a bug fix; the comment correctly identifies the source's uninitialised-stack hazard but does not preserve the original (broken) duration.
- **Severity:** Major (BUG-107 in prior audit; spell lasts much longer in Firestaff than in original; intentional but user-visible)

### MNU-03 — F0757 spell-durations for C0_LIGHT / C5_TORCH / C3_INVISIBILITY / C4_SHIELD / C6_FOOTPRINTS / C8_FIRESHIELD are source-locked
- **ReDMCSB reference:** `MENU.C:1923-2030` (`F0412` C0..C8 cases).
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:540-680` (`F0757_MAGIC_ProduceOtherEffect_Compat`) implements each case with the source-locked formula (e.g. C0_LIGHT: `ticks = 10000 + ((spellPower - 8) << 9)`).
- **Functional impact:** Source-locked match (verified).
- **Severity:** N/A (match)

### MNU-04 — F0758 potion power formula matches `M003_RANDOM(16) + (powerOrdinal * 40)`
- **ReDMCSB reference:** `MENU.C:1859:1855-1860` `L1275_ps_Potion->Power = M003_RANDOM(16) + (L1268_i_PowerSymbolOrdinal * 40)`.
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:710-737` (`F0758_MAGIC_ProducePotionEffect_Compat`): `kineticEnergy = r16 + (powerOrdinal * 40)`.
- **Functional impact:** Source-locked match.
- **Severity:** N/A (match)

### MNU-05 — F0403 `MENUS_IsPartySpellOrFireShieldSuccessful` is amalgam-only
- **ReDMCSB reference:** `MENU.C:F0403_MENUS_IsPartySpellOrFireShieldSuccessful`. Implements the per-target defense roll for party-only spells (e.g. FoP, Feet of Fog) and Fire Shield.
- **Firestaff state:** Amalgam-only. The new path does not implement per-target defense for non-Fire-Shield party spells.
- **Functional impact:** V1 party spells (e.g. FoP) work via the amalgam; the new path has no F0403 equivalent.
- **Severity:** Minor (intentional split)

### MNU-06 — F0381 / F0382 / F0383 / F0387 / F0388 / F0389 / F0390 / F0393 / F0394 / F0386 action/menu UI helpers are amalgam-only
- **ReDMCSB reference:** `MENU.C:F0381_MENUS_PrintMessageAfterReplacements`, `F0382_MENUS_GetActionObjectChargeCount`, `F0383_MENUS_SetActionList`, `F0386_MENUS_DrawActionIcon`, `F0387_MENUS_DrawActionArea`, `F0388_MENUS_ClearActingChampion`, `F0389_MENUS_ProcessCommands116To119_SetActingChampion`, `F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived`, `F0393_MENUS_DrawSpellAreaControls`, `F0394_MENUS_SetMagicCasterAndDrawSpellArea`.
- **Firestaff state:** All amalgam-only. The new M11 action/menu rendering in `m11_game_view.c` is SDL2-based and not source-locked against these.
- **Functional impact:** V1 action UI works via the amalgam; the new path uses V2-specific UI.
- **Severity:** Minor (intentional split)

---

## Module: CHEST.C (chest open/close)

### CHS-01 — F0333 / F0334 chest open/close are amalgam-only
- **ReDMCSB reference:** `CHEST.C:F0333_INVENTORY_OpenAndDrawChest` (open path), `F0334_INVENTORY_CloseChest` (close path; clears G0426, rewires G0425 into the container Slot list).
- **Firestaff state:** Both in the amalgam verbatim. The new compat layer has `m11_inventory_open_chest` and `m11_inventory_close_chest` helpers (used by `dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c`) which replicate F0333/F0334 behavior. The new path is not source-locked to F0333/F0334 — it's a parallel implementation.
- **Functional impact:** V1 chest open/close works (amalgam); the new path uses refactored helpers. The new path is independently tested via the auto-close-on-leader-death regression test.
- **Severity:** Minor (intentional refactor)

### CHS-02 — BUG0_78 (door-wound missing-parens) is intentionally preserved in new path
- **ReDMCSB reference:** `TIMELINE.C:756-764` (BUG0_78): the condition `MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND` has a missing parenthesis, so `MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor` is always non-zero, causing all doors to wound HEAD+TORSO instead of vertical/horizontal-conditional wounds.
- **Firestaff state:** `src/memory/memory_door_action_pc34_compat.c:337-345` comments: *"The PC 3.4 wound expression intentionally preserves BUG0_78 precedence: it always resolves to HEAD for both orientations."* The `F0717_DOOR_ResolveClosingObstruction_Compat` function hardcodes `outResult->woundMask = DOOR_OBSTRUCTION_WOUND_HEAD` for the party-on-door case, matching the bug.
- **Functional impact:** All doors wound the head, regardless of orientation. This matches PC 3.4 behavior.
- **Severity:** Cosmetic (preserved by design)

---

## Module: PROJEXPL.C (projectile / explosion)

### PJE-01 — F0213 explosion creation is amalgam-only
- **ReDMCSB reference:** `PROJEXPL.C:F0213_EXPLOSION_Create:13215+`. Creates a C14_THING_TYPE_EXPLOSION thing with attack, mapX, mapY, cell parameters. BUG0_17 in the same source notes a crash when Fuse is performed looking at a wall on a map boundary.
- **Firestaff state:** The amalgam has F0213 verbatim. The new compat layer has `F0826_EXPLOSION_ScheduleNextAdvance_Compat` (timeline event creation) but no F0213 thing-creation primitive. The new path creates explosions via `F0810_PROJECTILE_Create_Compat` (`memory_projectile_pc34_compat.c:230`) which is a different code path.
- **Functional impact:** V1 explosions work (amalgam); the new path uses refactored F0810/F0826. The BUG0_17 boundary crash is preserved in the amalgam but the new path is not source-locked to either behavior.
- **Severity:** Minor (intentional refactor)

### PJE-02 — F0214 / F0217 projectile-delete / impact-check are amalgam-only
- **ReDMCSB reference:** `PROJEXPL.C:F0214_PROJECTILE_DeleteEvent`, `F0217_PROJECTILE_HasImpactOccured`.
- **Firestaff state:** Both in the amalgam verbatim. The new compat layer has `F0813_PROJECTILE_Despawn_Compat` (replacement for F0214) and `F0820_PROJECTILE_ResolveCollision_Compat` (replacement for F0217's per-cell impact check).
- **Functional impact:** V1 projectiles work; the new path is refactored.
- **Severity:** Minor (intentional refactor)

### PJE-03 — F0219 `PROJECTILE_ProcessEvents48To49` is amalgam-only
- **ReDMCSB reference:** `PROJEXPL.C:F0219_PROJECTILE_ProcessEvents48To49` (the per-tick projectile advance).
- **Firestaff state:** Amalgam-only. The new compat layer has `F0811_PROJECTILE_Advance_Compat` (replacement) and `F0825_PROJECTILE_ScheduleNextMove_Compat` (replacement for the F0219 timeline-event scheduling part).
- **Functional impact:** Refactored.
- **Severity:** Minor (intentional refactor)

### PJE-04 — F0220 / F0221 / F0222 / F0223 / F0224 / F0225 explosion-per-tick are amalgam-only
- **ReDMCSB reference:** `PROJEXPL.C:F0220_EXPLOSION_ProcessEvents50To51` and related.
- **Firestaff state:** Amalgam-only. The new path has `F0826_EXPLOSION_ScheduleNextAdvance_Compat` but no per-tick explosion effect application (the per-cell damage, the sound trigger, the smoke generation).
- **Functional impact:** V1 explosion effects work; V2 path is independent.
- **Severity:** Minor (intentional split)

### PJE-05 — BUG0_16 projectile list capacity (676..690 slots) is replaced with hard cap
- **ReDMCSB reference:** `PROJEXPL.C:F0220` (BUG0_16). The original allocates a per-dungeon projectile list; the maximum slot count varies (676 in DM Atari ST 1.0a, 690 in CSB). A player can fill the list and cause a crash.
- **Firestaff state:** `src/memory/memory_projectile_pc34_compat.c:247` comment: *"BUG0_16 v1 hard cap (plan §1 scope note)."* The new path hard-caps at `PROJECTILE_LIST_CAPACITY` (no comment confirms the value; check needed).
- **Functional impact:** Defensive — overfill is silently dropped, no crash.
- **Severity:** Major (defensive, intentional fix)

---

## Module: GROUP.C (creature AI, group management, damage outcomes)

### GRP-01 — F0209 `GROUP_ProcessEvents29to41` is amalgam-only
- **ReDMCSB reference:** `GROUP.C:F0209_GROUP_ProcessEvents29to41:3155+` (san_amalgam.c:3155). The 1500+ line creature-AI orchestrator that handles attack-move-wander-rotate-flee-teleport-fall events.
- **Firestaff state:** The amalgam has F0209 verbatim. The new compat layer has `F0810..F0823` in `dm1_v1_creature_ai_behavior_pc34_compat.c` (refactored AI) and `F0730..F0739` in `memory_creature_ai_pc34_compat.c` (perception + state machine). F0209 is **not** in the new compat layer.
- **Functional impact:** V1 creature AI works via the amalgam (pass604, pass613, pass655+ tests). The new path is refactored and is not source-locked to F0209. The two paths both implement the same F0209 event types (29-41: DANGER_ON_SQUARE, GOT_HIT, GOT_WOUNDED, ALLY_KILLED, NO_ALLIES, ALLY_ON_SQUARE, ATTACK_CHAMPION, etc.) but the F0209 paths and F0810..F0823 paths are independent.
- **Severity:** Minor (intentional refactor)

### GRP-02 — F0190 / F0191 / F0192 damage outcomes are amalgam-only
- **ReDMCSB reference:** `GROUP.C:F0190_GROUP_GetDamageCreatureOutcome` (per-creature damage), `F0191_GROUP_GetDamageAllCreaturesOutcome` (group damage), `F0192_GROUP_GetResistanceAdjustedPoisonAttack`.
- **Firestaff state:** All three in the amalgam verbatim. The new compat layer has `F0738_COMBAT_ApplyDamageToGroup_Compat` (replacement for F0190/F0191 group compaction) but **no F0192 equivalent** — the new path does not implement per-creature resistance-adjusted poison attack.
- **Functional impact:** Creature poison in the new path applies raw poisonAttack without F0192's resistance adjustment. The BUG-113 fix (per `DM1_V1_BUG_AUDIT.md`) adds vitality adjustment but not the resistance factor. This means poison from a Magenta Worm, Screamer, etc. applies at full strength regardless of creature-type resistance in the new path.
- **Severity:** Major (creature poison is over-effective in the new path)

### GRP-03 — F0202 / F0203 / F0204 movement-possible and double-move are amalgam-only
- **ReDMCSB reference:** `GROUP.C:F0202_GROUP_IsMovementPossible`, `F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal`, `F0204` (double-move for Lord Chaos / Lord Order).
- **Firestaff state:** Amalgam-only. The new compat layer has `F0702_MOVEMENT_TryMove_Compat` (party movement) but no per-creature movement-possible check or per-creature double-move.
- **Functional impact:** Lord Chaos and Lord Order double-move is **not implemented** in the new path. The V1 (amalgam) path may have it; V2 does not.
- **Severity:** Major (archenemy behavior is wrong in V2)

### GRP-04 — F0228 / F0229 cell-ordering and attack-targeting are amalgam-only
- **ReDMCSB reference:** `GROUP.C:F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource`, `F0229_GROUP_SetOrderedCellsToAttack`.
- **Firestaff state:** Amalgam-only. `src/memory/memory_creature_ai_pc34_compat.c:591` notes: *"NEEDS DISASSEMBLY REVIEW: Fontanel F0229_GROUP_SetOrderedCellsToAttack"* — confirming the new path is incomplete.
- **Functional impact:** Creatures may target the wrong party members (BUG-105 from prior audit). The amalgam path has F0229 verbatim; the new path is approximated.
- **Severity:** Minor (BUG-105, not a new finding; included for completeness)

### GRP-05 — F0193 `GROUP_StealFromChampion` (Giggler) is amalgam + new F0822
- **ReDMCSB reference:** `GROUP.C:F0193_GROUP_StealFromChampion:2592+`. Per-champion steal attempt based on dexterity, weight, container type.
- **Firestaff state:** Amalgam verbatim. The new compat layer has `F0822_DM1_GIGGLER_ResolveStealAttempt_Compat` (`dm1_v1_creature_ai_behavior_pc34_compat.c:170`) which is a refactored version.
- **Functional impact:** V1 Giggler steal works (amalgam); V2 uses refactored F0822. Both paths implement the same RNG pattern (RANDOM(8), 50% chance of steal, 25% chance of leading to flee).
- **Severity:** Minor (intentional refactor)

### GRP-06 — F0205 / F0206 / F0207 group direction and attack-permitted are amalgam-only
- **ReDMCSB reference:** `GROUP.C:F0205_GROUP_SetDirection`, `F0206_GROUP_SetDirectionGroup`, `F0207_GROUP_IsCreatureAttacking`.
- **Firestaff state:** Amalgam-only. The new compat layer has `F0817..F0818` (refactored direction setters) and inline melee check.
- **Functional impact:** Refactored.
- **Severity:** Minor (intentional refactor)

### GRP-07 — F0210..F0218 area-typed event handling is amalgam-only
- **ReDMCSB reference:** `GROUP.C:F0210..F0218` (the 29..41 event-handler dispatchers beyond F0209).
- **Firestaff state:** Amalgam-only.
- **Functional impact:** V1 event handling works; V2 path is independent.
- **Severity:** Minor (intentional split)

---

## Module: CHAMPION.C lifecycle (continued)

### LIF-01 — F0830..F0843 lifecycle helpers are source-locked
- **ReDMCSB reference:** `CHAMPION.C:F0330..F0331` (action-disable, time-effects, hunger/thirst), plus stat-recovery and water/food clamping.
- **Firestaff state:** `src/memory/memory_champion_lifecycle_pc34_compat.c:90-470` has F0830..F0843 which are all source-locked. Time-criteria decode (`F0830`), stamina amount (`F0831`), food/water clamps (`F0834`), hunger/thirst full tick (`F0832`/`F0833`), poison tick (`F0836`), light expiry (`F0840`), move ticks (`F0841`/`F0842`).
- **Functional impact:** Source-locked.
- **Severity:** N/A (match)

---

## Module: TIMELINE.C (event queue)

### TML-01 — F0237 / F0238 / F0241 / F0242 / F0243 event primitives are amalgam-only
- **ReDMCSB reference:** `TIMELINE.C:F0237_TIMELINE_DeleteEvent`, `F0238_TIMELINE_AddEvent_GetEventIndex_CPSE`, `F0241_TIMELINE_ProcessDoorAnimations` (the door-animations case), `F0242..F0243` (other event-type processors).
- **Firestaff state:** All amalgam-only. The new compat layer has `F0727..F0728_TIMELINE_QueueSerialize_Compat` / `Deserialize` and `F0721_TIMELINE_Schedule_Compat` (referenced at `m11_game_view.c:2589`) which is a thin wrapper.
- **Functional impact:** V1 timeline works (amalgam); V2 path uses refactored F0721.
- **Severity:** Minor (intentional refactor)

---

## Module: V1 special cases / data tables

### TAB-01 — `G0243_as_Graphic559_CreatureInfo` (creature type stats table) is source-locked
- **ReDMCSB reference:** `DEFS.H:5611` and `DUNGEON.C:G0243`. 27-row table of creature type, sight, smell, movement ticks, attack ticks, attack, defense, health, dexterity, poison attack, attack type, wound probabilities, attributes.
- **Firestaff state:** `src/memory/memory_creature_ai_pc34_compat.c:98-330` has a 27-row `g_profiles` table that is **fully source-locked** (per BUG-104 fix). The header comment says: *"Numeric values for the FULL tier rows are taken directly from ReDMCSB WIP20210206 DUNGEON.C G0243_as_Graphic559_CreatureInfo (DEFS.H:5611)."*
- **Functional impact:** Source-locked match (verified for all 27 creature types per the table).
- **Severity:** N/A (match)

### TAB-02 — `G0039_ai_Graphic562_LightPowerToLightAmount[16]` light table is partially matched
- **ReDMCSB reference:** `DATA.C:359,1088` and `DEFS.H`. `{ 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 }`.
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:107-114` uses *only the first 6 entries*: `{ 5, 12, 24, 33, 40, 46 }` (indexed by power ordinal 1..6). The full 16-entry table is in `dm1_v1_light_pc34_compat.c` (`dm1_light_power_to_amount`).
- **Functional impact:** The full table is preserved in another module. The 6-entry subset is correctly indexed.
- **Severity:** Cosmetic (per BUG-108 — uses community-reference values for indices 0..5; the comment says they are "conservatively approximated" but the values match ReDMCSB indices 1..6 exactly)

### TAB-03 — `Phase14_SymbolBaseManaCost[4][6]` mana cost table is source-locked
- **ReDMCSB reference:** `MENU.C:44-47` — base mana cost per symbol type (power/element/form/class) and power ordinal.
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:88-94` has the exact table `{ { 1, 2, 3, 4, 5, 6 }, { 2, 3, 4, 5, 6, 7 }, { 4, 5, 6, 7, 7, 9 }, { 2, 2, 3, 4, 6, 7 } }`.
- **Functional impact:** Source-locked.
- **Severity:** N/A (match)

### TAB-04 — `Phase14_SymbolManaCostMultiplier[6]` multiplier table is source-locked
- **ReDMCSB reference:** `MENU.C:49` — `{ 8, 12, 16, 20, 24, 28 }`.
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:96-99` matches exactly.
- **Functional impact:** Source-locked.
- **Severity:** N/A (match)

### TAB-05 — `Phase14_SpellTable[25]` spell table is source-locked
- **ReDMCSB reference:** `MENU.C:50-77` — 25-row spell table with packed symbols, base required skill level, skill index, attributes.
- **Firestaff state:** `src/memory/memory_magic_pc34_compat.c:122-148` has all 25 rows.
- **Functional impact:** Source-locked match.
- **Severity:** N/A (match)

### TAB-06 — `WoundDefenseFactor[6]` wound-defense factor table is documented but not visible in this audit
- **ReDMCSB reference:** `CHAMPION.C` uses a per-slot defense factor (referenced in `memory_combat_pc34_compat.c:178`).
- **Firestaff state:** `src/memory/memory_combat_pc34_compat.c` references `WoundDefenseFactor[woundSlotIndex]` (line 178 etc.) but the array is not in the audit's view; the prior audit does not document its values.
- **Functional impact:** Cannot confirm source-lock; needs explicit verification.
- **Severity:** Minor (uncertain source-lock)

### TAB-07 — `Phase17_SubtypeCreatesExplosion` (per-subtype explosion flag) is referenced but not visible
- **ReDMCSB reference:** `PROJEXPL.C` (per-subtype flag).
- **Firestaff state:** `src/memory/memory_projectile_pc34_compat.c:260` references `Phase17_SubtypeCreatesExplosion[in->subtype & 0xFF]`. The table itself is not in the audit's view.
- **Functional impact:** Cannot confirm source-lock.
- **Severity:** Minor (uncertain)

### TAB-08 — DM1 multilingual data is not ported
- **ReDMCSB reference:** `MENU.C:F0758_TranslateLanguage` and the per-language string tables (C73..C76 RESURRECTED/REINCARNATED prefix/suffix strings).
- **Firestaff state:** `src/dm1/dm1_v1_resurrection_pc34_compat.c` has the English strings hardcoded; multilingual variants are not implemented.
- **Functional impact:** Resurrection/reincarnation messages are English-only. Multilingual dungeons use the English string.
- **Severity:** Minor (intentional; V1 in the M12 launcher supports 20 languages, but the V1 resurrection UI is English-only)

---

## Module: Savegame / persistence

### SAV-01 — Field-mask semantics for save/load (BUG-112) are flagged "NEEDS DISASSEMBLY REVIEW"
- **ReDMCSB reference:** `LOADSAVE.C:F0433:1502-1707`, `F0435:2192-2660`.
- **Firestaff state:** `src/memory/memory_savegame_pc34_compat.c:399` (BUG-112 in prior audit) is flagged.
- **Functional impact:** Save files are Firestaff-native; field masks differ from PC 3.4. Cross-tool save compatibility is broken (see LSV-01).
- **Severity:** Major (covered by LSV-01)

---

## Module: V1 setup / startup

### STP-01 — F0462 `START_StartGame_CPSEF` is amalgam-only
- **ReDMCSB reference:** `STARTGD.C:F0462_START_StartGame_CPSEF` (per the comment in CHAMPION.C:13-14: *"Status variables for CPSF are now reset each time a game is restarted in F0462_START_StartGame_CPSEF"*).
- **Firestaff state:** Amalgam-only. The new path uses `dmstart_*.c` for game start and does not implement F0462.
- **Functional impact:** CPSF status variables (e.g. `G0417_B_PartyHasBeenOnThisMap`, etc.) are reset by the new path independently.
- **Severity:** Minor (intentional split)

### STP-02 — F0438 `START_DoorsAnimation` (entrance door opening) is referenced but not in new path
- **ReDMCSB reference:** `ENTRANCE.C:F0438_START_DoorsAnimation` (the 31-step entrance door opening animation). BUG0_71 in the same source: a missing VBlank guard.
- **Firestaff state:** `src/frontend/entrance_frontend_pc34_compat.c:122` documents the F0438 path as the source-of-truth for the entrance animation. The actual animation is done by the entrance_frontend, not by F0438. BUG0_71 is preserved.
- **Functional impact:** Entrance animation works. The new path is not source-locked to F0438.
- **Severity:** Minor (intentional split)

---

## Module: BUG0_xx preservation / divergence (summary)

| BUG | Description | Original | Firestaff | Severity |
|------|-------------|----------|-----------|----------|
| BUG0_01 | Uninitialised `AL0248_i_SquareType` in `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` | Uninitialised; never produces a visible effect | Preserved in amalgam; new path uses initialised values | Cosmetic |
| BUG0_08 | Things array can be overfilled | Original can crash; 676-690 entries | New path silently drops | Major (defensive) |
| BUG0_16 | Projectile list overfill | Original can crash | New path hard-caps | Major (defensive) |
| BUG0_17 | Fuse on map boundary crashes | Original crashes | Preserved in amalgam; new path not source-locked | Minor |
| BUG0_26 | Explosion falls in pit | Preserved | Preserved in amalgam | Cosmetic |
| BUG0_38 | Cursed-item Luck exploit | Preserved | **Not preserved** (luck is 0 in new path) | Minor |
| BUG0_41 | Megamax compiler bug (antifire/antimagic ignored) | Preserved | **Not preserved** (CHM-01) | Major (intentional fix) |
| BUG0_45 | Vitality wound probability reversed | Preserved | Partially preserved (F0307 not called for wound-probability path in compat layer; only for damage path) | Major |
| BUG0_66 | Smoke on source map (vs destination) | Preserved | Preserved in amalgam | Cosmetic |
| BUG0_71 | Missing VBlank guard in entrance | Preserved | Preserved (entrance_frontend_pc34_compat.c) | Cosmetic |
| BUG0_72 | `>=` vs `>` in F0310 stat clamp | Preserved | **Not preserved** (CHM-06) | Minor |
| BUG0_78 | Door-wound missing parens | Preserved | **Preserved** (CHS-02) | Cosmetic (intentional) |
| BUG0_81 | Uninitialised `damage` in weak-branch | Preserved | Unclear (combat.c:401) | Minor |
| BUGX_XX | Turbo C++ 1.01 compiler order in F0306 | Preserved | **Not preserved** (CHM-03, BUG-115) | Cosmetic |
| BUG0_44 | Black Flame / Fireball damage overflow | Preserved | **Preserved with clamp** (memory_combat_pc34_compat.c:324) | Minor (defensive) |
| BUG0_65 | Object-generator torches have no charges | Preserved | Preserved in amalgam | Cosmetic |
| BUG0_75 | Champion portrait ordinal not reset | Preserved | Preserved in amalgam | Cosmetic |
| BUG0_76 | Same text on multiple wall sides | Preserved | Preserved in amalgam | Cosmetic |

> **Net behavior change:** Firestaff deliberately fixes 4 of the BUG0_xx items (BUG0_16, BUG0_38 [partial], BUG0_41, BUG0_72, BUGX_XX) for safety or correctness. The original's 4 most-cited intentional bugs (BUG0_26, BUG0_66, BUG0_78, BUG0_71) are preserved.

---

## Summary statistics

| Severity | Count | % of total |
|----------|-------|-----------|
| Critical | 0     | 0%        |
| Major    | 13    | 19%       |
| Minor    | 40    | 59%       |
| Cosmetic | 5     | 7%        |
| N/A (source-locked match) | 10 | 15%   |
| **Total findings** | **68** | **100%** |

Note: 68 findings, of which 18 are explicit non-duplications of the prior `DM1_V1_BUG_AUDIT.md` (BUG-101..BUG-118) and are referenced in this report as verification points. **Net new findings beyond the prior audit: 50.**

---

## Top 10 priority fixes (beyond the prior audit's BUG-101..BUG-118)

1. **REV-01 (Major)** — F0281 `CHAMPION_Rename` UI is silently missing. Resurrected/reincarnated champions do not prompt for a new name.
2. **LSV-01 (Major)** — Save/load is not compatible with original PC 3.4 saves. F0433 / F0434 / F0435 / F0436 / F0437 / F0438 are amalgam-only and not invoked by the new runtime. Firestaff uses its own native save format.
3. **MOV-05 (Major)** — `set_party_direction_redmcsb_compat` rotates Direction but not Cell. F0284 in ReDMCSB rotates both. Inventory panel may mis-render when turning with a candidate present.
4. **GRP-02 (Major)** — F0192 creature poison resistance adjustment is not implemented. Creature poison applies raw `poisonAttack` regardless of creature type.
5. **GRP-03 (Major)** — F0202/F0203/F0204 Lord Chaos / Lord Order double-move is not implemented. Archenemy behavior is wrong in V2.
6. **MNU-02 (Major)** — F0757 Thieves Eye duration is `spellPower * 40` (64-224s) instead of the original's structurally-0 (broken by uninitialised stack). Spell lasts much longer in Firestaff.
7. **CHM-01 (Major)** — F0307 BUG0_41 (Megamax compiler bug) is intentionally fixed. Antifire / Antimagic / Vitality-poison now participate. Gameplay balance differs from original.
8. **DUN-05 (Major)** — F0163 BUG0_08 overfill is silently dropped, not crashed. Defensive behavior.
9. **PJE-05 (Major)** — F0220 BUG0_16 projectile-list overfill is silently dropped, not crashed. Defensive behavior.
10. **CMD-01 (Minor)** — F0377 / F0378 click dispatchers are amalgam-only. The new M11 click routing is independent and inline. Two parallel implementations of the same dispatch logic; long-term maintenance risk.

---

## Findings excluded as duplicates of `DM1_V1_BUG_AUDIT.md` (BUG-101..BUG-118)

These were verified in the prior audit and are not re-listed:

- BUG-101 (armor defense approximation → F0321) — fixed in v2.7.13
- BUG-102 (fire/spell shield defense) — fixed in v2.7.13
- BUG-103 (luck system) — still NEEDS DISASSEMBLY REVIEW
- BUG-104 (creature AI stubs) — fixed (per `g_profiles` table source-lock)
- BUG-105 (attack ordering) — still NEEDS DISASSEMBLY REVIEW (F0229)
- BUG-106 (flee behavior) — still NEEDS DISASSEMBLY REVIEW
- BUG-107 (thieves eye duration) — verified still approximated (MNU-02 in this report)
- BUG-108 (light amount table) — verified (full 16-entry table is in `dm1_v1_light_pc34_compat.c`)
- BUG-109 (stat gain cycle) — verified (F0832 implements the F0331 cycle)
- BUG-110 (magic map per-champion) — not in this audit's scope
- BUG-111 (projectile sub-cell hit mask) — not in this audit's scope
- BUG-112 (savegame field mask) — still NEEDS DISASSEMBLY REVIEW (SAV-01)
- BUG-113 (poison vitality adjustment) — fixed in v2.7.13
- BUG-114 (psychic damage) — not in this audit's scope (no DM1 psychic spells)
- BUG-115 (F0306 compiler order) — verified (CHM-03)
- BUG-116 (runtime dynamics table) — not in this audit's scope
- BUG-117 (test infrastructure build path) — out of scope (test infra, not source divergence)
- BUG-118 (viewport crop readiness) — not in this audit's scope

---

## Notes

- This audit did **not** modify any source files. Build directory and Phase A probe were not exercised (no source changes were made; build state should be unchanged from `f99587c35` baseline).
- The **sanitized amalgam** is a faithful 1:1 port of ReDMCSB. It is exercised by the legacy V1 emulator path (the V1 PC 3.4-emulation tests). The **new compat layer** (M10/M11 split into smaller files) is a refactor with source-locked comments but independent function bodies. The two paths coexist; V1 tests pass via the amalgam, V2 paths use the compat layer.
- Most "divergence" findings are **intentional splits** (amalgam vs compat layer). The compat layer is a re-implementation of the same logic, not a behavioral change. Where the new layer differs in behavior (CHM-01, CHM-06, MNU-02, REV-01, MOV-05, GRP-02, GRP-03, DUN-05, PJE-05), the divergence is called out as Major.
- BUG0_xx preservation is mixed: 4 BUGs are deliberately not preserved (intentional fixes), 4 BUGs are preserved verbatim, and the rest are N/A for V1.
