# DM1 all-graphics phases 162–171 — champion mirror UI bridge helper batch

## Goal

Prepare the real V1 mirror click/recruitment UI flow by exposing safe, deterministic helpers around the source mirror data. This batch does not decode gameplay stats yet; it makes name/title/ordinal/slot handling reliable for the next UI wiring pass.

## Passes

- **162** — pack plain C champion names into source-shaped `Name[8]`.
- **163** — pack plain C titles into source-shaped `Title[20]`.
- **164** — find mirror TextString records by plain C name strings.
- **165** — recruit identity-only champion records by plain C name strings.
- **166** — get mirror ordinal by packed `Name[8]`.
- **167** — get mirror ordinal by packed `Title[20]`.
- **168** — expose next-free party slot helper.
- **169** — next-free slot helper skips occupied slots.
- **170** — expose full-party helper based on real occupied slots.
- **171** — expose public duplicate-name helper for party state.

## Added helpers

```c
F0618_CHAMPION_PackName_Compat(name, outName)
F0619_CHAMPION_PackTitle_Compat(title, outTitle)
F0620_CHAMPION_FindMirrorTextStringByNameString_Compat(things, name)
F0621_PARTY_AddChampionFromMirrorNameString_Compat(things, name, party)
F0622_CHAMPION_GetMirrorOrdinalByName_Compat(things, packedName)
F0623_CHAMPION_GetMirrorOrdinalByTitle_Compat(things, packedTitle)
F0624_PARTY_GetNextFreeChampionSlot_Compat(party)
F0625_PARTY_IsFull_Compat(party)
F0626_PARTY_ContainsChampionName_Compat(party, packedName)
```

## Gate

```text
PASS: Champion name pack helper pads source Name[8]
PASS: Champion title pack helper pads source Title[20]
PASS: Mirror finder accepts plain C champion name string
PASS: Party can recruit champion identity by plain C name string
PASS: Mirror ordinal lookup works by packed Name[8]
PASS: Mirror ordinal lookup by Title[20] matches same champion
PASS: Next-free party slot helper returns slot 0 for empty party
PASS: Next-free party slot helper skips occupied slot
PASS: Party full helper detects all champion slots occupied
PASS: Public party duplicate helper finds recruited packed Name[8]
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire V1 mirror click/recruitment UI to these helpers
- decode source encoded stat/skill/inventory fields into runtime structures
