# DM1 all-graphics phases 142–151 — champion mirror recruitment identity batch

## Goal

Wire the real DUNGEON.DAT champion mirror TextString parser closer to actual party state, without inventing gameplay stats or decoding the encoded source fields prematurely.

## Passes

- **142** — count real champion mirror TextString records in DUNGEON.DAT.
- **143** — find a champion mirror TextString by packed `Name[8]`.
- **144** — reject unknown packed names.
- **145** — add identity-only recruitment from a DUNGEON.DAT TextString index.
- **146** — mark recruited slot present and increment `championCount`.
- **147** — select first recruited champion as `activeChampionIndex`.
- **148** — copy source `Name[8]` and `Title[20]` into party state.
- **149** — copy source `sex` and encoded stat field into party state.
- **150** — recruit by packed `Name[8]` and reject duplicate champion names.
- **151** — preserve party map position/facing and reject full-party recruitment.

## Added helpers

```c
F0608_CHAMPION_CountMirrorTextStrings_Compat(things)
F0609_CHAMPION_FindMirrorTextStringByName_Compat(things, packedName)
F0610_PARTY_AddChampionFromMirrorTextString_Compat(things, textStringIndex, party)
F0611_PARTY_AddChampionFromMirrorName_Compat(things, packedName, party)
```

These helpers copy only source identity/carry-through fields:

- `Name[8]`
- `Title[20]`
- `sex`
- encoded mirror stat/skill/inventory text fields

They do **not** decode source stats/skills/inventory into gameplay state yet.

## Gate

```text
PASS: DUNGEON.DAT mirror parser counts many champion records
PASS: DUNGEON.DAT mirror parser finds STAMM by packed Name[8]
PASS: DUNGEON.DAT mirror parser rejects unknown packed Name[8]
PASS: Party can recruit champion identity from DUNGEON.DAT TextString index
PASS: Mirror recruitment marks first party slot present and increments count
PASS: Mirror recruitment selects first recruited champion as active
PASS: Mirror recruitment copies source name/title into party state
PASS: Mirror recruitment copies source sex and encoded stat field
PASS: Party can recruit champion identity by packed Name[8]
PASS: Mirror recruitment rejects duplicate champion name
PASS: Mirror recruitment preserves party map position and facing
PASS: Mirror recruitment rejects full party
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- connect these helpers to actual mirror click/recruitment UI flow
- decode encoded source stat/skill/inventory fields into runtime structures in separate source-backed passes
- remove synthetic test-only party setup once real recruitment path drives V1 endgame state
