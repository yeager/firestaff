# GAP C01-C24: Hall of Champions Champion Stats

## Status

**RESOLVED -- champion recruitment is mirror-record backed, not flat class defaults.**

The old note said Hall of Champions recruitment used `m11_stats_add_champion()` and therefore gave every champion the same 100/100/50 vitals and 30/30/30 stat defaults. That was a stale audit of the standalone stat helper, not the runtime recruitment path.

## Source Contract

ReDMCSB `REVIVE.C` `F0280_CHAMPION_AddCandidateChampionToParty` decodes the selected champion's mirror text:

- `REVIVE.C:227-234`: health, stamina, and mana are decoded from the mirror text and assigned to current and maximum rows before the resurrect/reincarnate panel opens.
- `REVIVE.C:235-245`: statistic rows are decoded from the same mirror record, with PC 3.4 clamping non-luck stats to at least 30 and luck minimum set to 10.
- `REVIVE.C:811-833`: reincarnation later mutates the already-materialized record; it is not the source of the base champion stats.

There is no source-backed path that derives starting champion stats from `G0243_as_Graphic559_CreatureInfo`, creature aspect bits, or a class-bonus table. Those belong to creature rendering/AI or later skill progression, not Hall mirror recruitment.

## Firestaff Runtime

Firestaff's DM1 runtime uses the `memory_champion_state_pc34_compat` mirror catalog:

- `F0606_CHAMPION_ParseMirrorTextIdentity_Compat()` parses decoded mirror text into packed name/title, sex, raw encoded fields, HP/stamina/mana, and attribute current/maximum rows.
- `F0652_CHAMPION_BuildMirrorCatalog_Compat()` collects valid mirror records from `DUNGEON.DAT` text strings.
- `F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat()` copies the full decoded `ChampionState_Compat` record into `PartyState_Compat`.
- `M11_GameView_RecruitChampionByMirrorOrdinal()` delegates to `F0673` when a C127 portrait/mirror route is accepted.

The separate `m11_stats_add_champion()` helper still initializes synthetic standalone test champions with flat values, but it is not the source-faithful Hall of Champions recruitment path.

## Verification

`dm1_v1_mirror_catalog_champion_stats_pc34_compat` proves the current runtime contract with a synthetic mirror record:

- decoded HP/stamina/mana survive recruitment;
- decoded strength/dexterity/wisdom/vitality/antimagic/antifire survive recruitment;
- the recruited party slot gets the catalog text-string index as its portrait index;
- the record is not replaced by the old flat 100/100/50 and 30-stat defaults.

`dm1_v1_champion_stats_load_source_lock` remains the separate source-lock test for load, movement cadence, stat color, and panel formatting helpers.

## Remaining Work

No gameplay fix is required for the old C01-C24 champion-stat claim. Remaining champion-panel work belongs to original runtime capture pairing and broader panel pixel evidence, not to adding class-derived stat bonuses.

Creature aspect-bit audits such as Black Flame, Vexirk, Materializer, Oitu, or Red Dragon should live under the creature/AI gap docs, not under Hall of Champions champion stats.
