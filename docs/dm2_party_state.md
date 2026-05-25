# DM2 V1 — Party State Persistence

## What's Persisted

The party (champion squad) is persisted entirely in the save file. Here's what's saved and how:

## Champion Base Stats (261 bytes each × up to 4 champions)

`struct Champion` (261 bytes) includes:
- **Names**: firstName[8], lastName[16]
- **Position/orientation**: absoluteDirection, squadPosition
- **HP/Stamina/Mana**: cur/max HP, stamina, mana
- **Poison/Runes**: PoisonValue, runesCount, spelledRunes[4]
- **Attributes**[7][2]: per-attribute cur/max pairs (luck, str, dex, wiz, vit, etc.)
- **Food/Water**: food, water
- **Combat state**: handCommand[2], handCooldown[2], handDefenseClass[2]
- **Timers and flags**: timerIndex, damageSuffered, heroFlag, bodyFlag
- **Inventory slot refs**: `inventory[INVENTORY_MAX_SLOT]` — ObjectID[30] handles to item records in the DB pool

## Inventory: The Item Record Chain

Each champion's 30 inventory slots (`inventory[30]`) are **ObjectID handles**, not inline item data. At save time, for each champion and each of their 30 slots:

```cpp
for (bp0e = 0; bp0e < glbChampionsCount; bp0e++) {
    ObjectID *bp04 = glbChampionSquad[bp0e].inventory;
    for (i16 bp12 = 0; bp12 < 30; bp12++) {
        if (WRITE_RECORD_CHECKCODE(*(bp04++), 0, 0) != 0)
            goto _14fa;
    }
}
```

`WRITE_RECORD_CHECKCODE` writes the item's DB record link, then follows the record chain (objects can link to other objects). This means inventory items are saved as **references into the DB pool** (`glbDBObjectData[dbXXX]`), not as inline data. The actual item records (type, charges, enchantments, etc.) live in the 16 database record pools and are saved separately.

## Leader Hand Possession

A single record link for what the party leader has in their active hand:
```cpp
if (WRITE_RECORD_CHECKCODE(glbLeaderHandPossession.object, 0, 0) != 0)
    goto _14fa;
```
`glbLeaderHandPossession` is a `struct LeaderPossession` (22 bytes) holding an ObjectID.

## How Champions Are Serialized in Save

The champion squad is SUPPRESS-encoded (bit-level RLE) as a block:
```cpp
SUPPRESS_WRITER(glbChampionSquad, _4976_3992, 261, glbChampionsCount)
```
- 261 bytes per champion
- `_4976_3992` is the per-field write mask (tells SUPPRESS which bits to actually store)
- Written for `glbChampionsCount` champions (1–4)

## What's NOT Directly in Champion Struct (Saved Elsewhere)

| Data | Where Saved |
|------|-------------|
| Champion class/kit | In champion creation record (part of DB) |
| Experience/level | In champion record in DB pool |
| Champion-specific quest state | In `glbIngameGlobVarFlags/Bytes/Words` |
| Per-champion spell memorizations | In `glbGlobalSpellEffects` or DB |
| Champion position in squad (TL/TR/BR/BL) | In `absoluteDirection/squadPosition` fields |

## Load: SUPPRESS_READER with Fill=1

On load, `SUPPRESS_READER` is used with a `fill` parameter set to `1`:
```cpp
SUPPRESS_READER(glbChampionSquad, _4976_3992, 261, glbChampionsCount, 1)
```
The `fill=1` means: if a field is absent/masked in the save, fill it with a default value (ensures champions are in a valid state even if some fields couldn't be read).

## Minion Association

Creature minions (charmed creatures) are tracked separately via:
- `glbMinionsObjectIDTable` — array of ObjectIDs for minion slots
- `glbMinionsAssocCount` — count of active minions
- `WRITE_MINION_ASSOC()` / `RECOVER_MINION_ASSOC()` — saves/restores which creatures are currently charmed and serving the party

## Global Spell Effects

`glbGlobalSpellEffects` (6 bytes) — global persistent spell state (e.g., anti-magic zones, curse auras) is saved/restored separately:
```cpp
SUPPRESS_WRITER(&glbGlobalSpellEffects, _4976_3a97, 6, 1)
SUPPRESS_READER(&glbGlobalSpellEffects, _4976_3a97, 6, 1, 1)
```

## What Is NOT Persisted Across Saves

- **Dungeon level state for other maps** — all maps are saved together (full dungeon state in one file)
- **Messages/logs** — message history is not persisted
- **Camera position** — recalculated on load from player position
- **Audio playback position** — not persisted (sounds restart on load)

## DM1 Comparison

DM1 persisted champion state per-champion into separate `CHAMP.DAT` files. Each file stored champion stats + inventory item handles. Dungeon state was per-level in separate `DUNGEON.DAT` files. DM2 consolidates all of this into a single `SKSave.dat` per slot using the SUPPRESS codec for compression.

## Summary

| Party Component | Persisted? | Mechanism |
|----------------|-----------|----------|
| Champion base stats (261B × 4) | Yes | SUPPRESS on `glbChampionSquad` |
| Champion inventory slots (30 × 4 = 120 ObjectIDs) | Yes | `WRITE_RECORD_CHECKCODE` chains |
| Leader hand possession | Yes | Single `WRITE_RECORD_CHECKCODE` |
| Global spell effects (6B) | Yes | SUPPRESS |
| Minion associations | Yes | `WRITE_MINION_ASSOC()` |
| Champion per-slot item records | Yes | Separate DB pool save |
| Global variables (flags/bytes/words) | Yes | SUPPRESS |