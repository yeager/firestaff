# Nexus V1 inventory evidence status

This page records what the current retail-data audit supports. Firestaff's
internal structures and UI helpers are implementation details; they do not by
themselves establish Nexus save layout, inventory slot meaning, or retail UI
behavior.

## Confirmed source data

- The European `PLRD` source contains 20 champion records. Firestaff's larger
  champion array is storage capacity, not evidence of additional retail
  champions.
- `ITEM.IBS` is an authentic image bank. Its image and palette records support
  visual extraction, but do not identify item behavior or inventory position.
- Nexus `DGN` and `DM.BIN` are parsed from the selected real game data. The
  retail `DM.BIN` hit rectangles are kept as source coordinates; rectangle
  membership alone does not identify the command associated with a region.
- Retail input documentation describes a hand pointer, backpack, champion
  slots, magic window, and item interactions. Firestaff has not bound all of
  these actions to verified runtime consumers.

## Not established by the current evidence

The following details must not be presented as Nexus retail facts without a
retail save/runtime trace or disassembly receipt tied to the relevant consumer:

- number and ordering of backpack or equipment slots;
- meanings of fields in Firestaff's `inventory[30]` champion buffer;
- inventory grid dimensions or panel coordinates;
- mapping between any HUD rectangle and a game action;
- item weight, encumbrance thresholds, food/water behavior, or item-use rules.

Older Firestaff UI code contains a 30-cell grid, seven equipment labels, and
DM1-derived rules. Those are not used here as Nexus evidence. See
[`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md) for
the current runtime admission boundary.

## Implementation status

Nexus inventory and item actions remain fail-closed until the Saturn action
dispatcher, save consumer, or equivalent retail evidence binds the data to
observable behavior. Visual assets can be extracted and inspected without
inventing those semantics. New evidence should include the exact retail asset
hash, Ymir `--debug` capture or disassembly address, and the consumer path that
reads or changes the relevant state.
