# DM1 all-graphics phase 130 — carry Champion.Title into V1 endgame

## Problem

Pass 128 drew endgame champion title text, but because `ChampionState_Compat` only carried packed names/portraits, the renderer used a bounded canonical-name title table. That narrowed visuals but was not the real source data path.

Source structs in `DEFS.H` include:

```c
char Name[8];
char Title[20];
```

`ENDGAME.C:F0444_STARTEND_Endgame` draws `Champion.Title` directly after `Champion.Name`.

## Change

Added raw title carry-through to `ChampionState_Compat`:

- `CHAMPION_TITLE_LENGTH = 20`
- `unsigned char title[CHAMPION_TITLE_LENGTH]`
- champion serialization now stores title bytes in the previous reserved span `[136..155]`
- deserialization restores the same 20 bytes
- V1 endgame title renderer now prefers `champion.title` when present and only falls back to the bounded canonical title table for older/synthetic state

This keeps the existing 256-byte champion serialized size and uses reserved bytes rather than expanding the format.

## Gate

Added invariant:

- `INV_GV_165K` — V1 endgame prefers raw `Champion.Title` bytes when present

Existing serialization probe was updated to round-trip a title byte sample.

```text
PASS INV_GV_165I V1 endgame prints source champion title after name
PASS INV_GV_165K V1 endgame prefers raw Champion.Title bytes when present
# summary: 443/443 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- populate `Champion.Title[20]` from raw DUNGEON.DAT champion data instead of relying on synthetic/manual test state
- remove canonical fallback once all real champion loads carry title bytes
- remove skill-level fallback once lifecycle/source state is complete
- ignore object modifiers exactly, if/when item modifiers are modeled in skill level computation
- endgame timing/music/restart loop
- original overlay comparison captures
