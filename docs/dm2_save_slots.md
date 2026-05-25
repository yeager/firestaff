# DM2 V1 — Save Slot System

## Slot Count: 10

DM2 provides exactly **10 save slots**, numbered 0–9. This is established by:
- `sksave_header_asc _4976_5250[10]` — array of 10 header structs used during load dialog
- The slot scanning loop in `_2066_33e7()`: `for (si = 0; si < 10; si++)` checks each slot for the magic marker pair

## Slot Identifiers

Each slot has a **42-byte ASCII header** (`sksave_header_asc`):
```
w0       : U16 — version flag (set to 1 on each save)
b2[34]   : U8[34] — null-terminated ASCII save name (max 33 chars + null)
w36      : U16 — slot index + 0x30 (so slot 0 -> 0x30 = '0' in ASCII)
w38      : U16 — magic 0xBEEF = valid slot marker
w40      : U16 — magic 0xDEAD = valid slot marker
```

A slot is **valid/occupied** when `w38 == 0xBEEF && w40 == 0xDEAD`. Empty slots have different values.

## Slot Name Entry

Slot names are entered through dialog `0x0d` (the save-name dialog), invoked from `_2066_33e7()`. The name string is stored in `_4976_5268` (34-byte buffer). Only the first 33 characters are used (null terminator at index 33). Name is copied into the header's `b2[34]` at save time.

## How a Slot is Selected for Save

1. `GAME_SAVE_MENU()` calls `_2066_33e7()` which scans all 10 slots.
2. If resuming an existing save (`_4976_5bf6 != 0`), it uses the previously loaded slot index (`_4976_525c`).
3. If starting fresh, it scans for the first empty slot (where the magic markers are not set to `0xBEEF/0xDEAD`). The loop:

```cpp
for (si = 0; si < 10; si++) {
    if (_4976_5250[si].w40 == 0xDEAD && _4976_5250[si].w38 == 0xBEEF) {
        _2066_33c4(_4976_5268, si);  // auto-generate a name for this slot
        break;
    }
}
```

4. User can type a custom name or accept the auto-generated one.
5. If all 10 slots are full and no overwrite is chosen, the dialog returns to slot selection.

## How a Slot is Selected for Load

`SELECT_LOAD_GAME()` (`_2066:32BB`) manages the load dialog:
- It opens dialog panel `0x80` with a list of all 10 slots.
- For each slot, it displays the save name from `b2`.
- Slot selection scrolls through up to 10 visible entries.
- Returns the selected slot index `si` (0–9), or `-1` if cancelled.

## Slot Selection in Load Dialog

The load dialog `SELECT_LOAD_GAME()` uses `_4976_4dfc` state machine to track user interaction:
- Case 1: user cancelled (returns `si = -1`)
- Case 2: user confirmed selection (`bp06 = 1` → break and return `si`)
- Case 3: scroll/refresh — recomputes visible entries and calls `_2066_398a(si)` to highlight the current slot

## Slot Identification in Header

The `w36` field stores `slot_index + 0x30`. This means:
- Slot 0 → 0x30 = ASCII '0'
- Slot 9 → 0x39 = ASCII '9'

This allows identifying the slot purely from the binary header without parsing a name.

## DM1 Comparison

DM1 had no slot system at all. DM1 saved per-champion (`CHAMP.DAT`) and per-dungeon-level (`DUNGEON.DAT`) as separate flat files. There was no named-slot concept and no slot-count limit beyond available disk space.

DM2 consolidates everything into one file per slot and provides a structured dialog for managing up to 10 named saves.

## Edge Cases

- **Empty slot auto-fill**: When the user confirms save without a name, `_2066_33c4` auto-generates a name (e.g. based on dungeon name or timestamp).
- **Corrupted slot**: Slots with valid magic but corrupted data cause `READ_DUNGEON_STRUCTURE` or `SUPPRESS_READER` to fail, which triggers a dialog error (0x1b = "The game could not be loaded").
- **Overwrite guard**: `GAME_SAVE_MENU()` checks `glbGameTick` and `_4976_523c` (last save tick) to prevent saving immediately after loading (`+100` tick gate). If the gate is not met, a confirmation dialog (0x0c) is shown first.