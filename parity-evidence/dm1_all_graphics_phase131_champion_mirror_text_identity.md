# DM1 all-graphics phase 131 — parse champion mirror text identity

## Problem

Pass 130 added raw `Champion.Title[20]` carry-through and made the endgame overlay prefer those bytes, but the real recruitment/load path still needs a source-shaped way to populate identity from DUNGEON.DAT mirror text.

DM1 champion mirror text is encoded in the dungeon text table as:

```text
NAME|TITLE||SEX|...
```

Examples from `verification-m10/dungeon-text/dungeon_text_probe.md` include:

```text
SYRA|CHILD OF NATURE||F|...
SONJA|SHE DEVIL||F|...
ALEX|ANDER||M|...
NABI|THE PROPHET||M|...
```

## Change

Added a small pure-data parser:

```c
F0606_CHAMPION_ParseMirrorTextIdentity_Compat(text, champ)
```

It writes only source identity bytes:

- `Champion.Name[8]`
- `Champion.Title[20]`

It does **not** invent stats, portrait data, lifecycle state, inventory, UI state, or recruitment behavior. Those stay owned by their existing paths.

## Gate

Extended the M10 movement/champion probe with source-format checks:

```text
PASS: Champion mirror text identity parser accepts NAME|TITLE||... source format
PASS: Champion mirror parser packs source Name[8]
PASS: Champion mirror parser packs source Title[20]
Status: PASS
```

Regression gates:

```text
firestaff_m11_game_view_probe: 443/443 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

## Remaining gaps

- wire the parser into real mirror recruitment / champion load when that path is implemented
- remove the endgame canonical-title fallback after real party state consistently carries title bytes
- continue replacing bounded bridge state with raw source state where available
