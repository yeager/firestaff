# DM1 all-graphics phases 172–196 — champion mirror presentation/state helper batch

## Goal

Prepare actual V1 mirror UI/recruitment wiring by adding presentation, validation, lookup, and slot-state helpers around real DUNGEON.DAT champion mirror records. This still avoids decoding gameplay stats/skills/inventory semantics.

## Passes

- **172** — packed-field trim length helper.
- **173** — unpack source `Name[8]` to C display string.
- **174** — unpack source `Title[20]` to C display string.
- **175** — report trimmed encoded stats field length.
- **176** — report trimmed encoded skills field length.
- **177** — report trimmed encoded inventory field length.
- **178** — validate uppercase source-encoded mirror fields.
- **179** — reject lowercase/non-source encoded field data.
- **180** — validate all encoded mirror payload fields together.
- **181** — get display name by mirror ordinal.
- **182** — get display title by mirror ordinal.
- **183** — find mirror TextString by plain C title string.
- **184** — count occupied party champion slots.
- **185** — query whether a specific champion slot is occupied.
- **186** — recount occupied slots from actual present flags.
- **187** — query whether active champion index points at a present slot.
- **188** — set active champion only if target slot is present.
- **189** — lookup party slot by packed `Name[8]`.
- **190** — lookup party slot by plain C name.
- **191** — idempotent add-if-absent by mirror ordinal.
- **192** — add-if-absent by plain C name.
- **193** — get mirror ordinal by plain C name.
- **194** — get mirror ordinal by plain C title.
- **195** — get TextString index by plain C name/title aliases.
- **196** — clear recruited champion slot and update party state.

## Notable fix inside batch

Real decoded DUNGEON.DAT champion records use newline separators, not only `|`. One old branch still used `strchr(..., '|')` for the skill/inventory boundary, which swallowed the first inventory byte into the skills field. Fixed it to use the same source separator helper as the rest of the parser.

## Gate

```text
PASS: Packed trim helper returns source name length without pad spaces
PASS: Champion unpack-name helper returns display name string
PASS: Champion unpack-title helper returns display title string
PASS: Encoded mirror field validator accepts uppercase source field
PASS: Encoded mirror field validator rejects lowercase/non-source field
PASS: Mirror ordinal can produce display name string
PASS: Mirror ordinal can produce display title string
PASS: Party recount helper reports recruited champion
PASS: Party active helper sees first recruited champion
PASS: Add-if-absent by mirror ordinal is idempotent for existing champion
PASS: Mirror ordinal by C name returns STAMM ordinal
PASS: Mirror ordinal by C title returns STAMM ordinal
PASS: TextString index by C name matches name finder
PASS: TextString index by C title matches title finder
PASS: Clear champion slot helper removes recruited slot
Status: PASS
```

Regression:

```text
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire V1 mirror click/recruitment UI to these helpers
- decode source encoded stat/skill/inventory payloads into runtime structures in separate source-backed passes
