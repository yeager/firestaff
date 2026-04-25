# DM1 all-graphics phases 152–161 — champion mirror collection + robust recruitment batch

## Goal

Strengthen real DUNGEON.DAT champion mirror handling before wiring UI click/recruitment. This batch stays source-carry-only: it enumerates and locates real mirror records and improves identity-only party insertion, without decoding stats/skills/inventory into gameplay values.

## Passes

- **152** — collect all real champion mirror TextString records into caller-owned arrays.
- **153** — assert DM1 DUNGEON.DAT exposes exactly 24 champion mirror records.
- **154** — resolve mirror ordinal → original TextString index.
- **155** — verify mirror ordinal 0 carries source champion `DAROOU`.
- **156** — reject out-of-range mirror ordinals.
- **157** — locate champion mirrors by packed `Title[20]`.
- **158** — count real mirror records by sex and verify `M + F == total`.
- **159** — expose packed-name existence helper and distinguish name/title fields.
- **160** — recruit identity by mirror ordinal.
- **161** — improve recruitment insertion: duplicate detection scans all present slots, and new recruits fill the first empty slot instead of blindly appending to `championCount`.

## Added helpers

```c
F0612_CHAMPION_CollectMirrorTextStrings_Compat(things, outChampions, outTextStringIndices, capacity)
F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(things, mirrorOrdinal)
F0614_PARTY_AddChampionFromMirrorOrdinal_Compat(things, mirrorOrdinal, party)
F0615_CHAMPION_FindMirrorTextStringByTitle_Compat(things, packedTitle)
F0616_CHAMPION_CountMirrorTextStringsBySex_Compat(things, sex)
F0617_CHAMPION_HasMirrorTextStringByName_Compat(things, packedName)
```

## Gate

```text
PASS: Mirror collection count matches mirror record counter
PASS: DM1 DUNGEON.DAT exposes 24 champion mirror records
PASS: Mirror ordinal 0 resolves to collected TextString index
PASS: Mirror ordinal 0 carries first source champion name DAROOU
PASS: Mirror ordinal helper rejects out-of-range ordinal
PASS: Mirror title finder locates BLADECASTER title
PASS: Mirror sex counters sum to total champion records
PASS: Mirror name existence helper distinguishes packed names from titles
PASS: Party can recruit champion identity by mirror ordinal
PASS: Mirror recruitment detects duplicate name outside compact count range
PASS: Mirror recruitment fills first empty slot instead of blindly appending
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- connect identity-only recruitment to actual V1 mirror click/recruitment UI flow
- decode encoded stat/skill/inventory fields into runtime structures in separate source-backed passes
