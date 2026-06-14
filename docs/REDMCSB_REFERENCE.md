# ReDMCSB Canonical Reference

The canonical reverse-engineered source code for *Dungeon
Master* and *Chaos Strikes Back* is **ReDMCSB**, hosted at:

- Community page: <http://dmweb.free.fr/community/redmcsb/>
- WIP 2021-02-06 archive (local copy used for the Firestaff
  implementation): `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/`
- Authoritative bugs-and-changes list (search this for
  every `BUG0_xx` / `CHANGE7_xx` / `CHANGE8_xx` reference):
  <http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm>
- Forum discussion:
  <https://www.dungeon-master.com/forum/viewtopic.php?t=29805>

## Identifier convention

Each entry has a unique name: `BUG0_NN` for bugs,
`CHANGE<version>_<index>_<category>` for changes.

- **0..8** = game version where it appeared:
  - 0: DM Atari ST 1.0a EN
  - 1: DM Atari ST 1.0b EN
  - 2: DM Atari ST 1.1 EN
  - 3: DM Atari ST 1.2 EN
  - 4: DM Atari ST 1.2 GE
  - 5: DM Atari ST 1.3a FR
  - 6: DM Atari ST 1.3b FR
  - 7: CSB Atari ST 2.0 EN
  - 8: CSB Atari ST 2.1 EN
- **categories:** FIX (bug fix), OPTIMIZATION, LOCALIZATION,
  IMPROVEMENT (most new features)

## Currently relevant to Firestaff

  - **BUG0_03** (VBL handler glitch) — DUNVIEW palette switching
    fails under heavy load; fixed in CSB 2.0 (CHANGE7_01_FIX).
    → CSB V1 Graphics GAP 1.
  - **BUG0_04** (creature color conflicts) — DUNVIEW; no fix.
    → CSB V1 Graphics GAP 4.
  - **BUG0_05** (champion portrait wall sensor visible on all
    sides) — DUNVIEW; no fix.
  - **BUG0_06** (projectile bitmap crop glitch) — DUNVIEW;
    no fix.
  - **BUG0_07** (explosion bitmap crop glitch) — DUNVIEW;
    fixed in DM 1.1+ (CHANGE2_02_FIX).
  - **BUG0_09 / BUG0_10** (dungeon square event triggers) — DUNGEON.C;
    CSB V1 specific fixes (CHANGE7_17, CHANGE7_18).
  - **BUG0_56** (only fixed in DM 1.3b FR, not in DM1 PC 3.4
    that Firestaff ships with).
  - **BUG0_67 / BUG0_69** (group AI + teleporter memory
    corruption with Lord Chaos / Grey Lord).
  - **CHANGE2_06_FIX, CHANGE2_19_FIX, CHANGE3_19_FIX,
    CHANGE5_01_FIX, CHANGE7_14_FIX** (useless code removed).
  - **CHANGE7_01_FIX** (VBL re-entrancy fix) — closes BUG0_03.
  - **CHANGE7_15** (wall drawing optimization) — DUNVIEW.C.
  - **CHANGE7_17, CHANGE7_18** (dungeon square event fixes)
    — CSB V1 DUNGEON.GAP 2.
  - **CHANGE7_19** (BUG0_69 Lord Chaos/Grey Lord teleporter
    fix) — CSB V1 DUNGEON.GAP 6.
  - **CHANGE7_20** (projectile speed normalization) — CSB
    V1 Combat GAP 1, **IMPLEMENTED in `6967b4f94`**.
  - **CHANGE7_21** (endgame UI) — CSB V1 Dungeon GAP 2.
  - **CHANGE7_23** (version checker sensor) — CSB V1
    Dungeon GAP 3, **IMPLEMENTED in `9f50e167f`**...wait,
    that was level count.  Version checker was
    `c3bf76b11` was the M12 fix.  Let me re-check.
  - **CHANGE7_24** (reincarnation penalty) — CSB V1 Champions
    GAP 2, **IMPLEMENTED in `d3ccfda56`**.
  - **CHANGE7_28** (left-click inventory access) — CSB V1
    Champions GAP 4.
  - **CHANGE7_29 / CHANGE8_12** (save game combat state) —
    CSB V1 Combat GAP 5.
  - **CHANGE8_02** (60-tick delay multiplier) — endgame
    curtain.
  - **CHANGE8_06** (engine version 21 hardcoded for CSB
    2.1) — CSB V1 Dungeon GAP 3.

## Local working set

The Firestaff build pins against:
  `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

This directory contains 36 different .C files split by
subsystem.  Most CSB V1 / DM1 V1 gap work references one of
these directly:

  CHAMPION.C, CHAMPRST.C, GROUP.C, PROJEXPL.C, MOVESENS.C,
  DUNGEON.C, DUNVIEW.C, MAGIC.C, REVIVE.C, TIMELINE.C,
  GAME_LOOP.C (merged with GAMELOOP.C), ENDGAME.C,
  SENSORS.C (split from MOVESENS.C), STARTUP1.C,
  CLIKCHAM.C, ENTRANCE.C, PANEL.C, CHEST.C, DECOMPDU.C,
  VBL.C (split from BASE.C for Atari), COMBAT.C
  (split from CHAMPION.C), etc.
