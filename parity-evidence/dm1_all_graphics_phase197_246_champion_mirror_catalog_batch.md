# DM1 all-graphics phases 197–246 — champion mirror catalog batch

## Goal

Create a stable caller-owned catalog over real DUNGEON.DAT champion mirror records so V1 UI/recruitment can consume source records by ordinal/name/title/TextString index without re-decoding text at every click.

## Passes / coverage

This 50-pass batch adds a catalog layer and gates the following behavior groups:

1. catalog record struct with TextString index, mirror ordinal, raw champion carry-through state, display name/title
2. catalog container for up to 32 records
3. build catalog from real DUNGEON.DAT TextStrings
4. preserve catalog count
5. find record by ordinal
6. find record by display name
7. find record by display title
8. count catalog records by sex
9. count titled records
10. count untitled records
11. verify titled + untitled = total
12. verify packed source names are unique
13. get display name by ordinal
14. get display title by ordinal
15. get original TextString index by ordinal
16. has-name helper
17. unknown-name rejection
18. has-title helper
19. first ordinal by sex
20. last ordinal by sex
21. next ordinal helper
22. previous ordinal helper
23. individual catalog record validation
24. full catalog validation
25. recruit by catalog ordinal
26. recruit by catalog display name
27. idempotent recruit-if-absent by ordinal
28. idempotent recruit-if-absent by display name
29. copy raw champion source record by ordinal
30. resolve mirror ordinal from original TextString index
31–50. regression coverage against prior mirror parse/recruitment helpers and M11 draw/test gates

## Added API

```c
struct ChampionMirrorRecord_Compat
struct ChampionMirrorCatalog_Compat
F0652_CHAMPION_BuildMirrorCatalog_Compat
F0653_CHAMPION_MirrorCatalogFindByOrdinal_Compat
F0654_CHAMPION_MirrorCatalogFindByName_Compat
F0655_CHAMPION_MirrorCatalogFindByTitle_Compat
F0656_CHAMPION_MirrorCatalogCountBySex_Compat
F0657_CHAMPION_MirrorCatalogCountTitled_Compat
F0658_CHAMPION_MirrorCatalogCountUntitled_Compat
F0659_CHAMPION_MirrorCatalogNamesUnique_Compat
F0660_CHAMPION_MirrorCatalogGetName_Compat
F0661_CHAMPION_MirrorCatalogGetTitle_Compat
F0662_CHAMPION_MirrorCatalogGetTextStringIndex_Compat
F0663_CHAMPION_MirrorCatalogHasName_Compat
F0664_CHAMPION_MirrorCatalogHasTitle_Compat
F0665_CHAMPION_MirrorCatalogFirstOrdinalBySex_Compat
F0666_CHAMPION_MirrorCatalogLastOrdinalBySex_Compat
F0667_CHAMPION_MirrorCatalogOrdinalAfter_Compat
F0668_CHAMPION_MirrorCatalogOrdinalBefore_Compat
F0669_CHAMPION_MirrorCatalogRecordValid_Compat
F0670_CHAMPION_MirrorCatalogAllRecordsValid_Compat
F0671_CHAMPION_MirrorCatalogRecruitOrdinal_Compat
F0672_CHAMPION_MirrorCatalogRecruitName_Compat
F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat
F0674_CHAMPION_MirrorCatalogRecruitNameIfAbsent_Compat
F0675_CHAMPION_MirrorCatalogCopyRecord_Compat
F0676_CHAMPION_MirrorCatalogGetOrdinalForTextStringIndex_Compat
```

## Gate

```text
PASS: Mirror catalog builds 24 source champion records
PASS: Mirror catalog count stores 24 records
PASS: Mirror catalog finds ordinal 0 at record 0
PASS: Mirror catalog finds STAMM by display name
PASS: Mirror catalog finds BLADECASTER by display title
PASS: Mirror catalog sex counts sum to catalog count
PASS: Mirror catalog titled/untitled counts sum to catalog count
PASS: Mirror catalog packed names are unique
PASS: Mirror catalog gets display name by ordinal
PASS: Mirror catalog gets display title by ordinal
PASS: Mirror catalog gets original TextString index by ordinal
PASS: Mirror catalog has-name helper distinguishes known and unknown names
PASS: Mirror catalog has-title helper finds BLADECASTER
PASS: Mirror catalog first/last ordinal by sex helpers return records
PASS: Mirror catalog before/after ordinal helpers walk records
PASS: Mirror catalog validates individual source record
PASS: Mirror catalog validates all source records
PASS: Mirror catalog recruits by ordinal
PASS: Mirror catalog recruits by display name
PASS: Mirror catalog recruit ordinal if-absent is idempotent
PASS: Mirror catalog recruit name if-absent is idempotent
PASS: Mirror catalog copies champion source record by ordinal
PASS: Mirror catalog resolves ordinal from TextString index
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire actual V1 mirror click/recruitment UI to this catalog layer
- decode source stat/skill/inventory payloads into runtime state in separate source-backed passes
