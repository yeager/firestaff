# Firestaff v2.7.23 — 2026-06-15

DM1 V1 hero subtitle (Group 7) is now visible, F0192 poison cloud
resistance-adjusted attack re-applied, CSB V1 closes all 3 remaining
OPEN-OMFATTANDE gaps (Champions 3 / Dungeon 4 / Graphics 6).

## DM1 V1

- **M12 extras subtitle (Group 7)**: the subtitle drawn in
  BESTIARY / ITEM ENCYCLOPEDIA / SCREENSHOT GALLERY hero areas is
  now redrawn on top of `m12_apply_graphics_overlay` (which
  BLACK-fills the mode 1 frame at y=34-680). Subtitle text is now
  visible in all three views. Verified via
  `firestaff_m12_extras_views_visual_capture`: 95 / 91 / 181 white px
  in subtitle area respectively.

- **F0192 poison cloud resistance-adjusted attack**: restored
  the `F0192_GROUP_GetResistanceAdjustedPoisonAttack` call that
  v1 had silently removed. ReDMCSB PROJEXPL.C:863 reassigns
  `L0530_i_Attack` from the F0192 result before calling F0191.
  Closes the `dm1_v1_projectile_explosion_render` sub-failure
  (poison cloud attack 96 with Mummy resistance 5 now correctly
  produces rawAttackValue 4, not 3).

- **Build hygiene**: untracked 1691 build artifacts in
  `builds/n2-build/` that had been committed before the
  `.gitignore 'builds/'` entry was added. Local builds continue
  to work (artifacts remain on disk for incremental builds).

## CSB V1

- **Champions GAP 4 (Left-Click Inventory, CHANGE7_28)**: dedicated
  regression test `csb_v1_champions_left_click_inventory_pc34_compat`
  (10/10 PASS) covering default-disabled, CSB-mode C125..C128
  mapping, out-of-range slots, and toggling.

- **Dungeon GAP 4 (Compressed dungeon, DECOMPDU.C F0455)**:
  source-faithful port of the bit-packed dungeon decompressor
  (MEDIA481 portable C path) using the 4-most-common /
  16-less-common / literal prefix-code scheme. Includes a matching
  encoder for round-trip testing, a bounded grid wrapper (up to 24
  levels of 64x64), a `CSB_DECOMPDU_ERR_*` enum, and bounds checks
  the 68k original lacked. Test `csb_v1_decompdu_pc34_compat`
  (32/32 PASS).

- **Graphics GAP 6 (CHANGE7_16 Code-to-Assembly)**: documents why
  a faithful 68k-asm port is impossible/moot in C (ReDMCSB source
  uses 68k-specific instructions like `movem` / `lea` / `dbra` that
  don't translate to ANSI C; the asm is hand-tuned for 8 MHz 68000
  with zero wait states; C code already runs ~5x faster on modern
  hardware). Ships C-only `__attribute__((hot))` perf shims for
  the three inner loops (blit-fast-path, sensor-dispatch,
  end-of-frame tick). Test `csb_v1_graphics_change7_16_pc34_compat`
  (22/22 PASS).

- **Champions GAP 3 (HoC delta, Champion Transfer/Import)**:
  real CSB v2.0 / v2.1 save importer that maps the CSB roster
  record into `CSB_V1_PartyState` / `CSB_V1_Champion`, applies the
  CHANGE7_24 reincarnation stat-cap on import, and stamps the
  party (`ImportSource=3`) so re-edits don't re-import.
  Test `csb_v1_save_import_path_pc34_compat` (35/35 PASS).

## Docs

- `docs/FINAL_GAPS.md` v2.7.23 snapshot — DM1: all 21 BUG items
  + Group 7 verified FIXED in HEAD `9f32b8a1`.
- `docs/FINAL_CSB_GAPS.md` v2.7.23 snapshot — CSB: 21/27 gaps
  closed (13 FIXED, 5 ALREADY-DONE, 0 OPEN-BOUNDED, 3 OPEN-OMFATTANDE
  all closed this release).

## Test regressions

- CSB suite: 114/114 PASS
- DM1 V1 critical: 11/11 PASS (creature_ai_behavior,
  magic_thieves_eye, champion_needs, f0128_viewport, f0306_stamina,
  savegame_native_export, hall_of_champions, f0192_projectile,
  monster_poison_cloud, etc.)
- Phase A probe: 23/23 invariants PASS
- m12_extras_views_smoke: PASS (7s)
- csb_v1_champions_left_click_inventory: PASS (10/10)
- csb_v1_decompdu: PASS (32/32)
- csb_v1_graphics_change7_16: PASS (22/22)
- csb_v1_save_import_path: PASS (35/35)
- csb_v1_graphics_extras (smoke for Graphics 2 / Champions 4 /
  Dungeon 6): PASS

## Known issues / future work

- DM1 FINAL_GAPS Group 8: 68 functional-divergence findings
  (mostly minor, source-locked clarifications) — bounded subset
  picked in follow-up release.
- DM2 / CSB / Nexus / Theron: separate milestones, not
  considered gaps for DM1 V1 parity.
